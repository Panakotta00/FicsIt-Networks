#pragma once
#include "FINCommandPointMesh.h"
#include "FINModuleBase.h"
#include "Containers/Array.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "FINArrowModuleBase.generated.h"


UCLASS()
class AFINArrowModuleBase : public AFINModuleBase{
	GENERATED_BODY()
	public:

	UPROPERTY(EditAnywhere, BlueprintType, BlueprintReadWrite, Replicated, SaveGame, meta=(NoResetToDefault))
	TArray<FFINPanelArrowAnchor> Anchors;

	TArray<FFINPanelArrowAnchor> LastConfiguredAnchors;
	
	bool HasConfiguration = false;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* CenterDotMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* CenterBridgeMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* CenterButtMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* CenterCapRoundMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* CenterCapSquareMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* CenterMiterMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* TipBlockedMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* TipRecessedBlockedMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* TipArrowMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* TipStraightMesh = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* TipExtendedStraightMesh = nullptr;

	AFINArrowModuleBase();

	

	UFUNCTION(BlueprintCallable)
	void RebuildComponents(AActor* Parent, USceneComponent* Attach, TArray<FFINPanelArrowAnchor>& OutAnchors, TArray<UStaticMeshComponent*>& OutParts);
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& transform) override;
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
private:
	static UStaticMeshComponent* CreateAndAddComponent(UStaticMesh* Mesh, AActor* Parent, USceneComponent* Attach,
	                                                   FVector Location, FRotator Rotation, TArray<UStaticMeshComponent*>& OutParts);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
	FString DebugPrintArrow(const FFINPanelArrow& Arrow);
	FString DebugPrintAnchor(FFINPanelArrowAnchor& Anchor);
	FString StructToHexString(void* Source, SIZE_T Size);
	virtual void Serialize(FArchive& ar) override;
};

