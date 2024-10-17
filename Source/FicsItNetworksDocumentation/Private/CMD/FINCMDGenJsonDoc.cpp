#include "FicsItNetworksDocumentation.h"
#include "FicsItReflection.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRBoolProperty.h"
#include "Reflection/FIRClassProperty.h"
#include "Reflection/FIRFloatProperty.h"
#include "Reflection/FIRIntProperty.h"
#include "Reflection/FIRObjectProperty.h"
#include "Reflection/FIRStrProperty.h"
#include "Reflection/FIRTraceProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRProperty.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "FINLua/FINLuaModule.h"
#include "Logging/StructuredLog.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UE_DISABLE_OPTIMIZATION
namespace FINGenJsonDoc {
	TSharedRef<FJsonObject> GenReflectionDataType(UFIRProperty* Property, TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>()) {
		if (Property->IsA<UFIRBoolProperty>()) {
			Obj->SetStringField("type", "Bool");
		} else if (Property->IsA<UFIRIntProperty>()) {
			Obj->SetStringField("type", "Int");
		} else if (Property->IsA<UFIRFloatProperty>()) {
			Obj->SetStringField("type", "Float");
		} else if (Property->IsA<UFIRStrProperty>()) {
			Obj->SetStringField("type", "String");
		} else if (UFIRObjectProperty* ObjProp = Cast<UFIRObjectProperty>(Property)) {
			Obj->SetStringField("type", "Object");
			if (UFIRClass* subclass = FFicsItReflectionModule::Get().FindClass(ObjProp->GetSubclass())) Obj->SetStringField("subclass", subclass->GetInternalName());
		} else if (UFIRTraceProperty* TraceProp = Cast<UFIRTraceProperty>(Property)) {
			Obj->SetStringField("type", "Trace");
			if (UFIRClass* subclass = FFicsItReflectionModule::Get().FindClass(TraceProp->GetSubclass())) Obj->SetStringField("subclass", subclass->GetInternalName());
		} else if (UFIRStructProperty* StructProp = Cast<UFIRStructProperty>(Property)) {
			Obj->SetStringField("type", "Struct");
			if (UFIRStruct* inner = FFicsItReflectionModule::Get().FindStruct(StructProp->GetInner())) Obj->SetStringField("inner", inner->GetInternalName());
		} else if (UFIRClassProperty* ClassProp = Cast<UFIRClassProperty>(Property)) {
			Obj->SetStringField("type", "Class");
			if (UFIRClass* subclass = FFicsItReflectionModule::Get().FindClass(ClassProp->GetSubclass())) Obj->SetStringField("subclass", subclass->GetInternalName());
		} else if (UFIRArrayProperty* ArrayProp = Cast<UFIRArrayProperty>(Property)) {
			Obj->SetStringField("type", "Array");
			if (ArrayProp->GetInnerType()) Obj->SetObjectField("inner", GenReflectionDataType(ArrayProp->GetInnerType()));
		}
		return Obj;
	}

	void FINGenRefBase(const TSharedPtr<FJsonObject>& Obj, const UFIRBase* Base) {
		Obj->SetStringField(TEXT("internalName"), Base->GetInternalName());
		if (!Base->GetDisplayName().IsEmpty()) Obj->SetStringField(TEXT("displayName"), Base->GetDisplayName().ToString());
		Obj->SetStringField(TEXT("description"), Base->GetDescription().ToString());
	}

