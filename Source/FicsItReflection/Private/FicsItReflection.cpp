#include "FicsItReflection.h"
#include "Reflection/FIRFuncProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/Source/FIRSourceStatic.h"
#include "Reflection/Source/FIRSourceUObject.h"
#include "AssetRegistryModule.h"
#include "FGFactoryConnectionComponent.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FIRSubsystem.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Patching/NativeHookManager.h"
#include "UObject/CoreRedirects.h"

DEFINE_LOG_CATEGORY(LogFicsItReflection);

UFIRClass* UFicsItReflection::FindClass(UClass* Clazz, bool bRecursive) {
	return FFicsItReflectionModule::Get().FindClass(Clazz, bRecursive);
}

UFIRStruct* UFicsItReflection::FindStruct(UScriptStruct* Struct, bool bRecursive) {
	return FFicsItReflectionModule::Get().FindStruct(Struct, bRecursive);
}

void UFGRailroadTrackConnectionComponent_SetSwitchPosition_Hook(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,int32)>& Scope, UFGRailroadTrackConnectionComponent* self, int32 Index) {
	FFIRRailroadSwitchForce* ForcedTrack = AFIRSubsystem::GetReflectionSubsystem(self)->GetForcedRailroadSwitch(self);
	if (ForcedTrack) {
		Scope(self, 0);
	}
}

void UFGRailroadTrackConnectionComponent_AddConnection_Hook(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* self, UFGRailroadTrackConnectionComponent* Connection) {
	AFIRSubsystem::GetReflectionSubsystem(self)->AddRailroadSwitchConnection(Scope, self, Connection);
}

void UFGRailroadTrackConnectionComponent_RemoveConnection_Hook(CallScope<void(*)(UFGRailroadTrackConnectionComponent*,UFGRailroadTrackConnectionComponent*)>& Scope, UFGRailroadTrackConnectionComponent* self, UFGRailroadTrackConnectionComponent* Connection) {
	AFIRSubsystem::GetReflectionSubsystem(self)->RemoveRailroadSwitchConnection(Scope, self, Connection);
}

void UFGFactoryConnectionComponent_PeekOutput_Hook(CallScope<bool(*)(const UFGFactoryConnectionComponent*,TArray<FInventoryItem>&,TSubclassOf<UFGItemDescriptor>)>& Scope, const UFGFactoryConnectionComponent* const_self, TArray<FInventoryItem>& out_items, TSubclassOf<UFGItemDescriptor> type) {
	UFGFactoryConnectionComponent* self = const_cast<UFGFactoryConnectionComponent*>(const_self);
	TOptional<TTuple<FCriticalSection&, FFIRFactoryConnectorSettings&>> OptionalSettings = AFIRSubsystem::GetReflectionSubsystem(self)->GetFactoryConnectorSettings(self);
	if (OptionalSettings.IsSet()) {
		FFIRFactoryConnectorSettings& Settings = OptionalSettings.GetValue().Value;
		if ((Settings.bBlocked && Settings.UnblockedTransfers == 0) || (Settings.AllowedItem != nullptr && Settings.AllowedItem != type)) {
			Scope.Override(false);
		} else {
			bool bSuccess = Scope(self, out_items, Settings.AllowedItem ? Settings.AllowedItem : type);
		}
		OptionalSettings.GetValue().Key.Unlock();
	}
}

void UFGFactoryConnectionComponent_GrabOutput_Hook(CallScope<bool(*)(UFGFactoryConnectionComponent*,FInventoryItem&,float&,TSubclassOf<UFGItemDescriptor>)>& Scope, UFGFactoryConnectionComponent* self, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type) {
	TOptional<TTuple<FCriticalSection&, FFIRFactoryConnectorSettings&>> OptionalSettings = AFIRSubsystem::GetReflectionSubsystem(self)->GetFactoryConnectorSettings(self);
	if (OptionalSettings.IsSet()) {
		FFIRFactoryConnectorSettings& Settings = OptionalSettings.GetValue().Value;
		if ((Settings.bBlocked && Settings.UnblockedTransfers == 0) || (Settings.AllowedItem != nullptr && type != nullptr && Settings.AllowedItem != type)) {
			Scope.Override(false);
		} else {
			bool bSuccess = Scope(self, out_item, out_OffsetBeyond, Settings.AllowedItem ? Settings.AllowedItem : type);
			if (bSuccess) Settings.UnblockedTransfers = FMath::Max(0, Settings.UnblockedTransfers-1);
		}
		OptionalSettings.GetValue().Key.Unlock();
	}
}

