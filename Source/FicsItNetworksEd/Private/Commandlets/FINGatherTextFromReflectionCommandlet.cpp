#include "FINGatherTextFromReflectionCommandlet.h"

#include "FicsItNetworks/Public/Reflection/FINReflection.h"
#include "Internationalization/InternationalizationManifest.h"

UFINGatherTextFromReflectionCommandlet::UFINGatherTextFromReflectionCommandlet() {
	IsEditor = true;
}

int32 UFINGatherTextFromReflectionCommandlet::Main(const FString& Params) {
	for (const TTuple<UClass*, UFINClass*>& Entry : FFINReflection::Get()->GetClasses()) {
		GatherClass(Entry.Value);
	}
	for (const TTuple<UScriptStruct*, UFINStruct*>& Entry : FFINReflection::Get()->GetStructs()) {
		GatherStruct(Entry.Value);
	}
	
	return 0;
}

FManifestContext BaseField(UFINBase* Base, FString Field) {
	while (Base) {
		Field = Base->GetInternalName() + TEXT("_") + Field;
		Base = Cast<UFINBase>(Base->GetOuter());
	}
	FManifestContext Context;
	Context.Key = FLocKey(Field);
	return Context;
}

void UFINGatherTextFromReflectionCommandlet::GatherStruct(UFINStruct* Struct) {
	if (!(Struct->GetStructFlags() & FIN_Struct_StaticSource)) return;
	GatherBase(Struct);
	for (UFINProperty* prop : Struct->GetProperties(false)) {
		GatherProperty(prop);
	}
	for (UFINFunction* func : Struct->GetFunctions(false)) {
		GatherFunction(func);
	}
}

void UFINGatherTextFromReflectionCommandlet::GatherClass(UFINClass* Class) {
	if (!(Class->GetStructFlags() & FIN_Struct_StaticSource)) return;
	GatherStruct(Class);
	for (UFINSignal* sig : Class->GetSignals(false)) {
		GatherSignal(sig);
	}
}

void UFINGatherTextFromReflectionCommandlet::GatherBase(UFINBase* Base) {
	GatherManifestHelper->AddSourceText(Namespace, FLocItem(Base->GetDisplayName().ToString()), BaseField(Base, TEXT("DisplayName")));
	GatherManifestHelper->AddSourceText(Namespace, FLocItem(Base->GetDescription().ToString()), BaseField(Base, TEXT("Description")));
}

void UFINGatherTextFromReflectionCommandlet::GatherProperty(UFINProperty* Property) {
	if (!(Property->GetPropertyFlags() & FIN_Prop_StaticSource)) return;
	GatherBase(Property);
}

void UFINGatherTextFromReflectionCommandlet::GatherFunction(UFINFunction* Function) {
	if (!(Function->GetFunctionFlags() & FIN_Func_StaticSource)) return;
	GatherBase(Function);
	for (UFINProperty* param : Function->GetParameters()) {
		GatherProperty(param);
	}
}

void UFINGatherTextFromReflectionCommandlet::GatherSignal(UFINSignal* Signal) {
	if (!(Signal->GetSignalFlags() & FIN_Signal_StaticSource)) return;
	GatherBase(Signal);
	for (UFINProperty* param : Signal->GetParameters()) {
		GatherProperty(param);
	}
}

bool UFINGatherTextFromReflectionCommandlet::ShouldRunInPreview(const TArray<FString>& Switches, const TMap<FString, FString>& ParamVals) const {
	return Super::ShouldRunInPreview(Switches, ParamVals);
}
