#include "FicsItReflection.h"
#include "Reflection/FIRFuncProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/Source/FIRSourceStatic.h"
#include "Reflection/Source/FIRSourceUObject.h"
#include "AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

DEFINE_LOG_CATEGORY(LogFicsItReflection, Log, All);

UFIRClass* UFicsItReflection::FindClass(UClass* Clazz, bool bRecursive) {
	return FFicsItReflectionModule::Get()->FindClass(Clazz, bRecursive);
}

UFIRStruct* UFicsItReflection::FindStruct(UScriptStruct* Struct, bool bRecursive) {
	return FFicsItReflectionModule::Get()->FindStruct(Struct, bRecursive);
}

FFicsItReflectionModule* FFicsItReflectionModule::Get() {
	static FFicsItReflectionModule* Self = nullptr;
	if (!Self) Self = new FFicsItReflectionModule();
	return Self;
}

void FFicsItReflectionModule::PopulateSources() {
	Sources.Add(GetDefault<UFIRSourceUObject>());
	Sources.Add(GetDefault<UFIRSourceStatic>());
}

void FFicsItReflectionModule::LoadAllTypes() {
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

#if WITH_EDITOR
	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/FicsItNetworks/"));
	AssetRegistry.ScanPathsSynchronous(PathsToScan, true);

	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add("/");
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UBlueprintGeneratedClass::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UClass::StaticClass()->GetClassPathName());
	AssetRegistry.GetAssets(Filter, AssetData);

	for (const FAssetData& Asset : AssetData) {
		FString Path = Asset.GetObjectPathString();
		if (!Path.EndsWith("_C")) Path += "_C";
		UClass* Class = LoadClass<UObject>(NULL, *Path);
		if (!Class) {
			Class = LoadClass<UObject>(NULL, *Path);
		}
		if (!Class) continue;
		FindClass(Class);
	}
#else
	TArray<FTopLevelAssetPath> BaseNames;
	BaseNames.Add(UObject::StaticClass()->GetClassPathName());
	TSet<FTopLevelAssetPath> Excluded;
	TSet<FTopLevelAssetPath> DerivedNames;
	AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);

	for (const FTopLevelAssetPath& ClassName : DerivedNames) {
		UClass* Class = TAssetSubclassOf(FSoftObjectPath(ClassName)).LoadSynchronous();
		if (Class->GetClassFlags() & (CLASS_Abstract | CLASS_Hidden) || Class->GetName().StartsWith("SKEL_")) continue;
		FindClass(Class);
	}
#endif

	for (TObjectIterator<UClass> Class; Class; ++Class) {
		if (!Class->GetName().StartsWith("SKEL_") && !Class->GetName().StartsWith("REINST_")) FindClass(*Class);
	}

	for (TObjectIterator<UScriptStruct> Struct; Struct; ++Struct) {
		if (!Struct->GetName().StartsWith("SKEL_") && !Struct->GetName().StartsWith("REINST_")) FindStruct(*Struct);
	}
}

UFIRClass* FFicsItReflectionModule::FindClass(UClass* Clazz, bool bRecursive, bool bTryToReflect) {
	if (!Clazz) return nullptr;
	do {
		// Find class in cache and retrun if found
		{
			UFIRClass** Class = Classes.Find(Clazz);
			if (Class) return *Class;
		}

		// try to load this exact class into cache and return it
		if (bTryToReflect) {
			UFIRClass* Class = nullptr;
			for (const UFIRSource* Source : Sources) {
				if (Source->ProvidesRequirements(Clazz)) {
					Class = NewObject<UFIRClass>(Clazz);
					break;
				}
			}
			if (Class) {
				Clazz->AddToRoot();
				Class->AddToRoot();
				Classes.Add(Clazz, Class);
				ClassesReversed.Add(Class, Clazz);
				for (const UFIRSource* Source : Sources) {
					Source->FillData(this, Class, Clazz);
				}
				ClassNames.Add(Class->GetInternalName(), Class);
				return Class;
			}
		}

		// go to the next super class
		if (Clazz == UObject::StaticClass()) Clazz = nullptr;
		else Clazz = Clazz->GetSuperClass();
	} while (Clazz && bRecursive);
	return nullptr;
}

