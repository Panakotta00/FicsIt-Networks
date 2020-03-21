#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableAttachmentSplitter.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "Network/FINNetworkConnector.h"
#include "FINCodeableSplitter.generated.h"

UCLASS(Blueprintable)
class AFINCodeableSplitter : public AFGBuildableAttachmentSplitter {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CodeableSplitter")
		UFINNetworkConnector* NetworkConnector;

	UPROPERTY(VisibleAnywhere, BlueprintREadWrite, Category = "CodeableSplitter")
		UFGFactoryConnectionComponent* Input;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CodeableSplitter")
		UFGFactoryConnectionComponent* Output1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CodeableSplitter")
		UFGFactoryConnectionComponent* Output2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CodeableSplitter")
		UFGFactoryConnectionComponent* Output3;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> InputQueue;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> OutputQueue1;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> OutputQueue2;

	UPROPERTY(SaveGame)
		TArray<FInventoryItem> OutputQueue3;

	AFINCodeableSplitter();

	// Begin AFGBuildable
	virtual void Factory_Tick(float dt) override;
	virtual bool Factory_PeekOutput_Implementation(const class UFGFactoryConnectionComponent* connection, TArray< FInventoryItem >& out_items, TSubclassOf< UFGItemDescriptor > type) const override;
	virtual bool Factory_GrabOutput_Implementation(class UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf< UFGItemDescriptor > type) override;
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
