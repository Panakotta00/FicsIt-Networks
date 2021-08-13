#pragma once
#include "Buildables/FGBuildable.h"
#include "FicsItNetworks/ModuleSystem/FINModuleSystemPanel.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FINSizeablePanel.generated.h"

UCLASS()
class AFINSizeablePanel  : public AFGBuildable {
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelUL = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelUC = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelUR = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelCL = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelCC = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelCR = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelDL = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelDC = nullptr;
	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* PanelDR = nullptr;
	
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadWrite)
	UFINModuleSystemPanel* ModularPanel = nullptr;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* Connector = nullptr;
	
	UPROPERTY(SaveGame, Replicated)
	int PanelWidth = 10;
	UPROPERTY(SaveGame, Replicated)
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

	

	static void SpawnComponents(TSubclassOf<UStaticMeshComponent> Class, int PanelWidth, int PanelHeight, UStaticMesh* ULMesh, UStaticMesh* UCMesh, UStaticMesh* URMesh, UStaticMesh* CLMesh, UStaticMesh* CCMesh, UStaticMesh* CRMesh, UStaticMesh* DLMesh, UStaticMesh* DCMesh, UStaticMesh* DRMesh, AActor* Parent, USceneComponent* Attach, TArray<UStaticMeshComponent*>& OutParts);
	static void SpawnEdgeComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, int scaleX, int scaleY, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts); 
	static void SpawnCornerComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts);

	void SetPanelSize(int width, int height);
};
