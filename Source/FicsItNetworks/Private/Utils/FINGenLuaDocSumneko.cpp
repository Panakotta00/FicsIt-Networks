#include "Utils/FINGenLuaDocSumneko.h"

#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "UObject/Package.h"

#pragma optimize("", off)
FString FINGenLuaGetTypeNameSumneko(const UFINBase *Base) {
	TArray<FString> SplitBasePackageName;
	Base->GetPackage()->GetName().ParseIntoArray(SplitBasePackageName, TEXT("/"), true);

	FString BasePackageName;
	for (auto BasePackageNamePart : SplitBasePackageName) {

		// get rid of unnecessary names
		if (BasePackageNamePart.Equals(TEXT("Script"))
			|| BasePackageNamePart.Equals(TEXT("Game"))
			|| BasePackageNamePart.Equals(TEXT("Engine"))
			|| BasePackageNamePart.Equals(TEXT("CoreUObject"))) {
			continue;
		}

		// making it easier and smaller to use types
		if (BasePackageNamePart.Equals(TEXT("FactoryGame"))) {
			BasePackageName += TEXT("Satis.");
			continue;
		} else if (BasePackageNamePart.Equals(TEXT("FicsItNetworks"))) {
			BasePackageName += TEXT("FIN.");
			continue;
		}

		BasePackageName += BasePackageNamePart + TEXT(".");
	}

	return BasePackageName + Base->GetInternalName();
}

FString FINGenLuaGetTypeSumneko(FFINReflection& Ref, const UFINProperty* Prop) {
	if (!Prop) {
		return "any";
	}
	
	switch (Prop->GetType()) {
		case FIN_NIL:
			return "nil";
		case FIN_BOOL:
			return "boolean";
		case FIN_INT:
		case FIN_FLOAT:
			return "number";
		case FIN_STR:
			return "string";
		case FIN_OBJ: {
			const UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Prop);
			const UFINClass* Class = Ref.FindClass(ObjProp->GetSubclass());
			if (!Class) return "Object";
			return FINGenLuaGetTypeNameSumneko(Class);
		}
		case FIN_TRACE: {
			const UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Prop);
			const UFINClass* Class = Ref.FindClass(TraceProp->GetSubclass());
			if (!Class) return "Object";
			return FINGenLuaGetTypeNameSumneko(Class);
		}
		case FIN_CLASS: {
			const UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Prop);
			const UFINClass* Class = Ref.FindClass(ClassProp->GetSubclass());
			if (!Class) return "Object";
			return FINGenLuaGetTypeNameSumneko(Class);
		}
		case FIN_STRUCT: {
			const UFINStructProperty* StructProp = Cast<UFINStructProperty>(Prop);
			const UFINStruct* Struct = Ref.FindStruct(StructProp->GetInner());
			if (!Struct) return "any";
			return FINGenLuaGetTypeNameSumneko(Struct);
		}
		case FIN_ARRAY: {
			const UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Prop);
			return FINGenLuaGetTypeSumneko(Ref, ArrayProp->GetInnerType()) + "[]";
		}
		default:
			return "any";
	}
}

FString FormatDescription(FString Description) {
	Description.ReplaceCharInline('\n', ' ');
	Description.ReplaceCharInline('\r', ' ');
	return Description;
}

void FINGenLuaDescriptionSumneko(FString& Documentation, FString Description) {
	Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

	FString Line;
	while (Description.Split(TEXT("\n"), &Line, &Description)) {
		Documentation.Append(TEXT("--- ") + Line + TEXT("<br>\n"));
	}
	Documentation.Append(TEXT("--- ") + Description + TEXT("\n"));
}

