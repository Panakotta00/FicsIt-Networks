#include "LuaProcessor/LuaStruct.h"

#include "LuaProcessor/LuaRef.h"
#include "LuaProcessor/LuaUtil.h"
#include "LuaProcessor/LuaProcessor.h"
#include "LuaProcessor/LuaInstance.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStruct.h"

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
			luaFIN_pushNetworkValue(L, val, FFINNetworkTrace());
		} 
		return result.Num();
	}

	int luaStructUnaryOperator(lua_State* L, const char* LuaOperator, const FString& OperatorName, bool bCauseError) {
		int thisIndex = 1;
		
		FLuaStruct* thisStructLua;
		const TSharedRef<FINStruct> ThisStruct = luaFIN_checkStruct(L, thisIndex, nullptr, &thisStructLua);
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
		const TSharedRef<FINStruct> ThisStruct = luaFIN_checkStruct(L, thisIndex, nullptr, &thisStructLua);
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
		
		const TSharedPtr<FINStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
		const TSharedPtr<FINStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
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
		
		const TSharedPtr<FINStruct> Struct1 = luaFIN_toStruct(L, 1, nullptr, false);
		const TSharedPtr<FINStruct> Struct2 = luaFIN_toStruct(L, 2, nullptr, false);
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
	
	int luaStructIndex(lua_State* L) {
		const int thisIndex = 1;
		const int nameIndex = 2;
		
		FLuaStruct* StructLua;
		const TSharedRef<FINStruct> Struct = luaFIN_checkStruct(L, thisIndex, nullptr, &StructLua);
		UFINStruct* Type = StructLua->Type;
		
		const FString MemberName = luaFIN_toFString(L, nameIndex);

		FFINExecutionContext Context(Struct->GetData());
		if (luaFIN_pushFunctionOrGetProperty(L, thisIndex, Type, MemberName, EFINFunctionFlags::FIN_Func_MemberFunc, EFINRepPropertyFlags::FIN_Prop_Attrib, Context, false)) {
			return 1;
		}
		
		return luaStructExecuteBinaryOperator(L, FIN_OP_TEXT(FIN_Operator_Index), 2, Struct, Type, &thisIndex);
	}

	int luaStructNewIndex(lua_State* L) {
		const int thisIndex = 1;
		const int nameIndex = 2;
		const int operandIndex = 3;
		
		FLuaStruct* StructLua;
		const TSharedRef<FINStruct> Struct = luaFIN_checkStruct(L, thisIndex, nullptr, &StructLua);
		UFINStruct* Type = StructLua->Type;
			
		const FString MemberName = luaFIN_toFString(L, nameIndex);

		FFINExecutionContext Context(Struct->GetData());
		if (luaFIN_tryExecuteSetProperty(L, thisIndex, Type, MemberName, EFINRepPropertyFlags::FIN_Prop_Attrib, Context, operandIndex, false)) {
			return 1;
		}
		
		return luaStructExecuteOperator(L, Struct, Type, FIN_OP_TEXT(FIN_Operator_NewIndex), {nameIndex, operandIndex}, &thisIndex);
	}

	int luaStructCall(lua_State* L) {
		const int thisIndex = 1;
		const int operandsStartIndex = 2;

		int top = lua_gettop(L);

		FLuaStruct* StructLua;
		const TSharedRef<FINStruct> Struct = luaFIN_checkStruct(L, thisIndex, nullptr, &StructLua);
		UFINStruct* Type = StructLua->Type;

		TArray<int> OperandIndices;
		for (int i = operandsStartIndex; i <= top; ++i) {
			OperandIndices.Add(i);
		}
		return luaStructExecuteOperator(L, Struct, Type, FIN_OP_TEXT(FIN_Operator_Call), OperandIndices, &thisIndex);
	}

	int luaStructToString(lua_State* L) {
		FLuaStruct* LuaStruct;
		const TSharedPtr<FINStruct> Struct = luaFIN_checkStruct(L, 1, nullptr, false, &LuaStruct);
		if (!Struct->GetData()) {
			lua_pushnil(L);
			return UFINLuaProcessor::luaAPIReturn(L, 1);
		}
		
		luaFIN_pushFString(L, LuaStruct->Type->GetInternalName() + "-Struct");
		return 1;
	}

	int luaStructUnpersist(lua_State* L) {
		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;

		const FFINDynamicStructHolder& Struct = *Storage.GetStruct(luaL_checkinteger(L, lua_upvalueindex(1)));

		luaFIN_pushStruct(L, Struct);
		
		return 1;
	}

	int luaStructPersist(lua_State* L) {
		TSharedPtr<FINStruct> Struct = luaFIN_checkStruct(L, 1, nullptr, false);

		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		FFINLuaProcessorStateStorage& Storage = Processor->StateStorage;
		lua_pushinteger(L, Storage.Add(Struct));

		lua_pushcclosure(L, &luaStructUnpersist, 1);
		
		return 1;
	}
	
	int luaStructGC(lua_State* L) {
		FLuaStruct* Struct = static_cast<FLuaStruct*>(lua_touserdata(L, 1));
		Struct->~FLuaStruct();
		return 0;
	}

	static constexpr luaL_Reg luaStructMetatable[] = {
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

	bool luaFIN_pushStruct(lua_State* L, const FINStruct& Struct) {
		UFINStruct* Type = FFINReflection::Get()->FindStruct(Struct.GetStruct());
		if (!Type) {
			lua_pushnil(L);
			return false;
		}
		
		FLuaStruct* LuaStruct = static_cast<FLuaStruct*>(lua_newuserdata(L, sizeof(FLuaStruct)));
		new (LuaStruct) FLuaStruct(Type, Struct, UFINLuaProcessor::luaGetProcessor(L)->GetKernel());
		luaL_setmetatable(L, FIN_LUA_STRUCT_METATABLE_NAME);

		return true;
	}

	TSharedPtr<FINStruct> luaFIN_convertToStruct(lua_State* L, int i, UFINStruct* Type, bool bAllowImplicitConstruction) {
		if (!(Type->GetStructFlags() & FIN_Struct_Constructable)) return nullptr;

		TSharedRef<FINStruct> Struct = MakeShared<FINStruct>(FFINReflection::Get()->FindScriptStruct(Type));
		
		int j = 0;
		for (UFINProperty* Prop : Type->GetProperties()) {
			if (!(Prop->GetPropertyFlags() & FIN_Prop_Attrib)) continue;
			if (lua_getfield(L, i, TCHAR_TO_UTF8(*Prop->GetInternalName())) == LUA_TNIL) {
				lua_geti(L, i, ++j);
			}
			TOptional<FINAny> Value = luaFIN_toNetworkValueByProp(L, -1, Prop, true, bAllowImplicitConstruction);
			lua_pop(L, 1);
			if (!Value.IsSet()) return nullptr;
			Prop->SetValue(Struct->GetData(), Value.GetValue());
		}

		return Struct;
	}

	FLuaStruct* luaFIN_toLuaStruct(lua_State* L, int Index, UFINStruct* Type) {
		FLuaStruct* LuaStruct = (FLuaStruct*)luaL_testudata(L, Index, FIN_LUA_STRUCT_METATABLE_NAME);
		if (LuaStruct && LuaStruct->Type->IsChildOf(Type)) {
			return LuaStruct;
		}
		return nullptr;
	}

	TSharedPtr<FINStruct> luaFIN_toStruct(lua_State* L, int Index, UFINStruct* Type, bool bAllowConstruction, FLuaStruct** OutLuaStruct) {
		if (bAllowConstruction && lua_istable(L, Index) && Type) {
			if (OutLuaStruct) *OutLuaStruct = nullptr;
			return luaFIN_convertToStruct(L, Index, Type, bAllowConstruction);
		}
		
		FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, Type);
		if (OutLuaStruct) *OutLuaStruct = LuaStruct;
		return LuaStruct->Struct;
	}

	TSharedRef<FINStruct> luaFIN_checkStruct(lua_State* L, int i, UFINStruct* Type, bool bAllowConstruction, FLuaStruct** luaStruct) {
		TSharedPtr<FINStruct> Struct = luaFIN_toStruct(L, i, Type, bAllowConstruction, luaStruct);
		if (!Struct.IsValid()) luaL_argerror(L, i, "Not a struct"); // TODO: Improve error message with why struct conv not was working, template mismatch, general no struct etc.
		return Struct.ToSharedRef();
	}

	int luaStructTypeCall(lua_State* L) {
		UFINStruct* Struct = luaFIN_toStructType(L, 1);
		if (!Struct) return 0;
		
		luaL_argcheck(L, Struct->GetStructFlags() & FIN_Struct_Constructable, 1, "Can not be constructed.");

		TSharedRef<FINStruct> Value = luaFIN_checkStruct(L, 2, Struct, true, nullptr);
		luaFIN_pushStruct(L, *Value);

		return 1;
	}

	int luaStructTypeUnpersist(lua_State* L) {
		FString StructName = luaFIN_checkFString(L, lua_upvalueindex(1));
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
		luaFIN_pushStructType(L, Struct);
		return 1;
	}

	int luaStructTypePersist(lua_State* L) {
		UFINStruct* Type = *(UFINStruct**)lua_touserdata(L, 1);
		luaFIN_pushFString(L, Type->GetInternalName());
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
			luaL_setmetatable(L, FIN_LUA_STRUCT_TYPE_METATABLE_NAME);
		} else {
			lua_pushnil(L);
		}
	}

	UFINStruct* luaFIN_toStructType(lua_State* L, int index) {
		if (lua_isnil(L, index)) return nullptr;
		return luaFIN_checkStructType(L, index);
	}

	UFINStruct* luaFIN_checkStructType(lua_State* L, int index) {
		UFINStruct* Struct = *(UFINStruct**)luaL_checkudata(L, index, FIN_LUA_STRUCT_TYPE_METATABLE_NAME);
		return Struct;
	}

	int luaStructLibIndex(lua_State* L) {
		FString StructName = luaFIN_checkFString(L, 2);
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(StructName);
		if (Struct) {
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

		// Register & Persist Struct-Metatable
		luaL_newmetatable(L, FIN_LUA_STRUCT_METATABLE_NAME);		// ..., StructMetatable
		luaL_setfuncs(L, luaStructMetatable, 0);
		lua_pushstring(L, FIN_LUA_STRUCT_METATABLE_NAME);				// ..., StructMetatable, string
		lua_setfield(L, -2, "__metatable");						// ..., StructMetatable
		PersistTable(FIN_LUA_STRUCT_METATABLE_NAME, -1);
		lua_pop(L, 1);												// ...

		// Register & Persist StructType-Metatable
		luaL_newmetatable(L, FIN_LUA_STRUCT_TYPE_METATABLE_NAME);	// ..., StructTypMetatable
		luaL_setfuncs(L, luaStructTypeMetatable, 0);
		lua_pushboolean(L, true);										// ..., StructTypeMetatable, bool
		lua_setfield(L, -2, "__metatable");						// ..., StructTypeMetatable
		PersistTable(FIN_LUA_STRUCT_TYPE_METATABLE_NAME, -1);
		lua_pop(L, 1);												// ...

		// Add & Persist StructLib as global 'structs'
		lua_newuserdata(L, 0);										// ..., StructLib
		luaL_newmetatable(L, FIN_LUA_STRUCT_LIB_METATABLE_NAME);	// ..., StructLib, StructLibMetatable
		luaL_setfuncs(L, luaStructLibMetatable, 0);
		lua_pushstring(L, FIN_LUA_STRUCT_LIB_METATABLE_NAME);			// ..., StructLib, StructLibMetatable, string
		lua_setfield(L, -2, "__metatable");						// ..., StructLib, StructLibMetatable
		PersistTable(FIN_LUA_STRUCT_LIB_METATABLE_NAME, -1);
		lua_setmetatable(L, -2);									// ..., StructLib
		lua_setglobal(L, "structs");								// ...
		PersistGlobal("structs");
	}
}
