#include "ComputerModules/PCI/FINComputerGPUT2.h"

#include "FGPlayerController.h"
#include "FINComputerRCO.h"
#include "FINMediaSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "FIRException.h"
#include "Fonts/FontMeasure.h"

const FName FFINGPUT2WidgetStyle::TypeName(TEXT("FFINGPUT2WidgetStyle"));

int32 FFINGPUT2DC_PushTransform::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateRenderTransform Transform = FSlateRenderTransform(TScale2<float>(Scale.X, Scale.Y), Translation);
	Transform = Transform.Concatenate(FSlateRenderTransform(TQuat2<float>(Rotation)));
	FGeometry NewGeometry = AllottedGeometry.MakeChild(Transform);
	Context.GeometryStack.Push(NewGeometry);
	return LayerId;
}

int32 FFINGPUT2DC_PushLayout::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FGeometry NewGeometry = AllottedGeometry.MakeChild(Size, FSlateLayoutTransform(Scale, Offset));
	Context.GeometryStack.Push(NewGeometry);
	return LayerId;
}

int32 FFINGPUT2DC_PopGeometry::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	if (Context.GeometryStack.Num() > 1) { // Needs to be 1 as there has to be at least one Geometry in the stack, that is the whole Widgets Geometry
		Context.GeometryStack.Pop(EAllowShrinking::No);
	}
	return LayerId;
}

int32 FFINGPUT2DC_PushClipRect::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateClippingZone Clip(AllottedGeometry.GetLayoutBoundingRect(FSlateRect::FromPointAndExtent(Position, Size)));
	Context.ClippingStack.Add(Clip);
	OutDrawElements.PushClip(Clip);
	return LayerId;
}

int32 FFINGPUT2DC_PushClipPolygon::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateClippingZone Clip(TopLeft, TopRight, BottomLeft, BottomRight);
	Context.ClippingStack.Add(Clip);
	OutDrawElements.PushClip(Clip);
	return LayerId;
}

int32 FFINGPUT2DC_PopClip::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	if (Context.GeometryStack.Num() > 0) {
		Context.ClippingStack.Pop();
		OutDrawElements.PopClip();
	}
	return LayerId;
}

int32 FFINGPUT2DC_Lines::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, Thickness);
	return LayerId;
}

int32 FFINGPUT2DC_Text::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateFontInfo Font = bUseMonospace ? Context.Style->MonospaceText : Context.Style->NormalText;
	Font.Size = Size;
	FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(Position)), Text, Font, ESlateDrawEffect::None, Color);
	return LayerId;
}

int32 FFINGPUT2DC_Spline::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateDrawElement::MakeSpline(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), Start, StartDirection, End, EndDirection, Thickness, ESlateDrawEffect::None, Color);
	return LayerId;
}

int32 FFINGPUT2DC_Bezier::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateDrawElement::MakeCubicBezierSpline(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), P0, P1, P2, P3, Thickness, ESlateDrawEffect::None, Color);
	return LayerId;
}

int32 FFINGPUT2DC_Box::OnPaint(FFINGPUT2DrawContext& Context, const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateBrush Brush = Context.Style->FilledBox;

	if (!Image.IsEmpty()) {
		UObject* Texture = AFINMediaSubsystem::GetMediaSubsystem(Context.WorldContext)->GetOrLoadTexture(Image);
		if (!Texture) return LayerId;
		Brush.SetResourceObject(Texture);
	}

	if (!ImageSize.IsZero()) {
		Brush.ImageSize = ImageSize;
		Brush.DrawAs = ESlateBrushDrawType::Image;
	}

	if (bVerticalTiling) {
		if (bHorizontalTiling) Brush.Tiling = ESlateBrushTileType::Both;
		else Brush.Tiling = ESlateBrushTileType::Vertical;
	} else if (bHorizontalTiling) {
		Brush.Tiling = ESlateBrushTileType::Horizontal;
	}
	
	if (bIsBorder) {
		Brush.Margin = Margin;
	}

	if (bIsRounded || bHasOutline) {
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	}

	if (bIsRounded) {
		Brush.OutlineSettings.CornerRadii = BorderRadii;
	} else {
		Brush.OutlineSettings.CornerRadii = FVector4d(0);
	}

	if (bHasOutline) {
		Brush.OutlineSettings.Width = OutlineThickness;
		Brush.OutlineSettings.Color = OutlineColor;
	}

	FVector2D Translation = Position;
	TOptional<FVector2d> RotationPoint;
	if (bHasCenteredOrigin) {
		Translation -= Size / 2;
	} else {
		RotationPoint = FVector2d(0);
	}

	double RotationAngle = FMath::DegreesToRadians(Rotation);
	
	FPaintGeometry PaintGeometry = AllottedGeometry.ToPaintGeometry(Size, FSlateLayoutTransform(Translation)); 
	
	FSlateDrawElement::MakeRotatedBox(OutDrawElements, LayerId++, PaintGeometry, &Brush, ESlateDrawEffect::None, RotationAngle, RotationPoint, FSlateDrawElement::ERotationSpace::RelativeToElement, Color);
	return LayerId;
}

