#include "FINLua/Reflection/LuaStruct.h"

#include "FINLua/Reflection/LuaRef.h"
#include "FINLuaProcessor.h"
#include "FINLuaReferenceCollector.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/LuaPersistence.h"
#include "tracy/Tracy.hpp"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FINLua {
	FLuaStruct::FLuaStruct(UFIRStruct* Type, const FFIRInstancedStruct& Struct, FFINLuaReferenceCollector* ReferenceCollector) : FFINLuaReferenceCollected(ReferenceCollector), Type(Type), Struct(MakeShared<FFIRInstancedStruct>(Struct)) {}

	UE_DISABLE_OPTIMIZATION_SHIP
	void FLuaStruct::CollectReferences(FReferenceCollector& Collector) {
		Collector.AddReferencedObject(Type);
		Collector.AddPropertyReferencesWithStructARO(FFIRInstancedStruct::StaticStruct(), &Struct.Get());
	}

	/**
	 * Tries to find a UFIRFunction* Operator for the given data.
	 * If none is found returns nullptr.
	 * If CauseErrorForIndex is not nullptr, causes an lua error instead, guaranteeing a non-nullptr return value.
	 */
	UFIRFunction* luaStructFindOperator(lua_State* L, UFIRStruct* Type, const FString& OperatorName, const TArray<int>& OperandIndices, TArray<FIRAny>& Operands, const int* CauseErrorForIndex) {
		ZoneScoped;
		
		UFIRFunction* func;
		int funcIndex = 0;
		while (true) {
			FString FuncName = OperatorName;
			if (funcIndex > 0) FuncName.AppendChar('_').AppendInt(funcIndex);
			func = Type->FindFIRFunction(FuncName);
			if (!func) break;
			funcIndex += 1;

			int ParameterIndex = 0;
			for (int OperandIndex : OperandIndices) {
				UFIRProperty* param1 = func->GetParameters()[ParameterIndex++];
				TOptional<FIRAny> otherValue = luaFIN_toNetworkValueByProp(L, OperandIndex, param1, true, false);
				if (!otherValue.IsSet()) break;
				Operands.Add(*otherValue);
			}
			if (OperandIndices.Num() != Operands.Num()) {
				Operands.Empty();
				continue;
			}
			break;
		}

		if (!func) {
			if (CauseErrorForIndex) luaL_error(L, "Invalid Operator for struct of type %s", lua_typename(L, *CauseErrorForIndex));
			return nullptr;
		}
				
		return func;
	}

	/**
	 * Tries to find and execute an operator for the given data.
	 * Returns the number of return values.
	 * If unable to find or execute any matching operator, returns a negative error value.
	 * If CauseErroForIndex is not nullptr, causes an lua error instead, guaranteeing a return value of >= 0
	 */
	int luaStructExecuteOperator(lua_State* L, const TSharedRef<FIRStruct>& Struct, UFIRStruct* Type, const FString& OperatorName, const TArray<int>& OperandIndices, const int* CauseErrorForIndex) {
		ZoneScoped;

		if (!IsValid(Type)) return -3;
		
		TArray<FIRAny> parameters;
		UFIRFunction* func = luaStructFindOperator(L, Type, OperatorName, OperandIndices, parameters, CauseErrorForIndex);
		if (!func) return -2;
		
		FFIRExecutionContext Ctx(Struct->GetData());
		TArray<FIRAny> result = func->Execute(Ctx, parameters);
		for (const FIRAny& val : result) {
			luaFIN_pushNetworkValue(L, val, FFIRTrace());
		} 
		return result.Num();
	}

	int luaStructUnaryOperator(lua_State* L, const char* LuaOperator, const FString& OperatorName, bool bCauseError) {
		int thisIndex = 1;
		
		FLuaStruct* ThisLuaStruct = luaFIN_checkLuaStruct(L, thisIndex, nullptr);

		return FMath::Max(0, luaStructExecuteOperator(L, ThisLuaStruct->Struct, ThisLuaStruct->Type, OperatorName, {}, bCauseError ? &thisIndex : nullptr));
	}

	/**
	 * Tries to find an operand order for the given operator settings.
	 * Returns true if a valid operand arrangement was found.
	 */
	bool luaStructBinaryOperatorGetOperands(lua_State* L, const char* LuaOperator, bool bCommutative, int& ThisIndex, int& OtherIndex) {
		ThisIndex = 1;
		OtherIndex = 2;
		if (LUA_TFUNCTION != luaL_getmetafield(L, 1, LuaOperator)) {
			if (bCommutative) {
				ThisIndex = 2;
				OtherIndex = 1;
			} else {
				return false;
			}
		}
		return true;
	}

	int luaStructExecuteBinaryOperator(lua_State* L, const FString& OperatorName, int OtherIndex, const TSharedRef<FIRStruct>& Struct, UFIRStruct* Type, const int* CauseErrorForIndex) {
		return luaStructExecuteOperator(L, Struct, Type, OperatorName, {OtherIndex}, CauseErrorForIndex);
	}

	int luaStructTryBinaryOperator(lua_State* L, const char* LuaOperator, const FString& OperatorName, bool bCommutative, bool bCauseError, int* ErrorCause = nullptr) {
		int thisIndex, otherIndex;
		if (!luaStructBinaryOperatorGetOperands(L, LuaOperator, bCommutative, thisIndex, otherIndex)) {
			if (bCauseError) return luaL_error(L, "Invalid Operator for types %s and %s", TCHAR_TO_UTF8(*luaFIN_typeName(L, 1)), TCHAR_TO_UTF8(*luaFIN_typeName(L, 2)));
			if (ErrorCause) *ErrorCause = -1;
			return -1;
		}

		FLuaStruct* ThisLuaStruct = luaFIN_checkLuaStruct(L, thisIndex, nullptr);

		int result = luaStructExecuteBinaryOperator(L, OperatorName, otherIndex, ThisLuaStruct->Struct, ThisLuaStruct->Type, nullptr);
		if (result < 0) {
			if (bCauseError) return luaL_error(L, "Invalid Operator for struct of type %s", TCHAR_TO_UTF8(*luaFIN_typeName(L, thisIndex)));
			if (ErrorCause) *ErrorCause = -2;
			return -2;
		}
		return result;
	}

	int luaStructBinaryOperator(lua_State* L, const char* LuaOperator, const FString& OperatorName, bool bCommutative = false) {
		return luaStructTryBinaryOperator(L, LuaOperator, OperatorName, bCommutative, true);
	}
