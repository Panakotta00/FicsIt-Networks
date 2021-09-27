#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "FGFactoryConnectionComponent.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FINCodeableSplitter.generated.h"

UCLASS()
class AFINCodeableSplitter : public AFGBuildableConveyorAttachment, public IFINSignalSender {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* NetworkConnector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Output2 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Output1 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Output3 = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFGFactoryConnectionComponent* Input1 = nullptr;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> InputQueue;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> OutputQueue1;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> OutputQueue2;

	UPROPERTY(SaveGame)
	TArray<FInventoryItem> OutputQueue3;

	AFINCodeableSplitter();
	~AFINCodeableSplitter();

	// Begin AActor
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	// End AActor

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	virtual bool Factory_PeekOutput_Implementation(const class UFGFactoryConnectionComponent* connection, TArray< FInventoryItem >& out_items, TSubclassOf< UFGItemDescriptor > type) const override;
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
	// TODO: Upgrade Implementation
	// End AFGBuildable

	//~ Begin IFGDismantleInterface
	virtual void GetDismantleRefund_Implementation( TArray< FInventoryStack >& out_refund ) const override;
	// End IFGDismantleInterface
	
	// Begin IFGSaveGame
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	// End IFGSaveGame

	// Begin IFINSignalSender
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
		InternalName = TEXT("CodeableSplitter");
		DisplayName = FText::FromString(TEXT("Codeable Splitter"));
	}
	
	/**
	 * This function transfers the next item from the input queue to the output queue with the given index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	bool netFunc_transferItem(int output);
	UFUNCTION()
    void netFuncMeta_transferItem(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "transferItem";
		DisplayName = FText::FromString("Transfer Item");
		Description = FText::FromString("Allows to transfer an item from the input queue to the given output queue if possible.");
		ParameterInternalNames.Add("output");
		ParameterDisplayNames.Add(FText::FromString("Output"));
		ParameterDescriptions.Add(FText::FromString("The index of the output queue you want to transfer the next item to (0 = middle, 1 = left, 2 = right)"));
		ParameterInternalNames.Add("transfered");
		ParameterDisplayNames.Add(FText::FromString("Transfered"));
		ParameterDescriptions.Add(FText::FromString("true if it was able to transfer the item."));
		Runtime = 1;
	}

	/**
	 * Allows to peek the next item at the input queue waiting to get transfered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	FInventoryItem netFunc_getInput();
	UFUNCTION()
    void netFuncMeta_getInput(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getInput";
		DisplayName = FText::FromString("Get Input");
		Description = FText::FromString("Returns the next item in the input queue.");
		ParameterInternalNames.Add("item");
		ParameterDisplayNames.Add(FText::FromString("Item"));
		ParameterDescriptions.Add(FText::FromString("The next item in the input queue."));
		Runtime = 1;
	}

	/**
	 * Checks if the output queue witht the given index is able to contain one more item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Components|CodeableSplitter")
	bool netFunc_canOutput(int output);
	UFUNCTION()
    void netFuncMeta_canOutput(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "canOutput";
		DisplayName = FText::FromString("Can Output");
		Description = FText::FromString("Allows to check if we can transfer an item to the given output queue.");
		ParameterInternalNames.Add("output");
		ParameterDisplayNames.Add(FText::FromString("Output"));
		ParameterDescriptions.Add(FText::FromString("The index of the output queue you want to check (0 = middle, 1 = left, 2 = right)"));
		ParameterInternalNames.Add("canTransfer");
		ParameterDisplayNames.Add(FText::FromString("Can Transfer"));
		ParameterDescriptions.Add(FText::FromString("True if you could transfer an item to the given output queue."));
		Runtime = 0;
	}

	/**
	 * This signal gets emit when a new item got pushed to the input queue.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Network|Components|CodeableSplitter")
	void netSig_ItemRequest(const FInventoryItem& Item);
	UFUNCTION()
    void netSigMeta_ItemRequest(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions) {
		InternalName = "ItemRequest";
		DisplayName = FText::FromString("Item Request");
		Description = FText::FromString("Triggers when a new item is ready in the input queue.");
		ParameterInternalNames.Add("item");
		ParameterDisplayNames.Add(FText::FromString("Item"));
		ParameterDescriptions.Add(FText::FromString("The new item in the input queue."));
	}
	
	/**
	 * This signal gets emitted when a item is popped from one of the output queues (aka it got outputted to an conveyor)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Network|Components|CodeableSplitter")
    void netSig_ItemOutputted(int output, const FInventoryItem& item);
	UFUNCTION()
    void netSigMeta_ItemOutputted(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions) {
		InternalName = "ItemOutputted";
		DisplayName = FText::FromString("Item Outputted");
		Description = FText::FromString("Triggers when an item is popped from on of the output queues (aka it got transferred to a conveyor).");
		ParameterInternalNames.Add("output");
		ParameterDisplayNames.Add(FText::FromString("Output"));
		ParameterDescriptions.Add(FText::FromString("The index of the output queue from which the item got removed."));
		ParameterInternalNames.Add("item");
		ParameterDisplayNames.Add(FText::FromString("Item"));
		ParameterDescriptions.Add(FText::FromString("The item removed from the output queue."));
	}

	TArray<FInventoryItem>& GetOutput(int output);
	TArray<FInventoryItem>& GetOutput(UFGFactoryConnectionComponent* connection, int32* Index = nullptr);
	const TArray<FInventoryItem>& GetOutput(const UFGFactoryConnectionComponent* connection, int32* Index = nullptr) const;
};
