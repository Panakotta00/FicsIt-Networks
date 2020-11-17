#include "FINReflection.h"

#include "AssetRegistryModule.h"
#include "FINArrayProperty.h"
#include "FINBoolProperty.h"
#include "FINClassProperty.h"
#include "FINFloatProperty.h"
#include "FINFuncProperty.h"
#include "FINIntProperty.h"
#include "FINObjectProperty.h"
#include "FINStaticReflectionSource.h"
#include "FINStrProperty.h"
#include "FINStructProperty.h"
#include "FINTraceProperty.h"
#include "FINUReflectionSource.h"

UFINClass* UFINReflection::FindClass(UClass* Clazz, bool bRecursive) {
	return FFINReflection::Get()->FindClass(Clazz, bRecursive);
}

FFINReflection* FFINReflection::Get() {
	static FFINReflection* Self = nullptr;
	if (!Self) Self = new FFINReflection();
	return Self;
}

void FFINReflection::PopulateSources() {
	Sources.Add(GetDefault<UFINUReflectionSource>());
	Sources.Add(GetDefault<UFINStaticReflectionSource>());
}

void FFINReflection::LoadAllClasses() {
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/Game/FicsItNetworks/"));
	AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);

	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add("/");
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.ClassNames.Add(UBlueprintGeneratedClass::StaticClass()->GetFName());
	Filter.ClassNames.Add(UClass::StaticClass()->GetFName());
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	for (const FAssetData& Asset : AssetData) {
		FString Path = Asset.ObjectPath.ToString();
		if (!Path.EndsWith("_C")) Path += "_C";
		UClass* Class = LoadClass<UObject>(NULL, *Path);
		if (!Class) {
			Class = LoadClass<UObject>(NULL, *Path);
		}
		if (!Class) continue;
		FindClass(Class);
	}

	for (TObjectIterator<UClass> Class; Class; ++Class) {
		if (!Class->GetName().StartsWith("SKEL_") && !Class->GetName().StartsWith("REINST_")) FindClass(*Class);
	}
}

UFINClass* FFINReflection::FindClass(UClass* Clazz, bool bRecursive, bool bTryToReflect) {
	if (!Clazz) return nullptr;
	do {
		// Find class in cache and retrun if found
		{
			UFINClass** Class = Classes.Find(Clazz);
			if (Class) return *Class;
		}

		// try to load this exact class into cache and return it
		if (bTryToReflect) {
			UFINClass* Class = nullptr;
			for (const UFINReflectionSource* Source : Sources) {
				if (Source->ProvidesRequirements(Clazz)) {
					Class = NewObject<UFINClass>();
					break;
				}
			}
			if (Class) {
				Clazz->AddToRoot();
				Class->AddToRoot();
				Classes.Add(Clazz, Class);
				for (const UFINReflectionSource* Source : Sources) {
					Source->FillData(this, Class, Clazz);
				}
				return Class;
			}
		}

		// go to the next super class
		if (Clazz == UObject::StaticClass()) Clazz = nullptr;
		else Clazz = Clazz->GetSuperClass();
	} while (Clazz && bRecursive);
	return nullptr;
}

void PrintProperty(FString Prefix, UFINProperty* Property) {
	FString Log = Prefix;
	if (Property->GetPropertyFlags() & FIN_Prop_Attrib) Log += "Attrib";
	else if (Property->GetPropertyFlags() & FIN_Prop_Param) Log += "Param";
	Log += " " + Property->GetInternalName() + " '" + Property->GetDisplayName().ToString() + "' Desc:'" + Property->GetDescription().ToString() + "' " + FString::FromInt(Property->GetType()) + " '" + Property->GetClass()->GetName() + "'";
	if (Property->GetPropertyFlags() & FIN_Prop_OutParam) Log += " Out";
	if (Property->GetPropertyFlags() & FIN_Prop_RetVal) Log += " RetVal";
	if (Property->GetPropertyFlags() & FIN_Prop_ReadOnly) Log += " ReadOnly";
	if (UFINFuncProperty* FINFuncProp = Cast<UFINFuncProperty>(Property)) {
		if (FINFuncProp->GetterFunc.Function) Log += " UFuncGetter";
		if (FINFuncProp->SetterFunc.Function) Log += " UFuncSetter";
		if ((bool)FINFuncProp->GetterFunc.GetterFunc) Log += " FuncGetter";
		if ((bool)FINFuncProp->SetterFunc.SetterFunc) Log += " FuncSetter";
	}
	if (UFINStructProperty* FINStructProp = Cast<UFINStructProperty>(Property)) {
		if (FINStructProp->Struct) Log += " " + FINStructProp->Struct->GetName();
	}
	if (Property->GetPropertyFlags() & FIN_Prop_RT_Sync) Log += " Sync";
	if (Property->GetPropertyFlags() & FIN_Prop_RT_Parallel) Log += " Parallel";
	if (Property->GetPropertyFlags() & FIN_Prop_RT_Async) Log += " Async";
	if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) Log += " Class";
	SML::Logging::error(TCHAR_TO_UTF8(*Log));
}

