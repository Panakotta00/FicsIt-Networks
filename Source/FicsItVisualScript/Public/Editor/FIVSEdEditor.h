#pragma once

#include "CoreMinimal.h"
#include "FIVSEdGraphViewerStyle.h"
#include "Brushes/SlateColorBrush.h"
#include "Components/Widget.h"
#include "Reflection/FINReflection.h"
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
	virtual void GetRelevantObjects_Implementation(TArray<FFINNetworkTrace>& OutObjects) override {}

	virtual void GetRelevantClasses_Implementation(TArray<UFINClass*>& OutClasses) override {
		for (const TPair<UClass*, UFINClass*>& Class : FFINReflection::Get()->GetClasses()) {
			OutClasses.Add(Class.Value);
		}
	}

	virtual void GetRelevantStructs_Implementation(TArray<UFINStruct*>& OutStructs) override {
		for (const TPair<UScriptStruct*, UFINStruct*>& Struct : FFINReflection::Get()->GetStructs()) {
			OutStructs.Add(Struct.Value);
		}
	}
	// End IFVSScriptContext_Interface
}; 