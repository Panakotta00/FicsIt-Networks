#include "Utils/FINGenReflectionDoc.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINBoolProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINFloatProperty.h"
#include "Reflection/FINIntProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStrProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "Reflection/FINStructProperty.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Logging/StructuredLog.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#define LOCTEXT_NAMESPACE "FicsIt-Networks"
#define stringify( name ) # name

TSharedPtr<FJsonObject> FINGenDataType(UFINProperty* Property, TSharedPtr<FJsonObject> Obj) {
	if (Property->IsA<UFINBoolProperty>()) {
		Obj->SetStringField("type", "Bool");
	} else if (Property->IsA<UFINIntProperty>()) {
		Obj->SetStringField("type", "Int");
	} else if (Property->IsA<UFINFloatProperty>()) {
		Obj->SetStringField("type", "Float");
	} else if (Property->IsA<UFINStrProperty>()) {
		Obj->SetStringField("type", "String");
	} else if (UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Property)) {
		Obj->SetStringField("type", "Object");
		UFINClass* subclass = FFINReflection::Get()->FindClass(ObjProp->GetSubclass());
		if (subclass) Obj->SetStringField("subclass", subclass->GetInternalName());
	} else if (UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Property)) {
		Obj->SetStringField("type", "Trace");
		UFINClass* subclass = FFINReflection::Get()->FindClass(TraceProp->GetSubclass());
		if (subclass) Obj->SetStringField("subclass", subclass->GetInternalName());
	} else if (UFINStructProperty* StructProp = Cast<UFINStructProperty>(Property)) {
		Obj->SetStringField("type", "Struct");
		UFINStruct* inner = FFINReflection::Get()->FindStruct(StructProp->GetInner());
		if (inner) Obj->SetStringField("inner", inner->GetInternalName());
	} else if (UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Property)) {
		Obj->SetStringField("type", "Class");
		UFINClass* subclass = FFINReflection::Get()->FindClass(ClassProp->GetSubclass());
		if (subclass) Obj->SetStringField("subclass", subclass->GetInternalName());
	} else if (UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Property)) {
		Obj->SetStringField("type", "Array");
		if (ArrayProp->GetInnerType()) Obj->SetObjectField("inner", FINGenDataType(ArrayProp->GetInnerType()));
	}
	return Obj;
}

void FINGenRefBase(TSharedPtr<FJsonObject> Obj, UFINBase* Base) {
	Obj->SetStringField(TEXT("internalName"), Base->GetInternalName());
	Obj->SetStringField(TEXT("displayName"), Base->GetDisplayName().ToString());
	Obj->SetStringField(TEXT("description"), Base->GetDescription().ToString());
}

void FINGenRefProp(TSharedPtr<FJsonObject> Obj, UFINProperty* Prop) {
	FINGenRefBase(Obj, Prop);
	Obj->SetObjectField(TEXT("type"), FINGenDataType(Prop));
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

void FINGenClasses(TArray<TSharedPtr<FJsonValue>>& Array, UFINClass* Class) {
	TSharedPtr<FJsonObject> ClassObj = MakeShared<FJsonObject>();
	FINGenRefClass(ClassObj, Class);
	Array.Add(MakeShared<FJsonValueObject>(ClassObj));

	TArray<FString> Names;
	TMap<FString, UFINClass*> NamesToClasses;
	for (UFINClass* ChildClass : Class->GetChildClasses()) {
		Names.Add(ChildClass->GetInternalName());
		NamesToClasses.Add(ChildClass->GetInternalName(), ChildClass);
	}
	Names.Sort();
	for (FString Name : Names) {
		FINGenClasses(Array, NamesToClasses[Name]);
	}
}

bool FINGenReflectionDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
	if (FParse::Command(&Command, TEXT("FINGenRefDoc"))) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Generating FicsIt-Networks Reflection Documentation..."));
		FFINReflection& Ref = *FFINReflection::Get();

		TArray<TSharedPtr<FJsonValue>> Classes;
		TArray<TSharedPtr<FJsonValue>> Structs;

		FINGenClasses(Classes, Ref.FindClass(UObject::StaticClass()));

		TArray<FString> Names;
		TMap<FString, UFINStruct*> NamesToStructs;
		for (TPair<UScriptStruct*, UFINStruct*> Struct : Ref.GetStructs()) {
			Names.Add(Struct.Value->GetInternalName());
			NamesToStructs.Add(Struct.Value->GetInternalName(), Struct.Value);
		}
		Names.Sort();
		for (FString Name : Names) {
			TSharedPtr<FJsonObject> StructObj = MakeShared<FJsonObject>();
			FINGenRefStruct(StructObj, NamesToStructs[Name]);
			Structs.Add(MakeShared<FJsonValueObject>(StructObj));
		}

		TSharedRef<FJsonObject> MainObj = MakeShared<FJsonObject>();
		MainObj->SetArrayField(TEXT("classes"), Classes);
		MainObj->SetArrayField(TEXT("structs"), Structs);
		
		FString JsonString;
		TSharedRef<TJsonWriter<>> Json = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(MainObj, Json);

		UE_LOG(LogFicsItNetworks, Display, TEXT("FicsIt-Networks Reflection Documentation generated!"));

		FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
		Path = FPaths::Combine(Path, TEXT("FINReflectionDocumentation.json"));

		UE_LOGFMT(LogFicsItNetworks, Display, "Saving FicsIt-Networks Reflection Documentation under: {Path}", Path);

		FFileHelper::SaveStringToFile(JsonString, *Path);

		UE_LOGFMT(LogFicsItNetworks, Display, "FicsIt-Networks Reflection Documentation Saved!");

		return true;
	}
	return false;
}

static FStaticSelfRegisteringExec FINGenReflectionDocStaticExec(&FINGenReflectionDoc);