void PrintFunction(FString Prefix, UFINFunction* Function) {
	FString Log = Prefix;
	Log += "Function: " + Function->GetInternalName();
	Log += " '" + Function->GetDisplayName().ToString() + "'";
	Log += " Desc:'" + Function->GetDescription().ToString() + "'";
	if (Function->GetFunctionFlags() & FIN_Func_VarArgs) Log += " Varargs";
	if (Function->GetFunctionFlags() & FIN_Func_RT_Sync) Log += " Sync";
	if (Function->GetFunctionFlags() & FIN_Func_RT_Parallel) Log += " Parallel";
	if (Function->GetFunctionFlags() & FIN_Func_RT_Async) Log += " Async";
	if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) Log += " Class";
	if (Function->GetFunctionFlags() & FIN_Func_StaticFunc) Log += " Static";
	SML::Logging::error(TCHAR_TO_UTF8(*Log));
	Prefix += " ";
	for (UFINProperty* Param : Function->GetParameters()) {
		PrintProperty(Prefix, Param);
	}
}

void FFINReflection::PrintReflection() {
	for (TPair<UClass*, UFINClass*> Class : Classes) {
		SML::Logging::error("Class: ", TCHAR_TO_UTF8(*Class.Value->GetInternalName()), " '", TCHAR_TO_UTF8(*Class.Value->GetDisplayName().ToString()), "' Desc:'", TCHAR_TO_UTF8(*Class.Value->GetDescription().ToString()), "'");
		for (UFINFunction* Function : Class.Value->GetFunctions()) {
			PrintFunction(" ", Function);
		}
		for (UFINProperty* Prop : Class.Value->GetProperties()) {
			PrintProperty(" ", Prop);
		}
	}
}

UFINProperty* FINCreateFINPropertyFromUProperty(UProperty* Property, UProperty* OverrideProperty, UObject* Outer) {
	UFINProperty* FINProp = nullptr;
	if (Cast<UStrProperty>(Property)) {
		UFINStrProperty* FINStrProp = NewObject<UFINStrProperty>(Outer);
		FINStrProp->Property = Cast<UStrProperty>(OverrideProperty);
		FINProp = FINStrProp;
	} else if (Cast<UIntProperty>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>(Outer);
		FINIntProp->Property = Cast<UIntProperty>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (Cast<UInt64Property>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>(Outer);
		FINIntProp->Property64 = Cast<UInt64Property>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (Cast<UFloatProperty>(Property)) {
		UFINFloatProperty* FINFloatProp = NewObject<UFINFloatProperty>(Outer);
		FINFloatProp->Property = Cast<UFloatProperty>(OverrideProperty);
		FINProp = FINFloatProp;
	} else if (Cast<UBoolProperty>(Property)) {
		UFINBoolProperty* FINBoolProp = NewObject<UFINBoolProperty>(Outer);
		FINBoolProp->Property = Cast<UBoolProperty>(OverrideProperty);
		FINProp = FINBoolProp;
	} else if (Cast<UClassProperty>(Property)) {
		UFINClassProperty* FINClassProp = NewObject<UFINClassProperty>(Outer);
		FINClassProp->Property = Cast<UClassProperty>(OverrideProperty);
		FINProp = FINClassProp;
	} else if (Cast<UObjectProperty>(Property)) {
		UFINObjectProperty* FINObjectProp = NewObject<UFINObjectProperty>(Outer);
		FINObjectProp->Property = Cast<UObjectProperty>(OverrideProperty);
		FINProp = FINObjectProp;
	} else  if (Cast<UStructProperty>(Property)) {
		UStructProperty* StructProp = Cast<UStructProperty>(OverrideProperty);
		if (StructProp->Struct == FFINNetworkTrace::StaticStruct()) {
			UFINTraceProperty* FINTraceProp = NewObject<UFINTraceProperty>(Outer);
			FINTraceProp->Property = StructProp;
			FINProp = FINTraceProp;
		} else {
			UFINStructProperty* FINStructProp = NewObject<UFINStructProperty>(Outer);
			FINStructProp->Property = StructProp;
			FINProp = FINStructProp;
		}
    } else if (Cast<UArrayProperty>(Property)) {
    	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(OverrideProperty);
	    UFINArrayProperty* FINArrayProp = NewObject<UFINArrayProperty>(Outer);
    	FINArrayProp->Property = ArrayProperty;
    	FINArrayProp->InnerType = FINCreateFINPropertyFromUProperty(ArrayProperty->Inner, FINArrayProp);
    	FINProp = FINArrayProp;
    }
	check(FINProp != nullptr);
	if (Property->PropertyFlags & CPF_OutParm) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
	if (Property->PropertyFlags & CPF_ReturnParm) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
	return FINProp;
}
