#include "FINGenLuaDoc.h"

#include "Developer/Profiler/Public/ProfilerCommon.h"
#include "FicsItNetworks/FicsItKernel/Processor/Lua/LuaInstance.h"
#include "FicsItNetworks/Reflection/FINArrayProperty.h"
#include "FicsItNetworks/Reflection/FINClassProperty.h"
#include "FicsItNetworks/Reflection/FINObjectProperty.h"
#include "FicsItNetworks/Reflection/FINReflection.h"
#include "FicsItNetworks/Reflection/FINTraceProperty.h"
#pragma optimize("", off)
FString FINGenLuaGetType(FFINReflection& Ref, UFINProperty* Prop) {
	if (!Prop) return "any";
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
		UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Prop);
		UFINClass* Class = Ref.FindClass(ObjProp->GetSubclass());
		if (!Class) return "Object";
		return Class->GetInternalName();
	} case FIN_TRACE: {
		UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Prop);
		UFINClass* Class = Ref.FindClass(TraceProp->GetSubclass());
		if (!Class) return "Object";
		return Class->GetInternalName();
	} case FIN_CLASS: {
		UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Prop);
		UFINClass* Class = Ref.FindClass(ClassProp->GetSubclass());
		if (!Class) return "Object" CLASS_INSTANCE_META_SUFFIX;
		return Class->GetInternalName() + CLASS_INSTANCE_META_SUFFIX;
	} case FIN_STRUCT: {
		UFINStructProperty* StructProp = Cast<UFINStructProperty>(Prop);
		UFINStruct* Struct = Ref.FindStruct(StructProp->GetInner());
		if (!Struct) return "any";
		return Struct->GetInternalName();
	} case FIN_ARRAY: {
		UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Prop);
		return FINGenLuaGetType(Ref, ArrayProp->GetInnerType()) + "[]";
	} default:
		return "any";
	}
}

FString FINGenLuaDescription(FString Description) {
	Description.ReplaceCharInline('\n', ' ');
	Description.ReplaceCharInline('\r', ' ');
	return Description;
}

void FINGenLuaDescription(FString& Documentation, FString Description) {
	Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

	FString Line;
	while (Description.Split(TEXT("\n"), &Line, &Description)) {
		Documentation.Append(TEXT("--- ") + Line + TEXT("\n"));
	}
	Documentation.Append(TEXT("--- ") + Description + TEXT("\n"));
}

void FINGenLuaProperty(FString& Documentation, FFINReflection& Ref, UFINProperty* Prop) {
	Documentation.Append(FString::Printf(TEXT("---@field public %s %s @%s\n"), *Prop->GetInternalName(), *FINGenLuaGetType(Ref, Prop), *FINGenLuaDescription(Prop->GetDescription().ToString())));
}

void FINGenLuaFunction(FString& Documentation, FFINReflection& Ref, FString Parent, UFINFunction* Func) {
	FINGenLuaDescription(Documentation, Func->GetDescription().ToString());
	FString ReturnValues;
	FString ParamList;
	for (UFINProperty* Prop : Func->GetParameters()) {
		EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();
		if (!(Flags & FIN_Prop_Param)) continue;
		if (Flags & FIN_Prop_OutParam) {
			if (ReturnValues.Len() > 0) ReturnValues.Append(",");
			ReturnValues.Append(*FINGenLuaGetType(Ref, Prop));
		} else {
			Documentation.Append(FString::Printf(TEXT("---@param %s %s @%s\n"), *Prop->GetInternalName(), *FINGenLuaGetType(Ref, Prop), *FINGenLuaDescription(Prop->GetDescription().ToString())));
			if (ParamList.Len() > 0) ParamList.Append(", "); 
			ParamList.Append(Prop->GetInternalName());
		}
	}
	if (ReturnValues.Len() > 0) Documentation.Append(FString::Printf(TEXT("---@return %s\n"), *ReturnValues));
	Documentation.Append(FString::Printf(TEXT("function %s:%s(%s) end\n"), *Parent, *Func->GetInternalName(), *ParamList));
}

void FINGenLuaClass(FString& Documentation, FFINReflection& Ref, UFINClass* Class) {
	FINGenLuaDescription(Documentation,  Class->GetDescription().ToString());
	Documentation.Append(FString::Printf(TEXT("---@class %s%s\n"), *Class->GetInternalName(), Class->GetParent() ? *(TEXT(":") + Class->GetParent()->GetInternalName()) : TEXT("")));
	for (UFINProperty* Prop : Class->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib)	FINGenLuaProperty(Documentation, Ref, Prop);
	}
	Documentation.Append(FString::Printf(TEXT("local %s\n"), *Class->GetInternalName()));
	for (UFINFunction* Func : Class->GetFunctions(false)) {
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) FINGenLuaFunction(Documentation, Ref, Class->GetInternalName(), Func);
	}
	Documentation.Append(TEXT("\n"));
}

void FINGenLuaStruct(FString& Documentation, FFINReflection& Ref, UFINStruct* Struct) {
	FINGenLuaDescription(Documentation, Struct->GetDescription().ToString());
	Documentation.Append(FString::Printf(TEXT("---@class %s\n"), *Struct->GetInternalName()));
	for (UFINProperty* Prop : Struct->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib)	FINGenLuaProperty(Documentation, Ref, Prop);
	}
	Documentation.Append(FString::Printf(TEXT("local %s\n"), *Struct->GetInternalName()));
	for (UFINFunction* Func : Struct->GetFunctions(false)) {
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) FINGenLuaFunction(Documentation, Ref, Struct->GetInternalName(), Func);
	}
	Documentation.Append(TEXT("\n"));
}

bool FINGenLuaDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
	if (FParse::Command(&Command, TEXT("FINGenLuaDoc"))) {
		FString Documentation;

		FFINReflection& Ref = *FFINReflection::Get();

		for (TPair<UClass*, UFINClass*> Class : Ref.GetClasses()) {
			FINGenLuaClass(Documentation, Ref, Class.Value);
		}
		for (TPair<UScriptStruct*, UFINStruct*> Struct : Ref.GetStructs()) {
			FINGenLuaStruct(Documentation, Ref, Struct.Value);
		}
		
		FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
		Path = FPaths::Combine(Path, TEXT("FINLuaDocumentation.lua"));
		FFileHelper::SaveStringToFile(Documentation, *Path);

		return true;
	}
	return false;
}
#pragma optimize("", on)

static FStaticSelfRegisteringExec FINGenLuaDocStaticExec(&FINGenLuaDoc);