void FINGenLuaPropertySumneko(FString& Documentation, FFINReflection& Ref, const FString& Parent, const UFINProperty* Prop) {
	Documentation.Append(TEXT("\n"));
	
	const EFINRepPropertyFlags PropFlags = Prop->GetPropertyFlags();
	FINGenLuaDescriptionSumneko(Documentation, TEXT("### Flags:"));
	
	if (PropFlags & FIN_Prop_RT_Sync) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
	}
	
	if (PropFlags & FIN_Prop_RT_Parallel) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
	}
	
	if (PropFlags & FIN_Prop_RT_Async) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Runtime Asynchronous - Can be changed anytime."));
	}
	
	if (PropFlags & FIN_Prop_ReadOnly) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Read Only - The value of this property can not be changed by code."));
	}

	Documentation.Append(FString::Printf(TEXT("---@type %s\n"), *FINGenLuaGetTypeSumneko(Ref, Prop)));
	Documentation.Append(FString::Printf(TEXT("%s.%s = nil\n"), *Parent, *Prop->GetInternalName()));
}

void FINGenLuaFunctionSumneko(FString& Documentation, FFINReflection& Ref, const FString& Parent, const UFINFunction* Func) {
	Documentation.Append(TEXT("\n"));
	
	FINGenLuaDescriptionSumneko(Documentation, Func->GetDescription().ToString());

	const EFINFunctionFlags funcFlags = Func->GetFunctionFlags();
	FINGenLuaDescriptionSumneko(Documentation, TEXT("### Flags:"));
	
	if (funcFlags & FIN_Func_RT_Sync) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
	}
	
	if (funcFlags & FIN_Func_RT_Parallel) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
	}
	
	if (funcFlags & FIN_Func_RT_Async) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Runtime Asynchronous - Can be changed anytime."));
	}
	
	if (funcFlags & FIN_Func_VarArgs) {
		FINGenLuaDescriptionSumneko(Documentation, TEXT("* Variable Arguments - Can have any additional arguments as described."));
	}

	FString ReturnValues;
	FString ParamList;
	for (const UFINProperty* Prop : Func->GetParameters()) {
		const EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();

		if (!(Flags & FIN_Prop_Param)) {
			continue;
		}
		
		if (Flags & FIN_Prop_OutParam) {
			if (ReturnValues.Len() > 0) {
				ReturnValues.Append(",");
			}
			
			ReturnValues.Append(*FINGenLuaGetTypeSumneko(Ref, Prop));
		}
		else {
			Documentation.Append(FString::Printf(
				TEXT("---@param %s %s @%s\n"),
				*Prop->GetInternalName(),
				*FINGenLuaGetTypeSumneko(Ref, Prop),
				*FormatDescription(Prop->GetDescription().ToString())
			));
			
			if (ParamList.Len() > 0) {
				ParamList.Append(", ");
			}
			
			ParamList.Append(Prop->GetInternalName());
		}
	}
	if (ReturnValues.Len() > 0) Documentation.Append(FString::Printf(TEXT("---@return %s\n"), *ReturnValues));
	Documentation.Append(FString::Printf(TEXT("function %s:%s(%s) end\n"), *Parent, *Func->GetInternalName(), *ParamList));
}