UFIRClass* FFicsItReflectionModule::FindClass(const FString& ClassName) const {
	UFIRClass* const* Class = ClassNames.Find(ClassName);
	if (Class) return *Class;
	else return nullptr;
}

UClass* FFicsItReflectionModule::FindUClass(UFIRClass* Class) const {
	UClass* const* UClass = ClassesReversed.Find(Class);
	if (UClass) return *UClass;
	else return nullptr;
}

UFIRStruct* FFicsItReflectionModule::FindStruct(UScriptStruct* Struct, bool bRecursive, bool bTryToReflect) {
	if (!Struct) return nullptr;
	do {
		// Find class in cache and retrun if found
		{
			UFIRStruct** FIRStruct = Structs.Find(Struct);
			if (FIRStruct) return *FIRStruct;
		}

		// try to load this exact class into cache and return it
		if (bTryToReflect) {
			UFIRStruct* FIRStruct = nullptr;
			for (const UFIRSource* Source : Sources) {
				if (Source->ProvidesRequirements(Struct)) {
					FIRStruct = NewObject<UFIRStruct>(Struct);
					break;
				}
			}
			if (FIRStruct) {
				Struct->AddToRoot();
				FIRStruct->AddToRoot();
				Structs.Add(Struct, FIRStruct);
				StructsReversed.Add(FIRStruct, Struct);
				for (const UFIRSource* Source : Sources) {
					Source->FillData(this, FIRStruct, Struct);
				}
				StructNames.Add(FIRStruct->GetInternalName(), FIRStruct);
				return FIRStruct;
			}
		}

		// go to the next super class
		Struct = Cast<UScriptStruct>(Struct->GetSuperStruct());
	} while (Struct && bRecursive);
	return nullptr;
}

UFIRStruct* FFicsItReflectionModule::FindStruct(const FString& StructName) const {
	UFIRStruct* const* Struct = StructNames.Find(StructName);
	if (Struct) return *Struct;
	else return nullptr;
}

UScriptStruct* FFicsItReflectionModule::FindScriptStruct(UFIRStruct* Struct) const {
	UScriptStruct* const* ScriptStruct = StructsReversed.Find(Struct);
	if (ScriptStruct) return *ScriptStruct;
	else return nullptr;
}

void PrintProperty(FString Prefix, UFIRProperty* Property) {
	FString Log = Prefix;
	if (Property->GetPropertyFlags() & FIR_Prop_Attrib) Log += "Attrib";
	else if (Property->GetPropertyFlags() & FIR_Prop_Param) Log += "Param";
	Log += " " + Property->GetInternalName() + " '" + Property->GetDisplayName().ToString() + "' Desc:'" + Property->GetDescription().ToString() + "' " + FString::FromInt(Property->GetType()) + " '" + Property->GetClass()->GetName() + "'";
	if (Property->GetPropertyFlags() & FIR_Prop_OutParam) Log += " Out";
	if (Property->GetPropertyFlags() & FIR_Prop_RetVal) Log += " RetVal";
	if (Property->GetPropertyFlags() & FIR_Prop_ReadOnly) Log += " ReadOnly";
	if (UFIRFuncProperty* FIRFuncProp = Cast<UFIRFuncProperty>(Property)) {
		if (FIRFuncProp->GetterFunc.Function) Log += " UFuncGetter";
		if (FIRFuncProp->SetterFunc.Function) Log += " UFuncSetter";
		if ((bool)FIRFuncProp->GetterFunc.GetterFunc) Log += " FuncGetter";
		if ((bool)FIRFuncProp->SetterFunc.SetterFunc) Log += " FuncSetter";
	}
	if (UFIRStructProperty* FIRStructProp = Cast<UFIRStructProperty>(Property)) {
		if (FIRStructProp->Struct) Log += " " + FIRStructProp->Struct->GetName();
	}
	if (Property->GetPropertyFlags() & FIR_Prop_RT_Sync) Log += " Sync";
	if (Property->GetPropertyFlags() & FIR_Prop_RT_Parallel) Log += " Parallel";
	if (Property->GetPropertyFlags() & FIR_Prop_RT_Async) Log += " Async";
	if (Property->GetPropertyFlags() & FIR_Prop_ClassProp) Log += " Class";
	UE_LOG(LogFicsItReflection, Display, TEXT("%s"), *Log);
}

