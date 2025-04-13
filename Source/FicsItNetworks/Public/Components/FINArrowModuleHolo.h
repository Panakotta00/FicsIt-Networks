#pragma once
#include "FGPopupWidgetContent.h"
#include "ModuleSystem/FINModuleSystemHolo.h"
#include "InputActionValue.h"
#include "ModSubsystem.h"
#include "NativeHookManager.h"
#include "FINArrowModuleHolo.generated.h"


UCLASS()
class AFINArrowModuleHolo : public AFINModuleSystemHolo {
	GENERATED_BODY()
public:

	AFINArrowModuleHolo();
	
	virtual void BeginPlay() override;
	void ConfigureProperties(const FInputActionValue& Value);

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End AActor

	// Begin AFGBuildableHologram
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual int32 GetBaseCostMultiplier() const override;
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual void SetHologramLocationAndRotation(const FHitResult& HitResult) override;
	virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) override;
	virtual void CheckValidFloor() override;
	virtual void ConfigureActor(AFGBuildable* inBuildable) const override;
	// End AFGBuildableHologram

	UFUNCTION()
	void PopupClosed(bool bConfirmed);
	
	void ShowPropertyDialog();
	void RebuildParts();
	void ConstructParts();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnInputMappingBound(const TArray<FName>& Actions);

	bool bNeedRebuild;
	bool bNeedColorUpdate;
	TArray<FFINPanelArrowAnchor> Anchors;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=EnhancedUserInput)
	class UInputMappingContext* MappingContext;

	UPROPERTY(EditDefaultsOnly, Category=EnhancedUserInput)
	class UInputAction* ConfigureAction;
	
	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;
	

private:
	UPROPERTY()
	TSoftClassPtr<UFGPopupWidget> PopupClass;
	UPROPERTY()
	TSoftClassPtr<class UFINPanelTraceConfigPopup> PopupContentClass;
};


UCLASS()
class FICSITNETWORKS_API AFINBuildgunHooks : public AModSubsystem{
	GENERATED_BODY()
public:
	AFINBuildgunHooks();
	~AFINBuildgunHooks();
	
	UFUNCTION()
	void OnRecipeSampled(TSubclassOf<UFGRecipe> Recipe);

protected:
	virtual void Init() override;

	FDelegateHandle DelegateHandle;
};


USTRUCT(BlueprintType)
struct FFINIconTextIntegerOption {
	GENERATED_BODY()
	FFINIconTextIntegerOption(): Icon(nullptr), Value(0) { }

	FFINIconTextIntegerOption(FString Text, UTexture2D* Icon, int Value) {
		this->Text = Text;
		this->Icon = Icon;
		this->Value = Value;
	}

	FFINIconTextIntegerOption(FString Text, int Value) {
		this->Text = Text;
		this->Icon = nullptr;
		this->Value = Value;
	}
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	FString Text;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UTexture2D* Icon;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int Value;
};

typedef TSharedPtr<FFINIconTextIntegerOption>  FFINArrowOptionType;

UCLASS()
class UFINPanelTraceConfigPopup : public UFGPopupWidgetContent {
	GENERATED_BODY()
public:
	static TSharedRef<SWidget> CreateItem(FFINArrowOptionType Value);
	FText GetCenterColor();
	// Begin UUserWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
		// End UUserWidget

	void SetTraceOuter(int Index, EFINPanelTraceEndTypes Type);
	void SetTraceInner(int Index, EFINPanelTraceStartTypes Type);
	void SetCenter(EFINPanelArrowCrossingTypes Type);
	void SetCenterColorText(FText ColorIn);
	FFINArrowOptionType GetTraceOuterItem(int Index, TArray<FFINArrowOptionType>& Source);
	FFINArrowOptionType GetTraceInnerItem(int Index, TArray<FFINArrowOptionType>& Source);
	FFINArrowOptionType GetCenterItem(TArray<FFINArrowOptionType>& Source);
	void SetTraceColorFromText(int Index, const FText& Text);
	void SetTraceColorInherit(int Index, bool Value, TSharedPtr<SEditableTextBox> BoundTextField);
	bool IsTraceColorInherit(int Index);
	FText GetTraceColorText(int Index);

	UPROPERTY()
	AFINArrowModuleHolo* Hologram = nullptr;
	
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboUpArrow_Outer;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboUpArrow_Inner;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboDownArrow_Outer;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboDownArrow_Inner;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboLeftArrow_Outer;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboLeftArrow_Inner;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboRightArrow_Outer;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboRightArrow_Inner;
	TSharedPtr<SComboBox<FFINArrowOptionType>> CboCenterStyle;

	
	TArray<FFINArrowOptionType> TraceOuter;
	TArray<FFINArrowOptionType> TraceInner;
	TArray<FFINArrowOptionType> Center;
	TSharedPtr<SEditableTextBox> TxtAnchorColor;
	TSharedPtr<SEditableTextBox> TxtUpArrowColor;
	TSharedPtr<SEditableTextBox> TxtDownArrowColor;
	TSharedPtr<SEditableTextBox> TxtLeftArrowColor;
	TSharedPtr<SEditableTextBox> TxtRightArrowColor;

	protected:
	// Begin UUserWidget
	virtual void NativeConstruct() override;
	// End UUserWidget

};