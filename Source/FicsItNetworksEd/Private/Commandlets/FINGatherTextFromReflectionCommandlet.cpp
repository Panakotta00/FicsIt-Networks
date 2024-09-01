#include "FINGatherTextFromReflectionCommandlet.h"

#include "FicsItNetworks/Public/Reflection/FINReflection.h"
#include "Internationalization/InternationalizationManifest.h"

UFINGatherTextFromReflectionCommandlet::UFINGatherTextFromReflectionCommandlet() {
	IsEditor = true;
}

int32 UFINGatherTextFromReflectionCommandlet::Main(const FString& Params) {
	for (const TTuple<UClass*, UFIRClass*>& Entry : FFINReflection::Get()->GetClasses()) {
		GatherClass(Entry.Value);
	}
	for (const TTuple<UScriptStruct*, UFIRStruct*>& Entry : FFINReflection::Get()->GetStructs()) {
		GatherStruct(Entry.Value);
	}
	
	return 0;
}

FManifestContext BaseField(UFIRBase* Base, FString Field) {
	while (Base) {
		Field = Base->GetInternalName() + TEXT("_") + Field;
		Base = Cast<UFIRBase>(Base->GetOuter());
	}
	FManifestContext Context;
	Context.Key = FLocKey(Field);
	return Context;
}

void UFINGatherTextFromReflectionCommandlet::GatherStruct(UFIRStruct* Struct) {
	if (!(Struct->GetStructFlags() & FIN_Struct_StaticSource)) return;
	GatherBase(Struct);
	for (UFIRProperty* prop : Struct->GetProperties(false)) {
		GatherProperty(prop);
	}
	for (UFIRFunction* func : Struct->GetFunctions(false)) {
		GatherFunction(func);
	}
}

void UFINGatherTextFromReflectionCommandlet::GatherClass(UFIRClass* Class) {
	if (!(Class->GetStructFlags() & FIN_Struct_StaticSource)) return;
	GatherStruct(Class);
	for (UFIRSignal* sig : Class->GetSignals(false)) {
		GatherSignal(sig);
	}
}

void UFINGatherTextFromReflectionCommandlet::GatherBase(UFIRBase* Base) {
	GatherManifestHelper->AddSourceText(Namespace, FLocItem(Base->GetDisplayName().ToString()), BaseField(Base, TEXT("DisplayName")));
	GatherManifestHelper->AddSourceText(Namespace, FLocItem(Base->GetDescription().ToString()), BaseField(Base, TEXT("Description")));
}

void UFINGatherTextFromReflectionCommandlet::GatherProperty(UFIRProperty* Property) {
	if (!(Property->GetPropertyFlags() & FIN_Prop_StaticSource)) return;
	GatherBase(Property);
}

void UFINGatherTextFromReflectionCommandlet::GatherFunction(UFIRFunction* Function) {
	if (!(Function->GetFunctionFlags() & FIN_Func_StaticSource)) return;
	GatherBase(Function);
	for (UFIRProperty* param : Function->GetParameters()) {
		GatherProperty(param);
	}
}

void UFINGatherTextFromReflectionCommandlet::GatherSignal(UFIRSignal* Signal) {
	if (!(Signal->GetSignalFlags() & FIN_Signal_StaticSource)) return;
	GatherBase(Signal);
	for (UFIRProperty* param : Signal->GetParameters()) {
		GatherProperty(param);
	}
}

bool UFINGatherTextFromReflectionCommandlet::ShouldRunInPreview(const TArray<FString>& Switches, const TMap<FString, FString>& ParamVals) const {
	return Super::ShouldRunInPreview(Switches, ParamVals);
}
