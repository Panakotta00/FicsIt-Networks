#pragma once

#include "CoreMinimal.h"
#include "FINComputerGPU.h"
#include "FIRInstancedStruct.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "Widgets/SLeafWidget.h"
#include "FINComputerGPUT2.generated.h"

USTRUCT(BlueprintType)
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2WidgetStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()

	virtual void GetResources( TArray< const FSlateBrush* >& OutBrushes ) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	
	static const FFINGPUT2WidgetStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo NormalText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo MonospaceText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush FilledBox;
};

UCLASS(BlueprintType, hidecategories=Object)
class FICSITNETWORKSCOMPUTER_API UFINGPUT2WidgetStyleContainer : public USlateWidgetStyleContainerBase {
public:
	GENERATED_BODY()

public:
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FFINGPUT2WidgetStyle GPUT2Style;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override {
		return static_cast<const struct FSlateWidgetStyle*>( &GPUT2Style );
	}
};

UENUM()
enum EFINGPUT2DrawCallType {
	FIN_GPUT2_DC_NONE,
	FIN_GPUT2_DC_LINE,
	FIN_GPUT2_DC_ELLIPSE,
	FIN_GPUT2_DC_TEXT,
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DrawContext {
	GENERATED_BODY()

	UObject* WorldContext = nullptr;
	const FFINGPUT2WidgetStyle* Style = nullptr;
	TArray<FGeometry> GeometryStack;
	TArray<FSlateClippingZone> ClippingStack;

	FFINGPUT2DrawContext() = default;
	FFINGPUT2DrawContext(UObject* WorldContext, const FFINGPUT2WidgetStyle* Style) : WorldContext(WorldContext), Style(Style) {}
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DrawCall {
	GENERATED_BODY()

	virtual ~FFINGPUT2DrawCall() = default;
	
	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const { return LayerId; }
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_PushTransform : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D Translation;

	UPROPERTY(SaveGame)
	double Rotation;

	UPROPERTY(SaveGame)
	FVector2D Scale;

	FFINGPUT2DC_PushTransform() = default;
	FFINGPUT2DC_PushTransform(FVector2D Translation, double Rotation, FVector2D Scale) : Translation(Translation), Rotation(Rotation), Scale(Scale) {}
	
	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_PushLayout : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D Offset;

	UPROPERTY(SaveGame)
	FVector2D Size;

	UPROPERTY(SaveGame)
	double Scale;

	FFINGPUT2DC_PushLayout() = default;
	FFINGPUT2DC_PushLayout(FVector2D Offset, FVector2D Size, double Scale) : Offset(Offset), Size(Size), Scale(Scale) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_PopGeometry : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_PushClipRect : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D Position;

	UPROPERTY(SaveGame)
	FVector2D Size;

	FFINGPUT2DC_PushClipRect() = default;
	FFINGPUT2DC_PushClipRect(FVector2D Position, FVector2D Size) : Position(Position), Size(Size) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_PushClipPolygon : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D TopLeft;

	UPROPERTY(SaveGame)
	FVector2D TopRight;

	UPROPERTY(SaveGame)
	FVector2D BottomLeft;

	UPROPERTY(SaveGame)
	FVector2D BottomRight;

	FFINGPUT2DC_PushClipPolygon() = default;
	FFINGPUT2DC_PushClipPolygon(FVector2D TopLeft, FVector2D TopRight, FVector2D BottomLeft, FVector2D BottomRight) : TopLeft(TopLeft), TopRight(TopRight), BottomLeft(BottomLeft), BottomRight(BottomRight) {}
	
	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_PopClip : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_Lines : public FFINGPUT2DrawCall {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	TArray<FVector2D> Points;

	UPROPERTY(SaveGame)
	double Thickness;

	UPROPERTY(SaveGame)
	FColor Color;

	FFINGPUT2DC_Lines() = default;
	FFINGPUT2DC_Lines(TArray<FVector2D> Points, double Thickness, FColor Color) : Points(Points), Thickness(Thickness), Color(Color) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_Text : public FFINGPUT2DrawCall {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FVector2D Position;

	UPROPERTY(SaveGame)
	FString Text;

	UPROPERTY(SaveGame)
	int64 Size;

	UPROPERTY(SaveGame)
	FColor Color;

	UPROPERTY(SaveGame)
	bool bUseMonospace;

	FFINGPUT2DC_Text() = default;
	FFINGPUT2DC_Text(FVector2D Position, FString Text, int64 Size, FColor Color, bool bUseMonospace) : Position(Position), Text(Text), Size(Size), Color(Color), bUseMonospace(bUseMonospace) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_Spline : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D Start;

	UPROPERTY(SaveGame)
	FVector2D StartDirection;

	UPROPERTY(SaveGame)
	FVector2D End;

	UPROPERTY(SaveGame)
	FVector2D EndDirection;

	UPROPERTY(SaveGame)
	double Thickness;

	UPROPERTY(SaveGame)
	FColor Color;

	FFINGPUT2DC_Spline() = default;
	FFINGPUT2DC_Spline(FVector2D Start, FVector2D StartDirection, FVector2D End, FVector2D EndDirection, double Thickness, FColor Color) : Start(Start), StartDirection(StartDirection), End(End), EndDirection(EndDirection), Thickness(Thickness), Color(Color) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_Bezier : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D P0;

	UPROPERTY(SaveGame)
	FVector2D P1;

	UPROPERTY(SaveGame)
	FVector2D P2;

	UPROPERTY(SaveGame)
	FVector2D P3;

	UPROPERTY(SaveGame)
	double Thickness;

	UPROPERTY(SaveGame)
	FColor Color;

	FFINGPUT2DC_Bezier() = default;
	FFINGPUT2DC_Bezier(FVector2D P0, FVector2D P1, FVector2D P2, FVector2D P3, double Thickness, FColor Color) : P0(P0), P1(P1), P2(P2), P3(P3), Thickness(Thickness), Color(Color) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT2DC_Box : public FFINGPUT2DrawCall {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector2D Position = FVector2D::Zero();

	UPROPERTY(SaveGame)
	FVector2D Size = FVector2D(100);

	UPROPERTY(SaveGame)
	double Rotation = 0.0;

	UPROPERTY(SaveGame)
	FColor Color = FColor::White;

	UPROPERTY(SaveGame)
	FString Image;

	UPROPERTY(SaveGame)
	FVector2D ImageSize = FVector2D::Zero();
	
	UPROPERTY(SaveGame)
	bool bHasCenteredOrigin = false;
	
	UPROPERTY(SaveGame)
	bool bHorizontalTiling = false;

	UPROPERTY(SaveGame)
	bool bVerticalTiling = false;
	
	UPROPERTY(SaveGame)
	bool bIsBorder = false;

	UPROPERTY(SaveGame)
	FVector4 Margin = FVector4::Zero();
	
	UPROPERTY(SaveGame)
	bool bIsRounded = false;

	UPROPERTY(SaveGame)
	FVector4 BorderRadii = FVector4::Zero();
	
	UPROPERTY(SaveGame)
	bool bHasOutline = false;

	UPROPERTY(SaveGame)
	double OutlineThickness = 0.0f;

	UPROPERTY(SaveGame)
	FColor OutlineColor = FColor::Transparent;

	FFINGPUT2DC_Box() = default;
	FFINGPUT2DC_Box(FVector2D Position, FVector2D Size, double Rotation, FColor Color, FString Image) : Position(Position), Size(Size), Rotation(Rotation), Color(Color), Image(Image) {}

	virtual int32 OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const override;
};

DECLARE_DELEGATE_TwoParams(FFINGPUT2CursorEvent, FVector2D, int);
DECLARE_DELEGATE_ThreeParams(FFINGPUT2WheelEvent, FVector2D, float, int);
DECLARE_DELEGATE_ThreeParams(FFINGPUT2KeyEvent, uint32, uint32, int);
DECLARE_DELEGATE_TwoParams(FFINGPUT2KeyCharEvent, TCHAR, int);

class FICSITNETWORKSCOMPUTER_API SFINGPUT2Widget : public SLeafWidget {
	SLATE_BEGIN_ARGS(SFINGPUT2Widget) {}
		SLATE_STYLE_ARGUMENT(FFINGPUT2WidgetStyle, Style)
		
		SLATE_ATTRIBUTE(TArray<FFIRInstancedStruct>, DrawCalls)
		SLATE_ATTRIBUTE(bool, CaptureMouseOnPress)

		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseDown)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseUp)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseMove)
		SLATE_EVENT(FFINGPUT2WheelEvent, OnMouseWheel)
		SLATE_EVENT(FFINGPUT2KeyEvent, OnKeyDown)
		SLATE_EVENT(FFINGPUT2KeyEvent, OnKeyUp)
		SLATE_EVENT(FFINGPUT2KeyCharEvent, OnKeyChar)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseLeave)
		SLATE_EVENT(FFINGPUT2CursorEvent, OnMouseEnter)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, UObject* WorldContext);

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	// End SWidget

private:
	UObject* WorldContext = nullptr;
	const FFINGPUT2WidgetStyle* Style = nullptr;
	
