#include "UI/MicroControlPanel/FINMCPLabelUI.h"
#include "Components/CanvasPanel.h"

UFINMCPLabelUI::UFINMCPLabelUI(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer) {}

bool UFINMCPLabelUI::Init(UWorld* World) {
	this->IsMouseCursorOnUI = false;

	return true;
}
