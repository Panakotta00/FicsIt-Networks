#include "FIVSUtils.h"

#include "FicsItReflection.h"
#include "FINNetworkComponent.h"

FString FIRObjectToString(UObject* InObj) {
	if (!InObj) return TEXT("Nil");
	FString Text = FFicsItReflectionModule::Get().FindClass(InObj->GetClass())->GetInternalName();
	if (InObj->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
		Text += TEXT(" ") + IFINNetworkComponent::Execute_GetID(InObj).ToString();
		FString Nick = IFINNetworkComponent::Execute_GetNick(InObj);
		if (Nick.Len() > 0) {
			Text += TEXT(" '") + Nick + TEXT("'");
		}
	}
	return Text;
}

FString FIRClassToString(UClass* InClass) {
	if (!InClass) return TEXT("Nil");
	return FFicsItReflectionModule::Get().FindClass(InClass)->GetInternalName() + TEXT("-Type");
}
