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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintAssignable)
	FScreenWidgetUpdate OnWidgetUpdate;

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

UCLASS()
class UFINScreenWidget : public UWidget {
	GENERATED_BODY()

private:
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY()
	AFINComputerScreen* Screen = nullptr;

	TSharedPtr<SBox> Container = nullptr;
	
public:
	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
	void SetScreen(AFINComputerScreen* NewScreen);

	UFUNCTION(BlueprintCallable, Category="Computer|Graphics")
    AFINComputerScreen* GetScreen();

	UFUNCTION()
	void OnNewWidget();

	UFUNCTION()
	void OnNewGPU();
protected:
    // UWidget interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};
