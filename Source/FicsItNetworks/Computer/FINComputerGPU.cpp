#include "FINComputerGPU.h"

#include <openexr/Deploy/include/ImathMath.h>


#include "WidgetBlueprintLibrary.h"
#include "WidgetLayoutLibrary.h"
#include "FicsItNetworks/Graphics/FINScreen.h"
#include "util/Logging.h"

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
	//SetActorTickEnabled(screen);
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
	FSlateFontInfo font;
	Widget = SNew(SScaleBox)
	.Stretch(EStretch::ScaleToFit)
	.Content()
	[
		(border = SNew(SBorder)
		.BorderImage(&boxBrush)
		.Padding(40)
		.Content()
		[
			(text = SNew(STextBlock)
			.SimpleTextMode(false)
			.TextShapingMethod(ETextShapingMethod::FullShaping)
			.Margin(0)
			.Text_Lambda([this]() {
				if (hasChanged) {
                	hasChanged = false;
                	UpdateTextBlockText();
                }
                return DisplayText;
			})
			.Font(font = FSlateFontInfo(LoadObject<UObject>(NULL, TEXT("Font'/Game/FicsItNetworks/GuiHelpers/Inconsolata_Font.Inconsolata_Font'")), 12, "InConsolata"))
			.Justification(ETextJustify::Center)
			).ToSharedRef()
		]
		.OnMouseButtonDown_Lambda([this, font](const FGeometry& g, const FPointerEvent& e) {
			FVector2D pos1 = e.GetScreenSpacePosition();
			FVector2D pos2 = pos1 * UWidgetLayoutLibrary::GetViewportScale(this);
			FVector2D pos3 = g.AbsoluteToLocal(pos2);
			FVector2D pos4 = g.AbsoluteToLocal(pos1); // local
			FVector2D pos5 = pos4 * UWidgetLayoutLibrary::GetViewportScale(this);

			FVector2D des = text->GetDesiredSize();
			FVector2D abs = g.GetAbsoluteSize();
			FVector2D local = g.GetLocalSize();
			FVector2D draw = g.GetDrawSize();
			
			SML::Logging::error("Pos1: ", pos1.X, " ", pos1.Y);
			SML::Logging::error("Pos2: ", pos2.X, " ", pos2.Y);
			SML::Logging::error("Pos3: ", pos3.X, " ", pos3.Y);
			SML::Logging::error("Pos4: ", pos4.X, " ", pos4.Y);
			SML::Logging::error("Pos5: ", pos5.X, " ", pos5.Y);
			SML::Logging::error("Desired : ", des.X, " ", des.Y);
			SML::Logging::error("Absolute: ", abs.X, " ", abs.Y);
			SML::Logging::error("Local   : ", local.X, " ", local.Y);
			SML::Logging::error("Draw   : ", draw.X, " ", draw.Y);

			FVector2D localScale = local / ScreenSize;
			FVector2D localScaleM = (local - 80) / ScreenSize;

			FVector2D localCharPos = pos4 / localScale;
			FVector2D localMCharPos = (pos4 - 40) / localScaleM;

			SML::Logging::error("LocalCharPos:", localCharPos.X, " ", localCharPos.Y);
			SML::Logging::error("LocalMCharPos:", localMCharPos.X, " ", localMCharPos.Y);
			
			netSig_OnClick(localMCharPos.X, localMCharPos.Y);
			return FReply::Handled();
		})).ToSharedRef()
	];
}

void AFINComputerGraphicsProcessor::UpdateTextBlockText() {
	FString text;
	for (const FString& line : TextGrid) {
		text += line + "\r\n";
	}
	if (text.Len() > 0) text.RemoveAt(text.Len()-3, 2);
	DisplayText = FText::FromString(text);
}

void AFINComputerGraphicsProcessor::SetScreenSize(FVector2D size) {
	ScreenSize = size;
	for (int i = 0; i < size.Y; ++i) {
		if (i >= TextGrid.Num()) TextGrid.Add("");
		FString& text = TextGrid[i];
		if (text.Len() < size.X) text = text.Append(FString::ChrN(size.X - text.Len(), ' '));
		else text = text.Left(size.X);
	}
}

void AFINComputerGraphicsProcessor::netSig_OnClick_Implementation(int x, int y) {}

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
			}
			x = oldX;
			if (returned) x = 0;
		}
		if (newLine) ++y;
	}
	hasChanged = true;
}
