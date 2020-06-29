#pragma once

#include "FINComputerModule.h"
#include "FicsItNetworks/Graphics/FINGraphicsProcessor.h"
#include "FINComputerGPU.generated.h"

UCLASS()
class AFINComputerGPU : public AFINComputerModule, public IFINGraphicsProcessor {
	GENERATED_BODY()
protected:
	UPROPERTY(SaveGame)
    UObject* Screen = nullptr;

	TSharedPtr<SWidget> Widget;
	bool shouldCreate = false;

public:
	AFINComputerGPU();
	
	// Begin AActor
    virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	// End AActor
	
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINGraphicsProcessor
	virtual void BindScreen(UObject* screen) override;
	virtual UObject* GetScreen() const override;
	virtual void RequestNewWidget() override;
	virtual void DropWidget() override;
	// End IFINGraphicsProcessor
	
	/**
     * Creates a new widget for use in the screen.
     * Does *not* tell the screen to use it.
     * Recreates the widget if it exists already.
     * Should get called by the client only
     */
    virtual TSharedPtr<SWidget> CreateWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void netSig_ScreenBound(UObject* oldScreen);
};
