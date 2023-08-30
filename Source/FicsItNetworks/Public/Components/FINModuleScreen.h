#pragma once

#include "FINModuleBase.h"
#include "Computer/FINComputerScreen.h"
#include "Components/WidgetComponent.h"
#include "FINModuleScreen.generated.h"

UCLASS()
class AFINModuleScreen : public AFINModuleBase, public IFINScreenInterface {
	GENERATED_BODY()
private:
    UPROPERTY(SaveGame, Replicated)
    FFINNetworkTrace GPU;

	UPROPERTY(Replicated)
	UObject* GPUPtr = nullptr;

	bool bDoGPUUpdate = false;
	
public:
    TSharedPtr<SWidget> Widget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWidgetComponent* WidgetComponent;

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

	AFINModuleScreen();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor
	
	// Begin IFINScreen
	virtual void BindGPU(const FFINNetworkTrace& gpu) override;
	virtual FFINNetworkTrace GetGPU() const override;
	virtual void SetWidget(TSharedPtr<SWidget> widget) override;
	virtual TSharedPtr<SWidget> GetWidget() const override;
	virtual void RequestNewWidget() override;
	// End IFINScreen

	UFUNCTION(NetMulticast, Reliable)
	void OnGPUValidationChanged(bool bValid, UObject* newGPU);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulti_OnGPUUpdate();
};