void PrintFunction(FString Prefix, UFIRFunction* Function) {
	FString Log = Prefix;
	Log += "Function: " + Function->GetInternalName();
	Log += " '" + Function->GetDisplayName().ToString() + "'";
	Log += " Desc:'" + Function->GetDescription().ToString() + "'";
	if (Function->GetFunctionFlags() & FIR_Func_VarArgs) Log += " Varargs";
	if (Function->GetFunctionFlags() & FIR_Func_RT_Sync) Log += " Sync";
	if (Function->GetFunctionFlags() & FIR_Func_RT_Parallel) Log += " Parallel";
	if (Function->GetFunctionFlags() & FIR_Func_RT_Async) Log += " Async";
	if (Function->GetFunctionFlags() & FIR_Func_ClassFunc) Log += " Class";
	if (Function->GetFunctionFlags() & FIR_Func_StaticFunc) Log += " Static";
	UE_LOG(LogFicsItReflection, Display, TEXT("%s"), *Log);
	Prefix += " ";
	for (UFIRProperty* Param : Function->GetParameters()) {
		PrintProperty(Prefix, Param);
	}
}

void PrintSignal(FString Prefix, UFIRSignal* Signal) {
	FString Log = Prefix;
	Log += "Signal: " + Signal->GetInternalName();
	Log += " '" + Signal->GetDisplayName().ToString() + "'";
	Log += " Desc:'" + Signal->GetDescription().ToString() + "'";
	UE_LOG(LogFicsItReflection, Display, TEXT("%s"), *Log);
	Prefix += " ";
	for (UFIRProperty* Param : Signal->GetParameters()) {
		PrintProperty(Prefix, Param);
	}
}

void FFicsItReflectionModule::PrintReflection() {
	for (TPair<UClass*, UFIRClass*> Class : Classes) {
		UE_LOG(LogFicsItReflection, Display, TEXT("Class: %s '%s' Desc:'%s'"), *Class.Value->GetInternalName(), *Class.Value->GetDisplayName().ToString(), *Class.Value->GetDescription().ToString());
		for (UFIRFunction* Function : Class.Value->GetFunctions()) {
			PrintFunction(" ", Function);
		}
		for (UFIRProperty* Prop : Class.Value->GetProperties()) {
			PrintProperty(" ", Prop);
		}
		for (UFIRSignal* Signal : Class.Value->GetSignals()) {
			PrintSignal(" ", Signal);
		}
	}

	for (TPair<UScriptStruct*, UFIRStruct*> FIRStruct : Structs) {
		FString Log = "Struct:";
		Log += " " + FIRStruct.Value->GetInternalName();
		Log += " '" + FIRStruct.Value->GetDisplayName().ToString() + "'";
		Log += " Desc:'" + FIRStruct.Value->GetDescription().ToString() + "'";
		for (UFIRFunction* Function : FIRStruct.Value->GetFunctions()) {
			PrintFunction(" ", Function);
		}
		for (UFIRProperty* Prop : FIRStruct.Value->GetProperties()) {
			PrintProperty(" ", Prop);
		}
		UE_LOG(LogFicsItReflection, Display, TEXT("Class: %s"), *Log);
	}
}
