#include "FINReflection.h"

#include "AssetRegistryModule.h"
#include "FINArrayProperty.h"
#include "FINBoolProperty.h"
#include "FINClassProperty.h"
#include "FINFloatProperty.h"
#include "FINFuncProperty.h"
#include "FINIntProperty.h"
#include "FINObjectProperty.h"
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
		//SML::Logging::error(TCHAR_TO_UTF8(*Class->GetFullName()));
		FindClass(Class);
	}

	for (TObjectIterator<UClass> Class; Class; ++Class) {
		FindClass(*Class);
	}
}

UFINClass* FFINReflection::FindClass(UClass* Clazz, bool bRecursive, bool bTryToReflect) {
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
	SML::Logging::error(TCHAR_TO_UTF8(*Log));
}

void PrintFunction(FString Prefix, UFINFunction* Function) {
	SML::Logging::error("Function: ", TCHAR_TO_UTF8(*Prefix), TCHAR_TO_UTF8(*Function->GetInternalName()), " '", TCHAR_TO_UTF8(*Function->GetDisplayName().ToString()), "' Desc:'", TCHAR_TO_UTF8(*Function->GetDescription().ToString()), "'");
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
	}
}

UFINProperty* FINCreateFINPropertyFromUProperty(UProperty* Property, UProperty* OverrideProperty) {
	UFINProperty* FINProp = nullptr;
	if (Cast<UStrProperty>(Property)) {
		UFINStrProperty* FINStrProp = NewObject<UFINStrProperty>();
		FINStrProp->Property = Cast<UStrProperty>(OverrideProperty);
		FINProp = FINStrProp;
	} else if (Cast<UIntProperty>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>();
		FINIntProp->Property = Cast<UIntProperty>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (Cast<UInt64Property>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>();
		FINIntProp->Property64 = Cast<UInt64Property>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (Cast<UFloatProperty>(Property)) {
		UFINFloatProperty* FINFloatProp = NewObject<UFINFloatProperty>();
		FINFloatProp->Property = Cast<UFloatProperty>(OverrideProperty);
		FINProp = FINFloatProp;
	} else if (Cast<UBoolProperty>(Property)) {
		UFINBoolProperty* FINBoolProp = NewObject<UFINBoolProperty>();
		FINBoolProp->Property = Cast<UBoolProperty>(OverrideProperty);
		FINProp = FINBoolProp;
	} else if (Cast<UClassProperty>(Property)) {
		UFINClassProperty* FINClassProp = NewObject<UFINClassProperty>();
		FINClassProp->Property = Cast<UClassProperty>(OverrideProperty);
		FINProp = FINClassProp;
	} else if (Cast<UObjectProperty>(Property)) {
		UFINObjectProperty* FINObjectProp = NewObject<UFINObjectProperty>();
		FINObjectProp->Property = Cast<UObjectProperty>(OverrideProperty);
		FINProp = FINObjectProp;
	} else  if (Cast<UStructProperty>(Property)) {
		UStructProperty* StructProp = Cast<UStructProperty>(OverrideProperty);
		if (StructProp->Struct == FFINNetworkTrace::StaticStruct()) {
			UFINTraceProperty* FINTraceProp = NewObject<UFINTraceProperty>();
			FINTraceProp->Property = StructProp;
			FINProp = FINTraceProp;
		} else {
			UFINStructProperty* FINStructProp = NewObject<UFINStructProperty>();
			FINStructProp->Property = StructProp;
			FINProp = FINStructProp;
		}
    } else if (Cast<UArrayProperty>(Property)) {
    	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(OverrideProperty);
	    UFINArrayProperty* FINArrayProp = NewObject<UFINArrayProperty>();
    	FINArrayProp->Property = ArrayProperty;
    	FINArrayProp->InnerType = FINCreateFINPropertyFromUProperty(ArrayProperty->Inner);
    	FINProp = FINArrayProp;
    }
	check(FINProp != nullptr);
	if (Property->PropertyFlags & CPF_OutParm) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
	if (Property->PropertyFlags & CPF_ReturnParm) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
	return FINProp;
}
