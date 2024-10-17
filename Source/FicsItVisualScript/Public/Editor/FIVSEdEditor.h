#pragma once

#include "CoreMinimal.h"
#include "FicsItReflection.h"
#include "FIVSEdGraphViewerStyle.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/Widget.h"
#include "Script/FIVSScriptContext.h"
#include "FIVSEdEditor.generated.h"

class UFIVSGraph;
class SFIVSEdGraphViewer;

UCLASS()
class UFIVSEdEditor : public UWidget {
	GENERATED_BODY()
protected:
	UPROPERTY()
	UFIVSGraph* Graph = nullptr;

	UPROPERTY()
	TScriptInterface<IFIVSScriptContext_Interface> Context;

	TSharedPtr<SFIVSEdGraphViewer> Viewer;
	SVerticalBox::FSlot* SelectionDetailsSlot;
	TSharedPtr<SBox> SelectionDetailsContainer;

	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FColor::FromHex("060606"));

	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget
	
public:
	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=( DisplayName="Style" ))
	FFIVSEdGraphViewerStyle Style;
	
	UFUNCTION(BlueprintCallable)
	void SetGraph(UFIVSGraph* InGraph);

	UFUNCTION(BlueprintCallable)
	UFIVSGraph* GetGraph() const;

	UFUNCTION(BlueprintCallable)
	void SetContext(TScriptInterface<IFIVSScriptContext_Interface> InContext);

	UFUNCTION(BlueprintCallable)
	TScriptInterface<IFIVSScriptContext_Interface> GetContext() const { return Context; }

	/**
	 * Updates the details panel of the editor to show details of the current selection.
	 */
	void UpdateSelection();
};

UCLASS(BlueprintType)
class UFIVSEditorContextMock : public UObject, public IFIVSScriptContext_Interface {
	GENERATED_BODY()

public:
	// Begin IFIVSScriptContext_Interface
	virtual void GetRelevantObjects_Implementation(TArray<FFIRTrace>& OutObjects) override {}

	virtual void GetRelevantClasses_Implementation(TArray<UFIRClass*>& OutClasses) override {
		for (const TPair<UClass*, UFIRClass*>& Class : FFicsItReflectionModule::Get().GetClasses()) {
			OutClasses.Add(Class.Value);
		}
	}

	virtual void GetRelevantStructs_Implementation(TArray<UFIRStruct*>& OutStructs) override {
		for (const TPair<UScriptStruct*, UFIRStruct*>& Struct : FFicsItReflectionModule::Get().GetStructs()) {
			OutStructs.Add(Struct.Value);
		}
	}
	// End IFVSScriptContext_Interface
}; 