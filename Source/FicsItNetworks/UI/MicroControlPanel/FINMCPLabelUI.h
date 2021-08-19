// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Delegates/DelegateCombinations.h"

#include "FINMCPLabelUI.generated.h"

/**
 * 
 */
UCLASS()
class FICSITNETWORKS_API UFINMCPLabelUI : public UUserWidget
{
	GENERATED_BODY()
public:

	UFINMCPLabelUI(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* RootPanel;
	
	bool IsMouseCursorOnUI;

	bool Init(UWorld* World);

};
