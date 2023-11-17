#pragma once
#include "FGSaveInterface.h"
#include "FicsItNetworksModule.h"
#include "Hologram/FGBlueprintHologram.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Patching/NativeHookManager.h"
#include "Subsystem/ModSubsystem.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"

#include "FINBlueprintParameterHooks.generated.h"

USTRUCT(Blueprintable)
struct FFINBlueprintParamData  {
	GENERATED_BODY()

	FFINBlueprintParamData() = default;

	FFINBlueprintParamData(const FString& Key, const FString& Value, const FString& Default)
		: Key(Key),
		  Value(Value),
		  DefaultValue(Default) {
		this->DefaultValue = Default;
		this->Key = Key;
		this->Value = Value;
	}

	FFINBlueprintParamData(const FString& Key, const FString& Default)
		: Key(Key),
		  DefaultValue(Default) {
		this->DefaultValue = Default;
		this->Key = Key;
		this->Value = Default;
	}

	FString Key;
	FString Value;
	FString DefaultValue;
};

class SFINPropertyRow : public SMultiColumnTableRow<TSharedRef<FFINBlueprintParamData>> {
	public:
	void Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const TSharedRef<FFINBlueprintParamData>& PropData);

	TSharedPtr<FFINBlueprintParamData> GetParamData() const { return ParamData; }
	
	protected:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	private:
	TSharedPtr<FFINBlueprintParamData> ParamData;
};

enum class EFINBlueprintParameterDialogAnswers : uint32 {
	Action_Cancel,
	Action_OK,
};



UCLASS()
class AFINBlueprintHologram : public AFGBlueprintHologram {
	GENERATED_BODY()

	
	public:
		void ShowPropertyDialog();
		virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
		FString ReplFunction(FRegexMatcher* Matcher);
		virtual AActor* Construct(TArray<AActor*>& out_children, FNetConstructionID NetConstructionID) override;

		virtual void SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) override;
		virtual void BeginPlay() override;

		TMap<FString, FFINBlueprintParamData> GetParameters() {
			return Properties;
		}

		virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
		virtual bool TrySnapToActor(const FHitResult& hitResult) override;


		void SetConfigured(bool bCond) {
			Configured = true;
		}

		void ApplyProperties(TArray<TSharedRef<FFINBlueprintParamData>> Rows);
		void PlaceHologram();

	private:
		UPROPERTY()
		TArray<UFINAdvancedNetworkConnectionComponent*> Connectors;

		TMap<FString, FFINBlueprintParamData> Properties;

		bool PropertiesSet = false;
	
		const char* ParameterPattern = "\\{\\$([A-Za-z]+\\w*)\\}";

		UPROPERTY()
		bool Configured;
};


UCLASS()
class FICSITNETWORKS_API AFINBlueprintParameterHooks : public AModSubsystem{
	GENERATED_BODY()
	
	public:

	
	AFINBlueprintParameterHooks();

	virtual void Init() override;

#pragma optimize("", off)
	static void PreHologramPlacementHook(CallScope<void(*)(AFGBlueprintHologram*, const FHitResult& hitResult)>& Scope, AFGBlueprintHologram* Hologram, const FHitResult& HitResult) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Pre Hologram Placement Hook"));
	}
	static void PostHologramPlacementHook(CallScope<void(*)(AFGBlueprintHologram*, const FHitResult& hitResult)>& Scope, AFGBlueprintHologram* Hologram, const FHitResult& HitResult) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Post Hologram Placement Hook"));		
	}
	static void ConfigureActorHook(CallScope<void(*)(AFGBlueprintHologram*, AFGBuildable* hitResult)>& Scope, AFGBlueprintHologram* Hologram, AFGBuildable* inBuildable) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Post Hologram Placement Hook"));		
	}
	static void ConstructHook(CallScope<AActor*(*)(AFGBlueprintHologram*, TArray<AActor*>&, FNetConstructionID)>& Scope, AFGBlueprintHologram* Hologram, TArray<AActor*>& out_children, FNetConstructionID NetConstructionID) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Post Hologram Placement Hook"));		
	}
#pragma optimize("", on)
};

UCLASS()
class UTestWindow : public UFGInteractWidget {
	GENERATED_BODY()
	

	protected:
		virtual void NativeConstruct() override;

	public:
		virtual TSharedRef<SWidget> RebuildWidget() override;

	void SetProperties(TMap<FString, FFINBlueprintParamData> Data);
	void ApplyProperties(TMap<FString, FFINBlueprintParamData> Data);

	protected:
	
	EFINBlueprintParameterDialogAnswers Result = EFINBlueprintParameterDialogAnswers::Action_Cancel;
	TSharedPtr<SListView<TSharedRef<FFINBlueprintParamData>>> ListView;
	
	TArray<TSharedRef<FFINBlueprintParamData>> Rows;
};