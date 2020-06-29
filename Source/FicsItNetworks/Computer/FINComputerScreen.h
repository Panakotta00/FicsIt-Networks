#pragma once

#include "Computer/FINComputerModule.h"
#include "FicsItNetworks/Graphics/FINScreen.h"
#include "Graphics/FINGraphicsProcessor.h"
#include "Network/FINNetworkComponent.h"
#include "FINComputerScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenWidgetUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScreenGPUUpdate);

UCLASS()
class AFINComputerScreen : public AFINComputerModule, public IFINScreen {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame)
	UObject* GPU = nullptr;
	
public:
	TSharedPtr<SWidget> Widget;

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

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
    // Begin IFINScreen
    virtual void BindGPU(UObject* gpu) override;
	virtual UObject* GetGPU() const override;
	virtual void SetWidget(TSharedPtr<SWidget> widget) override;
	// End IFINScreen
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
	AFINComputerScreen* Screen = nullptr;

	TSharedPtr<SBox> Container = nullptr;

	UFUNCTION()
    void OnNewWidget();

	UFUNCTION()
    void OnNewGPU();

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
	
public:
	/**
	 * Binds the given screen with this widget which will display the screen accordingly.
	 *
	 * @param[in]	NewScreen	The screen you want to bind to
	 */
	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
	void SetScreen(AFINComputerScreen* NewScreen);

	/**
	 * Returns the screen we are currently bound to.
	 *
	 * @return	the screen we are bound to
	 */
	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
    AFINComputerScreen* GetScreen();
};