UE_ENABLE_OPTIMIZATION_SHIP

	LuaModule(R"(/**
	 * @LuaModule		ReflectionSystemStructModule
	 * @DisplayName		Reflection-System Struct Module
	 * @Dependency		ReflectionSystemBaseModule
	 *
	 * This module provides all the functionallity for the usage of reflected structs in Lua.
	 */)", ReflectionSystemStruct) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	Struct
		 * @DisplayName		Struct
		 */)", Struct) {

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__add
			 * @DisplayName		Add
			 */)", __add) {
				return luaStructBinaryOperator(L, "__add", FIR_OP_TEXT(FIR_Operator_Add), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__sub
			 * @DisplayName		Subtract
			 */)", __sub) {
				return luaStructBinaryOperator(L, "__sub", FIR_OP_TEXT(FIR_Operator_Sub), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__mul
			 * @DisplayName		Multiply
			 */)", __mul) {
				return luaStructBinaryOperator(L, "__mul", FIR_OP_TEXT(FIR_Operator_Mul), true);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__div
			 * @DisplayName		Divide
			 */)", __div) {
				return luaStructBinaryOperator(L, "__div", FIR_OP_TEXT(FIR_Operator_Div), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__mod
			 * @DisplayName		Modulo
			 */)", __mod) {
				return luaStructBinaryOperator(L, "__mod", FIR_OP_TEXT(FIR_Operator_Mod), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__pow
			 * @DisplayName		Power
			 */)", __pow) {
				return luaStructBinaryOperator(L, "__pow", FIR_OP_TEXT(FIR_Operator_Pow), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__unm
			 * @DisplayName		Negate
			 */)", __unm) {
				return luaStructUnaryOperator(L, "__unm", FIR_OP_TEXT(FIR_Operator_Neg), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__idiv
			 * @DisplayName		Integer Division
			 */)", __idiv) {
				return luaStructBinaryOperator(L, "__idiv", FIR_OP_TEXT(FIR_Operator_FDiv), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__band
			 * @DisplayName		Bitwise AND
			 */)", __band) {
				return luaStructBinaryOperator(L, "__band", FIR_OP_TEXT(FIR_Operator_BitAND), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__bor
			 * @DisplayName		Bitwise OR
			 */)", __bor) {
				return luaStructBinaryOperator(L, "__bor", FIR_OP_TEXT(FIR_Operator_BitOR), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__bxor
			 * @DisplayName		Bitwise XOR
			 */)", __bxor) {
				return luaStructBinaryOperator(L, "__bxor", FIR_OP_TEXT(FIR_Operator_BitXOR), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__bnot
			 * @DisplayName		Bitwise Negation
			 */)", __bnot) {
				return luaStructUnaryOperator(L, "__bnot", FIR_OP_TEXT(FIR_Operator_BitNOT), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__shl
			 * @DisplayName		Shift Left
			 */)", __shl) {
				return luaStructBinaryOperator(L, "__shl", FIR_OP_TEXT(FIR_Operator_ShiftL), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__shr
			 * @DisplayName		Shift Right
			 */)", __shr) {
				return luaStructBinaryOperator(L, "__shr", FIR_OP_TEXT(FIR_Operator_ShiftR), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__concat
			 * @DisplayName		Concat
			 */)", __concat) {
				return luaStructBinaryOperator(L, "__concat", FIR_OP_TEXT(FIR_Operator_Concat), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__len
			 * @DisplayName		Length
			 */)", __len) {
				return luaStructUnaryOperator(L, "__len", FIR_OP_TEXT(FIR_Operator_Len), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__eq
			 * @DisplayName		Equal
			 */)", __eq) {
				int result = luaStructTryBinaryOperator(L, "__eq", FIR_OP_TEXT(FIR_Operator_Equals), true, false);
				if (result >= 0) return result;

				const TSharedPtr<FIRStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
				const TSharedPtr<FIRStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
				if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
					lua_pushboolean(L, false);
					return 1;
				}

				lua_pushboolean(L, Struct1->GetStruct()->CompareScriptStruct(Struct1->GetData(), Struct2->GetData(), 0));
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__lt
			 * @DisplayName		Less Than
			 */)", __lt) {
				int result = luaStructTryBinaryOperator(L, "__lt", FIR_OP_TEXT(FIR_Operator_LessThan), false, false);
				if (result >= 0) return result;

				const TSharedPtr<FIRStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
				const TSharedPtr<FIRStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
				if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
					lua_pushboolean(L, false);
					return 1;
				}

				lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) < Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__le
			 * @DisplayName		Less or Equal Than
			 */)", __le) {
				int result = luaStructTryBinaryOperator(L, "__le", FIR_OP_TEXT(FIR_Operator_LessOrEqualThan), false, false);
				if (result >= 0) return result;

				const TSharedPtr<FIRStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
				const TSharedPtr<FIRStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
				if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
					lua_pushboolean(L, false);
					return 1;
				}

				lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) <= Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
				return 1;
			}

			int luaStructIndexOp(lua_State* L) {
				return luaStructBinaryOperator(L, "__index", FIR_OP_TEXT(FIR_Operator_Index), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				ZoneScoped;

				const int thisIndex = 1;
				const int nameIndex = 2;

				FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, thisIndex, nullptr);
				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFIRExecutionContext Context(LuaStruct->Struct->GetData());
				int arg = luaFIN_pushFunctionOrGetProperty(L, thisIndex, LuaStruct->Type, MemberName, FIR_Func_MemberFunc, FIR_Prop_Attrib, Context, false);
				if (arg > 0) return arg;
				return luaStructExecuteBinaryOperator(L, FIR_OP_TEXT(FIR_Operator_Index), 2, LuaStruct->Struct, LuaStruct->Type, nullptr);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__newindex
			 * @DisplayName		New Index
			 */)", __newindex) {
				ZoneScoped;

				const int thisIndex = 1;
				const int nameIndex = 2;
				const int valueIndex = 3;

				FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, thisIndex, nullptr);
				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFIRExecutionContext Context(LuaStruct->Struct->GetData());
				if (luaFIN_tryExecuteSetProperty(L, thisIndex, LuaStruct->Type, MemberName, FIR_Prop_Attrib, Context, valueIndex, false)) {
					return 1;
				}

				return luaStructExecuteOperator(L, LuaStruct->Struct, LuaStruct->Type, FIR_OP_TEXT(FIR_Operator_NewIndex), {nameIndex, valueIndex}, &thisIndex);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__call
			 * @DisplayName		Call
			 */)", __call) {
				ZoneScoped;

				const int thisIndex = 1;
				const int operandsStartIndex = 2;

				int top = lua_gettop(L);

				FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, thisIndex, nullptr);

				TArray<int> OperandIndices;
				for (int i = operandsStartIndex; i <= top; ++i) {
					OperandIndices.Add(i);
				}
				return luaStructExecuteOperator(L, LuaStruct->Struct, LuaStruct->Type, FIR_OP_TEXT(FIN_Operator_Call), OperandIndices, &thisIndex);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, 1, nullptr);
				luaFIN_pushFString(L, FFicsItReflectionModule::StructReferenceText(LuaStruct->Type));
				return 1;
			}

			int next_resume(lua_State* L, int, lua_KContext) {
				return 2;
			}
			int next(lua_State* L) {
				FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, 1, nullptr);
				TArray<UFIRProperty*> props = LuaStruct->Type->GetProperties();

				UFIRProperty* prop = nullptr;
				if (lua_isnoneornil(L, 2)) {
					if (props.Num() > 0) {
						prop = props[0];
					}
				} else {
					FString prevKey = luaFIN_checkFString(L, 2);
					for (int i = 1; i < props.Num(); ++i) {
						if (props[i-1]->GetInternalName() == prevKey) {
							prop = props[i];
							break;
						}
					}
				}
				if (prop == nullptr) {
					lua_pushnil(L);
					return 1;
				}

				luaFIN_pushFString(L, prop->GetInternalName());
				FFIRExecutionContext Context(LuaStruct->Struct->GetData());
				luaFIN_getProperty(L, prop, Context, 0, next_resume);
				return 2;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__pairs
			 * @DisplayName		Pairs
			 */)", __pairs) {
				lua_pushcfunction(L, next);
				lua_pushvalue(L, 1);
				lua_pushnil(L);
				return 3;
			}

			int luaStructUnpersist(lua_State* L) {
				FFINLuaRuntimePersistenceState& Storage = luaFIN_getPersistence(L);

				const FFIRInstancedStruct& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));

				int num = luaL_checkinteger(L, lua_upvalueindex(2));

				luaFIN_pushStruct(L, Struct, num);

				for (int i = 1; i <= num; ++i) {
					lua_pushvalue(L, lua_upvalueindex(2+i));
					lua_setiuservalue(L, 1, i);
				}

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				TSharedPtr<FIRStruct> Struct = luaFIN_checkStruct(L, 1, nullptr, false);

				FFINLuaRuntimePersistenceState& Storage = luaFIN_getPersistence(L);
				lua_pushinteger(L, Storage.Add(Struct));
				int i = 0;
				while (true) {
					if (lua_getiuservalue(L, 1, i+1) == LUA_TNONE) {
						lua_pop(L, 1);
						break;
					}
					i += 1;
				}
				lua_pushinteger(L, i);
				lua_insert(L, 3);

				lua_pushcclosure(L, &luaStructUnpersist, 2+i);

				return 1;
			}
