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
	FCriticalSection DrawingMutex;
	
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
	
	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName, TMap<FString, FString>& PropertyInternalNames, TMap<FString, FText>& PropertyDisplayNames, TMap<FString, FText>& PropertyDescriptions, TMap<FString, int32>& PropertyRuntimes) {
    	InternalName = TEXT("GPUT1");
    	DisplayName = FText::FromString(TEXT("Computer GPU T1"));
    }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseDown(int x, int y, int btn);
	UFUNCTION()
    void netSigMeta_OnMouseDown(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseDown";
		DisplayName = FText::FromString("On Mouse Down");
		Description = FText::FromString("Triggers when a mouse button got pressed.");
		ParameterInternalNames.Add("x");
		ParameterDisplayNames.Add(FText::FromString("X"));
		ParameterDescriptions.Add(FText::FromString("The x position of the cursor."));
		ParameterInternalNames.Add("y");
		ParameterDisplayNames.Add(FText::FromString("Y"));
		ParameterDescriptions.Add(FText::FromString("The y position of the cursor."));
		ParameterInternalNames.Add("btn");
		ParameterDisplayNames.Add(FText::FromString("Button"));
		ParameterDescriptions.Add(FText::FromString("The Button-Bit-Field providing information about the pressed button event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseUp(int x, int y, int btn);
	UFUNCTION()
    void netSigMeta_OnMouseUp(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseUp";
		DisplayName = FText::FromString("On Mouse Up");
		Description = FText::FromString("Triggers when a mouse button got released.");
		ParameterInternalNames.Add("x");
		ParameterDisplayNames.Add(FText::FromString("X"));
		ParameterDescriptions.Add(FText::FromString("The x position of the cursor."));
		ParameterInternalNames.Add("y");
		ParameterDisplayNames.Add(FText::FromString("Y"));
		ParameterDescriptions.Add(FText::FromString("The y position of the cursor."));
		ParameterInternalNames.Add("btn");
		ParameterDisplayNames.Add(FText::FromString("Button"));
		ParameterDescriptions.Add(FText::FromString("The Button-Bit-Field providing information about the released button event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseMove(int x, int y, int btn);
	UFUNCTION()
    void netSigMeta_OnMouseMove(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseMove";
		DisplayName = FText::FromString("On Mouse Move");
		Description = FText::FromString("Triggers when the mouse cursor moves on the screen.");
		ParameterInternalNames.Add("x");
		ParameterDisplayNames.Add(FText::FromString("X"));
		ParameterDescriptions.Add(FText::FromString("The x position of the cursor."));
		ParameterInternalNames.Add("y");
		ParameterDisplayNames.Add(FText::FromString("Y"));
		ParameterDescriptions.Add(FText::FromString("The y position of the cursor."));
		ParameterInternalNames.Add("btn");
		ParameterDisplayNames.Add(FText::FromString("Button"));
		ParameterDescriptions.Add(FText::FromString("The Button-Bit-Field providing information about the move event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyDown(int64 c, int64 code, int btn);
	UFUNCTION()
    void netSigMeta_OnKeyDown(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnKeyDown";
		DisplayName = FText::FromString("On Key Down");
		Description = FText::FromString("Triggers when a key got pressed.");
		ParameterInternalNames.Add("c");
		ParameterDisplayNames.Add(FText::FromString("C"));
		ParameterDescriptions.Add(FText::FromString("The ASCII number of the character typed in."));
		ParameterInternalNames.Add("code");
		ParameterDisplayNames.Add(FText::FromString("Code"));
		ParameterDescriptions.Add(FText::FromString("The number code of the pressed key."));
		ParameterInternalNames.Add("btn");
		ParameterDisplayNames.Add(FText::FromString("Button"));
		ParameterDescriptions.Add(FText::FromString("The Button-Bit-Field providing information about the key press event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyUp(int64 c, int64 code, int btn);
	UFUNCTION()
    void netSigMeta_OnKeyUp(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnKeyUp";
		DisplayName = FText::FromString("On Key Up");
		Description = FText::FromString("Triggers when a key got released.");
		ParameterInternalNames.Add("c");
		ParameterDisplayNames.Add(FText::FromString("C"));
		ParameterDescriptions.Add(FText::FromString("The ASCII number of the character typed in."));
		ParameterInternalNames.Add("code");
		ParameterDisplayNames.Add(FText::FromString("Code"));
		ParameterDescriptions.Add(FText::FromString("The number code of the pressed key."));
		ParameterInternalNames.Add("btn");
		ParameterDisplayNames.Add(FText::FromString("Button"));
		ParameterDescriptions.Add(FText::FromString("The Button-Bit-Field providing information about the key release event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_ScreenSizeChanged(int oldW, int oldH);
	UFUNCTION()
    void netSigMeta_ScreenSizeChanged(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "ScreenSizeChanged";
		DisplayName = FText::FromString("Screen Size Changed");
		Description = FText::FromString("Triggers when the size of the text grid changed.");
		ParameterInternalNames.Add("oldW");
		ParameterDisplayNames.Add(FText::FromString("old Width"));
		ParameterDescriptions.Add(FText::FromString("The old width of the screen."));
		ParameterInternalNames.Add("oldH");
		ParameterDisplayNames.Add(FText::FromString("old Height"));
		ParameterDescriptions.Add(FText::FromString("The old height of the screen."));
		Runtime = 1;
	}
	
	UFUNCTION()
	void netFunc_bindScreen(FFINNetworkTrace NewScreen);
	UFUNCTION()
    void netFuncMeta_bindScreen(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "bindScreen";
		DisplayName = FText::FromString("Bind Screen");
		Description = FText::FromString("Binds this GPU to the given screen. Unbinds the already bound screen.");
		ParameterInternalNames.Add("newScreen");
		ParameterDisplayNames.Add(FText::FromString("New Screen"));
		ParameterDescriptions.Add(FText::FromString("The screen you want to bind this GPU to. Null if you want to unbind the screen."));
		Runtime = 1;
	}

	UFUNCTION()
	UObject* netFunc_getScreen();
	UFUNCTION()
    void netFuncMeta_getScreen(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getScreen";
		DisplayName = FText::FromString("Get Screen");
		Description = FText::FromString("Returns the currently bound screen.");
		ParameterInternalNames.Add("screen");
		ParameterDisplayNames.Add(FText::FromString("Screen"));
		ParameterDescriptions.Add(FText::FromString("The currently bound screen."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_setText(int x, int y, const FString& str);
	UFUNCTION()
    void netFuncMeta_setText(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setText";
		DisplayName = FText::FromString("Set Text");
		Description = FText::FromString("Draws the given text at the given position to the hidden screen buffer.");
		ParameterInternalNames.Add("x");
		ParameterDisplayNames.Add(FText::FromString("X"));
		ParameterDescriptions.Add(FText::FromString("The x coordinate at which the text should get drawn."));
		ParameterInternalNames.Add("y");
		ParameterDisplayNames.Add(FText::FromString("Y"));
		ParameterDescriptions.Add(FText::FromString("The y coordinate at which the text should get drawn."));
		ParameterInternalNames.Add("str");
		ParameterDisplayNames.Add(FText::FromString("String"));
		ParameterDescriptions.Add(FText::FromString("The text you want to draw on-to the buffer."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_fill(int x, int y, int dx, int dy, const FString& str);
	UFUNCTION()
    void netFuncMeta_fill(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "fill";
		DisplayName = FText::FromString("Fill");
		Description = FText::FromString("Draws the given character at all given positions in the given rectangle on-to the hidden screen buffer.");
		ParameterInternalNames.Add("x");
		ParameterDisplayNames.Add(FText::FromString("X"));
		ParameterDescriptions.Add(FText::FromString("The x coordinate at which the rectangle should get drawn. (upper-left corner)"));
		ParameterInternalNames.Add("y");
		ParameterDisplayNames.Add(FText::FromString("Y"));
		ParameterDescriptions.Add(FText::FromString("The y coordinate at which the rectangle should get drawn. (upper-left corner)"));
		ParameterInternalNames.Add("dx");
		ParameterDisplayNames.Add(FText::FromString("DX"));
		ParameterDescriptions.Add(FText::FromString("The width of the rectangle."));
		ParameterInternalNames.Add("dy");
		ParameterDisplayNames.Add(FText::FromString("DY"));
		ParameterDescriptions.Add(FText::FromString("The height of the rectangle."));
		ParameterInternalNames.Add("str");
		ParameterDisplayNames.Add(FText::FromString("String"));
		ParameterDescriptions.Add(FText::FromString("The character you want to use for the rectangle. (first char in the given string)"));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_getSize(int& w, int& h);
	UFUNCTION()
    void netFuncMeta_getSize(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "getSize";
		DisplayName = FText::FromString("Get Size");
		Description = FText::FromString("Returns the size of the text-grid (and buffer).");
		ParameterInternalNames.Add("w");
		ParameterDisplayNames.Add(FText::FromString("Width"));
		ParameterDescriptions.Add(FText::FromString("The width of the text-gird."));
		ParameterInternalNames.Add("h");
		ParameterDisplayNames.Add(FText::FromString("Height"));
		ParameterDescriptions.Add(FText::FromString("The height of the text-grid."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_setSize(int w, int h);
	UFUNCTION()
    void netFuncMeta_setSize(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setSize";
		DisplayName = FText::FromString("Set Size");
		Description = FText::FromString("Changes the size of the text-grid (and buffer).");
		ParameterInternalNames.Add("w");
		ParameterDisplayNames.Add(FText::FromString("Width"));
		ParameterDescriptions.Add(FText::FromString("The width of the text-gird."));
		ParameterInternalNames.Add("h");
		ParameterDisplayNames.Add(FText::FromString("Height"));
		ParameterDescriptions.Add(FText::FromString("The height of the text-grid."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_setForeground(float r, float g, float b, float a);
	UFUNCTION()
    void netFuncMeta_setForeground(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setForeground";
		DisplayName = FText::FromString("Set Foreground Color");
		Description = FText::FromString("Changes the foreground color that is used for the next draw calls.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red portion of the foreground color. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green portion of the foreground color. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue portion of the foreground color. (0.0 - 1.0)"));
		ParameterInternalNames.Add("a");
		ParameterDisplayNames.Add(FText::FromString("Alpha"));
		ParameterDescriptions.Add(FText::FromString("The opacity of the foreground color. (0.0 - 1.0)"));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_setBackground(float r, float g, float b, float a);
	UFUNCTION()
    void netFuncMeta_setBackground(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setBackround";
		DisplayName = FText::FromString("Set Background Color");
		Description = FText::FromString("Changes the background color that is used for the next draw calls.");
		ParameterInternalNames.Add("r");
		ParameterDisplayNames.Add(FText::FromString("Red"));
		ParameterDescriptions.Add(FText::FromString("The red portion of the background color. (0.0 - 1.0)"));
		ParameterInternalNames.Add("g");
		ParameterDisplayNames.Add(FText::FromString("Green"));
		ParameterDescriptions.Add(FText::FromString("The green portion of the background color. (0.0 - 1.0)"));
		ParameterInternalNames.Add("b");
		ParameterDisplayNames.Add(FText::FromString("Blue"));
		ParameterDescriptions.Add(FText::FromString("The blue portion of the background color. (0.0 - 1.0)"));
		ParameterInternalNames.Add("a");
		ParameterDisplayNames.Add(FText::FromString("Alpha"));
		ParameterDescriptions.Add(FText::FromString("The opacity of the background color. (0.0 - 1.0)"));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_flush();
	UFUNCTION()
    void netFuncMeta_flush(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "flush";
		DisplayName = FText::FromString("Flush");
		Description = FText::FromString("Flushes the hidden screen buffer to the visible screen buffer and so makes the draw calls visible.");
		Runtime = 1;
	}
};
