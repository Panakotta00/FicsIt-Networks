#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "FGFactoryConnectionComponent.h"
#include "Signals/FINSignalSender.h"
#include "FINCodeableMerger.generated.h"

UCLASS()
class AFINCodeableMerger : public AFGBuildableConveyorAttachment, public IFINSignalSender {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input2 = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input1 = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input3 = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Output1 = nullptr;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> OutputQueue;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue1;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue2;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue3;

	FCriticalSection Mutex;

	AFINCodeableMerger();
	~AFINCodeableMerger();

	//~ Begin IFGDismantleInterface
	virtual void GetDismantleRefund_Implementation( TArray< FInventoryStack >& out_refund, bool noBuildCostEnabled ) const override;
	// End IFGDismantleInterface
	
	// Begin IFINSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	virtual bool Factory_PeekOutput_Implementation(const class UFGFactoryConnectionComponent* connection, TArray< FInventoryItem >& out_items, TSubclassOf< UFGItemDescriptor > type) const override;
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// TODO: Upgrade Implementation
	// End AFGBuildable
	
private:
	/**
	 * This function is used in tick for internal handling of a input.
	 */
	void TickInput(UFGFactoryConnectionComponent* Connector, int InputID);
public:
	UFUNCTION()
	void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
		InternalName = TEXT("CodeableMerger");
		DisplayName = FText::FromString(TEXT("Codeable Merger"));
		PropertyInternalNames.Add("canOutput", "canOutput");
		PropertyDisplayNames.Add("canOutput", FText::FromString("Can Output"));
		PropertyDescriptions.Add("canOutput", FText::FromString("Is true if the output queue has a slot available for an item from one of the input queues."));
	}
	
	/**
	 * This function transfers the next item from the input queue with the given index to the output queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	bool netFunc_transferItem(int input);
	UFUNCTION()
	void netFuncMeta_transferItem(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "transferItem";
		DisplayName = FText::FromString("Transfer Item");
		Description = FText::FromString("Allows to transfer an item from the given input queue to the output queue if possible.");
		ParameterInternalNames.Add("input");
		ParameterDisplayNames.Add(FText::FromString("Input"));
		ParameterDescriptions.Add(FText::FromString("The index of the input queue you want to transfer the next item to the output queue. (0 = right, 1 = middle, 2 = left)"));
		ParameterInternalNames.Add("transfered");
		ParameterDisplayNames.Add(FText::FromString("Transfered"));
		ParameterDescriptions.Add(FText::FromString("true if it was able to transfer the item."));
		Runtime = 1;
	}

	/**
	 * Allows to peek the next item at the input queue with the given index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	FInventoryItem netFunc_getInput(int input);
	UFUNCTION()
	void netFuncMeta_getInput(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getInput";
		DisplayName = FText::FromString("Get Input");
		Description = FText::FromString("Returns the next item in the given input queue.");
		ParameterInternalNames.Add("input");
		ParameterDisplayNames.Add(FText::FromString("Input"));
		ParameterDescriptions.Add(FText::FromString("The index of the input queue you want to check (0 = right, 1 = middle, 2 = left)"));
		ParameterInternalNames.Add("item");
		ParameterDisplayNames.Add(FText::FromString("Item"));
		ParameterDescriptions.Add(FText::FromString("The next item in the input queue."));
		Runtime = 1;
	}

	/**
	 * Checks if the output queue with is able to contain one more item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	bool netPropGet_canOutput();

	/**
	 * This signal gets emit when a new item got pushed to the input queue with the given index.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Network|Components|CodeableSplitter")
	void netSig_ItemRequest(int input, FInventoryItem item);
	UFUNCTION()
    void netSigMeta_ItemRequest(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions) {
		InternalName = "ItemRequest";
		DisplayName = FText::FromString("Item Request");
		Description = FText::FromString("Triggers when a new item is ready in one of the input queues.");
		ParameterInternalNames.Add("input");
		ParameterDisplayNames.Add(FText::FromString("Input"));
		ParameterDescriptions.Add(FText::FromString("The index of the input queue at which the item is ready."));
		ParameterInternalNames.Add("item");
		ParameterDisplayNames.Add(FText::FromString("Item"));
		ParameterDescriptions.Add(FText::FromString("The new item in the input queue."));
	}
	
	/**
	 * This signal gets emitted when a item is popped from the output queue (aka it got outputted to an conveyor)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Network|Components|CodeableSplitter")
	void netSig_ItemOutputted(FInventoryItem item);
	UFUNCTION()
    void netSigMeta_ItemOutputted(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions) {
		InternalName = "ItemOutputted";
		DisplayName = FText::FromString("Item Outputted");
		Description = FText::FromString("Triggers when an item is popped from the output queue (aka it got transferred to a conveyor).");
		ParameterInternalNames.Add("item");
		ParameterDisplayNames.Add(FText::FromString("Item"));
		ParameterDescriptions.Add(FText::FromString("The item removed from the output queue."));
	}
	
	TArray<FInventoryItem>& GetInput(int input);
	TArray<FInventoryItem>& GetInput(UFGFactoryConnectionComponent* connection);
	const TArray<FInventoryItem>& GetInput(const UFGFactoryConnectionComponent* connection) const;
};
