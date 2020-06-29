#include "FINComputerGPU.h"

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
	if (MouseEvent.IsCommandDown())								mouseEvent |= 0b0000010000;
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

		TArray<FLinearColor> ForgroundLine;
		if (Pos.Y < this->Foreground.Get().Num()) {
			ForgroundLine = this->Foreground.Get()[Pos.Y];
		}
		TArray<FLinearColor> BackgroundLine;
		if (Pos.Y < this->Background.Get().Num()) {
			BackgroundLine = this->Background.Get()[Pos.Y];
		}
		
		for (Pos.X = 0; Pos.X < ScreenSize.X && Pos.X < Line.Len(); ++Pos.X) {
			FLinearColor Foreground = FLinearColor(1,1,1,1);
			if (Pos.X < ForgroundLine.Num()) {
				Foreground = ForgroundLine[Pos.X];
			}
			FLinearColor Background = FLinearColor(1,1,1,1);
			if (Pos.X < BackgroundLine.Num()) {
				Background = BackgroundLine[Pos.X];
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

AFINComputerGraphicsProcessor::AFINComputerGraphicsProcessor() {
	SetScreenSize(FVector2D(120, 30));
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerGraphicsProcessor::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (shouldCreate) {
		shouldCreate = false;
		if (!IsValid(Screen)) return;
		if (!Widget.IsValid()) CreateWidget();
		Cast<IFINScreen>(Screen)->SetWidget(Widget);
	}
}

void AFINComputerGraphicsProcessor::BeginPlay() {
	Super::BeginPlay();
}

bool AFINComputerGraphicsProcessor::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerGraphicsProcessor::BindScreen(UObject* screen) {
	if (screen) check(screen->GetClass()->ImplementsInterface(UFINScreen::StaticClass()))
	if (Screen == screen) return;
	UObject* oldScreen = Screen;
	Screen = nullptr;
	if (oldScreen) Cast<IFINScreen>(oldScreen)->BindGPU(nullptr);
	Screen = screen;
	if (screen) Cast<IFINScreen>(screen)->BindGPU(this);
	if (!screen) DropWidget();
}

UObject* AFINComputerGraphicsProcessor::GetScreen() const {
	return Screen;
}

void AFINComputerGraphicsProcessor::RequestNewWidget() {
	shouldCreate = true;
}

void AFINComputerGraphicsProcessor::DropWidget() {
	Widget.Reset();
}

void AFINComputerGraphicsProcessor::CreateWidget() {
	boxBrush = LoadObject<USlateBrushAsset>(NULL, TEXT("SlateBrushAsset'/Game/FicsItNetworks/Computer/UI/ComputerCaseBorder.ComputerCaseBorder'"))->Brush;
	Widget = SNew(SScaleBox)
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

void AFINComputerGraphicsProcessor::SetScreenSize(FVector2D size) {
	ScreenSize = size;
	for (int i = 0; i < size.Y; ++i) {
		if (i >= TextGrid.Num()) TextGrid.Add("");
		FString& text = TextGrid[i];
		if (text.Len() < size.X) text = text.Append(FString::ChrN(size.X - text.Len(), ' '));
		else text = text.Left(size.X);

		if (i >= Foreground.Num()) Foreground.Add(TArray<FLinearColor>());
		if (i >= Background.Num()) Background.Add(TArray<FLinearColor>());
		TArray<FLinearColor>& LineForeground = Foreground[i];
		TArray<FLinearColor>& LineBackground = Background[i];

		for (int x = 0; x < size.X; ++x) {
			if (x >= LineForeground.Num()) LineForeground.Add(CurrentForeground);
			if (x >= LineBackground.Num()) LineBackground.Add(CurrentBackground);
		}
	}
}

void AFINComputerGraphicsProcessor::netSig_OnMouseDown_Implementation(int x, int y, int btn) {}
void AFINComputerGraphicsProcessor::netSig_OnMouseUp_Implementation(int x, int y, int btn) {}
void AFINComputerGraphicsProcessor::netSig_OnMouseMove_Implementation(int x, int y, int btn) {}

void AFINComputerGraphicsProcessor::netFunc_bindScreen(UObject* Screen) {
	if (Screen == nullptr) {
		BindScreen(nullptr);
		return;
	}
	if (!Screen->GetClass()->ImplementsInterface(UFINScreen::StaticClass())) return;
	BindScreen(Screen);
}

UObject* AFINComputerGraphicsProcessor::netFunc_getScreen() {
	return Screen;
}

void AFINComputerGraphicsProcessor::netFunc_setText(int x, int y, const FString& str) {
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
					Foreground[y][x + dx] = CurrentForeground;
					Background[y][x + dx] = CurrentBackground;
				}
			}
			x = oldX;
			if (returned) x = 0;
		}
		if (newLine) ++y;
	}
}

void AFINComputerGraphicsProcessor::netFunc_fill(int x, int y, int dx, int dy, const FString& str) {
	FString c = str;
	if (FRegexMatcher(FRegexPattern("^[[:cntrl:]]?$"), c).FindNext()) c = " ";
	for (int ny = y; ny < y+dy; ++ny) {
		netFunc_setText(x, ny, FString::ChrN(dx, c[0]));
	}
}

void AFINComputerGraphicsProcessor::netFunc_getSize(int& w, int& h) {
	w = ScreenSize.X;
	h = ScreenSize.Y;
}

void AFINComputerGraphicsProcessor::netFunc_setSize(int w, int h) {
	ScreenSize = FVector2D(w, h);
}

void AFINComputerGraphicsProcessor::netFunc_setForeground(float r, float g, float b, float a) {
	CurrentForeground = FLinearColor(r, g, b, a);
}

void AFINComputerGraphicsProcessor::netFunc_setBackground(float r, float g, float b, float a) {
	CurrentBackground = FLinearColor(r, g, b, a);
}
