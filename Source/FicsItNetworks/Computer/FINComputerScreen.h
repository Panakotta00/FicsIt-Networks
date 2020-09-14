#pragma once

#include "Computer/FINComputerModule.h"
#include "FicsItNetworks/Graphics/FINScreenInterface.h"
#include "Network/FINNetworkCustomType.h"

#include "FINComputerScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenWidgetUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenGPUUpdate);

UCLASS()
class AFINComputerScreen : public AFINComputerModule, public IFINScreenInterface, public IFINNetworkCustomType {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame, Replicated)
	FFINNetworkTrace GPU;

	bool bWasGPUValid = false;
	
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
	// End IFINScreen
	
	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("ScreenDriver"); }
	// End IFINNetworkCustomType

	UFUNCTION(NetMulticast, Reliable)
	void OnGPUValidChanged(bool bWasGPUValid);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulti_OnGPUUpdate();
};
