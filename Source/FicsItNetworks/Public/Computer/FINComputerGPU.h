#pragma once

#include "Network/FINNetworkTrace.h"
#include "FINComputerModule.h"
#include "FINComputerScreen.h"
#include "FINPciDeviceInterface.h"
#include "Graphics/FINGPUInterface.h"
#include "Graphics/FINScreenInterface.h"
#include "FINComputerGPU.generated.h"

class AFGBuildableWidgetSign;

UCLASS()
class FICSITNETWORKS_API AFINComputerGPU : public AFINComputerModule, public IFINGPUInterface, public IFINPciDeviceInterface {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, SaveGame, Replicated, ReplicatedUsing=OnRep_Screen)
    FFINNetworkTrace Screen;

	AFINComputerGPU();

	// Begin AActor
	virtual void BeginPlay() override;
    virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	// End AActor

	// Begin IFINGraphicsProcessor
	virtual void BindScreen(const FFINNetworkTrace& screen) override;
	virtual FFINNetworkTrace GetScreen() const override;
	virtual void RequestNewWidget() override;
	// End IFINGraphicsProcessor

	UFUNCTION()
	TScriptInterface<IFINScreenInterface> GetScreenInterface();

	/**
     * Creates a new widget for use in the screen.
     * Does *not* tell the screen to use it.
     * Recreates the widget if it exists already.
     * Should get called by the client only
     */
    virtual TSharedPtr<SWidget> CreateWidget();

	/**
	 * This function converts a mouse events button states to a int which is actually a bit field
	 * for each of the button states.
	 * The states from least significant bit to most:
	 * - Left Mouse Button down
	 * - Right Mouse Button down
	 * - control key down
	 * - alt key down
	 * - shift key down
	 * - command key down
	 *
	 * @return	the integer holding the bit field
	 */
	static int MouseToInt(const FPointerEvent& MouseEvent);
	
	/**
	* This function converts a key events button states to a int which is actually a bit field
	* for each of the button states.
	* The states from least significant bit to most:
	* - 0
	* - 0
	* - control key down
	* - alt key down
	* - shift key down
	* - command key down
	*
	* @return	the integer holding the bit field
	*/
	static int InputToInt(const FInputEvent& InputEvent);

private:
	UFUNCTION()
	void OnRep_Screen();

	UFUNCTION()
	void UpdateScreen();

	UPROPERTY()
	FFINNetworkTrace OldScreenCache;
	
public:
	// Begin FIN Reflection
	UFUNCTION()
	void netFunc_bindScreen(FFINNetworkTrace NewScreen);
	UFUNCTION()
	void netFuncMeta_bindScreen(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "bindScreen";
		DisplayName = FText::FromString("Bind Screen");
		Description = FText::FromString("Binds this GPU to the given screen. Unbinds the already bound screen.");
		ParameterInternalNames.Add("newScreen");
		ParameterDisplayNames.Add(FText::FromString("New Screen"));
		ParameterDescriptions.Add(FText::FromString("The screen you want to bind this GPU to. Null if you want to unbind the screen."));
		Runtime = 0;
	}

	UFUNCTION()
	FVector2D netFunc_getScreenSize();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void netSig_ScreenBound(const FFINNetworkTrace& oldScreen);
	// End FIN Reflection
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
    TScriptInterface<IFINScreenInterface> GetScreen();

	/**
	 * Focuses the users input to the widget provided by the GPU
	 */
	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
	void Focus();
};

/**
 * Helper class to implement Screen interface for use with vanilla signs
 */
UCLASS()
class UFINGPUWidgetSign : public UObject, public IFINScreenInterface {
	GENERATED_BODY()
public:
	UPROPERTY()
	FFINNetworkTrace GPU;

	UPROPERTY()
	AFGBuildableWidgetSign* BuildableSign;

	TSharedPtr<SWidget> Widget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FScreenWidgetUpdate OnWidgetUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FScreenGPUUpdate OnGPUUpdate;

	// Begin IFINScreenInterface
	virtual void BindGPU(const FFINNetworkTrace& gpu) override;
	virtual FFINNetworkTrace GetGPU() const override;
	virtual void SetWidget(TSharedPtr<SWidget> widget) override;
	virtual TSharedPtr<SWidget> GetWidget() const override;
	// End IFINScreenInterface
};