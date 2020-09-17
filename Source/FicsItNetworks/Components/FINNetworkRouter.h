#pragma once

#include "FGBuildable.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Network/FINNetworkMessageInterface.h"
#include "FINNetworkRouter.generated.h"

UCLASS()
class AFINNetworkRouter : public AFGBuildable {
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector2;

	UPROPERTY()
	bool bIsPortWhitelist = false;
	
	UPROPERTY(SaveGame)
	TArray<int> PortList;

	UPROPERTY(SaveGame)
	bool bIsAddrWhitelist = false;

	UPROPERTY(SaveGame)
	TArray<FString> AddrList;

	UPROPERTY()
	TArray<FGuid> HandledMessages;

	AFINNetworkRouter();
	~AFINNetworkRouter();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	UFUNCTION()
	void netFunc_setPortWhitelist(bool bInWhitelist);

	UFUNCTION()
	bool netFunc_getPortWhitelist();

	UFUNCTION()
	void netFunc_addPortList(int port);

	UFUNCTION()
	void netFunc_removePortList(int port);

	UFUNCTION()
	void netFunc_setPortList(const TArray<int>& inPortList);

	UFUNCTION()
	TArray<int> netFunc_getPortList();

	UFUNCTION()
	void netFunc_setAddrWhitelist(bool bInWhitelist);

	UFUNCTION()
	bool netFunc_getAddrWhitelist();

	UFUNCTION()
	void netFunc_addAddrList(const FString& addr);

	UFUNCTION()
	void netFunc_removeAddrList(const FString& addr);

	UFUNCTION()
	void netFunc_setAddrList(const TArray<FString>& inAddrList);

	UFUNCTION()
	TArray<FString> netFunc_getAddrList();

	UFUNCTION(BlueprintImplementableEvent)
    void OnMessageHandled(bool bCon1or2, bool bSendOrReceive);
	
private:
	bool HandleMessage(AFINNetworkCircuit* SendingCircuit, FGuid ID, FFINNetworkTrace Sender, FGuid Reciever, int Port, const TFINDynamicStruct<FFINParameterList>& Data);

	UFUNCTION(NetMulticast, Unreliable)
    void NetMulti_OnMessageHandled(bool bCon1or2, bool bSendOrReceive);
};