UE_DISABLE_OPTIMIZATION_SHIP
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				FLuaStruct* Struct = luaFIN_checkLuaStruct(L, 1, nullptr);
				Struct->~FLuaStruct();
				return 0;
			}
UE_ENABLE_OPTIMIZATION_SHIP
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	StructType
		 * @DisplayName		Struct Type
		 */)", StructType) {

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__call
			 * @DisplayName		Call
			 */)", __call) {
				UFIRStruct* Struct = luaFIN_toStructType(L, 1);
				if (!Struct) return 0;

				luaL_argcheck(L, Struct->GetStructFlags() & FIR_Struct_Constructable, 1, "Can not be constructed.");

				TSharedRef<FIRStruct> Value = luaFIN_checkStruct(L, 2, Struct, true);
				luaFIN_pushStruct(L, *Value);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				const int thisIndex = 1;
				const int nameIndex = 2;

				UFIRStruct* Struct = luaFIN_toStructType(L, thisIndex);
				if (!Struct) return 0;

				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFIRExecutionContext Context(Struct);
				return luaFIN_pushFunctionOrGetProperty(L, thisIndex, Struct, MemberName, FIR_Func_StaticFunc, FIR_Prop_StaticProp, Context, true);
			}

			int luaStructTypeUnpersist(lua_State* L) {
				FString StructName = luaFIN_checkFString(L, lua_upvalueindex(1));
				UFIRStruct* Struct = FFicsItReflectionModule::Get().FindStruct(StructName);
				luaFIN_pushStructType(L, Struct);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				UFIRStruct* Type = *(UFIRStruct**)lua_touserdata(L, 1);
				luaFIN_pushFString(L, Type->GetInternalName());
				lua_pushcclosure(L, &luaStructTypeUnpersist, 1);
				return 1;
			}
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	StructLib
		 * @DisplayName		Struct Library
		 */)", StructLib) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				FString StructName = luaFIN_checkFString(L, 2);
				UFIRStruct* Struct = FFicsItReflectionModule::Get().FindStruct(StructName);
				if (Struct) {
					luaFIN_pushStructType(L, Struct);
				} else {
					lua_pushnil(L);
				}
				return 1;
			}
		}

		LuaModuleGlobalBareValue(R"(/**
		 * @LuaGlobal		structs		StructLib
		 * @DisplayName		Structs
		 *
		 * A peseudo table that can be used to look up struct types (which can then be used to easily construct a struct of that type).
		 * Ideal usage of it is `structs.MyStruct` (with a Constructor `structs.Vector(x,y,z)`).
		 * Since the type lookup occurs in the metatable-function, you can still use the []-Operator in the case
		 * you want to look up something based on a dynamic string e.g. `structs[myStringVar]` works just fine.
		)", structs) {
			lua_pushnil(L);
		}

		LuaModulePostSetup() {
			PersistenceNamespace("ReflectionStruct");

			lua_pushcfunction(L, Struct::luaStructUnpersist);
			PersistValue("StructUnpersist");

			lua_pushcfunction(L, StructType::luaStructTypeUnpersist);
			PersistValue("StructTypeUnpersist");

			lua_pushcfunction(L, Struct::next);
			PersistValue("StructNext");
			lua_pushcfunction(L, (lua_CFunction)(void*)(Struct::next_resume));
			PersistValue("StructNextResume");

			lua_newuserdata(L, 0);
			luaL_setmetatable(L, StructLib::_Name);
			lua_setglobal(L, "structs");
			PersistGlobal("structs");
		}
	}

	bool luaFIN_pushStruct(lua_State* L, const FIRStruct& Struct, int numUserValues) {
		// TODO: Check if required & if it is, also add similar behaviour for getters/coverters/etc including "any lua value to network value" system in LuaUtil
		if (Struct.GetStruct()->IsChildOf(FFINFuture::StaticStruct())) {
			luaFIN_pushFuture(L, Struct);
			return true;
		}

		UFIRStruct* Type = FFicsItReflectionModule::Get().FindStruct(Struct.GetStruct());

		FLuaStruct* LuaStruct = static_cast<FLuaStruct*>(lua_newuserdatauv(L, sizeof(FLuaStruct), numUserValues));

		new (LuaStruct) FLuaStruct(Type, Struct, luaFIN_getReferenceCollector(L));
		luaL_setmetatable(L, ReflectionSystemStruct::Struct::_Name);

		return true;
	}

	TSharedPtr<FIRStruct> luaFIN_convertToStruct(lua_State* L, int Index, UFIRStruct* Type, bool bAllowImplicitConstruction) {
		if (!(Type->GetStructFlags() & FIR_Struct_Constructable)) return nullptr;

		TSharedRef<FIRStruct> Struct = MakeShared<FIRStruct>(FFicsItReflectionModule::Get().FindScriptStruct(Type));

		int luaType = lua_type(L, Index);
		if (luaType == LUA_TNIL) return Struct;
		if (luaType != LUA_TTABLE) return nullptr;

		int j = 0;
		for (UFIRProperty* Prop : Type->GetProperties()) {
			if (!(Prop->GetPropertyFlags() & FIR_Prop_Attrib)) continue;
			if (lua_getfield(L, Index, TCHAR_TO_UTF8(*Prop->GetInternalName())) == LUA_TNIL) {
				lua_pop(L, 1);
				lua_geti(L, Index, ++j);
			}
			if (!lua_isnil(L, -1)) {
				TOptional<FIRAny> Value = luaFIN_toNetworkValueByProp(L, -1, Prop, true, bAllowImplicitConstruction);
				if (Value.IsSet()) Prop->SetValue(Struct->GetData(), Value.GetValue());
			}
			lua_pop(L, 1);
		}

		return Struct;
	}

	FLuaStruct* luaFIN_toLuaStruct(lua_State* L, int Index, UFIRStruct* ParentType) {
		FLuaStruct* LuaStruct = static_cast<FLuaStruct*>(luaL_testudata(L, Index, ReflectionSystemStruct::Struct::_Name));
		if (LuaStruct) {
			if (ParentType && LuaStruct->Type && !LuaStruct->Type->IsChildOf(ParentType)) {
				return nullptr;
			}
			return LuaStruct;
		}
		return nullptr;
	}

	FLuaStruct* luaFIN_checkLuaStruct(lua_State* L, int Index, UFIRStruct* ParentType) {
		FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, ParentType);
		if (!LuaStruct) {
			luaFIN_typeError(L, Index, FFicsItReflectionModule::StructReferenceText(ParentType));
		}
		return LuaStruct;
	}

	TSharedPtr<FIRStruct> luaFIN_toStruct(lua_State* L, int Index, UFIRStruct* ParentType, bool bAllowConstruction) {
		FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, ParentType);
		if (LuaStruct) return LuaStruct->Struct;

		if (bAllowConstruction && ParentType) {
			return luaFIN_convertToStruct(L, Index, ParentType, bAllowConstruction);
		}

		return nullptr;
	}