void UFGFactoryConnectionComponent_InternalGrabOutputInventory_Hook(CallScope<bool(*)(UFGFactoryConnectionComponent*,FInventoryItem&,TSubclassOf<UFGItemDescriptor>)>& Scope, UFGFactoryConnectionComponent* self, FInventoryItem& out_item, TSubclassOf<UFGItemDescriptor> type) {
	TOptional<TTuple<FCriticalSection&, FFIRFactoryConnectorSettings&>> OptionalSettings = AFIRSubsystem::GetReflectionSubsystem(self)->GetFactoryConnectorSettings(self);
	if (OptionalSettings.IsSet()) {
		FFIRFactoryConnectorSettings& Settings = OptionalSettings.GetValue().Value;
		if ((Settings.bBlocked && Settings.UnblockedTransfers == 0) || (Settings.AllowedItem != nullptr && type != nullptr && Settings.AllowedItem != type)) {
			Scope.Override(false);
		} else {
			bool bSuccess = Scope(self, out_item, Settings.AllowedItem ? Settings.AllowedItem : type);
			if (bSuccess) Settings.UnblockedTransfers = FMath::Max(0, Settings.UnblockedTransfers-1);
		}
		OptionalSettings.GetValue().Key.Unlock();
	}
}

void FFicsItReflectionModule::StartupModule() {
	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINNetworkTrace"), TEXT("/Script/FicsItReflection.FIRTrace")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINInstancedStruct"), TEXT("/Script/FicsItReflection.FIRInstancedStruct")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINHook"), TEXT("/Script/FicsItReflection.FIRHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINHookData"), TEXT("/Script/FicsItReflection.FIRHookData")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINHookSubsystem"), TEXT("/Script/FicsItReflection.AFIRHookSubsystem")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINException"), TEXT("/Script/FicsItReflection.FIRException")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionException"), TEXT("/Script/FicsItReflection.FIRReflectionException")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINAnyValue"), TEXT("/Script/FicsItReflection.FIRAnyValue")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINNetworkValueType"), TEXT("/Script/FicsItReflection.FIRValueType")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINUFunction"), TEXT("/Script/FicsItReflection.FIRUFunction")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINTraceProperty"), TEXT("/Script/FicsItReflection.FIRTraceProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStructProperty"), TEXT("/Script/FicsItReflection.FIRStructProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINStructFlags"), TEXT("/Script/FicsItReflection.FIRStructFlags")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStruct"), TEXT("/Script/FicsItReflection.FIRStruct")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStrProperty"), TEXT("/Script/FicsItReflection.FIRStrProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINSignalFlags"), TEXT("/Script/FicsItReflection.FIRSignalFlags")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINSignal"), TEXT("/Script/FicsItReflection.FIRSignal")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINRepPropertyFlags"), TEXT("/Script/FicsItReflection.FIRPropertyFlags")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINPropertyGetterFunc"), TEXT("/Script/FicsItReflection.FIRPropertyGetterFunc")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINPropertySetterFunc"), TEXT("/Script/FicsItReflection.FIRPropertySetterFunc")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINProperty"), TEXT("/Script/FicsItReflection.FIRProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINObjectProperty"), TEXT("/Script/FicsItReflection.FIRObjectProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINIntProperty"), TEXT("/Script/FicsItReflection.FIRIntProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINOperator"), TEXT("/Script/FicsItReflection.FIROperator")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINFunctionBadArgumentException"), TEXT("/Script/FicsItReflection.FIRFunctionBadArgumentException")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINFunction"), TEXT("/Script/FicsItReflection.FIRFunction")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINFuncProperty"), TEXT("/Script/FicsItReflection.FIRFuncProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINFloatProperty"), TEXT("/Script/FicsItReflection.FIRFloatProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINExecutionContext"), TEXT("/Script/FicsItReflection.FIRExecutionContext")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINClassProperty"), TEXT("/Script/FicsItReflection.FIRClassProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINClass"), TEXT("/Script/FicsItReflection.FIRClass")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINBoolProperty"), TEXT("/Script/FicsItReflection.FIRBoolProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINBase"), TEXT("/Script/FicsItReflection.FIRBase")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINArrayProperty"), TEXT("/Script/FicsItReflection.FIRArrayProperty")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINReflectionSource"), TEXT("/Script/FicsItReflection.FIRSource")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStaticReflectionSource"), TEXT("/Script/FicsItReflection.FIRSourceStatic")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINReflectionMetaRuntimeState "), TEXT("/Script/FicsItReflection.FIRMetaRuntimeState")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionBaseMeta"), TEXT("/Script/FicsItReflection.FIRBaseMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionPropertyMeta"), TEXT("/Script/FicsItReflection.FIRPropertyMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionFunctionParameterMeta"), TEXT("/Script/FicsItReflection.FIRFunctionParameterMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionFunctionMeta"), TEXT("/Script/FicsItReflection.FIRFunctionMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionSignalMeta"), TEXT("/Script/FicsItReflection.FIRSignalMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionTypeMeta"), TEXT("/Script/FicsItReflection.FIRTypeMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINReflectionClassMeta"), TEXT("/Script/FicsItReflection.FIRClassMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINUReflectionSource"), TEXT("/Script/FicsItReflection.FIRSourceUObject")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINRailroadSignalBlock"), TEXT("/Script/FicsItReflection.FIRRailroadSignalBlock")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINTargetPoint"), TEXT("/Script/FicsItReflection.FIRTargetPoint")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINTimeTableStop"), TEXT("/Script/FicsItReflection.FIRTimeTableStop")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINTrackGraph"), TEXT("/Script/FicsItReflection.FIRTrackGraph")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStaticReflectionHook"), TEXT("/Script/FicsItReflection.FIRStaticHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINFunctionHook"), TEXT("/Script/FicsItReflection.FIRFunctionHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINMultiFunctionHook"), TEXT("/Script/FicsItReflection.FIRMultiFunctionHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINBuildableHook"), TEXT("/Script/FicsItReflection.FIRBuildableHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINRailroadTrackHook"), TEXT("/Script/FicsItReflection.FIRRailroadTrackHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINTrainHook"), TEXT("/Script/FicsItReflection.FIRTrainHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINRailroadStationHook"), TEXT("/Script/FicsItReflection.FIRRailroadStationHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINRailroadSignalHook"), TEXT("/Script/FicsItReflection.FIRRailroadSignalHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINPipeHyperStartHook"), TEXT("/Script/FicsItReflection.FIRPipeHyperStartHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINFactoryConnectorHook"), TEXT("/Script/FicsItReflection.FIRFactoryConnectorHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINPipeConnectorHook"), TEXT("/Script/FicsItReflection.FIRPipeConnectorHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINPowerCircuitHook"), TEXT("/Script/FicsItReflection.FIRPowerCircuitHook")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINRailroadSwitchForce"), TEXT("/Script/FicsItReflection.FIRRailroadSwitchForce")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINFactoryConnectorSettings"), TEXT("/Script/FicsItReflection.FIRFactoryConnectorSettings")});

	FCoreRedirects::AddRedirectList(redirects, "FicsIt-Reflection");

	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
	SUBSCRIBE_METHOD(UFGRailroadTrackConnectionComponent::SetSwitchPosition, UFGRailroadTrackConnectionComponent_SetSwitchPosition_Hook);
	SUBSCRIBE_METHOD(UFGRailroadTrackConnectionComponent::AddConnection, UFGRailroadTrackConnectionComponent_AddConnection_Hook);
	SUBSCRIBE_METHOD(UFGRailroadTrackConnectionComponent::RemoveConnection, UFGRailroadTrackConnectionComponent_RemoveConnection_Hook);

	SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGRailroadTrackConnectionComponent::EndPlay, (void*)GetDefault<UFGRailroadTrackConnectionComponent>(), [](UActorComponent* self, EEndPlayReason::Type Reason) {
		if (Reason == EEndPlayReason::Destroyed && self->GetWorld()) {
			AFIRSubsystem* Subsystem = AFIRSubsystem::GetReflectionSubsystem(self);
			if (Subsystem) Subsystem->ForceRailroadSwitch(Cast<UFGRailroadTrackConnectionComponent>(self), -1);
		}
	});

	SUBSCRIBE_METHOD_VIRTUAL(UFGFactoryConnectionComponent::Factory_PeekOutput, GetDefault<UFGFactoryConnectionComponent>(), &UFGFactoryConnectionComponent_PeekOutput_Hook);
	SUBSCRIBE_METHOD_VIRTUAL(UFGFactoryConnectionComponent::Factory_Internal_PeekOutputInventory, GetDefault<UFGFactoryConnectionComponent>(), &UFGFactoryConnectionComponent_PeekOutput_Hook);
	SUBSCRIBE_METHOD_VIRTUAL(UFGFactoryConnectionComponent::Factory_GrabOutput, GetDefault<UFGFactoryConnectionComponent>(), &UFGFactoryConnectionComponent_GrabOutput_Hook);
	SUBSCRIBE_METHOD_VIRTUAL(UFGFactoryConnectionComponent::Factory_Internal_GrabOutputInventory, GetDefault<UFGFactoryConnectionComponent>(), &UFGFactoryConnectionComponent_InternalGrabOutputInventory_Hook);
#else
		FFIRGlobalRegisterHelper::Register();
		FFicsItReflectionModule::Get().PopulateSources();
		FFicsItReflectionModule::Get().LoadAllTypes();
#endif
	});
}

void FFicsItReflectionModule::ShutdownModule() {}

FFicsItReflectionModule& FFicsItReflectionModule::Get() {
	return FModuleManager::LoadModuleChecked<FFicsItReflectionModule>("FicsItReflection");
}

void FFicsItReflectionModule::PopulateSources() {
	Sources.Add(const_cast<UFIRSourceUObject*>(GetDefault<UFIRSourceUObject>()));
	Sources.Add(const_cast<UFIRSourceStatic*>(GetDefault<UFIRSourceStatic>()));
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

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFicsItReflectionModule, FicsItReflection)
