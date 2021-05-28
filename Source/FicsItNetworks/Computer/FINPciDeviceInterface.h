#pragma once

#include "Components/Widget.h"
#include "FicsItNetworks/Network/FINNetworkCircuitNode.h"
#include "FINPciDeviceInterface.generated.h"

UINTERFACE()
class FICSITNETWORKS_API UFINPciDeviceInterface : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKS_API IFINPciDeviceInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Computer|PCI")
	bool HasPCIWidget() const;
	virtual bool HasPCIWidget_Implementation() const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Computer|PCI")
	bool NeedsPCINetworkConnection() const;
	virtual bool NeedsPCINetworkConnection_Implementation() const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Computer|PCI")
	void SetPCINetworkConnection(const TScriptInterface<IFINNetworkCircuitNode>& InNode);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Computer|PCI")
	UWidget* CreatePCIWidget(APlayerController* OwningPlayer);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Computer|PCI")
	FString GetPCIWidgetName();
};