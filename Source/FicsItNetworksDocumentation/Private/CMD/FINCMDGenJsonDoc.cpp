#include "FicsItNetworksDocumentation.h"
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
#include "FINLua/FINLuaModule.h"
#include "Logging/StructuredLog.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Reflection/FINProperty.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UE_DISABLE_OPTIMIZATION
namespace FINGenJsonDoc {
	TSharedRef<FJsonObject> GenReflectionDataType(UFINProperty* Property, TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>()) {
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
			if (UFINClass* subclass = FFINReflection::Get()->FindClass(ObjProp->GetSubclass())) Obj->SetStringField("subclass", subclass->GetInternalName());
		} else if (UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Property)) {
			Obj->SetStringField("type", "Trace");
			if (UFINClass* subclass = FFINReflection::Get()->FindClass(TraceProp->GetSubclass())) Obj->SetStringField("subclass", subclass->GetInternalName());
		} else if (UFINStructProperty* StructProp = Cast<UFINStructProperty>(Property)) {
			Obj->SetStringField("type", "Struct");
			if (UFINStruct* inner = FFINReflection::Get()->FindStruct(StructProp->GetInner())) Obj->SetStringField("inner", inner->GetInternalName());
		} else if (UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Property)) {
			Obj->SetStringField("type", "Class");
			if (UFINClass* subclass = FFINReflection::Get()->FindClass(ClassProp->GetSubclass())) Obj->SetStringField("subclass", subclass->GetInternalName());
		} else if (UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Property)) {
			Obj->SetStringField("type", "Array");
			if (ArrayProp->GetInnerType()) Obj->SetObjectField("inner", GenReflectionDataType(ArrayProp->GetInnerType()));
		}
		return Obj;
	}

	void FINGenRefBase(const TSharedPtr<FJsonObject>& Obj, const UFINBase* Base) {
		Obj->SetStringField(TEXT("internalName"), Base->GetInternalName());
		Obj->SetStringField(TEXT("displayName"), Base->GetDisplayName().ToString());
		Obj->SetStringField(TEXT("description"), Base->GetDescription().ToString());
	}

	void FINGenRefProp(const TSharedPtr<FJsonObject>& Obj, UFINProperty* Prop) {
		FINGenRefBase(Obj, Prop);
		Obj->SetObjectField(TEXT("type"), GenReflectionDataType(Prop));

		EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();
		TArray<TSharedPtr<FJsonValue>> FlagArray;
		if (Flags & FIN_Prop_Attrib) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("Attrib")));
		if (Flags & FIN_Prop_ReadOnly) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("ReadOnly")));
		if (Flags & FIN_Prop_Param) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("Param")));
		if (Flags & FIN_Prop_OutParam) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("OutParam")));
		if (Flags & FIN_Prop_RetVal) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RetVal")));
		if (Flags & FIN_Prop_RT_Sync) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Sync")));
		if (Flags & FIN_Prop_RT_Parallel) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Parallel")));
		if (Flags & FIN_Prop_RT_Async) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Async")));
		if (Flags & FIN_Prop_ClassProp) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("ClassProp")));
		Obj->SetArrayField(TEXT("flags"), FlagArray);
	}

	void FINGenRefFunc(const TSharedPtr<FJsonObject>& Obj, const UFINFunction* Func) {
		FINGenRefBase(Obj, Func);
		TArray<TSharedPtr<FJsonValue>> parameters;
		for (UFINProperty* parameter : Func->GetParameters()) {
			TSharedPtr<FJsonObject> parameterObject = MakeShared<FJsonObject>();
			FINGenRefProp(parameterObject, parameter);
			parameters.Add(MakeShared<FJsonValueObject>(parameterObject));
		}
		Obj->SetArrayField(TEXT("parameters"), parameters);

		EFINFunctionFlags Flags = Func->GetFunctionFlags();
		TArray<TSharedPtr<FJsonValue>> FlagArray;
		if (Flags & FIN_Func_VarArgs) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("VarArgs")));
		if (Flags & FIN_Func_RT_Sync) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Sync")));
		if (Flags & FIN_Func_RT_Parallel) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Parallel")));
		if (Flags & FIN_Func_RT_Async) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Async")));
		if (Flags & FIN_Func_MemberFunc) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("MemberFunc")));
		if (Flags & FIN_Func_ClassFunc) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("ClassFunc")));
		if (Flags & FIN_Func_StaticFunc) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("StaticFunc")));
		if (Flags & FIN_Func_VarRets) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("VarRets")));
		Obj->SetArrayField(TEXT("flags"), FlagArray);
	}

	void FINGenRefStruct(const TSharedPtr<FJsonObject>& Obj, const UFINStruct* Struct) {
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

	void FINGenRefSignal(const TSharedPtr<FJsonObject>& Obj, const UFINSignal* Signal) {
		FINGenRefBase(Obj, Signal);
		TArray<TSharedPtr<FJsonValue>> parameters;
		for (UFINProperty* parameter : Signal->GetParameters()) {
			TSharedPtr<FJsonObject> parameterObject = MakeShared<FJsonObject>();
			FINGenRefProp(parameterObject, parameter);
			parameters.Add(MakeShared<FJsonValueObject>(parameterObject));
		}
		Obj->SetArrayField(TEXT("parameters"), parameters);
		Obj->SetBoolField(TEXT("isVarArgs"), Signal->IsVarArgs());
	}

	void FINGenRefClass(const TSharedPtr<FJsonObject>& Obj, const UFINClass* Class) {
		FINGenRefStruct(Obj, Class);
		TArray<TSharedPtr<FJsonValue>> Signals;
		for (UFINSignal* Signal : Class->GetSignals(false)) {
			TSharedPtr<FJsonObject> SignalObj = MakeShared<FJsonObject>();
			FINGenRefSignal(SignalObj, Signal);
			Signals.Add(MakeShared<FJsonValueObject>(SignalObj));
		}
		Obj->SetArrayField(TEXT("signals"), Signals);
	}

	void FINGenClasses(TArray<TSharedPtr<FJsonValue>>& Array, const UFINClass* Class) {
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

	TSharedPtr<FJsonObject> GenReflectionDoc() {
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

		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();
		doc->SetArrayField(TEXT("classes"), Classes);
		doc->SetArrayField(TEXT("structs"), Structs);

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaValue(const TSharedRef<FFINLuaModuleValue>& Value);

	TSharedRef<FJsonObject> GenLuaFunctionParameter(const FFINLuaFunctionParameter& Parameter) {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		doc->SetStringField(TEXT("displayName"), Parameter.DisplayName.ToString());
		doc->SetStringField(TEXT("description"), Parameter.Description.ToString());

		doc->SetStringField(TEXT("type"), Parameter.Type);

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaFunction(const FFINLuaFunction& Function) {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		doc->SetStringField(TEXT("valueType"), TEXT("function"));
		doc->SetStringField(TEXT("parameterSignature"), Function.ParameterSignature);
		doc->SetStringField(TEXT("returnValueSignature"), Function.ReturnValueSignature);

		TSharedRef<FJsonObject> parameters = MakeShared<FJsonObject>();
		for (const FFINLuaFunctionParameter& parameter : Function.Parameters) {
			parameters->SetObjectField(parameter.InternalName, GenLuaFunctionParameter(parameter));
		}
		doc->SetObjectField(TEXT("parameters"), parameters);

		TSharedRef<FJsonObject> returnValues = MakeShared<FJsonObject>();
		for (const FFINLuaFunctionParameter& returnValue : Function.ReturnValues) {
			returnValues->SetObjectField(returnValue.InternalName, GenLuaFunctionParameter(returnValue));
		}
		doc->SetObjectField(TEXT("returnValues"), returnValues);

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaTable(const FFINLuaTable& Table) {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		doc->SetStringField(TEXT("valueType"), TEXT("table"));

		TSharedRef<FJsonObject> fields = MakeShared<FJsonObject>();
		for (const FFINLuaTableField& field : Table.Fields) {
			if (!field.Value.IsValid()) continue;
			TSharedRef<FJsonObject> obj = GenLuaValue(field.Value.ToSharedRef());
			obj->SetStringField(TEXT("displayName"), field.DisplayName.ToString());
			obj->SetStringField(TEXT("description"), field.Description.ToString());
			fields->SetObjectField(field.Key, obj);
		}
		doc->SetObjectField(TEXT("fields"), fields);

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaBareValue(const FFINLuaModuleBareValue& BareValue) {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		doc->SetStringField(TEXT("valueType"), TEXT("bareValue"));
		doc->SetStringField(TEXT("luaType"), BareValue.Type);

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaValue(const TSharedRef<FFINLuaModuleValue>& Value) {
		auto typeID = Value->TypeID();
		if (typeID == FINTypeId<FFINLuaFunction>::ID()) {
			return GenLuaFunction(*StaticCastSharedRef<FFINLuaFunction>(Value));
		} else if (typeID == FINTypeId<FFINLuaTable>::ID()) {
			return GenLuaTable(*StaticCastSharedRef<FFINLuaTable>(Value));
		} else if (typeID == FINTypeId<FFINLuaModuleBareValue>::ID()) {
			return GenLuaBareValue(*StaticCastSharedRef<FFINLuaModuleBareValue>(Value));
		} else {
			return MakeShared<FJsonObject>();
		}
	}

	TSharedRef<FJsonObject> GenLuaMetatable(const FFINLuaMetatable& Metatable) {
		TSharedRef<FJsonObject> doc = GenLuaTable(*Metatable.Table);

		doc->SetStringField(TEXT("displayName"), Metatable.DisplayName.ToString());
		doc->SetStringField(TEXT("description"), Metatable.Description.ToString());

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaGlobal(const FFINLuaGlobal& Global) {
		TSharedRef<FJsonObject> doc = GenLuaValue(Global.Value.ToSharedRef());

		doc->SetStringField(TEXT("displayName"), Global.DisplayName.ToString());
		doc->SetStringField(TEXT("description"), Global.Description.ToString());

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaModule(const TSharedRef<FFINLuaModule>& Module) {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		doc->SetStringField(TEXT("description"), Module->Description.ToString());
		doc->SetStringField(TEXT("displayName"), Module->DisplayName.ToString());

		TArray<TSharedPtr<FJsonValue>> dependencies;
		for (const FString& dependency : Module->Dependencies) {
			dependencies.Add(MakeShared<FJsonValueString>(dependency));
		}
		doc->SetArrayField(TEXT("dependencies"), dependencies);

		TSharedRef<FJsonObject> metatables = MakeShared<FJsonObject>();
		for (const FFINLuaMetatable& metatable : Module->Metatables) {
			metatables->SetObjectField(metatable.InternalName, GenLuaMetatable(metatable));
		}
		doc->SetObjectField(TEXT("metatables"), metatables);

		TSharedRef<FJsonObject> globals = MakeShared<FJsonObject>();
		for (const FFINLuaGlobal& global : Module->Globals) {
			globals->SetObjectField(global.InternalName, GenLuaGlobal(global));
		}
		doc->SetObjectField(TEXT("globals"), globals);

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaDoc() {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		TSharedRef<FJsonObject> modules = MakeShared<FJsonObject>();
		const auto& moduleRegistry = FFINLuaModuleRegistry::GetInstance();
		for (const TSharedRef<FFINLuaModule>& module : moduleRegistry.Modules) {
			modules->SetObjectField(module->InternalName, GenLuaModule(module));
		}
		doc->SetObjectField(TEXT("modules"), modules);

		return doc;
	}

	FString GenDocumentation() {
		TSharedRef<FJsonObject> MainObj = MakeShared<FJsonObject>();

		MainObj->SetObjectField(TEXT("reflection"), GenReflectionDoc());
		MainObj->SetObjectField(TEXT("lua"), GenLuaDoc());

		FString JsonString;
		TSharedRef<TJsonWriter<>> Json = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(MainObj, Json);

		return JsonString;
	}

	bool ExecCMD(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
		if (FParse::Command(&Command, TEXT("FINGenJsonDoc"))) {
			UE_LOG(LogFicsItNetworksDocumentation, Display, TEXT("Generating FicsIt-Networks JSON Documentation..."));

			FString documentation = GenDocumentation();

			UE_LOG(LogFicsItNetworksDocumentation, Display, TEXT("FicsIt-Networks JSON Documentation generated!"));

			FString path;
			if (!FParse::Value(Command, TEXT("Path"), path)) {
				path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
				path = FPaths::Combine(path, TEXT("FINDocumentation.json"));
			}

			UE_LOGFMT(LogFicsItNetworksDocumentation, Display, "Saving FicsIt-Networks JSON Documentation under: {Path}", path);

			FFileHelper::SaveStringToFile(documentation, *path);

			UE_LOGFMT(LogFicsItNetworksDocumentation, Display, "FicsIt-Networks JSON Documentation Saved!");

			return true;
		}
		return false;
	}

	[[maybe_unused]] static FStaticSelfRegisteringExec SelfRegisterCMD(&ExecCMD);
}
UE_ENABLE_OPTIMIZATION
