#pragma once

#include "Computer/FINComputerModule.h"
#include "FicsItNetworks/Graphics/FINGraphicsProcessor.h"
#include "Network/Signals/FINSignalSender.h"

#include "FINComputerGPU.generated.h"

UCLASS()
class AFINComputerGraphicsProcessor : public AFINComputerModule, public IFINGraphicsProcessor {
	GENERATED_BODY()
private:
	UPROPERTY()
	UObject* Screen = nullptr;

	UPROPERTY(meta=(MultiLine="true"))
	FText DisplayText = FText::FromString("Display Text");

	UPROPERTY()
	TArray<FString> TextGrid;

	FVector2D ScreenSize = FVector2D(0,0);

	TSharedPtr<SScaleBox> Widget;
	TSharedPtr<SBorder> border;
	TSharedPtr<STextBlock> text;

	UPROPERTY()
	FSlateBrush boxBrush;

	bool hasChanged = false;
	bool shouldCreate = false;
	
public:
	AFINComputerGraphicsProcessor();

	// Begin AActor
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGSaveInterface
    virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
	// Begin IFINGraphicsProcessor
	virtual void BindScreen(UObject* screen) override;
	virtual UObject* GetScreen() const override;
    virtual void RequestNewWidget() override;
    virtual void DropWidget() override;
	// End IFINGraphicsProcessor

	/**
	 * Creates a new widget for use in the screen.
	 * Does *not* tell the screen to use it.
	 * Recreates the widget if it exists already.
	 * Shoul get called by the client only
	 */
	void CreateWidget();

	/**
	 * Creates the text for the text block from the text grid
	 * and sets the display text to it.
	 */
	void UpdateTextBlockText();

	/**
	 * Reallocates the TextGrid for the new given screen size.
	 *
	 * @param[in]	size	the new screen size
	 */
	void SetScreenSize(FVector2D size);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnClick(int x, int y);

	UFUNCTION()
	void netFunc_bindScreen(UObject* NewScreen);

	UFUNCTION()
	UObject* netFunc_getScreen();

	UFUNCTION()
	void netFunc_setText(int x, int y, const FString& str);
};