void FFINGPUT2WidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	FSlateWidgetStyle::GetResources(OutBrushes);

	OutBrushes.Add(&FilledBox);
}

const FFINGPUT2WidgetStyle& FFINGPUT2WidgetStyle::GetDefault() {
	static FFINGPUT2WidgetStyle* Default = nullptr;
	if (!Default) Default = new FFINGPUT2WidgetStyle();
	return *Default;
}

void SFINGPUT2Widget::Construct(const FArguments& InArgs, UObject* InWorldContext) {
	WorldContext = InWorldContext;
	Style = InArgs._Style;
	DrawCalls = InArgs._DrawCalls;
	OnMouseDownEvent = InArgs._OnMouseDown;
	OnMouseUpEvent = InArgs._OnMouseUp;
	OnMouseMoveEvent = InArgs._OnMouseMove;
	OnMouseEnterEvent = InArgs._OnMouseEnter;
	OnMouseLeaveEvent = InArgs._OnMouseLeave;
	OnMouseWheelEvent = InArgs._OnMouseWheel;
	OnKeyDownEvent = InArgs._OnKeyDown;
	OnKeyUpEvent = InArgs._OnKeyUp;
	OnKeyCharEvent = InArgs._OnKeyChar;
}

FVector2D SFINGPUT2Widget::ComputeDesiredSize(float LayoutScaleMultiplier) const {
	return FVector2D(100, 100);
}

int32 SFINGPUT2Widget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	FFINGPUT2DrawContext Context(WorldContext, Style);
	Context.GeometryStack.Add(AllottedGeometry);
	for (const FFIRInstancedStruct& DrawCallBase : DrawCalls.Get()) {
		FFINGPUT2DrawCall* DrawCall = DrawCallBase.GetPtr<FFINGPUT2DrawCall>();
		if (DrawCall) LayerId = DrawCall->OnPaint(Context, Args, Context.GeometryStack.Last(), OutDrawElements, LayerId, InWidgetStyle);
	}
	return LayerId;
}

FReply SFINGPUT2Widget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D Position = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	OnMouseDownEvent.ExecuteIfBound(Position, AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Handled().CaptureMouse(AsShared());
}

FReply SFINGPUT2Widget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D Position = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	OnMouseUpEvent.ExecuteIfBound(Position, AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SFINGPUT2Widget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetCursorDelta().Length() <= 0.0f) return FReply::Handled(); 
	FVector2D Position = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	OnMouseMoveEvent.ExecuteIfBound(Position, AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Handled();
}

FReply SFINGPUT2Widget::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D Position = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	OnMouseWheelEvent.ExecuteIfBound(Position, MouseEvent.GetWheelDelta(), AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Handled().ReleaseMouseCapture();
}

void SFINGPUT2Widget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D Position = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	OnMouseEnterEvent.ExecuteIfBound(Position, AFINComputerGPU::MouseToInt(MouseEvent));
}

void SFINGPUT2Widget::OnMouseLeave(const FPointerEvent& MouseEvent) {
	FVector2D Position = GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	OnMouseLeaveEvent.ExecuteIfBound(Position, AFINComputerGPU::MouseToInt(MouseEvent));
}

FReply SFINGPUT2Widget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Escape) return FReply::Unhandled();
	OnKeyDownEvent.ExecuteIfBound(InKeyEvent.GetCharacter(), InKeyEvent.GetKeyCode(), AFINComputerGPU::InputToInt(InKeyEvent));
	return FReply::Handled();
}

FReply SFINGPUT2Widget::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Escape) return FReply::Unhandled();
	OnKeyUpEvent.ExecuteIfBound(InKeyEvent.GetCharacter(), InKeyEvent.GetKeyCode(), AFINComputerGPU::InputToInt(InKeyEvent));
	return FReply::Handled();
}

