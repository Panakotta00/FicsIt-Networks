// ILikeBanas

#include "FINMCPLabelUI.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"



UFINMCPLabelUI::UFINMCPLabelUI(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}



bool UFINMCPLabelUI::Init(UWorld* World)
{
	//Turtioul = true;

	
	this->IsMouseCursorOnUI = false;

	return true;
}

