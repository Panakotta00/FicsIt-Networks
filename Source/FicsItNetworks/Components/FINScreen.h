#pragma once

#include "FGBuildable.h"
#include "WidgetComponent.h"
#include "Computer/FINComputerScreen.h"
#include "Graphics/FINScreenInterface.h"
#include "Network/FINNetworkConnector.h"
#include "FINScreen.generated.h"

UCLASS()
class AFINScreen : public AFGBuildable, public IFINScreenInterface {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame)
	UObject* GPU = nullptr;
	
public:
	TSharedPtr<SWidget> Widget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UFINNetworkConnector* Connector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWidgetComponent* WidgetComponent = nullptr;

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
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINScreenInterface
	void BindGPU(UObject* gpu) override;
	UObject* GetGPU() const override;
	void SetWidget(TSharedPtr<SWidget> widget) override;
	TSharedPtr<SWidget> GetWidget() const override;
	// End IFINScreenInterface
};
