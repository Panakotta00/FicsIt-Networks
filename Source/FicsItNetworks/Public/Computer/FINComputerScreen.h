#pragma once

#include "FINPciDeviceInterface.h"
#include "Computer/FINComputerModule.h"
#include "Graphics/FINScreenInterface.h"
#include "FINComputerScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenWidgetUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenGPUUpdate);

UCLASS()
class FICSITNETWORKS_API AFINComputerScreen : public AFINComputerModule, public IFINScreenInterface, public IFINPciDeviceInterface {
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	FFIRTrace GPU;

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
	// End AActor
	
    // Begin IFINScreen
    virtual void BindGPU(const FFIRTrace& gpu) override;
	virtual FFIRTrace GetGPU() const override;
	virtual void SetWidget(TSharedPtr<SWidget> widget) override;
	virtual TSharedPtr<SWidget> GetWidget() const override;
	// End IFINScreen
};
