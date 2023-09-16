#include "Computer/FINComputerGPUT2.h"

#include "FGPlayerController.h"
#include "Computer/FINComputerRCO.h"
#include "Interfaces/ISlateNullRendererModule.h"

const FName FFINGPUT2WidgetStyle::TypeName(TEXT("FFINReflectionUIStyleStruct"));

int32 FFINGPUT2DC_Line::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), {Start, End}, ESlateDrawEffect::None, Color, true, Thickness);
	return LayerId;
}

int32 FFINGPUT2DC_Ellipse::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	return FFINGPUT2DrawCall::OnPaint(Args, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
}

int32 FFINGPUT2DC_Text::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const {
	FSlateFontInfo Font = FSlateFontInfo(LoadObject<UObject>(NULL, TEXT("Font'/FicsItNetworks/UI/FiraCode.FiraCode'")), Size, "FiraCode-Regular");
	FSlateDrawElement::MakeText(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(Position)), Text, Font, ESlateDrawEffect::None, Color);
	return LayerId;
}

void FFINGPUT2WidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	FSlateWidgetStyle::GetResources(OutBrushes);
}

const FFINGPUT2WidgetStyle& FFINGPUT2WidgetStyle::GetDefault() {
	static FFINGPUT2WidgetStyle* Default = nullptr;
	if (!Default) Default = new FFINGPUT2WidgetStyle();
	return *Default;
}

void SFINGPUT2Widget::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	DrawCalls = InArgs._DrawCalls;
	OnMouseDownEvent = InArgs._OnMouseDown;
	OnMouseLeaveEvent = InArgs._OnMouseLeave;
	OnMouseMoveEvent = InArgs._OnMouseMove;
	OnMouseEnterEvent = InArgs._OnMouseEnter;
	OnMouseLeaveEvent = InArgs._OnMouseLeave;
	OnKeyDownEvent = InArgs._OnKeyDown;
	OnKeyUpEvent = InArgs._OnKeyUp;
	OnKeyCharEvent = InArgs._OnKeyChar;
}

FVector2D SFINGPUT2Widget::ComputeDesiredSize(float LayoutScaleMultiplier) const {
	return FVector2D(100, 100);
}

int32 SFINGPUT2Widget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	for (const FFINDynamicStructHolder& DrawCallBase : DrawCalls.Get()) {
		FFINGPUT2DrawCall* DrawCall = DrawCallBase.GetPtr<FFINGPUT2DrawCall>();
		if (DrawCall) LayerId = DrawCall->OnPaint(Args, AllottedGeometry, OutDrawElements, LayerId, InWidgetStyle);
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
		TArray<FFINDynamicStructHolder> Chunk;
		for (int i = 0; i < 10; ++i) {
			FFINDynamicStructHolder* DrawCall = DrawCalls2Send.Peek();
			if (!DrawCall) break;
			Chunk.Add(*DrawCall);
			DrawCalls2Send.Pop();
		}
		Client_AddDrawCallChunk(Chunk);
		if (DrawCalls2Send.IsEmpty()) {
			Client_FlushDrawCalls();
		}
	} else if (bFlushOverNetwork) {
		Client_CleanDrawCalls();
		bFlushOverNetwork = false;
		for (const FFINDynamicStructHolder& DrawCall : FlushedDrawCalls) {
			DrawCalls2Send.Enqueue(DrawCall);
		}
	}
}

TSharedPtr<SWidget> AFINComputerGPUT2::CreateWidget() {
	UFINComputerRCO* RCO = Cast<UFINComputerRCO>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
	return SNew(SFINGPUT2Widget)
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
		return FlushedDrawCalls;
	});
}

void AFINComputerGPUT2::FlushDrawCalls() {
	FlushedDrawCalls = DrawCalls;
	DrawCalls.Empty();
}

void AFINComputerGPUT2::netFunc_flush() {
	FlushDrawCalls();
	bFlushOverNetwork = true;
}

void AFINComputerGPUT2::netFunc_drawLine(FVector2D start, FVector2D end, double thickness, FLinearColor color) {
	DrawCalls.Add(FFINGPUT2DC_Line(start, end, thickness, color.QuantizeRound()));
}

void AFINComputerGPUT2::netFunc_drawEllipse(FVector2D position, double radius1, double radius2, double rotation, double thickness, FLinearColor color) {
	DrawCalls.Add(FFINGPUT2DC_Ellipse(position, radius1, radius2, rotation, thickness, color.QuantizeRound()));
}

void AFINComputerGPUT2::netFunc_drawText(FVector2D position, const FString& text, double size, FLinearColor color) {
	DrawCalls.Add(FFINGPUT2DC_Text(position, text, size, color.QuantizeRound()));
}

FVector2D AFINComputerGPUT2::netFunc_measureText(FString text, int64 size, bool bMonospace) {
	FSlateFontInfo Font = bMonospace ? Style.MonospaceText : Style.NormalText;
	Font.Size = size;

	/*TSharedRef<ISlateFontAtlasFactory> AtlasFactory = FModuleManager::Get().LoadModuleChecked<ISlateNullRendererModule>("SlateNullRenderer").CreateSlateFontAtlasFactory();
	TSharedRef<FSlateFontCache> Cache = MakeShared<FSlateFontCache>(AtlasFactory, ESlateTextureAtlasThreadId::Unknown);
	TSharedRef<FSlateFontMeasure> Measure = FSlateFontMeasure::Create(Cache);*/

	TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	
	return Measure->Measure(text, Font);
}

void AFINComputerGPUT2::netSig_OnMouseDown_Implementation(FVector2D position, int modifiers) {}
void AFINComputerGPUT2::netSig_OnMouseUp_Implementation(FVector2D position, int modifiers) {}
void AFINComputerGPUT2::netSig_OnMouseMove_Implementation(FVector2D position, int modifiers) {}
void AFINComputerGPUT2::netSig_OnKeyDown_Implementation(int64 c, int64 code, int modifiers) {}
void AFINComputerGPUT2::netSig_OnKeyUp_Implementation(int64 c, int64 code, int modifiers) {}
void AFINComputerGPUT2::netSig_OnKeyChar_Implementation(const FString& c, int modifiers) {}
void AFINComputerGPUT2::netSig_OnMouseEnter_Implementation(FVector2D position, int modifiers) {}
void AFINComputerGPUT2::netSig_OnMouseLeave_Implementation(FVector2D position, int modifiers) {}

void AFINComputerGPUT2::Client_CleanDrawCalls_Implementation() {
	if (HasAuthority()) return;
	DrawCalls.Empty();
}

void AFINComputerGPUT2::Client_AddDrawCallChunk_Implementation(const TArray<FFINDynamicStructHolder>& Chunk) {
	if (HasAuthority()) return;
	DrawCalls.Append(Chunk);
}

void AFINComputerGPUT2::Client_FlushDrawCalls_Implementation() {
	if (HasAuthority()) return;
	FlushDrawCalls();
}