	void FINGenRefProp(const TSharedPtr<FJsonObject>& Obj, UFIRProperty* Prop) {
		FINGenRefBase(Obj, Prop);
		Obj->SetObjectField(TEXT("type"), GenReflectionDataType(Prop));

		EFIRPropertyFlags Flags = Prop->GetPropertyFlags();
		TArray<TSharedPtr<FJsonValue>> FlagArray;
		if (Flags & FIR_Prop_Attrib) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("Attrib")));
		if (Flags & FIR_Prop_ReadOnly) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("ReadOnly")));
		if (Flags & FIR_Prop_Param) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("Param")));
		if (Flags & FIR_Prop_OutParam) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("OutParam")));
		if (Flags & FIR_Prop_RetVal) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RetVal")));
		if (Flags & FIR_Prop_RT_Sync) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Sync")));
		if (Flags & FIR_Prop_RT_Parallel) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Parallel")));
		if (Flags & FIR_Prop_RT_Async) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Async")));
		if (Flags & FIR_Prop_ClassProp) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("ClassProp")));
		if (Flags & FIR_Prop_StaticProp) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("StaticProp")));
		Obj->SetArrayField(TEXT("flags"), FlagArray);
	}

	void FINGenRefFunc(const TSharedPtr<FJsonObject>& Obj, const UFIRFunction* Func) {
		FINGenRefBase(Obj, Func);
		TArray<TSharedPtr<FJsonValue>> parameters;
		for (UFIRProperty* parameter : Func->GetParameters()) {
			TSharedPtr<FJsonObject> parameterObject = MakeShared<FJsonObject>();
			FINGenRefProp(parameterObject, parameter);
			parameters.Add(MakeShared<FJsonValueObject>(parameterObject));
		}
		Obj->SetArrayField(TEXT("parameters"), parameters);

		EFIRFunctionFlags Flags = Func->GetFunctionFlags();
		TArray<TSharedPtr<FJsonValue>> FlagArray;
		if (Flags & FIR_Func_VarArgs) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("VarArgs")));
		if (Flags & FIR_Func_RT_Sync) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Sync")));
		if (Flags & FIR_Func_RT_Parallel) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Parallel")));
		if (Flags & FIR_Func_RT_Async) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("RT_Async")));
		if (Flags & FIR_Func_MemberFunc) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("MemberFunc")));
		if (Flags & FIR_Func_ClassFunc) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("ClassFunc")));
		if (Flags & FIR_Func_StaticFunc) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("StaticFunc")));
		if (Flags & FIR_Func_VarRets) FlagArray.Add(MakeShared<FJsonValueString>(TEXT("VarRets")));
		Obj->SetArrayField(TEXT("flags"), FlagArray);
	}

	void FINGenRefStruct(const TSharedPtr<FJsonObject>& Obj, const UFIRStruct* Struct) {
		FINGenRefBase(Obj, Struct);
		if (Struct->GetParent()) Obj->SetStringField(TEXT("parent"), Struct->GetParent()->GetInternalName());
		TArray<TSharedPtr<FJsonValue>> Properties;
		for (UFIRProperty* Property : Struct->GetProperties(false)) {
			TSharedPtr<FJsonObject> PropertyObj = MakeShared<FJsonObject>();
			FINGenRefProp(PropertyObj, Property);
			Properties.Add(MakeShared<FJsonValueObject>(PropertyObj));
		}
		Obj->SetArrayField(TEXT("properties"), Properties);
		TArray<TSharedPtr<FJsonValue>> Functions;
		for (UFIRFunction* Function : Struct->GetFunctions(false)) {
			TSharedPtr<FJsonObject> FunctionObj = MakeShared<FJsonObject>();
			FINGenRefFunc(FunctionObj, Function);
			Functions.Add(MakeShared<FJsonValueObject>(FunctionObj));
		}
		Obj->SetArrayField(TEXT("functions"), Functions);
	}

	void FINGenRefSignal(const TSharedPtr<FJsonObject>& Obj, const UFIRSignal* Signal) {
		FINGenRefBase(Obj, Signal);
		TArray<TSharedPtr<FJsonValue>> parameters;
		for (UFIRProperty* parameter : Signal->GetParameters()) {
			TSharedPtr<FJsonObject> parameterObject = MakeShared<FJsonObject>();
			FINGenRefProp(parameterObject, parameter);
			parameters.Add(MakeShared<FJsonValueObject>(parameterObject));
		}
		Obj->SetArrayField(TEXT("parameters"), parameters);
		Obj->SetBoolField(TEXT("isVarArgs"), Signal->IsVarArgs());
	}

	void FINGenRefClass(const TSharedPtr<FJsonObject>& Obj, const UFIRClass* Class) {
		FINGenRefStruct(Obj, Class);
		TArray<TSharedPtr<FJsonValue>> Signals;
		for (UFIRSignal* Signal : Class->GetSignals(false)) {
			TSharedPtr<FJsonObject> SignalObj = MakeShared<FJsonObject>();
			FINGenRefSignal(SignalObj, Signal);
			Signals.Add(MakeShared<FJsonValueObject>(SignalObj));
		}
		Obj->SetArrayField(TEXT("signals"), Signals);
	}

	void FINGenClasses(TArray<TSharedPtr<FJsonValue>>& Array, const UFIRClass* Class) {
		TSharedPtr<FJsonObject> ClassObj = MakeShared<FJsonObject>();
		FINGenRefClass(ClassObj, Class);
		Array.Add(MakeShared<FJsonValueObject>(ClassObj));

		TArray<FString> Names;
		TMap<FString, UFIRClass*> NamesToClasses;
		for (UFIRClass* ChildClass : Class->GetChildClasses()) {
			Names.Add(ChildClass->GetInternalName());
			NamesToClasses.Add(ChildClass->GetInternalName(), ChildClass);
		}
		Names.Sort();
		for (FString Name : Names) {
			FINGenClasses(Array, NamesToClasses[Name]);
		}
	}

	TSharedPtr<FJsonObject> GenReflectionDoc() {
		auto Ref = FFicsItReflectionModule::Get();

		TArray<TSharedPtr<FJsonValue>> Classes;
		TArray<TSharedPtr<FJsonValue>> Structs;

		FINGenClasses(Classes, Ref.FindClass(UObject::StaticClass()));

		TArray<FString> Names;
		TMap<FString, UFIRStruct*> NamesToStructs;
		for (TPair<UScriptStruct*, UFIRStruct*> Struct : Ref.GetStructs()) {
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

		if (!Parameter.DisplayName.IsEmpty()) doc->SetStringField(TEXT("displayName"), Parameter.DisplayName.ToString());
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
			if (!field.DisplayName.IsEmpty()) obj->SetStringField(TEXT("displayName"), field.DisplayName.ToString());
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

		if (!Metatable.DisplayName.IsEmpty()) doc->SetStringField(TEXT("displayName"), Metatable.DisplayName.ToString());
		doc->SetStringField(TEXT("description"), Metatable.Description.ToString());

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaGlobal(const FFINLuaGlobal& Global) {
		TSharedRef<FJsonObject> doc = GenLuaValue(Global.Value.ToSharedRef());

		if (!Global.DisplayName.IsEmpty()) doc->SetStringField(TEXT("displayName"), Global.DisplayName.ToString());
		doc->SetStringField(TEXT("description"), Global.Description.ToString());

		return doc;
	}

	TSharedRef<FJsonObject> GenLuaModule(const TSharedRef<FFINLuaModule>& Module) {
		TSharedRef<FJsonObject> doc = MakeShared<FJsonObject>();

		if (!Module->DisplayName.IsEmpty()) doc->SetStringField(TEXT("displayName"), Module->DisplayName.ToString());
		doc->SetStringField(TEXT("description"), Module->Description.ToString());

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

			FFileHelper::SaveStringToFile(documentation, *path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

			UE_LOGFMT(LogFicsItNetworksDocumentation, Display, "FicsIt-Networks JSON Documentation Saved!");

			return true;
		}
		return false;
	}

	[[maybe_unused]] static FStaticSelfRegisteringExec SelfRegisterCMD(&ExecCMD);
}
UE_ENABLE_OPTIMIZATION
