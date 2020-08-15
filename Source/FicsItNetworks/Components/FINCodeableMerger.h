#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "FGFactoryConnectionComponent.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Network/FINNetworkCustomType.h"

#include "FINCodeableMerger.generated.h"

UCLASS()
class AFINCodeableMerger : public AFGBuildableConveyorAttachment, public IFINSignalSender, public IFINNetworkCustomType {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Output1 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input1 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input2 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input3 = nullptr;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> OutputQueue;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue1;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue2;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue3;

	UPROPERTY(SaveGame)
	TSet<FFINNetworkTrace> SignalListeners;

	AFINCodeableMerger();
	~AFINCodeableMerger();

	//~ Begin IFGDismantleInterface
	virtual void GetDismantleRefund_Implementation( TArray< FInventoryStack >& out_refund ) const override;
	// End IFGDismantleInterface
	
	// Begin IFINSignalSender
	virtual void AddListener_Implementation(FFINNetworkTrace listener) override;
	virtual void RemoveListener_Implementation(FFINNetworkTrace listener) override;
	virtual TSet<FFINNetworkTrace> GetListeners_Implementation() override;
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	virtual bool Factory_PeekOutput_Implementation(const class UFGFactoryConnectionComponent* connection, TArray< FInventoryItem >& out_items, TSubclassOf< UFGItemDescriptor > type) const override;
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// TODO: Upgrade Implementation
	// End AFGBuildable
	
	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("CodeableMerger"); }
	// End IFINNetworkCustomType

private:
	/**
	 * This function is used in tick for internal handling of a input.
	 */
	void TickInput(UFGFactoryConnectionComponent* Connector, int InputID);
public:
	
	/**
	 * This function transfers the next item from the input queue with the given index to the output queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	bool netFunc_transferItem(int input);

	/**
	 * Allows to peek the next item at the input queue with the given index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	FInventoryItem netFunc_getInput(int input);

	/**
	 * Checks if the output queue with is able to contain one more item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	bool netFunc_canOutput();

	/**
	 * This signal gets emit when a new item got pushed to the input queue with the given index.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Network|Components|CodeableSplitter")
	void netSig_ItemRequest(int input, const FInventoryItem& item);
	
	TArray<FInventoryItem>& GetInput(int input);
	TArray<FInventoryItem>& GetInput(UFGFactoryConnectionComponent* connection);
	const TArray<FInventoryItem>& GetInput(const UFGFactoryConnectionComponent* connection) const;
};
