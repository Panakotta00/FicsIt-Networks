#include "FINComputerGPUT1.h"

#include "WidgetBlueprintLibrary.h"
#include "WidgetLayoutLibrary.h"
#include "FicsItNetworks/Graphics/FINScreen.h"
#include "util/Logging.h"

void SScreenMonitor::Construct(const FArguments& InArgs) {
	Text = InArgs._Text;
	Foreground = InArgs._Foreground;
	Background = InArgs._Background;
	Font = InArgs._Font;
	ScreenSize = InArgs._ScreenSize;
	OnMouseDownEvent = InArgs._OnMouseDown;
	OnMouseUpEvent = InArgs._OnMouseUp;
	OnMouseMoveEvent = InArgs._OnMouseMove;
	SetCanTick(false);
}

TArray<FString> SScreenMonitor::GetText() const {
	return Text.Get();
}

FVector2D SScreenMonitor::GetScreenSize() const {
	return ScreenSize.Get();
}

void SScreenMonitor::SetScreenSize(FVector2D ScreenSize) {
	this->ScreenSize = ScreenSize;
}

FVector2D SScreenMonitor::GetCharSize() const {
	FSlateApplication& app = FSlateApplication::Get();
	FSlateRenderer* renderer = app.GetRenderer();
	TSharedRef<FSlateFontMeasure> measure = renderer->GetFontMeasureService();
	return measure->Measure(L" ", Font.Get());
}

FVector2D SScreenMonitor::LocalToCharPos(FVector2D Pos) const {
	return Pos / GetCharSize();
}

int SScreenMonitor::MouseToInt(const FPointerEvent& MouseEvent) {
	int mouseEvent = 0;
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))	mouseEvent |= 0b0000000001;
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))	mouseEvent |= 0b0000000010;
	if (MouseEvent.IsControlDown())								mouseEvent |= 0b0000000100;
	if (MouseEvent.IsShiftDown())								mouseEvent |= 0b0000001000;
	if (MouseEvent.IsAltDown())									mouseEvent |= 0b0000010000;
	if (MouseEvent.IsCommandDown())								mouseEvent |= 0b0000100000;
	return mouseEvent;
}

FVector2D SScreenMonitor::ComputeDesiredSize(float f) const {
	return GetCharSize() * ScreenSize.Get();
}

int32 SScreenMonitor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	FSlateApplication& app = FSlateApplication::Get();
	FSlateRenderer* renderer = app.GetRenderer();
	TSharedRef<FSlateFontMeasure> measure = renderer->GetFontMeasureService();
	FVector2D CharSize = measure->Measure(L" ", Font.Get());
	FVector2D ScreenSize = this->ScreenSize.Get();
	FSlateBrush boxBrush = FSlateBrush();
	
	const TArray<FString>& TextGrid = Text.Get();
	for (FVector2D Pos = FVector2D::ZeroVector; Pos.Y < ScreenSize.Y && Pos.Y < TextGrid.Num(); ++Pos.Y) {
		const FString Line = TextGrid[Pos.Y];
		
		for (Pos.X = 0; Pos.X < ScreenSize.X && Pos.X < Line.Len(); ++Pos.X) {
			FLinearColor Foreground = FLinearColor(1,1,1,1);
			if (Pos.Y * ScreenSize.X + Pos.X < this->Foreground.Get().Num()) {
				Foreground = this->Foreground.Get()[Pos.Y * ScreenSize.X + Pos.X];
			}
			FLinearColor Background = FLinearColor(1,1,1,1);
			if (Pos.Y * ScreenSize.X + Pos.X < this->Background.Get().Num()) {
				Background = this->Background.Get()[Pos.Y * ScreenSize.X + Pos.X];
			}

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(Pos * CharSize, CharSize, 1),
				&boxBrush,
				ESlateDrawEffect::None,
				InWidgetStyle.GetColorAndOpacityTint() * Background);
			FSlateDrawElement::MakeText(
		        OutDrawElements,
		        LayerId,
		        AllottedGeometry.ToOffsetPaintGeometry(Pos * CharSize),
		        Line.Mid(Pos.X,1),
		        Font.Get(),
		        ESlateDrawEffect::None,
		        InWidgetStyle.GetColorAndOpacityTint() * Foreground
            );
		}
	}
	return LayerId;
}

FReply SScreenMonitor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D CharPos = LocalToCharPos(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	
	if (OnMouseDownEvent.IsBound()) return OnMouseDownEvent.Execute(CharPos.X, CharPos.Y, MouseToInt(MouseEvent));
	return FReply::Unhandled();
}

FReply SScreenMonitor::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D CharPos = LocalToCharPos(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	if (OnMouseUpEvent.IsBound()) return OnMouseUpEvent.Execute(CharPos.X, CharPos.Y, MouseToInt(MouseEvent));
	return FReply::Unhandled(); 
}

FReply SScreenMonitor::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D CharPos = LocalToCharPos(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	int x = CharPos.X;
	int y = CharPos.Y;
	if (lastMoveX == x && lastMoveY == y) return FReply::Unhandled();
	lastMoveX = x;
	lastMoveY = y;
	if (OnMouseMoveEvent.IsBound()) return OnMouseMoveEvent.Execute(x, y, MouseToInt(MouseEvent));
	return FReply::Unhandled();
}

SScreenMonitor::SScreenMonitor() {
	
}

AFINComputerGPUT1::AFINComputerGPUT1() {
	SetScreenSize(FVector2D(120, 30));
}

