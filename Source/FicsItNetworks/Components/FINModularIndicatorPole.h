#pragma once

#include "FINModularIndicatorPoleModule.h"
#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FINModularIndicatorPole.generated.h"


UCLASS()
class AFINModularIndicatorPole : public AFGBuildable {
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* NormalBaseMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* NormalExtensionMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* NormalAttachmentMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* VerticalBaseMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* VerticalExtensionMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* VerticalAttachmentMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* ConnectorMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly, SaveGame, Replicated)
	int Extension = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* Connector;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector ModuleConnectionPoint;

	UPROPERTY(EditDefaultsOnly)
	FVector ConnectorLocation = FVector();

	UPROPERTY(SaveGame, Replicated)
	AFINModularIndicatorPoleModule* ChildModule;
	
	UPROPERTY()
	bool bHasChanged = false;

	UPROPERTY(EditDefaultsOnly, SaveGame, Replicated)
	bool Vertical = false;
	
	AFINModularIndicatorPole();
	
	// Begin AActor
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFGDismantleInterface
	virtual int32 GetDismantleRefundReturnsMultiplier() const override;
	// End IFGDismantleInterface

	virtual void GetChildDismantleActors_Implementation(TArray<AActor*>& out_ChildDismantleActors) const override;

	static void SpawnComponents(TSubclassOf<UStaticMeshComponent> Class, int Extension, bool IsVertical,
	                            UStaticMesh* BaseMesh,
	                            UStaticMesh* ExtMesh,
	                            UStaticMesh* AtachMesh,
	                            UStaticMesh* ConnectorMesh,
	                            AActor* Parent, USceneComponent* Attach,
	                            TArray<UStaticMeshComponent*>& OutParts);
	
	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
		InternalName = TEXT("ModularIndicatorPole");
		DisplayName = FText::FromString(TEXT("Modular Indicator Pole"));
	}
	
	UFUNCTION()
	AFINModularIndicatorPoleModule* netFunc_getNext();
	UFUNCTION()
	void netFuncMeta_getNext(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getNext";
		DisplayName = FText::FromString("Get Next");
		Description = FText::FromString("Returns the next pole module if any.");
		ParameterInternalNames.Add("next");
		ParameterDisplayNames.Add(FText::FromString("Next module"));
		ParameterDescriptions.Add(FText::FromString("The next module in this chain."));
		Runtime = 1;
	}

	UFUNCTION()
	AFINModularIndicatorPoleModule* netFunc_getModule(int Index);

	UFUNCTION()
	void netFuncMeta_getModule(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getModule";
		DisplayName = FText::FromString("Get Module");
		Description = FText::FromString("Gets the module at the given position in the stack");
		ParameterInternalNames.Add("module");
		ParameterDisplayNames.Add(FText::FromString("Module"));
		ParameterDescriptions.Add(FText::FromString("The module at the given offset in the stack or nil if none"));
		ParameterInternalNames.Add("index");
		ParameterDisplayNames.Add(FText::FromString("Module Offset"));
		ParameterDescriptions.Add(FText::FromString("The index in the stack, 0 being the first module"));
	}
};
