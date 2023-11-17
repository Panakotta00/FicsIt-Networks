#pragma once

#include "Network/FINNetworkCircuitNode.h"
#include "Components/Widget.h"
#include "FINPciDeviceInterface.generated.h"

UINTERFACE(Blueprintable)
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