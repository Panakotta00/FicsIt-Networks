#pragma once

#include "FIVSEdGraphViewer.h"
#include "FIVSEdStyle.h"
#include "Components/Widget.h"
#include "FIVSEdEditor.generated.h"

class UFIVSGraph;

UCLASS()
class UFIVSEdEditor : public UWidget {
	GENERATED_BODY()
protected:
	UPROPERTY()
	UFIVSGraph* Graph = nullptr;

	TSharedPtr<SFIVSEdGraphViewer> Viewer;

	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget
	
public:
	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=( DisplayName="Style" ))
	FFIVSEdStyle Style;
	
	UFUNCTION(BlueprintCallable)
	void SetGraph(UFIVSGraph* InGraph);

	UFUNCTION(BlueprintCallable)
	UFIVSGraph* GetGraph() const;
};