UE_DISABLE_OPTIMIZATION_SHIP
	TSharedPtr<FIRStruct> luaFIN_toUStruct(lua_State* L, int Index, UStruct* ParentType, bool bAllowConstruction) {
		FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, nullptr);
		if (LuaStruct) {
			if (!LuaStruct->Struct->GetStruct() || !LuaStruct->Struct->GetData() || (ParentType && !LuaStruct->Struct->GetStruct()->IsChildOf(ParentType))) {
				return nullptr;
			}
			return LuaStruct->Struct;
		}
		return nullptr;
	}

	TSharedRef<FIRStruct> luaFIN_checkStruct(lua_State* L, int Index, UFIRStruct* ParentType, bool bAllowConstruction) {
		TSharedPtr<FIRStruct> Struct = luaFIN_toStruct(L, Index, ParentType, bAllowConstruction);
		if (!Struct.IsValid()) luaFIN_typeError(L, Index, FFicsItReflectionModule::StructReferenceText(ParentType));
		return Struct.ToSharedRef();
	}

	TSharedRef<FIRStruct> luaFIN_checkUStruct(lua_State* L, int Index, UStruct* ParentType, bool bAllowConstruction) {
		TSharedPtr<FIRStruct> Struct = luaFIN_toUStruct(L, Index, ParentType, bAllowConstruction);
		if (!Struct.IsValid() || !Struct->GetStruct() || !Struct->GetData()) luaFIN_typeError(L, Index, ParentType->GetName());
		return Struct.ToSharedRef();
	}
UE_ENABLE_OPTIMIZATION_SHIP
	void luaFIN_pushStructType(lua_State* L, UFIRStruct* Struct) {
		if (Struct) {
			*(UFIRStruct**)lua_newuserdata(L, sizeof(UFIRStruct*)) = Struct;
			luaL_setmetatable(L, ReflectionSystemStruct::StructType::_Name);
		} else {
			lua_pushnil(L);
		}
	}

	UFIRStruct* luaFIN_toStructType(lua_State* L, int index) {
		if (lua_isnil(L, index)) return nullptr;
		return luaFIN_checkStructType(L, index);
	}

	UFIRStruct* luaFIN_checkStructType(lua_State* L, int index) {
		UFIRStruct* Struct = *(UFIRStruct**)luaL_checkudata(L, index, ReflectionSystemStruct::StructType::_Name);
		return Struct;
	}

	FString luaFIN_getLuaStructTypeName() {
		return UTF8_TO_TCHAR(ReflectionSystemStruct::Struct::_Name);
	}
}
