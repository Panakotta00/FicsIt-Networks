#include "FINGenReflectionDoc.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "FicsItNetworks/Reflection/FINReflection.h"
#include "Framework/Application/SlateApplication.h"
#include "Serialization/JsonSerializerMacros.h"

#define LOCTEXT_NAMESPACE "FicsIt-Networks"
#define stringify( name ) # name

void FINGenRefBase(TSharedPtr<FJsonObject> Obj, UFINBase* Base) {
	Obj->SetStringField(TEXT("internalName"), Base->GetInternalName());
	Obj->SetStringField(TEXT("displayName"), Base->GetDisplayName().ToString());
	Obj->SetStringField(TEXT("description"), Base->GetDescription().ToString());
}

void FINGenRefProp(TSharedPtr<FJsonObject> Obj, UFINProperty* Prop) {
	FINGenRefBase(Obj, Prop);
	UEnum* ValTypeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT(stringify(EFINNetworkValueType)), true);
	Obj->SetStringField(TEXT("type"), ValTypeEnum->GetNameByIndex(Prop->GetType()).ToString());
	EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();
	Obj->SetBoolField(TEXT("Flag_Attrib"), (bool) (Flags & FIN_Prop_Attrib));
	Obj->SetBoolField(TEXT("Flag_ReadOnly"), (bool) (Flags & FIN_Prop_ReadOnly));
	Obj->SetBoolField(TEXT("Flag_Param"), (bool) (Flags & FIN_Prop_Param));
	Obj->SetBoolField(TEXT("Flag_OutParam"), (bool) (Flags & FIN_Prop_OutParam));
	Obj->SetBoolField(TEXT("Flag_RetVal"), (bool) (Flags & FIN_Prop_RetVal));
	Obj->SetBoolField(TEXT("Flag_RT_Sync"), (bool) (Flags & FIN_Prop_RT_Sync));
	Obj->SetBoolField(TEXT("Flag_RT_Parallel"), (bool) (Flags & FIN_Prop_RT_Parallel));
	Obj->SetBoolField(TEXT("Flag_RT_Async"), (bool) (Flags & FIN_Prop_RT_Async));
	Obj->SetBoolField(TEXT("Flag_ClassProp"), (bool) (Flags & FIN_Prop_ClassProp));
}

void FINGenRefFunc(TSharedPtr<FJsonObject> Obj, UFINFunction* Func) {
	FINGenRefBase(Obj, Func);
	TArray<TSharedPtr<FJsonValue>> Parameters;
	for (UFINProperty* Parameter : Func->GetParameters()) {
		TSharedPtr<FJsonObject> ParamterObj = MakeShared<FJsonObject>();
		FINGenRefProp(ParamterObj, Parameter);
		Parameters.Add(MakeShared<FJsonValueObject>(ParamterObj));
	}
	Obj->SetArrayField(TEXT("parameters"), Parameters);
	EFINFunctionFlags Flags = Func->GetFunctionFlags();
	Obj->SetBoolField(TEXT("Flag_VarArgs"), (bool) (Flags & FIN_Func_VarArgs));
	Obj->SetBoolField(TEXT("Flag_RT_Sync"), (bool) (Flags & FIN_Func_RT_Sync));
	Obj->SetBoolField(TEXT("Flag_RT_Parallel"), (bool) (Flags & FIN_Func_RT_Parallel));
	Obj->SetBoolField(TEXT("Flag_RT_Async"), (bool) (Flags & FIN_Func_RT_Async));
	Obj->SetBoolField(TEXT("Flag_MemberFunc"), (bool) (Flags & FIN_Func_MemberFunc));
	Obj->SetBoolField(TEXT("Flag_ClassFunc"), (bool) (Flags & FIN_Func_ClassFunc));
	Obj->SetBoolField(TEXT("Flag_StaticFunc"), (bool) (Flags & FIN_Func_StaticFunc));
	Obj->SetBoolField(TEXT("Flag_VarRets"), (bool) (Flags & FIN_Func_VarRets));
}

