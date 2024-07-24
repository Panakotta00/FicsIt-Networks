#include "FicsItNetworksLuaModule.h"
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

	void WriteProperty(FStringBuilderBase& Documentation, FFINReflection& Ref, UFINProperty* Prop) {
		Documentation.Appendf(TEXT("---@field public %s %s %s\n"), *Prop->GetInternalName(), *GetType(Ref, Prop), *GetInlineDescription(Prop->GetDescription().ToString()));
	}

	void WriteFunction(FStringBuilderBase& Str, FFINReflection& Ref, FString Parent, UFINFunction* Func) {
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

		FString ParamList;
		for (UFINProperty* parameter : parameters) {
			if (ParamList.Len() > 0) ParamList.Append(", ");
			ParamList.Append(parameter->GetInternalName());
			Str.Appendf(TEXT("---@param %s %s %s\n"), *parameter->GetInternalName(), *GetType(Ref, parameter), *GetInlineDescription(parameter->GetDescription().ToString()));
		}
		for (UFINProperty* returnValue : returnValues) {
			Str.Appendf(TEXT("---@return %s %s %s\n"), *GetType(Ref, returnValue), *returnValue->GetInternalName(), *GetInlineDescription(returnValue->GetDescription().ToString()));
		}
		Str.Appendf(TEXT("function %s:%s(%s) end\n"), *Parent, *Func->GetInternalName(), *ParamList);

		// TODO: Add support for Operator Overloading
	}

	void WriteStruct(FStringBuilderBase& Str, FFINReflection& Ref, UFINStruct* Struct) {
		WriteMultiLineDescription(Str,  Struct->GetDescription().ToString());

		{
			FString ClassDeclaration = Struct->GetInternalName();
			if (UFINStruct* parent = Struct->GetParent()) {
				ClassDeclaration += TEXT(" : ") + Struct->GetParent()->GetInternalName();
			}
			Str.Appendf(TEXT("---@class %s\n"), *ClassDeclaration);

			for (UFINProperty* Prop : Struct->GetProperties(false)) {
				if ((Prop->GetPropertyFlags() & FIN_Prop_Attrib) && !(Prop->GetPropertyFlags() & FIN_Prop_ClassProp)) {
					WriteProperty(Str, Ref, Prop);
				}
			}

			Str.Appendf(TEXT("%s = {}\n"), *Struct->GetInternalName());

			for (UFINFunction* Func : Struct->GetFunctions(false)) {
				if ((Func->GetFunctionFlags() & FIN_Func_MemberFunc) && !(Func->GetFunctionFlags() & FIN_Func_ClassFunc)) {
					WriteFunction(Str, Ref, Struct->GetInternalName(), Func);
				}
			}
		}

		{
			WriteMultiLineDescription(Str, Struct->GetDescription().ToString());

			FString ClassDeclaration = Struct->GetInternalName();
			if (UFINStruct* parent = Struct->GetParent()) {
				ClassDeclaration += TEXT("-Class : ") + Struct->GetParent()->GetInternalName() + TEXT("-Class");
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
					WriteFunction(Str, Ref, Struct->GetInternalName(), Func);
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
		FString parameterList;
		for (const auto& parameter : Function.Parameters) {
			Str.Appendf(TEXT("---@param %s %s %s\n"), *parameter.InternalName, *parameter.Type, *parameter.Description.ToString());
			if (parameterList.Len() > 0) {
				parameterList.Append(TEXT(", "));
			}
			parameterList.Append(parameter.InternalName);
		}
		for (const auto& returnValue : Function.ReturnValues) {
			Str.Appendf(TEXT("---@return %s %s %s\n"), *returnValue.Type, *returnValue.InternalName, *returnValue.Description.ToString());
		}
		Str.Appendf(TEXT("function %s(%s) end\n"), *Identifier, *parameterList);
	}

	void WriteLuaValue(FStringBuilderBase& Str, const TSharedPtr<FFINLuaModuleValue>& Value, const FString& Identifier) {
		auto typeID = Value->TypeID();
		if (typeID == FINTypeId<FFINLuaFunction>::ID()) {
			WriteLuaFunction(Str, *StaticCastSharedPtr<FFINLuaFunction>(Value), Identifier);
		} else if (typeID == FINTypeId<FFINLuaTable>::ID()) {
			WriteLuaTable(Str, *StaticCastSharedPtr<FFINLuaTable>(Value), Identifier);
		} else {
			Str.Appendf(TEXT("%s = nil\n"), *Identifier);
		}
		Str.Append(TEXT("\n"));
	}

	void WriteMetatable(FStringBuilderBase& Str, const FFINLuaMetatable& Metatable) {
		WriteMultiLineDescription(Str, Metatable.Description.ToString());

		Str.Appendf(TEXT("---@class %s\n"), *Metatable.InternalName);

		const FFINLuaTableField* index = nullptr;
		for (const FFINLuaTableField& field : Metatable.Table->Fields) {
			if (field.Key == TEXT("__index") && field.Value->TypeID() == FINTypeId<FFINLuaTable>::ID()) {
				index = &field;
			}
			if (field.Key.StartsWith(TEXT("__"))) {
				// TODO: Add operator definition
			}
			// TODO: Add overloads for call operator
		}

		Str.Appendf(TEXT("%s = {}\n\n"), *Metatable.InternalName);

		if (index) {
			const FFINLuaTable& table = *StaticCastSharedPtr<FFINLuaTable>(index->Value);
			for (const FFINLuaTableField& field : table.Fields) {
				WriteMultiLineDescription(Str, field.Description.ToString());
				WriteLuaValue(Str, field.Value, Metatable.InternalName + TEXT(".") + field.Key);
				Str.Append(TEXT("\n"));
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

	bool FINGenLuaDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
		if (FParse::Command(&Command, TEXT("FINGenLuaDoc"))) {
			UE_LOG(LogFicsItNetworksLua, Display, TEXT("Generating FicsIt-Networks Lua Documentation..."));

			FStringBuilderBase str;

			str.Append(TEXT("---@meta\n\n"));

			FFINReflection& reflection = *FFINReflection::Get();
			for (TPair<UClass*, UFINClass*> Class : reflection.GetClasses()) {
				WriteStruct(str, reflection, Class.Value);
			}
			for (TPair<UScriptStruct*, UFINStruct*> Struct : reflection.GetStructs()) {
				WriteStruct(str, reflection, Struct.Value);
			}

			auto& moduleRegistry = FFINLuaModuleRegistry::GetInstance();
			for (const TSharedRef<FFINLuaModule>& module : moduleRegistry.Modules) {
				WriteModule(str, module);
			}

			UE_LOG(LogFicsItNetworksLua, Display, TEXT("FicsIt-Networks Lua Documentation generated!"));

			FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
			Path = FPaths::Combine(Path, TEXT("FINLuaDocumentation.lua"));

			UE_LOGFMT(LogFicsItNetworksLua, Display, "Saving FicsIt-Networks Lua Documentation under: {Path}", Path);

			FFileHelper::SaveStringToFile(str, *Path);

			UE_LOGFMT(LogFicsItNetworksLua, Display, "FicsIt-Networks Lua Documentation Saved!");

			return true;
		}
		return false;
	}

	static FStaticSelfRegisteringExec FINGenLuaDocStaticExec(&FINGenLuaDoc);
}
UE_ENABLE_OPTIMIZATION_SHIP

