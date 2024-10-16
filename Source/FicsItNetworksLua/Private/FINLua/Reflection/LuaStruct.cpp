#include "FINLua/Reflection/LuaStruct.h"

#include "FINLua/Reflection/LuaRef.h"
#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/LuaPersistence.h"
#include "tracy/Tracy.hpp"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FINLua {
	FLuaStruct::FLuaStruct(UFINStruct* Type, const FFINDynamicStructHolder& Struct, UFINKernelSystem* Kernel) : Type(Type), Struct(MakeShared<FFINDynamicStructHolder>(Struct)), Kernel(Kernel) {
		Kernel->AddReferencer(this, &CollectReferences);
	}

	FLuaStruct::FLuaStruct(const FLuaStruct& Other) : Type(Other.Type), Struct(Other.Struct), Kernel(Other.Kernel) {
		Kernel->AddReferencer(this, &CollectReferences);
	}
	
	FLuaStruct::~FLuaStruct() {
		Kernel->RemoveReferencer(this);
	}

	void FLuaStruct::CollectReferences(void* Obj, FReferenceCollector& Collector) {
		FLuaStruct* Self = static_cast<FLuaStruct*>(Obj);
		Collector.AddReferencedObject(Self->Type);
		Self->Struct->AddStructReferencedObjects(Collector);
	}

