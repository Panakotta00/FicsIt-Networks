#pragma once

#include "FGBuildable.h"
#include "WidgetComponent.h"
#include "Computer/FINComputerScreen.h"
#include "Graphics/FINScreenInterface.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Network/FINNetworkCustomType.h"

#include "FINScreen.generated.h"

UCLASS()
class AFINScreen : public AFGBuildable, public IFINScreenInterface, public IFINNetworkCustomType {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame, Replicated)
	FFINNetworkTrace GPU;

	bool bWasGPUValid = false;
	
public:
	TSharedPtr<SWidget> Widget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINAdvancedNetworkConnectionComponent* Connector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWidgetComponent* WidgetComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* ScreenMiddle = nullptr;

	UPROPERTY(EditDefaultsOnly)
    UStaticMesh* ScreenEdge = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UStaticMesh* ScreenCorner = nullptr;

	UPROPERTY(SaveGame, Replicated)
	int ScreenWidth = 1;

	UPROPERTY(SaveGame, Replicated)
	int ScreenHeight = 1;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	bool bGPUChanged = false;
	
	/**
	* This event gets triggered when a new widget got set by the GPU
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FScreenWidgetUpdate OnWidgetUpdate;

	/**
	* This event gets triggered when a new GPU got bound
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FScreenGPUUpdate OnGPUUpdate;
	
	AFINScreen();

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

	// Begin IFINScreenInterface
	void BindGPU(const FFINNetworkTrace& gpu) override;
	FFINNetworkTrace GetGPU() const override;
	void SetWidget(TSharedPtr<SWidget> widget) override;
	TSharedPtr<SWidget> GetWidget() const override;
	// End IFINScreenInterface

	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("Screen"); }
	// End IFINNetworkCustomType

	UFUNCTION(NetMulticast, Reliable)
	void OnGPUValidChanged(bool bWasGPUValid);

	UFUNCTION()
	void netFunc_getSize(int& w, int& h);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulti_OnGPUUpdate();
	
	static void SpawnComponents(int ScreenWidth, int ScreenHeight, UStaticMesh* MiddlePartMesh, UStaticMesh* EdgePartMesh, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, TArray<UStaticMeshComponent*>& OutParts);
	static void SpawnEdgeComponent(int x, int y, int r, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts);
	static void SpawnCornerComponent(int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts);
};