void FINGenLuaSignalSumneko(FString& Documentation, FFINReflection& Ref, const FString& Parent, const UFINSignal* Signal) {
	Documentation.Append(TEXT("\n"));
	
	FINGenLuaDescriptionSumneko(Documentation, Signal->GetDescription().ToString() + TEXT("\n"));

	FINGenLuaDescriptionSumneko(Documentation, TEXT("### returns from event.pull:\n```"));

	Documentation.Append(TEXT("--- local signalName, component"));
	for (const UFINProperty* Prop : Signal->GetParameters()) {
		Documentation.Append(TEXT(", ") + Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")));
	}
	if (Signal->IsVarArgs()) {
		Documentation.Append(TEXT(", ..."));
	}
	Documentation.Append(TEXT(" = event.pull()\n--- ```\n"));

	FINGenLuaDescriptionSumneko(Documentation, FString::Printf(TEXT("- `signalName: \"%s\"`"), *Signal->GetInternalName()));
	FINGenLuaDescriptionSumneko(Documentation, FString::Printf(TEXT("- `component: %s`"), *Parent));
	
	for (const UFINProperty* Prop : Signal->GetParameters()) {
		FINGenLuaDescriptionSumneko(Documentation, FString::Printf(
			TEXT("- `%s: %s` \n%s"),
			*Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")),
			*FINGenLuaGetTypeSumneko(Ref, Prop),
			*Prop->GetDescription().ToString()
		));
	}

	// hard coding the type is maybe not the best choice
	Documentation.Append(TEXT("---@deprecated\n---@type FIN.Signal\n")); //TODO: keep up to date
	Documentation.Append(FString::Printf(
		TEXT("%s.%s = { isVarArgs = %s }\n"),
		*Parent,
		*Signal->GetInternalName(),
		Signal->IsVarArgs() ? TEXT("true") : TEXT("false")
	));
}

void FINGenLuaClassSumneko(FString& Documentation, FFINReflection& Ref, const UFINClass* Class) {
	Documentation.Append(TEXT("\n"));
	
	FINGenLuaDescriptionSumneko(Documentation, Class->GetDescription().ToString());
	Documentation.Append(FString::Printf(
		TEXT("---@class %s%s\nlocal %s\n"),
		*FINGenLuaGetTypeNameSumneko(Class),
		Class->GetParent() ? *(TEXT(" : ") + FINGenLuaGetTypeNameSumneko(Class->GetParent())) : TEXT(""),
		*Class->GetInternalName()
	));
	
	for (const UFINProperty* Prop : Class->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib) {
			FINGenLuaPropertySumneko(Documentation, Ref, Class->GetInternalName(), Prop);
		}
	}
	
	for (const UFINFunction* Func : Class->GetFunctions(false)) {
		//TODO: filter FIN_Operators
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) {
			FINGenLuaFunctionSumneko(Documentation, Ref, Class->GetInternalName(), Func);
		}
	}

	for (const UFINSignal* Signal : Class->GetSignals(false)) {
		FINGenLuaSignalSumneko(Documentation, Ref, Class->GetInternalName(), Signal);
	}
}

void FINGenLuaStructSumneko(FString& Documentation, FFINReflection& Ref, const UFINStruct* Struct) {
	Documentation.Append(TEXT("\n"));

	FINGenLuaDescriptionSumneko(Documentation, Struct->GetDescription().ToString());
	Documentation.Append(FString::Printf(
		TEXT("---@class %s\nlocal %s\n"),
		*FINGenLuaGetTypeNameSumneko(Struct),
		*Struct->GetInternalName()
	));
	
	for (const UFINProperty* Prop : Struct->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib) {
			FINGenLuaPropertySumneko(Documentation, Ref, Struct->GetInternalName(), Prop);
		}
	}
	
	for (const UFINFunction* Func : Struct->GetFunctions(false)) {
		//TODO: filter FIN_Operators
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) {
			FINGenLuaFunctionSumneko(Documentation, Ref, Struct->GetInternalName(), Func);
		}
	}
}

bool FINGenLuaDocSumneko(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
	if (FParse::Command(&Command, TEXT("FINGenLuaDocSumneko"))) {
		FString Documentation;

		Documentation.Append(TEXT("---@meta\n---@diagnostic disable\n\n"));
		
		FFINReflection& Ref = *FFINReflection::Get();

		for (TPair<UClass*, UFINClass*> const Class : Ref.GetClasses()) {
			FINGenLuaClassSumneko(Documentation, Ref, Class.Value);
		}
		for (TPair<UScriptStruct*, UFINStruct*> const Struct : Ref.GetStructs()) {
			FINGenLuaStructSumneko(Documentation, Ref, Struct.Value);
		}

		FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
		Path = FPaths::Combine(Path, TEXT("FINLuaDocumentationSumneko.lua"));
		FFileHelper::SaveStringToFile(Documentation, *Path);

		return true;
	}
	return false;
}
#pragma optimize("", on)

// ReSharper disable once CppDeclaratorNeverUsed
static FStaticSelfRegisteringExec FINGenLuaDocSumnekoStaticExec(&FINGenLuaDocSumneko);