UE_DISABLE_OPTIMIZATION_SHIP
	/**
	 * Tries to find a UFINFunction* Operator for the given data.
	 * If none is found returns nullptr.
	 * If CauseErrorForIndex is not nullptr, causes an lua error instead, guaranteeing a non-nullptr return value.
	 */
	UFINFunction* luaStructFindOperator(lua_State* L, UFINStruct* Type, const FString& OperatorName, const TArray<int>& OperandIndices, TArray<FINAny>& Operands, const int* CauseErrorForIndex) {
		ZoneScoped;
		
		UFINFunction* func;
		int funcIndex = 0;
		while (true) {
			FString FuncName = OperatorName;
			if (funcIndex > 0) FuncName.AppendChar('_').AppendInt(funcIndex);
			func = Type->FindFINFunction(FuncName);
			if (!func) break;
			funcIndex += 1;

			int ParameterIndex = 0;
			for (int OperandIndex : OperandIndices) {
				UFINProperty* param1 = func->GetParameters()[ParameterIndex++];
				TOptional<FINAny> otherValue = luaFIN_toNetworkValueByProp(L, OperandIndex, param1, true, false);
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
	int luaStructExecuteOperator(lua_State* L, const TSharedRef<FINStruct>& Struct, UFINStruct* Type, const FString& OperatorName, const TArray<int>& OperandIndices, const int* CauseErrorForIndex) {
		ZoneScoped;
		
		TArray<FINAny> parameters;
		UFINFunction* func = luaStructFindOperator(L, Type, OperatorName, OperandIndices, parameters, CauseErrorForIndex);
		if (!func) return -2;
		
		FFINExecutionContext Ctx(Struct->GetData());
		TArray<FINAny> result = func->Execute(Ctx, parameters);
		for (const FINAny& val : result) {
			luaFIN_pushNetworkValue(L, val, FFINNetworkTrace());
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

	int luaStructExecuteBinaryOperator(lua_State* L, const FString& OperatorName, int OtherIndex, const TSharedRef<FINStruct>& Struct, UFINStruct* Type, const int* CauseErrorForIndex) {
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
				return luaStructBinaryOperator(L, "__add", FIN_OP_TEXT(FIN_Operator_Add), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__sub
			 * @DisplayName		Subtract
			 */)", __sub) {
				return luaStructBinaryOperator(L, "__sub", FIN_OP_TEXT(FIN_Operator_Sub), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__mul
			 * @DisplayName		Multiply
			 */)", __mul) {
				return luaStructBinaryOperator(L, "__mul", FIN_OP_TEXT(FIN_Operator_Mul), true);
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__div
			 * @DisplayName		Divide
			 */)", __div) {
				return luaStructBinaryOperator(L, "__div", FIN_OP_TEXT(FIN_Operator_Div), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__mod
			 * @DisplayName		Modulo
			 */)", __mod) {
				return luaStructBinaryOperator(L, "__mod", FIN_OP_TEXT(FIN_Operator_Mod), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__pow
			 * @DisplayName		Power
			 */)", __pow) {
				return luaStructBinaryOperator(L, "__pow", FIN_OP_TEXT(FIN_Operator_Pow), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__unm
			 * @DisplayName		Negate
			 */)", __unm) {
				return luaStructUnaryOperator(L, "__unm", FIN_OP_TEXT(FIN_Operator_Neg), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__idiv
			 * @DisplayName		Integer Division
			 */)", __idiv) {
				return luaStructBinaryOperator(L, "__idiv", FIN_OP_TEXT(FIN_Operator_FDiv), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__band
			 * @DisplayName		Bitwise AND
			 */)", __band) {
				return luaStructBinaryOperator(L, "__band", FIN_OP_TEXT(FIN_Operator_BitAND), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__bor
			 * @DisplayName		Bitwise OR
			 */)", __bor) {
				return luaStructBinaryOperator(L, "__bor", FIN_OP_TEXT(FIN_Operator_BitOR), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__bxor
			 * @DisplayName		Bitwise XOR
			 */)", __bxor) {
				return luaStructBinaryOperator(L, "__bxor", FIN_OP_TEXT(FIN_Operator_BitXOR), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__bnot
			 * @DisplayName		Bitwise Negation
			 */)", __bnot) {
				return luaStructUnaryOperator(L, "__bnot", FIN_OP_TEXT(FIN_Operator_BitNOT), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__shl
			 * @DisplayName		Shift Left
			 */)", __shl) {
				return luaStructBinaryOperator(L, "__shl", FIN_OP_TEXT(FIN_Operator_ShiftL), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__shr
			 * @DisplayName		Shift Right
			 */)", __shr) {
				return luaStructBinaryOperator(L, "__shr", FIN_OP_TEXT(FIN_Operator_ShiftR), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__concat
			 * @DisplayName		Concat
			 */)", __concat) {
				return luaStructBinaryOperator(L, "__concat", FIN_OP_TEXT(FIN_Operator_Concat), false);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__len
			 * @DisplayName		Length
			 */)", __len) {
				return luaStructUnaryOperator(L, "__len", FIN_OP_TEXT(FIN_Operator_Len), true);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__eq
			 * @DisplayName		Equal
			 */)", __eq) {
				int result = luaStructTryBinaryOperator(L, "__eq", FIN_OP_TEXT(FIN_Operator_Equals), true, false);
				if (result >= 0) return result;

				const TSharedPtr<FINStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
				const TSharedPtr<FINStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
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
				int result = luaStructTryBinaryOperator(L, "__lt", FIN_OP_TEXT(FIN_Operator_LessThan), false, false);
				if (result >= 0) return result;

				const TSharedPtr<FINStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
				const TSharedPtr<FINStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
				if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
					lua_pushboolean(L, false);
					return UFINLuaProcessor::luaAPIReturn(L, 1);
				}

				lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) < Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__le
			 * @DisplayName		Less or Equal Than
			 */)", __le) {
				int result = luaStructTryBinaryOperator(L, "__le", FIN_OP_TEXT(FIN_Operator_LessOrEqualThan), false, false);
				if (result >= 0) return result;

				const TSharedPtr<FINStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
				const TSharedPtr<FINStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
				if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
					lua_pushboolean(L, false);
					return UFINLuaProcessor::luaAPIReturn(L, 1);
				}

				lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) <= Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			int luaStructIndexOp(lua_State* L) {
				return luaStructBinaryOperator(L, "__index", FIN_OP_TEXT(FIN_Operator_Index), false);
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

				FFINExecutionContext Context(LuaStruct->Struct->GetData());
				int arg = luaFIN_pushFunctionOrGetProperty(L, thisIndex, LuaStruct->Type, MemberName, EFINFunctionFlags::FIN_Func_MemberFunc, EFINRepPropertyFlags::FIN_Prop_Attrib, Context, false);
				if (arg > 0) return arg;
				return luaStructExecuteBinaryOperator(L, FIN_OP_TEXT(FIN_Operator_Index), 2, LuaStruct->Struct, LuaStruct->Type, nullptr);
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

				FFINExecutionContext Context(LuaStruct->Struct->GetData());
				if (luaFIN_tryExecuteSetProperty(L, thisIndex, LuaStruct->Type, MemberName, EFINRepPropertyFlags::FIN_Prop_Attrib, Context, valueIndex, false)) {
					return 1;
				}

				return luaStructExecuteOperator(L, LuaStruct->Struct, LuaStruct->Type, FIN_OP_TEXT(FIN_Operator_NewIndex), {nameIndex, valueIndex}, &thisIndex);
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
				return luaStructExecuteOperator(L, LuaStruct->Struct, LuaStruct->Type, FIN_OP_TEXT(FIN_Operator_Call), OperandIndices, &thisIndex);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				FLuaStruct* LuaStruct = luaFIN_checkLuaStruct(L, 1, nullptr);
				luaFIN_pushFString(L, FFINReflection::StructReferenceText(LuaStruct->Type));
				return 1;
			}

			int luaStructUnpersist(lua_State* L) {
				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;

				const FFINDynamicStructHolder& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));

				luaFIN_pushStruct(L, Struct);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				TSharedPtr<FINStruct> Struct = luaFIN_checkStruct(L, 1, nullptr, false);

				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
				lua_pushinteger(L, Storage.Add(Struct));

				lua_pushcclosure(L, &luaStructUnpersist, 1);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		Garbage Collect
			 */)", __gc) {
				FLuaStruct* Struct = luaFIN_checkLuaStruct(L, 1, nullptr);
				Struct->~FLuaStruct();
				return 0;
			}
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	StructType
		 * @DisplayName		Struct Type
		 */)", StructType) {

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__call
			 * @DisplayName		Call
			 */)", __call) {
				UFINStruct* Struct = luaFIN_toStructType(L, 1);
				if (!Struct) return 0;

				luaL_argcheck(L, Struct->GetStructFlags() & FIN_Struct_Constructable, 1, "Can not be constructed.");

				TSharedRef<FINStruct> Value = luaFIN_checkStruct(L, 2, Struct, true);
				luaFIN_pushStruct(L, *Value);

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				const int thisIndex = 1;
				const int nameIndex = 2;

				UFINStruct* Struct = luaFIN_toStructType(L, thisIndex);
				if (!Struct) return 0;

				FString MemberName = luaFIN_toFString(L, nameIndex);

				FFINExecutionContext Context(Struct);
				return luaFIN_pushFunctionOrGetProperty(L, thisIndex, Struct, MemberName, FIN_Func_StaticFunc, FIN_Prop_StaticProp, Context, true);
			}

			int luaStructTypeUnpersist(lua_State* L) {
				FString StructName = luaFIN_checkFString(L, lua_upvalueindex(1));
				UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
				luaFIN_pushStructType(L, Struct);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
				UFINStruct* Type = *(UFINStruct**)lua_touserdata(L, 1);
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
				UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
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


			lua_newuserdata(L, 0);
			luaL_setmetatable(L, StructLib::_Name);
			lua_setglobal(L, "structs");
			PersistGlobal("structs");
		}
	}

	bool luaFIN_pushStruct(lua_State* L, const FINStruct& Struct) {
		// TODO: Check if required & if it is, also add similar behaviour for getters/coverters/etc including "any lua value to network value" system in LuaUtil
		if (Struct.GetStruct()->IsChildOf(FFINFuture::StaticStruct())) {
			luaFIN_pushFuture(L, Struct);
			return true;
		}

		UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct.GetStruct());
		if (!Type) {
			lua_pushnil(L);
			return false;
		}

		FLuaStruct* LuaStruct = static_cast<FLuaStruct*>(lua_newuserdata(L, sizeof(FLuaStruct)));
		new (LuaStruct) FLuaStruct(Type, Struct, UFINLuaProcessor::luaGetProcessor(L)->GetKernel());
		luaL_setmetatable(L, ReflectionSystemStruct::Struct::_Name);

		return true;
	}

	TSharedPtr<FINStruct> luaFIN_convertToStruct(lua_State* L, int Index, UFINStruct* Type, bool bAllowImplicitConstruction) {
		if (!(Type->GetStructFlags() & FIN_Struct_Constructable)) return nullptr;

		TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(FFINReflection::Get()->FindScriptStruct(Type));

		int luaType = lua_type(L, Index);
		if (luaType == LUA_TNIL) return Struct;
		if (luaType != LUA_TTABLE) return nullptr;

		int j = 0;
		for (UFINProperty* Prop : Type->GetProperties()) {
			if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
			if (lua_getfield(L, Index, TCHAR_TO_UTF8(*Prop->GetInternalName())) == LUA_TNIL) {
				lua_pop(L, 1);
				lua_geti(L, Index, ++j);
			}
			if (!lua_isnil(L, -1)) {
				TOptional<FINAny> Value = luaFIN_toNetworkValueByProp(L, -1, Prop, true, bAllowImplicitConstruction);
				if (Value.IsSet()) Prop->SetValue(Struct->GetData(), Value.GetValue());
			}
			lua_pop(L, 1);
		}

		return Struct;
	}

	FLuaStruct* luaFIN_toLuaStruct(lua_State* L, int Index, UFINStruct* ParentType) {
		FLuaStruct* LuaStruct = static_cast<FLuaStruct*>(luaL_testudata(L, Index, ReflectionSystemStruct::Struct::_Name));
		if (LuaStruct && LuaStruct->Type->IsChildOf(ParentType)) {
			return LuaStruct;
		}
		return nullptr;
	}

	FLuaStruct* luaFIN_checkLuaStruct(lua_State* L, int Index, UFINStruct* ParentType) {
		FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, ParentType);
		if (!LuaStruct) {
			luaFIN_typeError(L, Index, FFINReflection::StructReferenceText(ParentType));
		}
		return LuaStruct;
	}

	TSharedPtr<FINStruct> luaFIN_toStruct(lua_State* L, int Index, UFINStruct* ParentType, bool bAllowConstruction) {
		FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, ParentType);
		if (LuaStruct) return LuaStruct->Struct;

		if (bAllowConstruction && ParentType) {
			return luaFIN_convertToStruct(L, Index, ParentType, bAllowConstruction);
		}

		return nullptr;
	}

	TSharedRef<FINStruct> luaFIN_checkStruct(lua_State* L, int Index, UFINStruct* ParentType, bool bAllowConstruction) {
		TSharedPtr<FINStruct> Struct = luaFIN_toStruct(L, Index, ParentType, bAllowConstruction);
		if (!Struct.IsValid()) luaFIN_typeError(L, Index, FFINReflection::StructReferenceText(ParentType));
		return Struct.ToSharedRef();
	}

	void luaFIN_pushStructType(lua_State* L, UFINStruct* Struct) {
		if (Struct) {
			*(UFINStruct**)lua_newuserdata(L, sizeof(UFINStruct*)) = Struct;
			luaL_setmetatable(L, ReflectionSystemStruct::StructType::_Name);
		} else {
			lua_pushnil(L);
		}
	}

	UFINStruct* luaFIN_toStructType(lua_State* L, int index) {
		if (lua_isnil(L, index)) return nullptr;
		return luaFIN_checkStructType(L, index);
	}

	UFINStruct* luaFIN_checkStructType(lua_State* L, int index) {
		UFINStruct* Struct = *(UFINStruct**)luaL_checkudata(L, index, ReflectionSystemStruct::StructType::_Name);
		return Struct;
	}

	FString luaFIN_getLuaStructTypeName() {
		return UTF8_TO_TCHAR(ReflectionSystemStruct::Struct::_Name);
	}
}