void FINGenRefStruct(TSharedPtr<FJsonObject> Obj, UFINStruct* Struct) {
	FINGenRefBase(Obj, Struct);
	if (Struct->GetParent()) Obj->SetStringField(TEXT("parent"), Struct->GetParent()->GetInternalName());
	TArray<TSharedPtr<FJsonValue>> Properties;
	for (UFINProperty* Property : Struct->GetProperties(false)) {
		TSharedPtr<FJsonObject> PropertyObj = MakeShared<FJsonObject>();
		FINGenRefProp(PropertyObj, Property);
		Properties.Add(MakeShared<FJsonValueObject>(PropertyObj));
	}
	Obj->SetArrayField(TEXT("properties"), Properties);
	TArray<TSharedPtr<FJsonValue>> Functions;
	for (UFINFunction* Function : Struct->GetFunctions(false)) {
		TSharedPtr<FJsonObject> FunctionObj = MakeShared<FJsonObject>();
		FINGenRefFunc(FunctionObj, Function);
		Functions.Add(MakeShared<FJsonValueObject>(FunctionObj));
	}
	Obj->SetArrayField(TEXT("functions"), Functions);
}

void FINGenRefSignal(TSharedPtr<FJsonObject> Obj, UFINSignal* Signal) {
	FINGenRefBase(Obj, Signal);
	TArray<TSharedPtr<FJsonValue>> Parameters;
	for (UFINProperty* Parameter : Signal->GetParameters()) {
		TSharedPtr<FJsonObject> ParamterObj = MakeShared<FJsonObject>();
		FINGenRefProp(ParamterObj, Parameter);
		Parameters.Add(MakeShared<FJsonValueObject>(ParamterObj));
	}
	Obj->SetArrayField(TEXT("parameters"), Parameters);
	Obj->SetBoolField(TEXT("isVarArgs"), Signal->IsVarArgs());
}

void FINGenRefClass(TSharedPtr<FJsonObject> Obj, UFINClass* Class) {
	FINGenRefStruct(Obj, Class);
	TArray<TSharedPtr<FJsonValue>> Signals;
	for (UFINSignal* Signal : Class->GetSignals(false)) {
		TSharedPtr<FJsonObject> SignalObj = MakeShared<FJsonObject>();
		FINGenRefSignal(SignalObj, Signal);
		Signals.Add(MakeShared<FJsonValueObject>(SignalObj));
	}
	Obj->SetArrayField(TEXT("signals"), Signals);
}

bool FINGenReflectionDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
	if (FParse::Command(&Command, TEXT("FINGenRefDoc"))) {
		FFINReflection& Ref = *FFINReflection::Get();

		TArray<TSharedPtr<FJsonValue>> Classes;
		TArray<TSharedPtr<FJsonValue>> Structs;

		for (TPair<UClass*, UFINClass*> Class : Ref.GetClasses()) {
			TSharedPtr<FJsonObject> ClassObj = MakeShared<FJsonObject>();
			FINGenRefClass(ClassObj, Class.Value);
			Classes.Add(MakeShared<FJsonValueObject>(ClassObj));
		}
		for (TPair<UScriptStruct*, UFINStruct*> Struct : Ref.GetStructs()) {
			TSharedPtr<FJsonObject> StructObj = MakeShared<FJsonObject>();
			FINGenRefStruct(StructObj, Struct.Value);
			Structs.Add(MakeShared<FJsonValueObject>(StructObj));
		}

		TSharedRef<FJsonObject> MainObj = MakeShared<FJsonObject>();
		MainObj->SetArrayField(TEXT("classes"), Classes);
		MainObj->SetArrayField(TEXT("structs"), Structs);
		
		FString JsonString;
		TSharedRef<TJsonWriter<>> Json = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(MainObj, Json);

		FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
		Path = FPaths::Combine(Path, TEXT("FINReflectionDocumentation.json"));
		FFileHelper::SaveStringToFile(JsonString, *Path);
		return true;
	}
	return false;
}

static FStaticSelfRegisteringExec FINGenReflectionDocStaticExec(&FINGenReflectionDoc);