	TAttribute<TArray<FFIRInstancedStruct>> DrawCalls;
	
	FFINGPUT2CursorEvent OnMouseDownEvent;
	FFINGPUT2CursorEvent OnMouseUpEvent;
	FFINGPUT2CursorEvent OnMouseMoveEvent;
	FFINGPUT2WheelEvent OnMouseWheelEvent;
	FFINGPUT2CursorEvent OnMouseEnterEvent;
	FFINGPUT2CursorEvent OnMouseLeaveEvent;
	FFINGPUT2KeyEvent OnKeyDownEvent;
	FFINGPUT2KeyEvent OnKeyUpEvent;
	FFINGPUT2KeyCharEvent OnKeyCharEvent;
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINComputerGPUT2 : public AFINComputerGPU {
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
	
	void AddDrawCall(TFIRInstancedStruct<FFINGPUT2DrawCall> DrawCall);

	UFUNCTION(NetMulticast, Reliable)
	void Client_CleanDrawCalls();
	UFUNCTION(NetMulticast, Reliable)
	void Client_AddDrawCallChunk(const TArray<FFIRInstancedStruct>& Chunk);
	UFUNCTION(NetMulticast, Reliable)
	void Client_FlushDrawCalls();

	// Begin FIN Reflection
	UFUNCTION()
	void netFunc_flush();
	UFUNCTION()
	void netFuncMeta_flush(FString& InternalName, FText& DisplayName, FText& Description, int32& Runtime) {
		InternalName = "flush";
		DisplayName = FText::FromString("Flush");
		Description = FText::FromString("Flushes all draw calls to the visible draw call buffer to show all changes at once. The draw buffer gets cleared afterwards.");
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_pushTransform(FVector2D translation, double rotation, FVector2D scale);
	UFUNCTION()
	void netFuncMeta_pushTransform(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "pushTransform";
		DisplayName = FText::FromString("Push Transform");
		Description = FText::FromString("Pushes a transformation to the geometry stack. All subsequent drawcalls will be transformed through all previously pushed geometries and this one. Be aware, only all draw calls till, this geometry gets pop'ed are transformed, previous draw calls (and draw calls after the pop) are unaffected by this.");
		ParameterInternalNames.Add("translation");
		ParameterDisplayNames.Add(FText::FromString("Translation"));
		ParameterDescriptions.Add(FText::FromString("The local translation that is supposed to happen to all further drawcalls. Translation can be also thought as 'repositioning'."));
		ParameterInternalNames.Add("rotation");
		ParameterDisplayNames.Add(FText::FromString("Rotation"));
		ParameterDescriptions.Add(FText::FromString("The local rotation that gets applied to all subsequent draw calls. The origin of the rotation is the whole screens center point. The value is in degrees."));
		ParameterInternalNames.Add("scale");
		ParameterDisplayNames.Add(FText::FromString("Scale"));
		ParameterDescriptions.Add(FText::FromString("The scale that gets applied to the whole screen localy along the (rotated) axis. No change in scale is (1,1)."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_pushLayout(FVector2D offset, FVector2D size, double scale);
	UFUNCTION()
	void netFuncMeta_pushLayout(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "pushLayout";
		DisplayName = FText::FromString("Push Layout");
		Description = FText::FromString("Pushes a layout to the geometry stack. All subsequent drawcalls will be transformed through all previously pushed geometries and this one. Be aware, only all draw calls, till this geometry gets pop'ed are transformed, previous draw calls (and draw calls after the pop) are unaffected by this.");
		ParameterInternalNames.Add("offset");
		ParameterDisplayNames.Add(FText::FromString("Offset"));
		ParameterDescriptions.Add(FText::FromString("The local translation (or offset) that is supposed to happen to all further drawcalls. Translation can be also thought as 'repositioning'."));
		ParameterInternalNames.Add("size");
		ParameterDisplayNames.Add(FText::FromString("Size"));
		ParameterInternalNames.Add("scale");
		ParameterDisplayNames.Add(FText::FromString("Scale"));
		ParameterDescriptions.Add(FText::FromString("The scale that gets applied to the whole screen localy along both axis. No change in scale is 1."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_popGeometry();
	UFUNCTION()
	void netFuncMeta_popGeometry(FString& InternalName, FText& DisplayName, FText& Description, int32& Runtime) {
		InternalName = "popGeometry";
		DisplayName = FText::FromString("Pop Geometry");
		Description = FText::FromString("Pops the top most geometry from the geometry stack. The latest geometry on the stack gets removed first. (Last In, First Out)");
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_pushClipRect(FVector2D position, FVector2D size);
	UFUNCTION()
	void netFuncMeta_pushClipRect(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "pushClipRect";
		DisplayName = FText::FromString("Push Clip Rectangle");
		Description = FText::FromString("Pushes a rectangle to the clipping stack. All subsequent drawcalls will be clipped to only be visible within this clipping zone and all previously pushed clipping zones. Be aware, only all draw calls, till this clipping zone gets pop'ed are getting clipped by it, previous draw calls (and draw calls after the pop) are unaffected by this.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The local position of the upper left corner of the clipping rectangle."));
		ParameterInternalNames.Add("size");
		ParameterDisplayNames.Add(FText::FromString("Size"));
		ParameterDescriptions.Add(FText::FromString("The size of the clipping rectangle."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_pushClipPolygon(FVector2D topLeft, FVector2D topRight, FVector2D bottomLeft, FVector2D bottomRight);
	UFUNCTION()
	void netFuncMeta_pushClipPolygon(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "pushClipPolygon";
		DisplayName = FText::FromString("Push Clipping Polygon");
		Description = FText::FromString("Pushes a 4 pointed polygon to the clipping stack. All subsequent drawcalls will be clipped to only be visible within this clipping zone and all previously pushed clipping zones. Be aware, only all draw calls, till this clipping zone gets pop'ed are getting clipped by it, previous draw calls (and draw calls after the pop) are unaffected by this.");
		ParameterInternalNames.Add("topLeft");
		ParameterDisplayNames.Add(FText::FromString("Top Left"));
		ParameterDescriptions.Add(FText::FromString("The local position of the top left point."));
		ParameterInternalNames.Add("topRight");
		ParameterDisplayNames.Add(FText::FromString("Top Right"));
		ParameterDescriptions.Add(FText::FromString("The local position of the top right point."));
		ParameterInternalNames.Add("bottomLeft");
		ParameterDisplayNames.Add(FText::FromString("Bottom Left"));
		ParameterDescriptions.Add(FText::FromString("The local position of the top right point."));
		ParameterInternalNames.Add("bottomRight");
		ParameterDisplayNames.Add(FText::FromString("Bottom Right"));
		ParameterDescriptions.Add(FText::FromString("The local position of the bottom right point."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_popClip();
	UFUNCTION()
	void netFuncMeta_popClip(FString& InternalName, FText& DisplayName, FText& Description, int32& Runtime) {
		InternalName = "popClip";
		DisplayName = FText::FromString("Pop Clipping Zone");
		Description = FText::FromString("Pops the top most clipping zone from the clipping stack. The latest clipping zone on the stack gets removed first. (Last In, First Out)");
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_drawLines(TArray<FVector2D> points, double thickness, FLinearColor color);
	UFUNCTION()
	void netFuncMeta_drawLines(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "drawLines";
		DisplayName = FText::FromString("Draw Lines");
		Description = FText::FromString("Draws connected lines through all given points with the given thickness and color.");
		ParameterInternalNames.Add("points");
		ParameterDisplayNames.Add(FText::FromString("Points"));
		ParameterDescriptions.Add(FText::FromString("The local points that get connected by lines one after the other."));
		ParameterInternalNames.Add("thickness");
		ParameterDisplayNames.Add(FText::FromString("thickness"));
		ParameterDescriptions.Add(FText::FromString("The thickness of the lines."));
		ParameterInternalNames.Add("color");
		ParameterDisplayNames.Add(FText::FromString("Color"));
		ParameterDescriptions.Add(FText::FromString("The color of the lines."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_drawText(FVector2D position, const FString& text, int64 size, FLinearColor color, bool monospace);
	UFUNCTION()
	void netFuncMeta_drawText(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "drawText";
		DisplayName = FText::FromString("Draw Text");
		Description = FText::FromString("Draws some Text at the given position (top left corner of the text), text, size, color and rotation.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the top left corner of the text."));
		ParameterInternalNames.Add("text");
		ParameterDisplayNames.Add(FText::FromString("Text"));
		ParameterDescriptions.Add(FText::FromString("The text to draw."));
		ParameterInternalNames.Add("size");
		ParameterDisplayNames.Add(FText::FromString("Size"));
		ParameterDescriptions.Add(FText::FromString("The font size used."));
		ParameterInternalNames.Add("color");
		ParameterDisplayNames.Add(FText::FromString("Color"));
		ParameterDescriptions.Add(FText::FromString("The color of the text."));
		ParameterInternalNames.Add("monospace");
		ParameterDisplayNames.Add(FText::FromString("Monospace"));
		ParameterDescriptions.Add(FText::FromString("True if a monospace font should be used."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_drawSpline(FVector2D start, FVector2D startDirection, FVector2D end, FVector2D endDirection, double thickness, FLinearColor color);
	UFUNCTION()
	void netFuncMeta_drawSpline(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "drawSpline";
		DisplayName = FText::FromString("Draw Spline");
		Description = FText::FromString("Draws a Spline from one position to another with given directions, thickness and color.");
		ParameterInternalNames.Add("start");
		ParameterDisplayNames.Add(FText::FromString("Start"));
		ParameterDescriptions.Add(FText::FromString("The local position of the start point of the spline."));
		ParameterInternalNames.Add("startDirections");
		ParameterDisplayNames.Add(FText::FromString("Start Direction"));
		ParameterDescriptions.Add(FText::FromString("The direction of the spline of how it exists the start point."));
		ParameterInternalNames.Add("end_");
		ParameterDisplayNames.Add(FText::FromString("End"));
		ParameterDescriptions.Add(FText::FromString("The local position of the end point of the spline."));
		ParameterInternalNames.Add("endDirection");
		ParameterDisplayNames.Add(FText::FromString("End Directions"));
		ParameterDescriptions.Add(FText::FromString("The direction of how the spline enters the end position."));
		ParameterInternalNames.Add("thickness");
		ParameterDisplayNames.Add(FText::FromString("Thickness"));
		ParameterDescriptions.Add(FText::FromString("The thickness of the line drawn."));
		ParameterInternalNames.Add("color");
		ParameterDisplayNames.Add(FText::FromString("Color"));
		ParameterDescriptions.Add(FText::FromString("The color of the line drawn."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_drawBezier(FVector2D p0, FVector2D p1, FVector2D p2, FVector2D p3, double thickness, FLinearColor color);
	UFUNCTION()
	void netFuncMeta_drawBezier(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "drawBezier";
		DisplayName = FText::FromString("Draw Bezier");
		Description = FText::FromString("Draws a Cubic Bezier Spline from one position to another with given control points, thickness and color.");
		ParameterInternalNames.Add("p0");
		ParameterDisplayNames.Add(FText::FromString("P0"));
		ParameterDescriptions.Add(FText::FromString("The local position of the start point of the spline."));
		ParameterInternalNames.Add("p1");
		ParameterDisplayNames.Add(FText::FromString("P1"));
		ParameterDescriptions.Add(FText::FromString("The local position of the first control point."));
		ParameterInternalNames.Add("p2");
		ParameterDisplayNames.Add(FText::FromString("P2"));
		ParameterDescriptions.Add(FText::FromString("The local position of the second control point."));
		ParameterInternalNames.Add("p3");
		ParameterDisplayNames.Add(FText::FromString("P3"));
		ParameterDescriptions.Add(FText::FromString("The local position of the end point of the spline."));
		ParameterInternalNames.Add("thickness");
		ParameterDisplayNames.Add(FText::FromString("Thickness"));
		ParameterDescriptions.Add(FText::FromString("The thickness of the line drawn."));
		ParameterInternalNames.Add("color");
		ParameterDisplayNames.Add(FText::FromString("Color"));
		ParameterDescriptions.Add(FText::FromString("The color of the line drawn."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_drawBox(FFINGPUT2DC_Box boxSettings);
	UFUNCTION()
	void netFuncMeta_drawBox(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "drawBox";
		DisplayName = FText::FromString("Draw Box");
		Description = FText::FromString("Draws a box.");
		ParameterInternalNames.Add("boxSettings");
		ParameterDisplayNames.Add(FText::FromString("Box Settings"));
		ParameterDescriptions.Add(FText::FromString("The settings of the box you want to draw."));
		Runtime = 2;
	}

	UFUNCTION()
	void netFunc_drawRect(FVector2D position, FVector2D size, FLinearColor color, FString image, double rotation);
	UFUNCTION()
	void netFuncMeta_drawRect(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "drawRect";
		DisplayName = FText::FromString("Draw Rectangle");
		Description = FText::FromString("Draws a Rectangle with the upper left corner at the given local position, size, color and rotation around the upper left corner.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The local position of the upper left corner of the rectangle."));
		ParameterInternalNames.Add("size");
		ParameterDisplayNames.Add(FText::FromString("Size"));
		ParameterDescriptions.Add(FText::FromString("The size of the rectangle."));
		ParameterInternalNames.Add("color");
		ParameterDisplayNames.Add(FText::FromString("Color"));
		ParameterDescriptions.Add(FText::FromString("The color of the rectangle."));
		ParameterInternalNames.Add("image");
		ParameterDisplayNames.Add(FText::FromString("Image"));
		ParameterDescriptions.Add(FText::FromString("If not empty string, should be image reference that should be placed inside the rectangle."));
		ParameterInternalNames.Add("rotation");
		ParameterDisplayNames.Add(FText::FromString("Rotation"));
		ParameterDescriptions.Add(FText::FromString("The rotation of the rectangle around the upper left corner in degrees."));
		Runtime = 2;
	}

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
	void netSig_OnMouseWheel(FVector2D position, float wheelDelta, int modifiers);
	UFUNCTION()
	void netSigMeta_OnMouseWheel(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnMouseWheel";
		DisplayName = FText::FromString("On Mouse Wheel");
		Description = FText::FromString("Triggers when the mouse cursor moves on the screen.");
		ParameterInternalNames.Add("position");
		ParameterDisplayNames.Add(FText::FromString("Position"));
		ParameterDescriptions.Add(FText::FromString("The position of the cursor."));
		ParameterInternalNames.Add("wheelDelta");
		ParameterDisplayNames.Add(FText::FromString("Wheel Delta"));
		ParameterDescriptions.Add(FText::FromString("The delta value of how much the mouse wheel got moved."));
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
	FCriticalSection DrawingMutex;
	
	UPROPERTY(SaveGame)
	TArray<FFIRInstancedStruct> BackBufferDrawCalls;

	UPROPERTY(SaveGame)
	TArray<FFIRInstancedStruct> FrontBufferDrawCalls;

	TQueue<FFIRInstancedStruct> DrawCalls2Send;

	UPROPERTY()
	bool bFlushOverNetwork = true;
};
