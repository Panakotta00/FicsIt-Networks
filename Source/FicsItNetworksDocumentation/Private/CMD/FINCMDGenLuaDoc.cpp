#include "FicsItNetworksDocumentation.h"
#include "FINLua/FINLuaModule.h"
#include "Logging/StructuredLog.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"

UE_DISABLE_OPTIMIZATION_SHIP
namespace FINGenLuaDoc {
	FString GetType(FFINReflection& Ref, UFINProperty* Prop) {
		if (!Prop) return TEXT("any");
		switch (Prop->GetType()) {
			case FIN_NIL:
				return TEXT("nil");
			case FIN_BOOL:
				return TEXT("boolean");
			case FIN_INT:
				return TEXT("integer");
			case FIN_FLOAT:
				return TEXT("number");
			case FIN_STR:
				return TEXT("string");
			case FIN_OBJ: {
				UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Prop);
				UFINClass* Class = Ref.FindClass(ObjProp->GetSubclass());
				if (!Class) return TEXT("Object");
				return Class->GetInternalName();
			} case FIN_TRACE: {
				UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Prop);
				UFINClass* Class = Ref.FindClass(TraceProp->GetSubclass());
				if (!Class) return TEXT("Object");
				return Class->GetInternalName();
			} case FIN_CLASS: {
				UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Prop);
				UFINClass* Class = Ref.FindClass(ClassProp->GetSubclass());
				if (!Class) return TEXT("Object-Class");
				return Class->GetInternalName() + TEXT("-Class");
			} case FIN_STRUCT: {
				UFINStructProperty* StructProp = Cast<UFINStructProperty>(Prop);
				UFINStruct* Struct = Ref.FindStruct(StructProp->GetInner());
				if (!Struct) return TEXT("any");
				return Struct->GetInternalName();
			} case FIN_ARRAY: {
				UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Prop);
				return GetType(Ref, ArrayProp->GetInnerType()) + TEXT("[]");
			} default:
				return TEXT("any");
		}
	}

	TOptional<FString> MatchLuaOperator(UFINFunction* Func) {
		TOptional<EFINOperator> op = Func->IsOperator();
		if (!op.IsSet()) return {};
		switch (*op) {
			case FIN_Op_Add:
				return FString(TEXT("add"));
			case FIN_Op_Sub:
				return FString(TEXT("sub"));
			case FIN_Op_Mul:
				return FString(TEXT("mul"));
			case FIN_Op_Div:
				return FString(TEXT("div"));
			case FIN_Op_Mod:
				return FString(TEXT("mod"));
			case FIN_Op_Pow:
				return FString(TEXT("pow"));
			case FIN_Op_Neg:
				return FString(TEXT("unm"));
			case FIN_Op_FDiv:
				return FString(TEXT("idiv"));
			case FIN_Op_BitAND:
				return FString(TEXT("band"));
			case FIN_Op_BitOR:
				return FString(TEXT("bor"));
			case FIN_Op_BitXOR:
				return FString(TEXT("bxor"));
			case FIN_Op_BitNOT:
				return FString(TEXT("bnot"));
			case FIN_Op_ShiftL:
				return FString(TEXT("shl"));
			case FIN_Op_ShiftR:
				return FString(TEXT("shr"));
			case FIN_Op_Concat:
				return FString(TEXT("concat"));
			case FIN_Op_Len:
				return FString(TEXT("len"));
			case FIN_Op_Equals:
				return FString(TEXT("eq"));
			case FIN_Op_LessThan:
				return FString(TEXT("lt"));
			case FIN_Op_LessOrEqualThan:
				return FString(TEXT("le"));
			case FIN_Op_Index:
				return FString(TEXT("index"));
			case FIN_Op_NewIndex:
				return FString(TEXT("newindex"));
			case FIN_Op_Call:
				return FString(TEXT("call"));
			default:
				return {};
		}
	}

	FString GetInlineDescription(FString Description) {
		Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

		Description.ReplaceInline(TEXT("\n"), TEXT("<br>"));
		Description.ReplaceCharInline('\r', ' ');

		return Description;
	}

	void WriteMultiLineDescription(FStringBuilderBase& Str, FString Description) {
		Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

		FString Line;
		while (Description.Split(TEXT("\n"), &Line, &Description)) {
			Str.Appendf(TEXT("--- %s\n"), *Line);
		}
		Str.Appendf(TEXT("--- %s\n"), *Description);
	}

	void WriteFuture(FStringBuilderBase& Str, const FString& TypeIdentifier, const FString& FunctionName, const FString& TypedParameterList) {
		FString identifier = FString::Printf(TEXT("Future_%s_%s"), *TypeIdentifier, *FunctionName);
		Str.Appendf(TEXT("---@class %s : Future\n"), *identifier);
		Str.Appendf(TEXT("%s = {}\n"), *identifier);

		FString returnValues;
		if (!TypedParameterList.IsEmpty()) {
			returnValues = FString::Printf(TEXT(":(%s)"), *TypedParameterList);
		}

		Str.Appendf(TEXT("---@type fun(self:%s)%s\n"), *identifier, *returnValues);
		Str.Appendf(TEXT("function %s:await() end\n"), *identifier);

		Str.Appendf(TEXT("---@type fun(self:%s)%s\n"), *identifier, *returnValues);
		Str.Appendf(TEXT("function %s:get() end\n"), *identifier);

		Str.Appendf(TEXT("---@type fun(self:%s):boolean\n"), *identifier);
		Str.Appendf(TEXT("function %s:canGet() end\n"), *identifier);
	}

	void WriteProperty(FStringBuilderBase& Documentation, FFINReflection& Ref, UFINProperty* Prop) {
		FString Identifier = Prop->GetInternalName();
		if (Prop->GetPropertyFlags() & (FIN_Prop_ClassProp | FIN_Prop_StaticProp)) {
			Identifier += TEXT("-Class");
		}
		Documentation.Appendf(TEXT("---@field public %s %s %s\n"), *Identifier, *GetType(Ref, Prop), *GetInlineDescription(Prop->GetDescription().ToString()));
	}

	void WriteFunction(FStringBuilderBase& Str, FFINReflection& Ref, const FString& Parent, const FString& Type, UFINFunction* Func) {
		if (MatchLuaOperator(Func)) return;

		WriteMultiLineDescription(Str, Func->GetDescription().ToString());

		TArray<UFINProperty*> parameters;
		TArray<UFINProperty*> returnValues;
		for (UFINProperty* Prop : Func->GetParameters()) {
			EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();
			if (!(Flags & FIN_Prop_Param)) continue;
			if (Flags & FIN_Prop_OutParam) {
				returnValues.Add(Prop);
			} else {
				parameters.Add(Prop);
			}
		}

		TArray<FString> paramList;
		TArray<FString> typedParameterList;
		typedParameterList.Add(TEXT("self:") + Type);
		for (UFINProperty* parameter : parameters) {
			paramList.Add(parameter->GetInternalName());
			typedParameterList.Add(parameter->GetInternalName() + TEXT(":") + GetType(Ref, parameter));
			Str.Appendf(TEXT("---@param %s %s %s\n"), *parameter->GetInternalName(), *GetType(Ref, parameter), *GetInlineDescription(parameter->GetDescription().ToString()));
		}
		if (Func->FunctionFlags & FIN_Func_VarArgs) {
			paramList.Add(TEXT("..."));
			typedParameterList.Add(TEXT("...:any"));
			Str.Append(TEXT("---@param ... any\n"));
		}

		bool bFuture = !(Func->GetFunctionFlags() & FIN_Func_RT_Parallel);

		TArray<FString> returnValueList;
		for (UFINProperty* returnValue : returnValues) {
			returnValueList.Add(returnValue->GetInternalName() + TEXT(":") + GetType(Ref, returnValue));
			if (!bFuture) {
				Str.Appendf(TEXT("---@return %s %s %s\n"), *GetType(Ref, returnValue), *returnValue->GetInternalName(), *GetInlineDescription(returnValue->GetDescription().ToString()));
			}
		}
		if (Func->FunctionFlags & FIN_Func_VarRets) {
			returnValueList.Add(TEXT("...:any"));
			if (!bFuture) {
				Str.Append(TEXT("---@return ... any\n"));
			}
		}
		FString joinedReturnValueList = FString::Join(returnValueList, TEXT(","));
		FString futureParameterList;
		if (bFuture) {
			FString futureTypeIdentifier = FString::Printf(TEXT("Future_%s_%s"), *Type, *Func->GetInternalName());
			futureParameterList = joinedReturnValueList;
			joinedReturnValueList = futureTypeIdentifier;
			Str.Appendf(TEXT("---@return %s\n"), *futureTypeIdentifier);
		}

		FString functionTypedReturn;
		if (!returnValueList.IsEmpty()) {
			functionTypedReturn = FString::Printf(TEXT(":(%s)"), *joinedReturnValueList);
		}

		Str.Appendf(TEXT("---@type (fun(%s)%s)|ReflectionFunction\n"), *FString::Join(typedParameterList, TEXT(",")), *functionTypedReturn);

		FString Identifier = Func->GetInternalName();
		if (Func->GetFunctionFlags() & (FIN_Func_ClassFunc | FIN_Func_StaticFunc)) {
			Identifier += TEXT("-Class");
		}

		Str.Appendf(TEXT("function %s:%s(%s) end\n"), *Parent, *Identifier, *FString::Join(paramList, TEXT(", ")));

		if (bFuture) {
			WriteFuture(Str, Type, Func->GetInternalName(), futureParameterList);
		}

		// TODO: Add support for Operator Overloading
	}

	void WriteStruct(FStringBuilderBase& Str, FFINReflection& Ref, const UFINStruct* Struct) {
		if (Struct->GetInternalName() == TEXT("Future")) return;

		WriteMultiLineDescription(Str,  Struct->GetDescription().ToString());

		{
			FString ClassIdentifier = Struct->GetInternalName();
			FString ClassDeclaration = ClassIdentifier;
			if (UFINStruct* parent = Struct->GetParent()) {
				ClassDeclaration += TEXT(" : ") + parent->GetInternalName();
			}
			Str.Appendf(TEXT("---@class %s\n"), *ClassDeclaration);

			for (UFINProperty* Prop : Struct->GetProperties(false)) {
				if ((Prop->GetPropertyFlags() & FIN_Prop_Attrib) && !(Prop->GetPropertyFlags() & FIN_Prop_ClassProp)) {
					WriteProperty(Str, Ref, Prop);
				}
			}

			for (UFINFunction* Func : Struct->GetFunctions(false)) {
				if (!((Func->GetFunctionFlags() & FIN_Func_MemberFunc) && !(Func->GetFunctionFlags() & FIN_Func_ClassFunc))) continue;
				TOptional<FString> op = MatchLuaOperator(Func);
				if (!op) continue;
				FString parameter;
				FString returnValue;
				for (UFINProperty* prop : Func->GetParameters()) {
					if (prop->GetPropertyFlags() & FIN_Prop_Param){
						if (prop->GetPropertyFlags() & FIN_Prop_OutParam) {
							returnValue = GetType(Ref, prop);
						} else {
							parameter = GetType(Ref, prop);
						}
					}
				}

				Str.Appendf(TEXT("---@operator %s(%s):%s\n"), **op, *parameter, *returnValue);
			}

			Str.Appendf(TEXT("%s = {}\n"), *Struct->GetInternalName());

			for (UFINFunction* Func : Struct->GetFunctions(false)) {
				if ((Func->GetFunctionFlags() & FIN_Func_MemberFunc) && !(Func->GetFunctionFlags() & FIN_Func_ClassFunc)) {
					WriteFunction(Str, Ref, Struct->GetInternalName(), ClassIdentifier, Func);
				}
			}
		}

		{
			WriteMultiLineDescription(Str, Struct->GetDescription().ToString());

			FString ClassIdentifier = Struct->GetInternalName() + TEXT("-Class");
			FString ClassDeclaration = ClassIdentifier;
			if (UFINStruct* parent = Struct->GetParent()) {
				ClassDeclaration += TEXT(" : ") + parent->GetInternalName() + TEXT("-Class");
			}
			Str.Appendf(TEXT("---@class %s\n"), *ClassDeclaration);

			for (UFINProperty* Prop : Struct->GetProperties(false)) {
				if ((Prop->GetPropertyFlags() & FIN_Prop_Attrib) && (Prop->GetPropertyFlags() & FIN_Prop_ClassProp)) {
					WriteProperty(Str, Ref, Prop);
				}
			}

			Str.Appendf(TEXT("%s_Class = {}\n"), *Struct->GetInternalName());

			for (UFINFunction* Func : Struct->GetFunctions(false)) {
				if ((Func->GetFunctionFlags() & FIN_Func_MemberFunc) && (Func->GetFunctionFlags() & FIN_Func_ClassFunc)) {
					WriteFunction(Str, Ref, Struct->GetInternalName(), ClassIdentifier, Func);
				}
			}
		}

		Str.Append(TEXT("\n"));
	}

	void WriteLuaValue(FStringBuilderBase& Str, const TSharedPtr<FFINLuaModuleValue>& Value, const FString& Identifier);

	void WriteLuaTable(FStringBuilderBase& Str, const FFINLuaTable& Table, const FString& Identifier) {
		Str.Append(TEXT("---@type table\n"));
		Str.Appendf(TEXT("%s = {}\n"), *Identifier);

		for (const FFINLuaTableField& field : Table.Fields) {
			WriteMultiLineDescription(Str, field.Description.ToString());
			WriteLuaValue(Str, field.Value, Identifier + TEXT(".") + field.Key);
			Str.Append(TEXT("\n"));
		}
	}

	void WriteLuaFunction(FStringBuilderBase& Str, const FFINLuaFunction& Function, const FString& Identifier) {
		TArray<FString> parameterList;
		TArray<FString> typedParameterList;
		for (const auto& parameter : Function.Parameters) {
			Str.Appendf(TEXT("---@param %s %s %s\n"), *parameter.InternalName, *parameter.Type, *parameter.Description.ToString());
			parameterList.Add(parameter.InternalName);
			typedParameterList.Add(parameter.InternalName + TEXT(":") + parameter.Type);
		}

		TArray<FString> typedReturnValueList;
		for (const auto& returnValue : Function.ReturnValues) {
			Str.Appendf(TEXT("---@return %s %s %s\n"), *returnValue.Type, *returnValue.InternalName, *returnValue.Description.ToString());
			typedReturnValueList.Add(returnValue.InternalName + TEXT(":") + returnValue.Type);
		}

		FString functionReturn;
		if (!typedReturnValueList.IsEmpty()) {
			functionReturn = FString::Printf(TEXT(":(%s)"), *FString::Join(typedReturnValueList, TEXT(",")));
		}

		Str.Appendf(TEXT("---@type (fun(%s)%s)|ModuleTableFunction\n"), *FString::Join(typedParameterList, TEXT(",")), *functionReturn);
		Str.Appendf(TEXT("function %s(%s) end\n"), *Identifier, *FString::Join(parameterList, TEXT(", ")));
	}

	void WriteLuaBareValue(FStringBuilderBase& Str, const FFINLuaModuleBareValue& BareValue, const FString& Identifier) {
		if (!BareValue.Type.IsEmpty()) {
			Str.Appendf(TEXT("---@type %s\n"), *BareValue.Type);
		}
		Str.Appendf(TEXT("%s = nil\n"), *Identifier);
	}

	void WriteLuaValue(FStringBuilderBase& Str, const TSharedPtr<FFINLuaModuleValue>& Value, const FString& Identifier) {
		auto typeID = Value->TypeID();
		if (typeID == FINTypeId<FFINLuaFunction>::ID()) {
			WriteLuaFunction(Str, *StaticCastSharedPtr<FFINLuaFunction>(Value), Identifier);
		} else if (typeID == FINTypeId<FFINLuaTable>::ID()) {
			WriteLuaTable(Str, *StaticCastSharedPtr<FFINLuaTable>(Value), Identifier);
		} else if (typeID == FINTypeId<FFINLuaModuleBareValue>::ID()) {
			WriteLuaBareValue(Str, *StaticCastSharedPtr<FFINLuaModuleBareValue>(Value), Identifier);
		} else {
			Str.Appendf(TEXT("%s = nil\n"), *Identifier);
		}
		Str.Append(TEXT("\n"));
	}

	void WriteMetatable(FStringBuilderBase& Str, const FFINLuaMetatable& Metatable) {
		WriteMultiLineDescription(Str, Metatable.Description.ToString());

		Str.Appendf(TEXT("---@class %s\n"), *Metatable.InternalName);

		Str.Appendf(TEXT("%s = {}\n\n"), *Metatable.InternalName);

		const FFINLuaTableField* index = nullptr;
		for (const FFINLuaTableField& field : Metatable.Table->Fields) {
			if (field.Key == TEXT("__index") && field.Value->TypeID() == FINTypeId<FFINLuaTable>::ID()) {
				index = &field;
			}
			if (!field.Key.StartsWith(TEXT("__"))) {
				WriteMultiLineDescription(Str, field.Description.ToString());
				WriteLuaValue(Str, field.Value, Metatable.InternalName + TEXT(".") + field.Key);
				Str.Append(TEXT("\n"));
			}
			// TODO: Add operator definition
			// TODO: Add overloads for call operator
		}

		if (index) {
			const FFINLuaTable& table = *StaticCastSharedPtr<FFINLuaTable>(index->Value);
			for (const FFINLuaTableField& field : table.Fields) {
				WriteMultiLineDescription(Str, field.Description.ToString());
				WriteLuaValue(Str, field.Value, Metatable.InternalName + TEXT(".") + field.Key);
				Str.Append(TEXT("\n"));
			}
		}

		if (Metatable.InternalName == TEXT("ClassLib")) {
			FFINReflection& reflection = *FFINReflection::Get();
			for (auto[Class, FINClass] : reflection.GetClasses()) {
				Str.Appendf(TEXT("---@type %s-Class\n"), *FINClass->GetInternalName());
				Str.Appendf(TEXT("ClassLib.%s = {}\n"), *FINClass->GetInternalName());
			}
		} else if (Metatable.InternalName == TEXT("StructLib")) {
			FFINReflection& reflection = *FFINReflection::Get();
			for (auto[Struct, FINStruct] : reflection.GetStructs()) {
				Str.Appendf(TEXT("---@type fun(table):%s\n"), *FINStruct->GetInternalName());
				Str.Appendf(TEXT("function StructLib.%s(t) end\n"), *FINStruct->GetInternalName());
			}
		}
	}

	void WriteGlobal(FStringBuilderBase& Str, const FFINLuaGlobal& Global) {
		WriteMultiLineDescription(Str, Global.Description.ToString());

		WriteLuaValue(Str, Global.Value, Global.InternalName);

		Str.Append(TEXT("\n"));
	}

	void WriteModule(FStringBuilderBase& Str, const TSharedRef<FFINLuaModule>& Module) {
		for (const FFINLuaMetatable& metatable : Module->Metatables) {
			WriteMetatable(Str, metatable);
		}
		for (const FFINLuaGlobal& global : Module->Globals) {
			WriteGlobal(Str, global);
		}
	}

	FString GenDocumentation() {
		FStringBuilderBase str;

		str.Append(TEXT("---@meta\n\n"));

		const auto& moduleRegistry = FFINLuaModuleRegistry::GetInstance();
		for (const TSharedRef<FFINLuaModule>& module : moduleRegistry.Modules) {
			WriteModule(str, module);
		}

		FFINReflection& reflection = *FFINReflection::Get();
		for (TPair<UClass*, UFINClass*> Class : reflection.GetClasses()) {
			WriteStruct(str, reflection, Class.Value);
		}
		for (TPair<UScriptStruct*, UFINStruct*> Struct : reflection.GetStructs()) {
			WriteStruct(str, reflection, Struct.Value);
		}

		return str.ToString();
	}

	bool ExecCMD(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
		if (FParse::Command(&Command, TEXT("FINGenLuaDoc"))) {
			UE_LOG(LogFicsItNetworksDocumentation, Display, TEXT("Generating FicsIt-Networks Lua Documentation..."));

			FString documentation = GenDocumentation();

			UE_LOG(LogFicsItNetworksDocumentation, Display, TEXT("FicsIt-Networks Lua Documentation generated!"));

			FString path;
			if (!FParse::Value(Command, TEXT("Path"), path)) {
				path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
				path = FPaths::Combine(path, TEXT("FINLuaDocumentation.lua"));
			}

			UE_LOGFMT(LogFicsItNetworksDocumentation, Display, "Saving FicsIt-Networks Lua Documentation under: {Path}", path);

			FFileHelper::SaveStringToFile(documentation, *path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

			UE_LOGFMT(LogFicsItNetworksDocumentation, Display, "FicsIt-Networks Lua Documentation Saved!");

			return true;
		}
		return false;
	}

	[[maybe_unused]] static FStaticSelfRegisteringExec SelfRegisterCMD(&ExecCMD);
}
UE_ENABLE_OPTIMIZATION_SHIP

