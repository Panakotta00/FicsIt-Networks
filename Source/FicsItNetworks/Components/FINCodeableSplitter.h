#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "FGFactoryConnectionComponent.h"
#include "Network/FINNetworkConnector.h"
#include "FINCodeableSplitter.generated.h"

UCLASS()
class AFINCodeableSplitter : public AFGBuildableConveyorAttachment, public IFINSignalSender {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UFINNetworkConnector* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UFGFactoryConnectionComponent* InputConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UFGFactoryConnectionComponent* Output1 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UFGFactoryConnectionComponent* Output2 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UFGFactoryConnectionComponent* Output3 = nullptr;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> InputQueue;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> OutputQueue1;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> OutputQueue2;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> OutputQueue3;

	UPROPERTY(SaveGame)
		TSet<FFINNetworkTrace> SignalListeners;

	AFINCodeableSplitter();
	~AFINCodeableSplitter();

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

	/**
	 * This function transfers the next item from the input queue to the output queue with the given index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
		bool netFunc_transferItem(int output);

	/**
	 * Allows to peek the next item at the input queue waiting to get transfered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
		UClass* netFunc_getInput();

	/**
	 * Checks if the output queue witht the given index is able to contain one more item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
		bool netFunc_canOutput(int output);

	/**
	 * This signal gets emit when a new item got pushed to the input queue.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Network|Components|CodeableSplitter")
		void netSig_ItemRequest(UClass* item);

	TArray<FInventoryItem>& GetOutput(int output);
	TArray<FInventoryItem>& GetOutput(UFGFactoryConnectionComponent* connection);
	const TArray<FInventoryItem>& GetOutput(const UFGFactoryConnectionComponent* connection) const;
};