TSharedPtr<SWidget> AFINComputerGPUT1::CreateWidget() {
	boxBrush = LoadObject<USlateBrushAsset>(NULL, TEXT("SlateBrushAsset'/Game/FicsItNetworks/Computer/UI/ComputerCaseBorder.ComputerCaseBorder'"))->Brush;
	return SNew(SScaleBox)
	.Stretch(EStretch::ScaleToFit)
	.Content()
	[
		SNew(SBorder)
		.BorderImage(&boxBrush)
		.Padding(40)
		.Content()
		[
			SNew(SScreenMonitor)
			.ScreenSize_Lambda([this]() {
				return ScreenSize;
			})
			.Text_Lambda([this]() {
                return TextGrid;
			})
			.Foreground_Lambda([this]() {
				return Foreground;
			})
			.Background_Lambda([this]() {
				return Background;
			})
			.Font(FSlateFontInfo(LoadObject<UObject>(NULL, TEXT("Font'/Game/FicsItNetworks/GuiHelpers/Inconsolata_Font.Inconsolata_Font'")), 12, "InConsolata"))
			.OnMouseDown_Lambda([this](int x, int y, int btn) {
				netSig_OnMouseDown(x, y, btn);
				return FReply::Handled();
			})
			.OnMouseUp_Lambda([this](int x, int y, int btn) {
                netSig_OnMouseUp(x, y, btn);
                return FReply::Handled();
            })
            .OnMouseMove_Lambda([this](int x, int y, int btn) {
                netSig_OnMouseMove(x, y, btn);
                return FReply::Handled();
            })
		]
	];
}

void AFINComputerGPUT1::SetScreenSize(FVector2D size) {
	if (ScreenSize == size) return;
	FVector2D oldScreenSize = ScreenSize;
	ScreenSize = size;

	TextGrid.Empty();
	Foreground.Empty();
	Background.Empty();

	for (int y = 0; y < size.Y; ++y) {
		TextGrid.Add(FString::ChrN(size.X, ' '));
		for (int x = 0; x < size.X; ++x) {
			Foreground.Add(CurrentForeground);
			Background.Add(CurrentBackground);
		}
	}

	netSig_ScreenSizeChanged(oldScreenSize.X, oldScreenSize.Y);
}

void AFINComputerGPUT1::netSig_OnMouseDown_Implementation(int x, int y, int btn) {}
void AFINComputerGPUT1::netSig_OnMouseUp_Implementation(int x, int y, int btn) {}
void AFINComputerGPUT1::netSig_OnMouseMove_Implementation(int x, int y, int btn) {}
void AFINComputerGPUT1::netSig_ScreenSizeChanged_Implementation(int oldW, int oldH) {}

void AFINComputerGPUT1::netFunc_bindScreen(UObject* Screen) {
	if (Screen == nullptr) {
		BindScreen(nullptr);
		return;
	}
	if (!Screen->GetClass()->ImplementsInterface(UFINScreen::StaticClass())) return;
	BindScreen(Screen);
}

UObject* AFINComputerGPUT1::netFunc_getScreen() {
	return Screen;
}

void AFINComputerGPUT1::netFunc_setText(int x, int y, const FString& str) {
	FString toSet = str;
	while (toSet.Len() > 0) {
		FString Line;
		bool newLine = toSet.Split("\n", &Line, &toSet);
		if (!newLine) {
			Line = toSet;
			toSet = "";
		}
		while (Line.Len() > 0) {
			FString inLine;
			bool returned = Line.Split("\r", &inLine, &Line);
			if (!returned) {
				inLine = Line;
				Line = "";
			}
			int oldX = x + inLine.Len();
			if (y >= 0 && x < ScreenSize.X && y < ScreenSize.Y) {
				if (x < 0) {
					inLine.RemoveAt(0, FMath::Abs(x));
					x = 0;
				}
				FString& text = TextGrid[y];
				text.RemoveAt(x, inLine.Len());
				text.InsertAt(x, inLine);
				text = text.Left(ScreenSize.X);
				for (int dx = 0; dx < inLine.Len(); ++dx) {
					Foreground[y * ScreenSize.X + x + dx] = CurrentForeground;
					Background[y * ScreenSize.X + x + dx] = CurrentBackground;
				}
			}
			x = oldX;
			if (returned) x = 0;
		}
		if (newLine) ++y;
	}
}

void AFINComputerGPUT1::netFunc_fill(int x, int y, int dx, int dy, const FString& str) {
	FString c = str;
	if (FRegexMatcher(FRegexPattern("^[[:cntrl:]]?$"), c).FindNext()) c = " ";
	for (int ny = y; ny < y+dy; ++ny) {
		netFunc_setText(x, ny, FString::ChrN(dx, c[0]));
	}
}

void AFINComputerGPUT1::netFunc_getSize(int& w, int& h) {
	w = ScreenSize.X;
	h = ScreenSize.Y;
}

void AFINComputerGPUT1::netFunc_setSize(int w, int h) {
	SetScreenSize(FVector2D(FMath::Clamp(w, 1, 150), FMath::Clamp(h, 1, 50)));
}

void AFINComputerGPUT1::netFunc_setForeground(float r, float g, float b, float a) {
	CurrentForeground = FLinearColor(FMath::Clamp(r, 0.0f, 1.0f), FMath::Clamp(g, 0.0f, 1.0f), FMath::Clamp(b, 0.0f, 1.0f), FMath::Clamp(a, 0.0f, 1.0f));
}

void AFINComputerGPUT1::netFunc_setBackground(float r, float g, float b, float a) {
	CurrentBackground = FLinearColor(FMath::Clamp(r, 0.0f, 1.0f), FMath::Clamp(g, 0.0f, 1.0f), FMath::Clamp(b, 0.0f, 1.0f), FMath::Clamp(a, 0.0f, 1.0f));
}
