﻿#include "CoreMinimal.h"
#include "Reflection/Source/FIRSourceStaticMacros.h"
#include "FINNetworkComponent.h"
#include "FINNetworkConnectionComponent.h"
#include "FINNetworkUtils.h"

ExtendClass(UObject)
BeginProp(RString, nick, "Nick", "**Only available for Network Components!** Allows access to the Network Components Nick.") {
	UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(self);
	if (NetworkHandler) {
		FIRReturn IFINNetworkComponent::Execute_GetNick(NetworkHandler);
	} else {
		FIRReturn FIRAny();
	}
} PropSet() {
	UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(self);
	if (NetworkHandler) {
		IFINNetworkComponent::Execute_SetNick(NetworkHandler, Val);
	}
} EndProp()
BeginProp(RString, id, "ID", "**Only available for Network Components!** Allows access to the Network Components UUID.") {
	UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(self);
	if (NetworkHandler) {
		FIRReturn IFINNetworkComponent::Execute_GetID(NetworkHandler).ToString();
	} else {
		FIRReturn FIRAny();
	}
} EndProp()
BeginProp(RBool, isNetworkComponent, "Is Network Component", "True if this object is a network component and has a id and nick.") {
	UObject* NetworkHandler = UFINNetworkUtils::FindNetworkComponentFromObject(self);
	FIRReturn (NetworkHandler != nullptr);
} EndProp()
EndClass()

ExtendClass(AActor)
BeginFunc(getNetworkConnectors, "Get Network Connectors", "Returns the name of network connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFINNetworkConnectionComponent>>, connectors, "Connectors", "The factory connectors this actor has.")
	Body()
	FIRArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFINNetworkConnectionComponent::StaticClass())) continue;
		UObject* connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(connector))) continue;
		Output.Add(Ctx.GetTrace() / connector);
	}
	connectors = Output;
} EndFunc()
EndClass()