FReply SFINGPUT2Widget::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) {
	OnKeyCharEvent.ExecuteIfBound(InCharacterEvent.GetCharacter(), AFINComputerGPU::InputToInt(InCharacterEvent));
	return FReply::Handled();
}

bool SFINGPUT2Widget::IsInteractable() const {
	return true;
}

bool SFINGPUT2Widget::SupportsKeyboardFocus() const {
	return true;
}

void AFINComputerGPUT2::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (!DrawCalls2Send.IsEmpty()) {
		for (int i = 0; i < 10 && !DrawCalls2Send.IsEmpty(); ++i) {
			TArray<FFIRInstancedStruct> Chunk;
			for (int j = 0; j < 10; ++j) {
				FFIRInstancedStruct* DrawCall = DrawCalls2Send.Peek();
				if (!DrawCall) break;
				Chunk.Add(*DrawCall);
				DrawCalls2Send.Pop();
			}
			Client_AddDrawCallChunk(Chunk);
		}
		if (DrawCalls2Send.IsEmpty()) {
			Client_FlushDrawCalls();
		}
	} else if (bFlushOverNetwork) {
		Client_CleanDrawCalls();
		bFlushOverNetwork = false;
		for (const FFIRInstancedStruct& DrawCall : FrontBufferDrawCalls) {
			DrawCalls2Send.Enqueue(DrawCall);
		}
	}
}

TSharedPtr<SWidget> AFINComputerGPUT2::CreateWidget() {
	UFINComputerRCO* RCO = Cast<UFINComputerRCO>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
	return SNew(SFINGPUT2Widget, this)
	.Style(&Style)
	.OnMouseDown_Lambda([this, RCO](FVector2D position, int modifiers) {
		RCO->GPUT2MouseEvent(this, 0, position, modifiers);
	})
	.OnMouseUp_Lambda([this, RCO](FVector2D position, int modifiers) {
		RCO->GPUT2MouseEvent(this, 1, position, modifiers);
	})
	.OnMouseMove_Lambda([this, RCO](FVector2D position, int modifiers) {
		RCO->GPUT2MouseEvent(this, 2, position, modifiers);
	})
	.OnMouseWheel_Lambda([this, RCO](FVector2D position, float delta, int modifiers) {
		RCO->GPUT2MouseWheelEvent(this, position, delta, modifiers);
	})
	.OnMouseEnter_Lambda([this, RCO](FVector2D position, int modifiers) {
		RCO->GPUT2MouseEvent(this, 3, position, modifiers);
	})
	.OnMouseLeave_Lambda([this, RCO](FVector2D position, int modifiers) {
		RCO->GPUT2MouseEvent(this, 4, position, modifiers);
	})
	.OnKeyDown_Lambda([this, RCO](uint32 c, uint32 key, int modifiers) {
		RCO->GPUT2KeyEvent(this, 0, c, key, modifiers);
	})
	.OnKeyUp_Lambda([this, RCO](uint32 c, uint32 key, int modifiers) {
		RCO->GPUT2KeyEvent(this, 1, c, key, modifiers);
	})
	.OnKeyChar_Lambda([this, RCO](TCHAR c, int modifiers) {
		RCO->GPUT2KeyCharEvent(this, FString::Chr(c), modifiers);
	})
	.DrawCalls_Lambda([this]() {
		return FrontBufferDrawCalls;
	});
}

void AFINComputerGPUT2::FlushDrawCalls() {
	FScopeLock Lock(&DrawingMutex);
	FrontBufferDrawCalls = BackBufferDrawCalls;
	BackBufferDrawCalls.Empty();
}

void AFINComputerGPUT2::AddDrawCall(TFIRInstancedStruct<FFINGPUT2DrawCall> DrawCall) {
	FScopeLock Lock(&DrawingMutex);
	BackBufferDrawCalls.Add(DrawCall);
}

void AFINComputerGPUT2::netFunc_flush() {
	FlushDrawCalls();
	bFlushOverNetwork = true;
}

void AFINComputerGPUT2::netFunc_pushTransform(FVector2D translation, double rotation, FVector2D scale) {
	AddDrawCall(FFINGPUT2DC_PushTransform(translation, rotation, scale));
}

void AFINComputerGPUT2::netFunc_pushLayout(FVector2D offset, FVector2D size, double scale) {
	AddDrawCall(FFINGPUT2DC_PushLayout(offset, size, scale));
}

