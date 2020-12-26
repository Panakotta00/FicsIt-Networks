#pragma once

#include "FINComputerGPU.h"
#include "SInvalidationPanel.h"

#include "FINComputerGPUT1.generated.h"

DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FScreenCursorEventHandler, int, int, int);
DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FScreenKeyEventHandler, uint32, uint32, int);

class SScreenMonitor : public SLeafWidget {
	SLATE_BEGIN_ARGS(SScreenMonitor) : _Text(),
		_Font(),
		_ScreenSize()
		{
			_Clipping = EWidgetClipping::OnDemand;
		}
		SLATE_ATTRIBUTE(TArray<FString>, Text)
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)
		SLATE_ATTRIBUTE(FVector2D, ScreenSize)
		SLATE_ATTRIBUTE(TArray<FLinearColor>, Foreground)
		SLATE_ATTRIBUTE(TArray<FLinearColor>, Background)

		SLATE_EVENT(FScreenCursorEventHandler, OnMouseDown)
		SLATE_EVENT(FScreenCursorEventHandler, OnMouseUp)
		SLATE_EVENT(FScreenCursorEventHandler, OnMouseMove)
		SLATE_EVENT(FScreenKeyEventHandler, OnKeyDown)
		SLATE_EVENT(FScreenKeyEventHandler, OnKeyUp)
	SLATE_END_ARGS()

public:
    void Construct( const FArguments& InArgs );

	/**
	 * Returns the currently displayed text grid.
	 *
	 * @return	the currently displayed text grid
	 */
	TArray<FString> GetText() const;

	/**
	 * Allows you to get information about the character screen size.
	 *
	 * @return	the screen size
	 */
	FVector2D GetScreenSize() const;

	/**
	 * Allows you to set the screen size of the display.
	 *
	 * @param[in]	ScreenSize	the new screen size for the display
	 */
	void SetScreenSize(FVector2D ScreenSize);

	/**
	 * This function returns the size of a single displayed character slot in the widgets local space
	 *
	 * @return	the character slot size in local space
	 */
	FVector2D GetCharSize() const;

	/**
	 * This function converts a local space coordinate to a character grid coordinate.
	 *
	 * @parm[in]	the local space coordinate you want to convert
	 * @return	the character coordinate
	 */
	FVector2D LocalToCharPos(FVector2D Pos) const;

	/**
	 * This function converts a mouse events button states to a int which is actually a bit field
	 * for each of the button states.
	 * The states from least significant bit to most:
	 * - Left Mouse Button down
	 * - Right Mouse Button down
	 * - control key down
	 * - alt key down
	 * - shift key down
	 * - command key down
	 *
	 * @return	the integer holding the bit field
	 */
	static int MouseToInt(const FPointerEvent& MouseEvent);

	/**
	* This function converts a key events button states to a int which is actually a bit field
	* for each of the button states.
	* The states from least significant bit to most:
	* - 0
	* - 0
	* - control key down
	* - alt key down
	* - shift key down
	* - command key down
	*
	* @return	the integer holding the bit field
	*/
	static int InputToInt(const FInputEvent& InputEvent);
	
private:
    TAttribute<TArray<FString>> Text;
	TAttribute<TArray<FLinearColor>> Foreground;
	TAttribute<TArray<FLinearColor>> Background;
	TAttribute<FSlateFontInfo> Font;
	TAttribute<FVector2D> ScreenSize;
	FScreenCursorEventHandler OnMouseDownEvent;
	FScreenCursorEventHandler OnMouseUpEvent;
	FScreenCursorEventHandler OnMouseMoveEvent;
	FScreenKeyEventHandler OnKeyDownEvent;
	FScreenKeyEventHandler OnKeyUpEvent;

	int lastMoveX = -1;
	int lastMoveY = -1;
    
public:
	SScreenMonitor();

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	// End SWidget
};

UCLASS()
class AFINComputerGPUT1 : public AFINComputerGPU {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame, Replicated)
	TArray<FString> TextGrid;

	UPROPERTY(SaveGame, Replicated)
	FVector2D ScreenSize;

	UPROPERTY(SaveGame)
	FLinearColor CurrentForeground = FLinearColor(1,1,1,1);

	UPROPERTY(SaveGame)
	FLinearColor CurrentBackground = FLinearColor(0,0,0,0);

	UPROPERTY(SaveGame, Replicated)
	TArray<FLinearColor> Foreground;

	UPROPERTY(SaveGame, Replicated)
	TArray<FLinearColor> Background;

	UPROPERTY(SaveGame)
	TArray<FString> TextGridBuffer;

	UPROPERTY(SaveGame)
	TArray<FLinearColor> ForegroundBuffer;

	UPROPERTY(SaveGame)
	TArray<FLinearColor> BackgroundBuffer;
	
	UPROPERTY()
	FSlateBrush boxBrush;

	TSharedPtr<SInvalidationPanel> CachedInvalidation;
	bool bFlushed = false;
	
public:
	AFINComputerGPUT1();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin IFINGraphicsPorcessingUnit
	virtual void BindScreen(const FFINNetworkTrace& Screen) override;
	// End IFINGraphicsProcessingUnit

	virtual TSharedPtr<SWidget> CreateWidget() override;

	/**
	* Reallocates the TextGrid for the new given screen size.
	*
	* @param[in]	size	the new screen size
	*/
	void SetScreenSize(FVector2D size);

	/**
	 * Validates the screen widget on all clients and server
	 */
	UFUNCTION(NetMulticast, Reliable)
	void Flush();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseDown(int x, int y, int btn);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseUp(int x, int y, int btn);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseMove(int x, int y, int btn);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyDown(int64 c, int64 code, int btn);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyUp(int64 c, int64 code, int btn);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_ScreenSizeChanged(int oldW, int oldH);
	
	UFUNCTION()
	void netFunc_bindScreen(FFINNetworkTrace NewScreen);

	UFUNCTION()
	UObject* netFunc_getScreen();

	UFUNCTION()
	void netFunc_setText(int x, int y, const FString& str);

	UFUNCTION()
	void netFunc_fill(int x, int y, int dx, int dy, const FString& str);

	UFUNCTION()
	void netFunc_getSize(int& w, int& h);

	UFUNCTION()
	void netFunc_setSize(int w, int h);

	UFUNCTION()
	void netFunc_setForeground(float r, float g, float b, float a);

	UFUNCTION()
	void netFunc_setBackground(float r, float g, float b, float a);

	UFUNCTION()
	void netFunc_flush();
};
