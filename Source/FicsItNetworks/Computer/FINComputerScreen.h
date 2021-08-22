#pragma once

#include "FINPciDeviceInterface.h"
#include "FicsItNetworks/Computer/FINComputerModule.h"
#include "FicsItNetworks/Graphics/FINScreenInterface.h"
#include "FINComputerScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenWidgetUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenGPUUpdate);

UCLASS()
class FICSITNETWORKS_API AFINComputerScreen : public AFINComputerModule, public IFINScreenInterface, public IFINPciDeviceInterface {
	GENERATED_BODY()
	
protected:
	UPROPERTY(SaveGame, Replicated)
	FFINNetworkTrace GPU;

	UPROPERTY(Replicated)
	UObject* GPUPtr = nullptr;

	bool bDoGPUUpdate = false;
	
public:
	TSharedPtr<SWidget> Widget;

	AFINComputerScreen();
	
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

	// Begin AActor
	virtual void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type endPlayReason);
	virtual void Tick(float DeltaSeconds) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
    // Begin IFINScreen
    virtual void BindGPU(const FFINNetworkTrace& gpu) override;
	virtual FFINNetworkTrace GetGPU() const override;
	virtual void SetWidget(TSharedPtr<SWidget> widget) override;
	virtual TSharedPtr<SWidget> GetWidget() const override;
	virtual void RequestNewWidget() override;
	// End IFINScreen
	
	UFUNCTION(NetMulticast, Reliable)
	void OnGPUValidChanged(bool bValid, UObject* newGPU);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulti_OnGPUUpdate();
};