void AFINComputerGPUT2::netFunc_popGeometry() {
	AddDrawCall(FFINGPUT2DC_PopGeometry());
}

void AFINComputerGPUT2::netFunc_pushClipRect(FVector2D position, FVector2D size) {
	AddDrawCall(FFINGPUT2DC_PushClipRect(position, size));
}

void AFINComputerGPUT2::netFunc_pushClipPolygon(FVector2D topLeft, FVector2D topRight, FVector2D bottomLeft, FVector2D bottomRight) {
	AddDrawCall(FFINGPUT2DC_PushClipPolygon(topLeft, topRight, bottomLeft, bottomRight));
}

void AFINComputerGPUT2::netFunc_popClip() {
	AddDrawCall(FFINGPUT2DC_PopClip());
}

void AFINComputerGPUT2::netFunc_drawLines(TArray<FVector2D> points, double thickness, FLinearColor color) {
	AddDrawCall(FFINGPUT2DC_Lines(points, thickness, color.QuantizeRound()));
}

void AFINComputerGPUT2::netFunc_drawText(FVector2D position, const FString& text, int64 size, FLinearColor color, bool monospace) {
	AddDrawCall(FFINGPUT2DC_Text(position, text, size, color.QuantizeRound(), monospace));
}

void AFINComputerGPUT2::netFunc_drawSpline(FVector2D start, FVector2D startDirection, FVector2D end, FVector2D endDirection, double thickness, FLinearColor color) {
	AddDrawCall(FFINGPUT2DC_Spline(start, startDirection, end, endDirection, thickness, color.QuantizeRound()));
}

void AFINComputerGPUT2::netFunc_drawBezier(FVector2D p1, FVector2D p2, FVector2D p3, FVector2D p4, double thickness, FLinearColor color) {
	AddDrawCall(FFINGPUT2DC_Bezier(p1, p2, p3, p4, thickness, color.QuantizeRound()));
}

void AFINComputerGPUT2::netFunc_drawBox(FFINGPUT2DC_Box BoxSettings) {
	BackBufferDrawCalls.Add(BoxSettings);
}

void AFINComputerGPUT2::netFunc_drawRect(FVector2D position, FVector2D size, FLinearColor color, FString image, double rotation) {
	BackBufferDrawCalls.Add(FFINGPUT2DC_Box(position, size, rotation, color.QuantizeRound(), image));
}

namespace {
	// Dedicated servers have no Slate application and are built without FreeType (WITH_FREETYPE=0 in SlateCore.Build.cs),
	// so Slate can't measure text there. The monospace font is measured manually instead,
	// using the metrics of FiraCode (the GPU T2 monospace font) and the same rounding FreeType applies on clients.
	constexpr double FINMonospaceUnitsPerEm = 1950.0;
	constexpr double FINMonospaceAdvance = 1200.0;
	constexpr double FINMonospaceHeight = 2400.0; // ascender 1800 - descender -600
	constexpr double FINMonospaceBaseline = -600.0;

	TSharedPtr<FSlateFontMeasure> GetFontMeasure() {
		if (!FSlateApplication::IsInitialized()) return nullptr;
		FSlateRenderer* Renderer = FSlateApplication::Get().GetRenderer();
		if (!Renderer) return nullptr;
		return Renderer->GetFontMeasureService();
	}

	int64 GetMonospaceMetric(int64 Size, double Units) {
		// Slate renders fonts at 96 DPI, FreeType rounds to whole pixels per em and then rounds the scaled metric
		const int64 PixelsPerEm = FMath::RoundToInt64(FMath::Max<int64>(Size, 0) * 96.0 / 72.0);
		return FMath::RoundToInt64(PixelsPerEm * Units / FINMonospaceUnitsPerEm);
	}

	FVector2D MeasureMonospaceText(const FString& Text, int64 Size) {
		int32 Lines = 1;
		int32 LineLength = 0;
		int32 MaxLineLength = 0;
		for (TCHAR Char : Text) {
			if (Char == '\n') {
				Lines++;
				LineLength = 0;
			} else {
				MaxLineLength = FMath::Max(MaxLineLength, ++LineLength);
			}
		}
		return FVector2D(MaxLineLength * GetMonospaceMetric(Size, FINMonospaceAdvance), Lines * GetMonospaceMetric(Size, FINMonospaceHeight));
	}

