#pragma once

#include "CoreMinimal.h"
#include "FGPopupWidgetContent.h"
#include "Hologram/FGBlueprintHologram.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Subsystem/ModSubsystem.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "FINBlueprintParameterHooks.generated.h"

UCLASS()
class AFINBlueprintHologram : public AFGBlueprintHologram {
	GENERATED_BODY()
public:
	AFINBlueprintHologram();
	
	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin AFGHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID NetConstructionID) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	// End AFGHologram
	
	void ShowPropertyDialog();
	void PlaceHologram();

	UFUNCTION()
	void PopupClosed(bool bConfirmed);

public:
	UPROPERTY()
	TMap<FString, FString> Parameters;

private:
	UPROPERTY()
	TSubclassOf<UFGPopupWidget> PopupClass;
	UPROPERTY()
	TSubclassOf<UFINBlueprintParameterPopup> PopupContentClass;
	
	UPROPERTY()
	bool bConfigured = false;

	UPROPERTY()
	bool bAccepted = false;
};

UCLASS()
class FICSITNETWORKS_API AFINBlueprintParameterHooks : public AModSubsystem{
	GENERATED_BODY()
public:
	virtual void Init() override;
};

class SFINBlueprintParameterRow : public SMultiColumnTableRow<TSharedRef<FString>> {
public:
	void Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const FString& InVariableName, AFINBlueprintHologram* InHologram);

protected:
	// Begin SMultiColumnTableRow
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	// End SMultiColumnTableRow

private:
	FString VariableName;
	AFINBlueprintHologram* Hologram = nullptr;
};

UCLASS()
class UFINBlueprintParameterPopup : public UFGPopupWidgetContent {
	GENERATED_BODY()
public:
	// Begin UUserWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UUserWidget

	UPROPERTY()
	AFINBlueprintHologram* Hologram = nullptr;
	
protected:
	// Begin UUserWidget
	virtual void NativeConstruct() override;
	// End UUserWidget

private:
	TArray<TSharedRef<FString>> Rows;
};
