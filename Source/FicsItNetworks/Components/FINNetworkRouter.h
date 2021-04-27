#pragma once

#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FINNetworkRouter.generated.h"

UENUM()
enum EFINNetworkRouterLampFlags {
	FIN_NetRouter_None		= 0b0000,
	FIN_NetRouter_Con1_Rx	= 0b0001,
	FIN_NetRouter_Con1_Tx	= 0b0010,
	FIN_NetRouter_Con1		= 0b0011,
	FIN_NetRouter_Con2_Rx	= 0b0100,
	FIN_NetRouter_Con2_Tx	= 0b1000,
	FIN_NetRouter_Con2		= 0b1100,
};
ENUM_CLASS_FLAGS(EFINNetworkRouterLampFlags);

UCLASS()
class AFINNetworkRouter : public AFGBuildable {
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector2;

	UPROPERTY(SaveGame)
	bool bIsPortWhitelist = false;
	
	UPROPERTY(SaveGame)
	TArray<int> PortList;

	UPROPERTY(SaveGame)
	bool bIsAddrWhitelist = false;

	UPROPERTY(SaveGame)
	TArray<FString> AddrList;

	UPROPERTY()
	TArray<FGuid> HandledMessages;
	FCriticalSection HandleMessageMutex;

	EFINNetworkRouterLampFlags LampFlags;

	AFINNetworkRouter();
	~AFINNetworkRouter();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor
	
	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
		InternalName = TEXT("NetworkRouter");
		DisplayName = FText::FromString(TEXT("Network Router"));
		PropertyInternalNames.Add("isPortWhitelist", "isPortWhitelist");
		PropertyDisplayNames.Add("isPortWhitelist", FText::FromString("Is Port Whitelist"));
		PropertyDescriptions.Add("isPortWhitelist", FText::FromString("True if the port filter list is used as whitelist."));
		PropertyInternalNames.Add("isAddrWhitelist", "isAddrWhitelist");
		PropertyDisplayNames.Add("isAddrWhitelist", FText::FromString("Is Address Whitelist"));
		PropertyDescriptions.Add("isAddrWhitelist", FText::FromString("True if the address filter list is used as whitelist."));
	}

	UFUNCTION()
	void netPropSet_isWhitelist(bool bInWhitelist);
	UFUNCTION()
	bool netPropGet_isWhitelist();

	UFUNCTION()
	void netFunc_addPortList(int port);
	UFUNCTION()
    void netFuncMeta_addPortList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "addPortList";
		DisplayName = FText::FromString("Add Port to List");
		Description = FText::FromString("Adds a given port to the port filter list.");
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port you want to add to the list."));
		Runtime = 1;
	}
	
	UFUNCTION()
	void netFunc_removePortList(int port);
	UFUNCTION()
    void netFuncMeta_removePortList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "removePortList";
		DisplayName = FText::FromString("Remove Port from List");
		Description = FText::FromString("Removes the given port from the port filter list.");
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port you want to remove from the list."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_setPortList(const TArray<int>& inPortList);
	UFUNCTION()
    void netFuncMeta_setPortList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setPortList";
		DisplayName = FText::FromString("Set Port List");
		Description = FText::FromString("Overrides the port filter list with the given array.");
		ParameterInternalNames.Add("ports");
		ParameterDisplayNames.Add(FText::FromString("Ports"));
		ParameterDescriptions.Add(FText::FromString("The port array you want to override the filter list with."));
		Runtime = 1;
	}

	UFUNCTION()
	TArray<int> netFunc_getPortList();
	UFUNCTION()
    void netFuncMeta_getPortList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getPortList";
		DisplayName = FText::FromString("Get Port List");
		Description = FText::FromString("Allows to get all the ports of the port filter list as array.");
		ParameterInternalNames.Add("ports");
		ParameterDisplayNames.Add(FText::FromString("Ports"));
		ParameterDescriptions.Add(FText::FromString("The port array of the filter list."));
		Runtime = 1;
	}

	UFUNCTION()
	void netPropSet_isAddrWhitelist(bool bInWhitelist);
	UFUNCTION()
	bool netPropGet_isAddrWhitelist();

	UFUNCTION()
	void netFunc_addAddrList(const FString& addr);
	void netFuncMeta_addAddrList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "addAddrList";
		DisplayName = FText::FromString("Add Address to List");
		Description = FText::FromString("Adds a given address to the address filter list.");
		ParameterInternalNames.Add("addr");
		ParameterDisplayNames.Add(FText::FromString("Address"));
		ParameterDescriptions.Add(FText::FromString("The address you want to add to the list."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_removeAddrList(const FString& addr);
	UFUNCTION()
    void netFuncMeta_removeAddrList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "removeAddrList";
		DisplayName = FText::FromString("Remove Address from List");
		Description = FText::FromString("Removes the given address from the address filter list.");
		ParameterInternalNames.Add("addr");
		ParameterDisplayNames.Add(FText::FromString("Address"));
		ParameterDescriptions.Add(FText::FromString("The address you want to remove from the list."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_setAddrList(const TArray<FString>& inAddrList);
	UFUNCTION()
    void netFuncMeta_setAddrList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setAddrList";
		DisplayName = FText::FromString("Set Address List");
		Description = FText::FromString("Overrides the address filter list with the given array.");
		ParameterInternalNames.Add("addresses");
		ParameterDisplayNames.Add(FText::FromString("Addresses"));
		ParameterDescriptions.Add(FText::FromString("The address array you want to override the filter list with."));
		Runtime = 1;
	}

	UFUNCTION()
	TArray<FString> netFunc_getAddrList();
	UFUNCTION()
    void netFuncMeta_getAddrList(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getAddrList";
		DisplayName = FText::FromString("Get Address List");
		Description = FText::FromString("Allows to get all the addresses of the address filter list as array.");
		ParameterInternalNames.Add("addresses");
		ParameterDisplayNames.Add(FText::FromString("Addresses"));
		ParameterDescriptions.Add(FText::FromString("The address array of the filter list."));
		Runtime = 1;
	}

	UFUNCTION(BlueprintImplementableEvent)
    void OnMessageHandled(bool bCon1or2, bool bSendOrReceive);
	
private:
	bool HandleMessage(AFINNetworkCircuit* SendingCircuit, const FGuid& ID, const FGuid& Sender, const FGuid& Reciever, int Port, const TArray<FFINAnyNetworkValue>& Data);

	UFUNCTION(NetMulticast, Unreliable)
    void NetMulti_OnMessageHandled(EFINNetworkRouterLampFlags Flags);
};
