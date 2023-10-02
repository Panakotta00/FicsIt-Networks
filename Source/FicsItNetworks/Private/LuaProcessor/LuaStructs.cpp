#include "LuaProcessor/LuaStructs.h"
#include "LuaProcessor/LuaRef.h"
#include "LuaProcessor/LuaUtil.h"
#include "LuaProcessor/LuaProcessor.h"
#include "LuaProcessor/LuaInstance.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStruct.h"

#define STRUCT_TYPE_METATABLE_NAME "SimpleStructRefMetatable"

#define PersistParams \
	const std::string& _persist_namespace, \
	const int _persist_permTableIdx, \
	const int _persist_upermTableIdx

namespace FINLua {
	TMap<UFINStruct*, FString> StructToMetaName;
	TMap<FString, UFINStruct*> MetaNameToStruct;
	FCriticalSection StructMetaNameLock;

	UFINStruct* luaStructFindStructFromMetaName(const FString& MetatableName) {
		FScopeLock ScopeLock(&StructMetaNameLock);
		UFINStruct** Struct = MetaNameToStruct.Find(MetatableName);
		if (Struct) return *Struct;
		return nullptr;
	}
	
	TSharedPtr<FINStruct> luaGetStruct(lua_State* L, int i, FLuaStruct** LStructPtr) {
		UFINStruct* Type = luaGetStructType(L, i);
		if (!Type) return nullptr;
		TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(Cast<UScriptStruct>(Type->GetOuter()));
		FLuaStruct* LStruct = luaGetStruct(L, i, Struct);
		if (LStructPtr) *LStructPtr = LStruct;
		return Struct;
	}

	TSharedPtr<FINStruct> luaFIN_converttostruct(lua_State* L, int i, UFINStruct* Type, bool bAllowImplicitConstruction) {
		if (!(Type->GetStructFlags() & FIN_Struct_Constructable)) return nullptr;

		TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(FFINReflection::Get()->FindScriptStruct(Type));
		
		int j = 0;
		for (UFINProperty* Prop : Type->GetProperties()) {
			if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
			if (lua_getfield(L, i, TCHAR_TO_UTF8(*Prop->GetInternalName())) == LUA_TNIL) {
				lua_geti(L, i, ++j);
			}
			FINAny Value;
			if (bAllowImplicitConstruction || luaFIN_getnetworkvaluetype(L, -1).IsSet()) {
				Value = luaToProperty(L, Prop, -1);
			} else {
				lua_pop(L, 1);
				return nullptr;
			}
			lua_pop(L, 1);
			Prop->SetValue(Struct->GetData(), Value);
		}

		return Struct;
	}

	TSharedPtr<FINStruct> luaFIN_tostruct(lua_State* L, int i, UFINStruct* Type, FLuaStruct** luaStruct, bool bAllowConstruction) {
		if (bAllowConstruction && lua_istable(L, i) && Type) {
			return luaFIN_converttostruct(L, i, Type, bAllowConstruction);
		}
		UFINStruct* FoundStruct = luaFIN_getstructtype(L, i);
		if ((Type && FoundStruct == Type) || (!Type && FoundStruct)) {
			FLuaStruct* LuaStruct = (FLuaStruct*)lua_touserdata(L, i);
			if (luaStruct) *luaStruct = LuaStruct;
			return LuaStruct->Struct;
		}
		return nullptr;
	}

	TSharedRef<FINStruct> luaFIN_checkstruct(lua_State* L, int i, UFINStruct* Type, FLuaStruct** luaStruct) {
		TSharedPtr<FINStruct> Struct = luaFIN_tostruct(L, i, Type, luaStruct);
		if (!Struct.IsValid()) luaL_argerror(L, i, "Not a struct"); // TODO: Improve error message with why struct conv not was working, template mismatch, general no struct etc.
		return Struct.ToSharedRef();
	}

