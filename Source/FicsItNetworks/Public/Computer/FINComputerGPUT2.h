#pragma once

#include "CoreMinimal.h"
#include "FINComputerGPU.h"
#include "Network/FINDynamicStructHolder.h"
#include "FINComputerGPUT2.generated.h"

UENUM()
enum EFINGPUT2DrawCallType {
	FIN_GPUT2_DC_NONE,
	FIN_GPUT2_DC_LINE,
	FIN_GPUT2_DC_ELLIPSE,
	FIN_GPUT2_DC_TEXT,
};

USTRUCT()
struct FFINGPUT2DrawCall {
	GENERATED_BODY()

	virtual ~FFINGPUT2DrawCall() = default;
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const { return LayerId; }
};

USTRUCT()
struct FFINGPUT2DC_Line : public FFINGPUT2DrawCall {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FVector2D Start;

	UPROPERTY(SaveGame)
	FVector2D End;

	UPROPERTY(SaveGame)
	double Thickness;

	UPROPERTY(SaveGame)
	FColor Color;

	FFINGPUT2DC_Line() = default;
	FFINGPUT2DC_Line(FVector2D Start, FVector2D End, double Thickness, FColor Color) : Start(Start), End(End), Thickness(Thickness), Color(Color) {}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FFINGPUT2DC_Ellipse : public FFINGPUT2DrawCall {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FVector2D Position;

	UPROPERTY(SaveGame)
	double Radius1;

	UPROPERTY(SaveGame)
	double Radius2;

	UPROPERTY(SaveGame)
	double Rotation;

	UPROPERTY(SaveGame)
	double Thickness;

	UPROPERTY(SaveGame)
	FColor Color;

	FFINGPUT2DC_Ellipse() = default;
	FFINGPUT2DC_Ellipse(FVector2D Position, double Radius1, double Radius2, double Rotation, double Thickness, FColor Color) : Position(Position), Radius1(Radius1), Radius2(Radius2), Rotation(Rotation), Thickness(Thickness), Color(Color) {}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FFINGPUT2DC_Text : public FFINGPUT2DrawCall {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FVector2D Position;

	UPROPERTY(SaveGame)
	FString Text;

	UPROPERTY(SaveGame)
	double Size;

	UPROPERTY(SaveGame)
	FColor Color;

	FFINGPUT2DC_Text() = default;
	FFINGPUT2DC_Text(FVector2D Position, FString Text, double Size, FColor Color) : Position(Position), Text(Text), Size(Size), Color(Color) {}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT(BlueprintType)
struct FFINGPUT2WidgetStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()

	virtual void GetResources( TArray< const FSlateBrush* >& OutBrushes ) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	
	static const FFINGPUT2WidgetStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo NormalText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo MonospaceText;
};

UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class UFINGPUT2WidgetStyleContainer : public USlateWidgetStyleContainerBase {
public:
	GENERATED_BODY()

public:
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FFINGPUT2WidgetStyle GPUT2Style;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override {
		return static_cast<const struct FSlateWidgetStyle*>( &GPUT2Style );
	}
};


DECLARE_DELEGATE_TwoParams(FFINGPUT2CursorEvent, FVector2D, int);
DECLARE_DELEGATE_ThreeParams(FFINGPUT2KeyEvent, uint32, uint32, int);
DECLARE_DELEGATE_TwoParams(FFINGPUT2KeyCharEvent, TCHAR, int);

class SFINGPUT2Widget : public SLeafWidget {
	SLATE_BEGIN_ARGS(SFINGPUT2Widget) {}
		SLATE_STYLE_ARGUMENT(FFINGPUT2WidgetStyle, Style)
		
		SLATE_ATTRIBUTE(TArray<FFINDynamicStructHolder>, DrawCalls)
		SLATE_ATTRIBUTE(bool, CaptureMouseOnPress)

		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseDown)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseUp)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseMove)
		SLATE_EVENT(FFINGPUT2KeyEvent, OnKeyDown)
		SLATE_EVENT(FFINGPUT2KeyEvent, OnKeyUp)
		SLATE_EVENT(FFINGPUT2KeyCharEvent, OnKeyChar)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseLeave)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseEnter)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	// End SWidget

private:
	const FFINGPUT2WidgetStyle* Style = nullptr;
	
	TAttribute<TArray<FFINDynamicStructHolder>> DrawCalls;
	
	FFINGPUT2CursorEvent OnMouseDownEvent;
	FFINGPUT2CursorEvent OnMouseUpEvent;
	FFINGPUT2CursorEvent OnMouseMoveEvent;
	FFINGPUT2CursorEvent OnMouseEnterEvent;
	FFINGPUT2CursorEvent OnMouseLeaveEvent;
	FFINGPUT2KeyEvent OnKeyDownEvent;
	FFINGPUT2KeyEvent OnKeyUpEvent;
	FFINGPUT2KeyCharEvent OnKeyCharEvent;
};

