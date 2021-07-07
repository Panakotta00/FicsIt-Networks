#pragma once

#include "FIVSEdGraphViewer.h"
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

public:
	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

	UFUNCTION(BlueprintCallable)
	void SetGraph(UFIVSGraph* InGraph);

	UFUNCTION(BlueprintCallable)
	UFIVSGraph* GetGraph() const;
};