	FLuaStruct* luaGetStruct(lua_State* L, int i, TSharedRef<FINStruct>& Struct) {
		i = lua_absindex(L, i);
		UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct->GetStruct());
		if (!Type) {
			return nullptr;
		}
		if (lua_istable(L, i)) {
			if (!(Type->GetStructFlags() & FIN_Struct_Constructable)) return nullptr;
			int j = 0;
			for (UFINProperty* Prop : Type->GetProperties()) {
				if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
				++j;
				if (lua_getfield(L, i, TCHAR_TO_UTF8(*Prop->GetInternalName())) == LUA_TNIL) {
					lua_pop(L, 1);
					lua_geti(L, i, j);
				}
				FINAny Value;
				luaToNetworkValue(L, -1, Value);
				lua_pop(L, 1);
				Prop->SetValue(Struct->GetData(), Value);
			}
		} else if (lua_isuserdata(L, i)) {
			StructMetaNameLock.Lock();
			FString MetaName = StructToMetaName[Type];
			StructMetaNameLock.Unlock();
			FLuaStruct* LStruct = static_cast<FLuaStruct*>(luaL_checkudata(L, i, TCHAR_TO_UTF8(*MetaName)));
			Struct = LStruct->Struct;
			return LStruct;
		}
		return nullptr;
	}
	
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

	void luaStruct(lua_State* L, const FINStruct& Struct) {
		UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct.GetStruct());
		if (!Type) {
			lua_pushnil(L);
			return;
		}
		setupStructMetatable(L, Type);
		FLuaStruct* LStruct = static_cast<FLuaStruct*>(lua_newuserdata(L, sizeof(FLuaStruct)));
		new (LStruct) FLuaStruct(Type, Struct, UFINLuaProcessor::luaGetProcessor(L)->GetKernel());
		luaL_setmetatable(L, TCHAR_TO_UTF8(*StructToMetaName[Type]));
	}

	UFINStruct* luaGetStructType(lua_State* L, int i) {
		FString TypeName;
		if (luaL_getmetafield(L, i, "__name") == LUA_TSTRING) {
			TypeName = lua_tostring(L, -1);
			lua_pop(L, 1);
		} else if (lua_type(L, i) == LUA_TLIGHTUSERDATA) {
			TypeName = "light userdata";
		} else {
			TypeName = luaL_typename(L, i);
		}
		StructMetaNameLock.Lock();
		UFINStruct** Type = MetaNameToStruct.Find(TypeName);
		StructMetaNameLock.Unlock();
		if (!Type) return nullptr;
		return *Type;
	}

	UFINStruct* luaFIN_getstructtype(lua_State* L, int i) {
		if (lua_type(L, i) != LUA_TUSERDATA) return nullptr;
		int type = luaL_getmetafield(L, i, "__name");
		if (type != LUA_TSTRING) {
			if (type != LUA_TNIL) lua_pop(L, 1);
			return nullptr;
		}
		FString TypeName = luaFIN_tofstring(L, -1);
		lua_pop(L, 1);
		StructMetaNameLock.Lock();
		UFINStruct** Type = MetaNameToStruct.Find(TypeName);
		StructMetaNameLock.Unlock();
		if (!Type) return nullptr;
		return *Type;
	}

	int luaStructFuncCall(lua_State* L) {
		// get function
		LuaRefFuncData* Func = static_cast<LuaRefFuncData*>(luaL_checkudata(L, lua_upvalueindex(1), LUA_REF_FUNC_DATA));
		
		// get and check instance
		StructMetaNameLock.Lock();
		FString* MetaNamePtr = StructToMetaName.Find(Cast<UFINStruct>(Func->Struct));
		if (!MetaNamePtr) {
			StructMetaNameLock.Unlock();
			return luaL_argerror(L, 1, "Function name is invalid (internal error)");
		}
		const FString MetaName = *MetaNamePtr;
		StructMetaNameLock.Unlock();
		FLuaStruct* Instance = static_cast<FLuaStruct*>(luaL_checkudata(L, 1, TCHAR_TO_UTF8(*MetaName)));
		if (!Instance->Struct->GetData()) return luaL_argerror(L, 1, "Struct is invalid");

		// call the function
		return luaCallFINFunc(L, Func->Func, FFINExecutionContext(Instance->Struct->GetData()), "Struct");
	}

	/**
	 * Tries to find a UFINFunction* Operator for the given data.
	 * If none is found returns nullptr.
	 * If CauseErrorForIndex is not nullptr, causes an lua error instead, guaranteeing a non-nullptr return value.
	 */
	UFINFunction* luaStructFindOperator(lua_State* L, UFINStruct* Type, const FString& OperatorName, const TArray<int>& OperandIndices, TArray<FINAny>& Operands, const int* CauseErrorForIndex) {
		UFINFunction* func;
		int funcIndex = 0;
		while (true) {
			FString FuncName = OperatorName;
			if (funcIndex > 0) FuncName.AppendChar('_').AppendInt(funcIndex);
			func = Type->FindFINFunction(FuncName);
			if (!func) {
				if (CauseErrorForIndex) luaL_error(L, "Invalid Operator for struct of type %s", lua_typename(L, *CauseErrorForIndex));
				return nullptr;
			}
			funcIndex += 1;

			int ParameterIndex = 0;
			for (int OperandIndex : OperandIndices) {
				UFINProperty* param1 = func->GetParameters()[ParameterIndex++];
				TOptional<FINAny> otherValue = luaFIN_tonetworkvaluebyprop(L, OperandIndex, param1, true, false);
				if (!otherValue.IsSet()) break;
				Operands.Add(*otherValue);
			}
			if (OperandIndices.Num() != Operands.Num()) {
				Operands.Empty();
				continue;
			}
			break;
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
		TArray<FINAny> parameters;
		UFINFunction* func = luaStructFindOperator(L, Type, OperatorName, OperandIndices, parameters, CauseErrorForIndex);
		if (!func) return -2;
		
		FFINExecutionContext Ctx(Struct->GetData());
		TArray<FINAny> result = func->Execute(Ctx, parameters);
		for (const FINAny& val : result) {
			networkValueToLua(L, val, FFINNetworkTrace());
		} 
		return result.Num();
	}

	int luaStructUnaryOperator(lua_State* L, const char* LuaOperator, const FString& OperatorName, bool bCauseError) {
		int thisIndex = 1;
		
		FLuaStruct* thisStructLua;
		const TSharedRef<FINStruct> ThisStruct = luaFIN_checkstruct(L, thisIndex, nullptr, &thisStructLua);
		UFINStruct* StructType = thisStructLua->Type;

		return FMath::Max(0, luaStructExecuteOperator(L, ThisStruct, StructType, OperatorName, {}, bCauseError ? &thisIndex : nullptr));
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
			if (bCauseError) return luaL_error(L, "Invalid Operator for types %s and %s", lua_typename(L, 1), lua_typename(L, 2));
			if (ErrorCause) *ErrorCause = -1;
			return -1;
		}
		
		FLuaStruct* thisStructLua;
		const TSharedRef<FINStruct> ThisStruct = luaFIN_checkstruct(L, thisIndex, nullptr, &thisStructLua);
		UFINStruct* StructType = thisStructLua->Type;
		
		int result = luaStructExecuteBinaryOperator(L, OperatorName, otherIndex, ThisStruct, StructType, nullptr);
		if (result < 0) {
			if (bCauseError) return luaL_error(L, "Invalid Operator for struct of type %s", lua_typename(L, thisIndex));
			if (ErrorCause) *ErrorCause = -2;
			return -2;
		}
		return result;
	}

	int luaStructBinaryOperator(lua_State* L, const char* LuaOperator, const FString& OperatorName, bool bCommutative = false) {
		return luaStructTryBinaryOperator(L, LuaOperator, OperatorName, bCommutative, true);
	}

	int luaStructAdd(lua_State* L) {
		return luaStructBinaryOperator(L, "__add", FIN_OP_TEXT(FIN_Operator_Add), true);
	}

	int luaStructSub(lua_State* L) {
		return luaStructBinaryOperator(L, "__sub", FIN_OP_TEXT(FIN_Operator_Sub), false);
	}

	int luaStructMul(lua_State* L) {
		return luaStructBinaryOperator(L, "__mul", FIN_OP_TEXT(FIN_Operator_Mul), true);
	}

	int luaStructDiv(lua_State* L) {
		return luaStructBinaryOperator(L, "__div", FIN_OP_TEXT(FIN_Operator_Div), false);
	}

	int luaStructMod(lua_State* L) {
		return luaStructBinaryOperator(L, "__mod", FIN_OP_TEXT(FIN_Operator_Mod), false);
	}

	int luaStructPow(lua_State* L) {
		return luaStructBinaryOperator(L, "__pow", FIN_OP_TEXT(FIN_Operator_Pow), false);
	}

	int luaStructUnm(lua_State* L) {
		return luaStructUnaryOperator(L, "__unm", FIN_OP_TEXT(FIN_Operator_Neg), true);
	}
	
	int luaStructIdiv(lua_State* L) {
		return luaStructBinaryOperator(L, "__idiv", FIN_OP_TEXT(FIN_Operator_FDiv), false);
	}

	int luaStructBand(lua_State* L) {
		return luaStructBinaryOperator(L, "__band", FIN_OP_TEXT(FIN_Operator_BitAND), true);
	}

	int luaStructBor(lua_State* L) {
		return luaStructBinaryOperator(L, "__bor", FIN_OP_TEXT(FIN_Operator_BitOR), true);
	}
	
	int luaStructBxor(lua_State* L) {
		return luaStructBinaryOperator(L, "__bxor", FIN_OP_TEXT(FIN_Operator_BitXOR), true);
	}

	int luaStructBnot(lua_State* L) {
		return luaStructUnaryOperator(L, "__bnot", FIN_OP_TEXT(FIN_Operator_BitNOT), true);
	}

	int luaStructShl(lua_State* L) {
		return luaStructBinaryOperator(L, "__shl", FIN_OP_TEXT(FIN_Operator_ShiftL), false);
	}

	int luaStructShr(lua_State* L) {
		return luaStructBinaryOperator(L, "__shr", FIN_OP_TEXT(FIN_Operator_ShiftR), false);
	}

	int luaStructConcat(lua_State* L) {
		return luaStructBinaryOperator(L, "__concat", FIN_OP_TEXT(FIN_Operator_Concat), false);
	}

	int luaStructLen(lua_State* L) {
		return luaStructUnaryOperator(L, "__len", FIN_OP_TEXT(FIN_Operator_Len), true);
	}
	
	int luaStructEq(lua_State* L) {
		int result = luaStructTryBinaryOperator(L, "__eq", FIN_OP_TEXT(FIN_Operator_Equals), true, false);
		if (result >= 0) return result;
		
		const TSharedPtr<FINStruct> Struct1 = luaFIN_tostruct(L, 1);
		const TSharedPtr<FINStruct> Struct2 = luaFIN_tostruct(L, 2);
		if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
			lua_pushboolean(L, false);
			return 1;
		}
		
		lua_pushboolean(L, Struct1->GetStruct()->CompareScriptStruct(Struct1->GetData(), Struct2->GetData(), 0));
		return 1;
	}
	
	int luaStructLt(lua_State* L) {
		int result = luaStructTryBinaryOperator(L, "__lt", FIN_OP_TEXT(FIN_Operator_LessThan), false, false);
		if (result >= 0) return result;
		
		const TSharedPtr<FINStruct> Struct1 = luaFIN_tostruct(L, 1);
		const TSharedPtr<FINStruct> Struct2 = luaFIN_tostruct(L, 2);
		if (!Struct1->GetData() || !Struct2->GetData() || Struct1->GetStruct() != Struct2->GetStruct()) {
			lua_pushboolean(L, false);
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
		
		lua_pushboolean(L, Struct1->GetStruct()->GetStructTypeHash(Struct1->GetData()) < Struct2->GetStruct()->GetStructTypeHash(Struct2->GetData()));
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}
	
	int luaStructLe(lua_State* L) {
		int result = luaStructTryBinaryOperator(L, "__le", FIN_OP_TEXT(FIN_Operator_LessOrEqualThan), false, false);
		if (result >= 0) return result;
		
		const TSharedPtr<FINStruct> Struct1 = luaFIN_tostruct(L, 1);
		const TSharedPtr<FINStruct> Struct2 = luaFIN_tostruct(L, 2);
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
	
	int luaStructIndex(lua_State* L) {
		FLuaStruct* StructLua;
		const TSharedRef<FINStruct> Struct = luaFIN_checkstruct(L, 1, nullptr, &StructLua);
		UFINStruct* Type = StructLua->Type;
		
		const FString MemberName = lua_tostring(L, 2);
		
		StructMetaNameLock.Lock();
		const FString MetaName = StructToMetaName[Type];
		StructMetaNameLock.Unlock();
		
		int result = luaFindGetMember(L, Type, FFINExecutionContext(Struct->GetData()), MemberName, MetaName + "_" + MemberName, &luaStructFuncCall, false);
		if (result > 0) return result;

		return FMath::Max(0, luaStructExecuteBinaryOperator(L, FIN_OP_TEXT(FIN_Operator_Index), 2, Struct, Type, nullptr));
	}

	int luaStructNewIndex(lua_State* L) {
		const int thisIndex = 1;
		const int nameIndex = 2;
		const int operandIndex = 3;
		
		FLuaStruct* StructLua;
		const TSharedRef<FINStruct> Struct = luaFIN_checkstruct(L, thisIndex, nullptr, &StructLua);
		UFINStruct* Type = StructLua->Type;
			
		const FString MemberName = lua_tostring(L, nameIndex);
		
		int result = luaFindSetMember(L, Type, FFINExecutionContext(Struct->GetData()), MemberName, false, false);
		if (result > 0) return result;
		
		return luaStructExecuteOperator(L, Struct, Type, FIN_OP_TEXT(FIN_Operator_NewIndex), {nameIndex, operandIndex}, &thisIndex);
	}

	int luaStructCall(lua_State* L) {
		const int thisIndex = 1;
		const int operandsStartIndex = 2;

		int top = lua_gettop(L);

		FLuaStruct* StructLua;
		const TSharedRef<FINStruct> Struct = luaFIN_checkstruct(L, thisIndex, nullptr, &StructLua);
		UFINStruct* Type = StructLua->Type;

		TArray<int> OperandIndices;
		for (int i = operandsStartIndex; i <= top; ++i) {
			OperandIndices.Add(i);
		}
		return luaStructExecuteOperator(L, Struct, Type, FIN_OP_TEXT(FIN_Operator_Call), OperandIndices, &thisIndex);
	}

	int luaStructToString(lua_State* L) {
		const TSharedPtr<FINStruct> Struct = luaGetStruct(L, 1);
		if (!Struct->GetData()) {
			lua_pushboolean(L, false);
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
		
		lua_pushstring(L, TCHAR_TO_UTF8(*(FFINReflection::Get()->FindStruct(Struct->GetStruct())->GetInternalName() + "-Struct")));
		return 1;
	}

	int luaStructUnpersist(lua_State* L) {
		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		
		// get persist storage
		FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
		
		// get struct
		const FFINDynamicStructHolder& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));
		
		// create instance
		luaStruct(L, Struct);
		
		return 1;
	}

	int luaStructPersist(lua_State* L) {
		// get struct
		TSharedPtr<FINStruct> Struct = luaGetStruct(L, 1);

		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		
		// get persist storage
		FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;

		// push struct to persist
		lua_pushinteger(L, Storage.Add(Struct));
		
		// create & return closure
		lua_pushcclosure(L, &luaStructUnpersist, 1);
		return 1;
	}
	
	int luaStructGC(lua_State* L) {
		FLuaStruct* Struct = static_cast<FLuaStruct*>(lua_touserdata(L, 1));
		Struct->~FLuaStruct();
		return 0;
	}

	static const luaL_Reg luaStructMetatable[] = {
		{"__add", luaStructAdd},
		{"__sub", luaStructSub},
		{"__mul", luaStructMul},
		{"__div", luaStructDiv},
		{"__mod", luaStructMod},
		{"__pow", luaStructPow},
		{"__unm", luaStructUnm},
		{"__idiv", luaStructIdiv},
		{"__band", luaStructBand},
		{"__bor", luaStructBor},
		{"__bxor", luaStructBxor},
		{"__bnot", luaStructBnot},
		{"__shl", luaStructShl},
		{"__shr", luaStructShr},
		{"__concat", luaStructConcat},
		{"__len", luaStructLen},
		{"__eq", luaStructEq},
		{"__lt", luaStructLt},
		{"__le", luaStructLe},
		{"__index", luaStructIndex},
		{"__newindex", luaStructNewIndex},
		{"__call", luaStructCall},
		{"__tostring", luaStructToString},
		{"__persist", luaStructPersist},
		{"__gc", luaStructGC},
		{NULL, NULL}
	};

	int luaFindStruct(lua_State* L) {
		const int args = lua_gettop(L);

		for (int i = 1; i <= args; ++i) {
			const bool isT = lua_istable(L, i);

			TArray<FString> StructNames;
			if (isT) {
				const auto count = lua_rawlen(L, i);
				for (int j = 1; j <= count; ++j) {
					lua_geti(L, i, j);
					if (!lua_isstring(L, -1)) return luaL_argerror(L, i, "array contains non-string");
					StructNames.Add(lua_tostring(L, -1));
					lua_pop(L, 1);
				}
				lua_newtable(L);
			} else {
				if (!lua_isstring(L, i)) return luaL_argerror(L, i, "is not string");
				StructNames.Add(lua_tostring(L, i));
			}
			int j = 0;
			TArray<UFINStruct*> Structs;
			FFINReflection::Get()->GetStructs().GenerateValueArray(Structs);
			for (const FString& StructName : StructNames) {
				UFINStruct** Struct = Structs.FindByPredicate([StructName](UFINStruct* Struct) {
					if (Struct->GetInternalName() == StructName) return true;
					return false;
				});
				if (Struct) newInstance(L, FINTrace(*Struct));
				else lua_pushnil(L);
				if (isT) lua_seti(L, -2, ++j);
			}
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}
	
	void setupStructMetatable(lua_State* L, UFINStruct* Struct) {
		FScopeLock ScopeLock(&StructMetaNameLock);
		
		lua_getfield(L, LUA_REGISTRYINDEX, "PersistUperm");
		lua_getfield(L, LUA_REGISTRYINDEX, "PersistPerm");
		PersistSetup("StructSystem", -2);

		FString TypeName = Struct->GetInternalName();
		if (luaL_getmetatable(L, TCHAR_TO_UTF8(*TypeName)) != LUA_TNIL) {
			lua_pop(L, 3);
			return;
		}
		lua_pop(L, 1);
		luaL_newmetatable(L, TCHAR_TO_UTF8(*TypeName));							// ..., InstanceMeta
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__metatable");
		luaL_setfuncs(L, luaStructMetatable, 0);
		lua_newtable(L);															// ..., InstanceMeta, InstanceCache
		lua_setfield(L, -2, LUA_REF_CACHE);									// ..., InstanceMeta
		PersistTable(TCHAR_TO_UTF8(*TypeName), -1);
		lua_pop(L, 3);															// ...
		MetaNameToStruct.FindOrAdd(TypeName) = Struct;
		StructToMetaName.FindOrAdd(Struct) = TypeName;
	}

	int luaStructTypeCall(lua_State* L) {
		UFINStruct* Struct = luaFIN_tostructtype(L, 1);
		if (!Struct) return 0;
		UScriptStruct* ScriptStruct = FFINReflection::Get()->FindScriptStruct(Struct);
		if (!ScriptStruct) return 0;
		TSharedRef<FINStruct> StructContainer = MakeShared<FINStruct>(ScriptStruct);
		luaGetStruct(L, 2, StructContainer);
		luaStruct(L, *StructContainer);
		return 1;
	}

	int luaStructTypeUnpersist(lua_State* L) {
		FString StructName = luaFIN_checkfstring(L, lua_upvalueindex(1));
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
		luaFIN_pushStructType(L, Struct);
		return 1;
	}

	int luaStructTypePersist(lua_State* L) {
		UFINStruct* Type = *(UFINStruct**)lua_getuservalue(L, 1);
		luaFIN_pushfstring(L, Type->GetInternalName());
		lua_pushcclosure(L, &luaStructTypeUnpersist, 1);
		return 1;
	}

	static const luaL_Reg luaStructTypeMetatable[] = {
		{"__call", luaStructTypeCall},
		{"__persist", luaStructTypePersist},
		{nullptr, nullptr}
	};

	void luaFIN_pushStructType(lua_State* L, UFINStruct* Struct) {
		if (Struct) {
			*(UFINStruct**)lua_newuserdata(L, sizeof(UFINStruct*)) = Struct;
			luaL_setmetatable(L, STRUCT_TYPE_METATABLE_NAME);
		} else {
			lua_pushnil(L);
		}
	}

	UFINStruct* luaFIN_tostructtype(lua_State* L, int index) {
		if (lua_isnil(L, index)) return nullptr;
		return luaFIN_checkStructType(L, index);
	}

	UFINStruct* luaFIN_checkStructType(lua_State* L, int index) {
		UFINStruct* Struct = *(UFINStruct**)luaL_checkudata(L, index, STRUCT_TYPE_METATABLE_NAME);
		return Struct;
	}

	int luaStructLibIndex(lua_State* L) {
		FString StructName = luaFIN_checkfstring(L, 2);
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
		if (Struct && (Struct->GetStructFlags() & FIN_Struct_Constructable)) {
			luaFIN_pushStructType(L, Struct);
		} else {
			lua_pushnil(L);
		}
		return 1;
	}

	static const luaL_Reg luaStructLibMetatable[] = {
		{"__index", luaStructLibIndex},
		{NULL, NULL}
	};

	void setupStructSystem(lua_State* L) {
		PersistSetup("StructSystem", -2);
		
		lua_pushcfunction(L, luaStructFuncCall);			// ..., InstanceFuncCall
		PersistValue("StructFuncCall");				// ...
		lua_pushcfunction(L, luaStructUnpersist);			// ..., LuaInstanceUnpersist
		PersistValue("StructUnpersist");				// ...
		lua_register(L, "findStruct", luaFindStruct);
		PersistGlobal("findStruct")

		lua_newuserdata(L, 0);					// ..., StructLib
		luaL_newmetatable(L, "StructLib");				// ..., StructLib, StructLibMetatable
		luaL_setfuncs(L, luaStructLibMetatable, 0);
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__metatable");
		PersistTable("StructLib", -1);
		lua_setmetatable(L, -2);						// ..., StructLib
		lua_setglobal(L, "structs");					// ...
		PersistGlobal("structs");

		lua_pushcfunction(L, luaStructLibIndex);				// ..., LuaStructLibIndex
		PersistValue("StructStructLibIndex");				// ...

		luaL_newmetatable(L, STRUCT_TYPE_METATABLE_NAME);
		luaL_setfuncs(L, luaStructTypeMetatable, 0);
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__metatable");
		PersistTable(STRUCT_TYPE_METATABLE_NAME, -1);
		lua_pop(L, 1);
	}
}