UCLASS()
class AFINComputerGPUT2 : public AFINComputerGPU {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FFINGPUT2WidgetStyle Style;
	
	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	// End AActor
	
	// Begin AFINComputerGPU
	virtual TSharedPtr<SWidget> CreateWidget() override;
	// End AFINComputerGPU

	UFUNCTION()
	void FlushDrawCalls();

	UFUNCTION(NetMulticast, Reliable)
	void Client_CleanDrawCalls();
	UFUNCTION(NetMulticast, Reliable)
	void Client_AddDrawCallChunk(const TArray<FFINDynamicStructHolder>& Chunk);
	UFUNCTION(NetMulticast, Reliable)
	void Client_FlushDrawCalls();

	// Begin FIN Reflection
	UFUNCTION()
	void netFunc_flush();

	UFUNCTION()
	void netFunc_drawLine(FVector2D start, FVector2D end, double thickness, FLinearColor color);

	UFUNCTION()
	void netFunc_drawEllipse(FVector2D position, double radius1, double radius2, double rotation, double thickness, FLinearColor color);

	UFUNCTION()
	void netFunc_drawText(FVector2D position, const FString& text, double size, FLinearColor color);

	UFUNCTION()
	FVector2D netFunc_measureText(FString text, int64 size, bool bMonospace);
	UFUNCTION()
	void netFuncMeta_measureText(int32& Runtime) {
		Runtime = 0;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseDown(FVector2D position, int modifiers);
	UFUNCTION()
    void netSigMeta_OnMouseDown(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseDown";
		DisplayName = FText::FromString("On Mouse Down");
		Description = FText::FromString("Triggers when a mouse button got pressed.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the cursor."));
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifier-Bit-Field providing information about the pressed button event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseUp(FVector2D position, int modifiers);
	UFUNCTION()
    void netSigMeta_OnMouseUp(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseUp";
		DisplayName = FText::FromString("On Mouse Up");
		Description = FText::FromString("Triggers when a mouse button got released.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the cursor."));
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the released button event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseMove(FVector2D position, int modifiers);
	UFUNCTION()
    void netSigMeta_OnMouseMove(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseMove";
		DisplayName = FText::FromString("On Mouse Move");
		Description = FText::FromString("Triggers when the mouse cursor moves on the screen.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the cursor."));
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the move event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseEnter(FVector2D position, int modifiers);
	UFUNCTION()
	void netSigMeta_OnMouseEnter(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseEnter";
		DisplayName = FText::FromString("On Mouse Enter");
		Description = FText::FromString("Triggers when the mouse cursor enters the screen area.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the cursor."));
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the move event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnMouseLeave(FVector2D position, int modifiers);
	UFUNCTION()
	void netSigMeta_OnMouseLeave(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseLeave";
		DisplayName = FText::FromString("On Mouse Leave");
		Description = FText::FromString("Triggers when the mouse cursor leaves the screen area.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the cursor."));
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the move event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyDown(int64 c, int64 code, int modifiers);
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
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the key press event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyUp(int64 c, int64 code, int modifiers);
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
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the key release event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void netSig_OnKeyChar(const FString& c, int modifiers);
	UFUNCTION()
	void netSigMeta_OnKeyChar(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnKeyChar";
		DisplayName = FText::FromString("On Key Character");
		Description = FText::FromString("Triggers when a character key got 'clicked' and essentially a character got typed in, usful for text input.");
		ParameterInternalNames.Add("c");
		ParameterDisplayNames.Add(FText::FromString("Character"));
		ParameterDescriptions.Add(FText::FromString("The character that got typed in as string."));
		ParameterInternalNames.Add("modifiers");
		ParameterDisplayNames.Add(FText::FromString("Modifiers"));
		ParameterDescriptions.Add(FText::FromString("The Modifiers-Bit-Field providing information about the key release event.\nBits:\n1th left mouse pressed\n2th right mouse button pressed\n3th ctrl key pressed\n4th shift key pressed\n5th alt key pressed\n6th cmd key pressed"));
		Runtime = 1;
	}
	// End FIN Reflection
	
private:
	UPROPERTY(SaveGame)
	TArray<FFINDynamicStructHolder> DrawCalls;

	UPROPERTY(SaveGame)
	TArray<FFINDynamicStructHolder> FlushedDrawCalls;

	TQueue<FFINDynamicStructHolder> DrawCalls2Send;

	UPROPERTY()
	bool bFlushOverNetwork = true;
};