	void CheckMonospaceOnly(bool bMonospace) {
		if (!bMonospace) throw FFIRException(TEXT("Measuring the non-monospace font is not available on dedicated servers"));
	}

	FVector2D MeasureText(const TSharedPtr<FSlateFontMeasure>& Measure, const FFINGPUT2WidgetStyle& Style, const FString& Text, int64 Size, bool bMonospace) {
		if (!Measure) {
			CheckMonospaceOnly(bMonospace);
			return MeasureMonospaceText(Text, Size);
		}
		FSlateFontInfo Font = bMonospace ? Style.MonospaceText : Style.NormalText;
		Font.Size = Size;
		return Measure->Measure(Text, Font);
	}

	int64 GetFontHeight(const TSharedPtr<FSlateFontMeasure>& Measure, const FFINGPUT2WidgetStyle& Style, int64 Size, bool bMonospace) {
		if (!Measure) {
			CheckMonospaceOnly(bMonospace);
			return GetMonospaceMetric(Size, FINMonospaceHeight);
		}
		FSlateFontInfo Font = bMonospace ? Style.MonospaceText : Style.NormalText;
		Font.Size = Size;
		return Measure->GetMaxCharacterHeight(Font);
	}

	int64 GetFontBaseline(const TSharedPtr<FSlateFontMeasure>& Measure, const FFINGPUT2WidgetStyle& Style, int64 Size, bool bMonospace) {
		if (!Measure) {
			CheckMonospaceOnly(bMonospace);
			return GetMonospaceMetric(Size, FINMonospaceBaseline);
		}
		FSlateFontInfo Font = bMonospace ? Style.MonospaceText : Style.NormalText;
		Font.Size = Size;
		return Measure->GetBaseline(Font);
	}
}

FVector2D AFINComputerGPUT2::netFunc_measureText(FString text, int64 size, bool bMonospace) {
	return MeasureText(GetFontMeasure(), Style, text, size, bMonospace);
}

int64 AFINComputerGPUT2::netFunc_getFontHeight(int64 size, bool bMonospace) {
	return GetFontHeight(GetFontMeasure(), Style, size, bMonospace);
}

int64 AFINComputerGPUT2::netFunc_getFontBaseline(int64 size, bool bMonospace) {
	return GetFontBaseline(GetFontMeasure(), Style, size, bMonospace);
}

TArray<FVector2D> AFINComputerGPUT2::netFunc_measureTextBatch(TArray<FString> text, TArray<int64> size, TArray<bool> bMonospace) {
	size_t n = std::min(std::min(text.Num(), size.Num()), bMonospace.Num());

	TArray<FVector2D> results;
	results.Reserve(n);

	TSharedPtr<FSlateFontMeasure> Measure = GetFontMeasure();

	for (size_t i = 0; i < n; i++) {
		results.Add(MeasureText(Measure, Style, text[i], size[i], bMonospace[i]));
	}

	return results;
}

TArray<int64> AFINComputerGPUT2::netFunc_getFontHeightBatch(TArray<int64> size, TArray<bool> bMonospace) {
	size_t n = std::min(size.Num(), bMonospace.Num());

	TArray<int64> results;
	results.Reserve(n);

	TSharedPtr<FSlateFontMeasure> Measure = GetFontMeasure();

	for (size_t i = 0; i < n; i++) {
		results.Add(GetFontHeight(Measure, Style, size[i], bMonospace[i]));
	}

	return results;
}

TArray<int64> AFINComputerGPUT2::netFunc_getFontBaselineBatch(TArray<int64> size, TArray<bool> bMonospace) {
	size_t n = std::min(size.Num(), bMonospace.Num());

	TArray<int64> results;
	results.Reserve(n);

	TSharedPtr<FSlateFontMeasure> Measure = GetFontMeasure();

	for (size_t i = 0; i < n; i++) {
		results.Add(GetFontBaseline(Measure, Style, size[i], bMonospace[i]));
	}

	return results;
}

void AFINComputerGPUT2::Client_CleanDrawCalls_Implementation() {
	if (HasAuthority()) return;
	BackBufferDrawCalls.Empty();
}

void AFINComputerGPUT2::Client_AddDrawCallChunk_Implementation(const TArray<FFIRInstancedStruct>& Chunk) {
	if (HasAuthority()) return;
	BackBufferDrawCalls.Append(Chunk);
}

void AFINComputerGPUT2::Client_FlushDrawCalls_Implementation() {
	if (HasAuthority()) return;
	FlushDrawCalls();
}

