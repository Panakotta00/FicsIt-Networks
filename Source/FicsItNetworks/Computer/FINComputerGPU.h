#pragma once

#include "FINComputerModule.h"
#include "WidgetComponent.h"
#include "WidgetInteractionComponent.h"
#include "FicsItNetworks/Graphics/FINGraphicsProcessor.h"
#include "FINComputerGPU.generated.h"

UCLASS()
class AFINComputerGPU : public AFINComputerModule, public IFINGraphicsProcessor {
	GENERATED_BODY()
protected:
	UPROPERTY(SaveGame)
    UObject* Screen = nullptr;

	TSharedPtr<SWidget> Widget;
	bool bShouldCreate = false;

public:
	AFINComputerGPU();
	
	// Begin AActor
    virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
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

/**
* Used by a Screen to display the widget in a UMG environment.
* Updates the used Widget according to the bound screen.
*/
UCLASS()
class UFINScreenWidget : public UWidget {
	GENERATED_BODY()

private:
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY()
    UObject* Screen = nullptr;

	TSharedPtr<SBox> Container = nullptr;

protected:
    // UWidget interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
	
public:
	UFUNCTION(BlueprintCallable)
    void OnNewWidget();

	UFUNCTION(BlueprintCallable)
    void OnNewGPU();
	
    /**
     * Binds the given screen with this widget which will display the screen accordingly.
     *
     * @param[in]	NewScreen	The screen you want to bind to
     */
    UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
    void SetScreen(UObject* NewScreen);

	/**
	 * Returns the screen we are currently bound to.
	 *
	 * @return	the screen we are bound to
	 */
	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
    UObject* GetScreen();

	/**
	 * Focuses the users input to the widget provided by the GPU
	 */
	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
	void Focus();
};
