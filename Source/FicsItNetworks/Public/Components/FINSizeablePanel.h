#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildable.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINSizeablePanel.generated.h"

class UFINModuleSystemPanel;

UCLASS()
class AFINSizeablePanel  : public AFGBuildable {
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelCornerMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelSideMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelCenterMesh = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelCenterMeshNoConnector = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelConnectorMesh = nullptr;
	
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadWrite)
	UFINModuleSystemPanel* ModularPanel = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* Connector = nullptr;
	
	UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
	int PanelWidth = 10;
	UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
	int PanelHeight = 10;
	
	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	UPROPERTY()
	UStaticMeshComponent* Plane = nullptr;
	
	AFINSizeablePanel();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& transform) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	// End AActor

	// Begin AFGBuildable
	virtual int32 GetDismantleRefundReturnsMultiplier() const override;
	// End AFGBuildable

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	void ConstructParts();

	static void SpawnComponents(TSubclassOf<UStaticMeshComponent> Class, int PanelWidth, int PanelHeight,
                                        UStaticMesh* ULMesh,
                                        UStaticMesh* UCMesh,
                                        UStaticMesh* CCMesh,
                                        UStaticMesh* CCMesh_NoCon,
                                        UStaticMesh* ConnectorMesh,
                                        AActor* Parent, USceneComponent* Attach,
                                        TArray<UStaticMeshComponent*>& OutParts);
	static void SpawnEdgeComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, int scaleX, int scaleY, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts); 
	static void SpawnCornerComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts);

	UFUNCTION(BlueprintCallable)
	static TArray<FItemAmount> GetModifiedCost(const int Width, const int Height) {
		const int Cells = FGenericPlatformMath::Max(Width * Height, 1);
		
		TArray<FItemAmount> Items = TArray<FItemAmount>();
		Items.Push(FItemAmount(LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Resource/Parts/CircuitBoard/Desc_CircuitBoard.Desc_CircuitBoard_C")), 1));
		Items.Push(FItemAmount(LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Resource/Parts/IronPlateReinforced/Desc_IronPlateReinforced.Desc_IronPlateReinforced_C")), FGenericPlatformMath::Max(1, Cells / 10)));
		Items.Push(FItemAmount(LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Resource/Parts/Wire/Desc_Wire.Desc_Wire_C")), 2 * Cells));

		return Items;
	}
	
	void SetPanelSize(int width, int height);
};
