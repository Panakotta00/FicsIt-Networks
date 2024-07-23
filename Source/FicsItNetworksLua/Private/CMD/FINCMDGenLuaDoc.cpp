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
			Str.Append(TEXT("--- ") + Line + TEXT("\n"));
		}
		Str.Append(TEXT("--- ") + Description + TEXT("\n"));
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

		FString ClassDeclaration = Struct->GetInternalName();
		if (UFINStruct* parent = Struct->GetParent()) {
			ClassDeclaration += TEXT(" : ") + Struct->GetParent()->GetInternalName();
		}
		Str.Appendf(TEXT("---@class %s\n"), ClassDeclaration);

		for (UFINProperty* Prop : Struct->GetProperties(false)) {
			if (Prop->GetPropertyFlags() & FIN_Prop_Attrib)	WriteProperty(Str, Ref, Prop);
		}

		Str.Appendf(TEXT("local %s\n"), *Struct->GetInternalName());

		for (UFINFunction* Func : Struct->GetFunctions(false)) {
			if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) WriteFunction(Str, Ref, Struct->GetInternalName(), Func);
		}

		Str.Append(TEXT("\n"));
	}

	bool FINGenLuaDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
		if (FParse::Command(&Command, TEXT("FINGenLuaDoc"))) {
			UE_LOG(LogFicsItNetworks, Display, TEXT("Generating FicsIt-Networks Lua Documentation..."));

			FStringBuilderBase str;

			str.Append(TEXT("@meta\n\n"));

			FFINReflection& reflection = *FFINReflection::Get();
			for (TPair<UClass*, UFINClass*> Class : reflection.GetClasses()) {
				WriteStruct(str, reflection, Class.Value);
			}
			for (TPair<UScriptStruct*, UFINStruct*> Struct : reflection.GetStructs()) {
				WriteStruct(str, reflection, Struct.Value);
			}

			UE_LOG(LogFicsItNetworks, Display, TEXT("FicsIt-Networks Lua Documentation generated!"));

			FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
			Path = FPaths::Combine(Path, TEXT("FINLuaDocumentation.lua"));

			UE_LOGFMT(LogFicsItNetworks, Display, "Saving FicsIt-Networks Lua Documentation under: {Path}", Path);

			FFileHelper::SaveStringToFile(str, *Path);

			UE_LOGFMT(LogFicsItNetworks, Display, "FicsIt-Networks Lua Documentation Saved!");

			return true;
		}
		return false;
	}

	static FStaticSelfRegisteringExec FINGenLuaDocStaticExec(&FINGenLuaDoc);
}
UE_ENABLE_OPTIMIZATION_SHIP

