#include "Computer/FINComputerGPUT1.h"
#include "FINComponentUtility.h"
#include "Computer/FINComputerRCO.h"
#include "Graphics/FINScreenInterface.h"
#include "FGPlayerController.h"
#include "Async/ParallelFor.h"
#include "Engine/NetworkSettings.h"
#include "Net/UnrealNetwork.h"
#include "Slate/SlateBrushAsset.h"
#include "Widgets/SInvalidationPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

const FFINGPUT1BufferPixel FFINGPUT1BufferPixel::InvalidPixel;

void SScreenMonitor::Construct(const FArguments& InArgs, UObject* InWorldContext) {
	Buffer = InArgs._Buffer;
	Font = InArgs._Font;
	OnMouseDownEvent = InArgs._OnMouseDown;
	OnMouseUpEvent = InArgs._OnMouseUp;
	OnMouseMoveEvent = InArgs._OnMouseMove;
	OnKeyDownEvent = InArgs._OnKeyDown;
	OnKeyUpEvent = InArgs._OnKeyUp;
	OnKeyCharEvent = InArgs._OnKeyChar;
	SetCanTick(false);
	WorldContext = InWorldContext;
}

FFINGPUT1Buffer SScreenMonitor::GetBuffer() const {
	const FFINGPUT1Buffer* Buf = Buffer.Get();
	if (Buf) return *Buf;
	else return FFINGPUT1Buffer();
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

FVector2D SScreenMonitor::ComputeDesiredSize(float f) const {
	const FFINGPUT1Buffer* Buf = Buffer.Get();
	int X, Y;
	Buf->GetSize(X, Y);
	return GetCharSize() * FVector2D(X, Y);
}

int32 SScreenMonitor::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	FVector2D CharSize = GetCharSize();
	const FFINGPUT1Buffer* Buf = Buffer.Get();
	int Width, Height;
	Buf->GetSize(Width, Height);
	FSlateBrush boxBrush = FSlateBrush();
	FSlateFontInfo FontToUse = Font.Get();
	
	for (int Y = 0; Y < Height; ++Y) {
		for (int X = 0; X < Width; ++X) {
			const FFINGPUT1BufferPixel& Pixel = Buf->Get(X, Y);
			
			FString Char = FString::Chr(Pixel.Character);
			if (Char.TrimStartAndEnd().Len() > 0) FSlateDrawElement::MakeText(
                OutDrawElements,
                LayerId+1,
                AllottedGeometry.ToOffsetPaintGeometry(FVector2D(X,Y) * CharSize),
                *Char,
				FontToUse,
                ESlateDrawEffect::None,
                Pixel.ForegroundColor
            );
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2D(X, Y) * CharSize, (CharSize*1), 1),
				&boxBrush,
				ESlateDrawEffect::None,
				Pixel.BackgroundColor);
		}
	}
	return LayerId+1;
}

FReply SScreenMonitor::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D CharPos = LocalToCharPos(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	
	if (OnMouseDownEvent.IsBound()) return OnMouseDownEvent.Execute(CharPos.X, CharPos.Y, AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Unhandled();
}

FReply SScreenMonitor::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D CharPos = LocalToCharPos(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	if (OnMouseUpEvent.IsBound()) return OnMouseUpEvent.Execute(CharPos.X, CharPos.Y, AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Unhandled(); 
}

FReply SScreenMonitor::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D CharPos = LocalToCharPos(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	int x = CharPos.X;
	int y = CharPos.Y;
	if (lastMoveX == x && lastMoveY == y) return FReply::Unhandled();
	lastMoveX = x;
	lastMoveY = y;
	if (OnMouseMoveEvent.IsBound()) return OnMouseMoveEvent.Execute(x, y, AFINComputerGPU::MouseToInt(MouseEvent));
	return FReply::Unhandled();
}

FReply SScreenMonitor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (HandleShortCut(InKeyEvent)) return FReply::Handled();
	if (OnKeyDownEvent.IsBound()) return OnKeyDownEvent.Execute(InKeyEvent.GetCharacter(), InKeyEvent.GetKeyCode(), AFINComputerGPU::InputToInt(InKeyEvent));
	return FReply::Unhandled();
}

FReply SScreenMonitor::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (OnKeyUpEvent.IsBound()) return OnKeyUpEvent.Execute(InKeyEvent.GetCharacter(), InKeyEvent.GetKeyCode(), AFINComputerGPU::InputToInt(InKeyEvent));
	return FReply::Unhandled();
}

FReply SScreenMonitor::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) {
	if (OnKeyCharEvent.IsBound()) return OnKeyCharEvent.Execute(InCharacterEvent.GetCharacter(), AFINComputerGPU::InputToInt(InCharacterEvent));
	return FReply::Unhandled();
}

bool SScreenMonitor::IsInteractable() const {
	return true;
}

bool SScreenMonitor::SupportsKeyboardFocus() const {
	return true;
}

bool IsAction(UObject* Context, const FKeyEvent& InKeyEvent, const FName& ActionName) {
	TArray<FInputActionKeyMapping> Mappings = Context->GetWorld()->GetFirstPlayerController()->PlayerInput->GetKeysForAction(ActionName);
	if (Mappings.Num() > 0) {
		const FInputActionKeyMapping& Mapping = Mappings[0];
		return Mapping.Key == InKeyEvent.GetKey() &&
			Mapping.bAlt == InKeyEvent.GetModifierKeys().IsAltDown() &&
			Mapping.bCmd == InKeyEvent.GetModifierKeys().IsCommandDown() &&
			Mapping.bCtrl == InKeyEvent.GetModifierKeys().IsControlDown() &&
			Mapping.bShift == InKeyEvent.GetModifierKeys().IsShiftDown();
	}
	return false;
}

bool SScreenMonitor::HandleShortCut(const FKeyEvent& InKeyEvent) {
	if (IsAction(WorldContext, InKeyEvent, TEXT("FicsItNetworks.CopyScreen"))) {
		int Width, Height;
		Buffer.Get()->GetSize(Width, Height);
		FString AllText = Buffer.Get()->GetAsText();
		FString FormattedText = "";
		int i = 0;
		while (i < AllText.Len()) {
			FormattedText += AllText.Mid(i, Width).TrimEnd() + '\n';
			i += Width;
		}
		UFINComponentUtility::ClipboardCopy(FormattedText);
		return true;
	} else if (IsAction(WorldContext, InKeyEvent, TEXT("FicsItNetworks.PasteScreen"))) {
		FString PasteText;
		FWindowsPlatformApplicationMisc::ClipboardPaste(PasteText);
		for (TCHAR CharKey : PasteText) {
			FCharacterEvent CharacterEvent(CharKey, FModifierKeysState(), InKeyEvent.GetUserIndex(), false);
			FSlateApplication::Get().ProcessKeyCharEvent(CharacterEvent);
		}
		return true;
	}
	return false;
}

SScreenMonitor::SScreenMonitor() {
	
}

void AFINComputerGPUT1::Multicast_BeginBackBufferReplication_Implementation(FIntPoint Size) {
	if (!HasAuthority()) {
		BackBuffer.SetSize(Size.X, Size.Y);
	}
}

void AFINComputerGPUT1::Multicast_AddBackBufferChunk_Implementation(int64 InOffset, const TArray<FFINGPUT1BufferPixel>& Chunk) {
	if (!HasAuthority()) {
		BackBuffer.SetChunk(InOffset, Chunk);
	}
}

void AFINComputerGPUT1::Multicast_EndBackBufferReplication_Implementation() {
	if (!HasAuthority()) {
		FrontBuffer = BackBuffer;
		if (CachedInvalidation) CachedInvalidation->InvalidateRootChildOrder();
	}
}

AFINComputerGPUT1::AFINComputerGPUT1() {
	PrimaryActorTick.bCanEverTick = false;
	
	SetScreenSize(120, 30);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
	bAlwaysRelevant = true;
	NetPriority = 2.0;
}

void AFINComputerGPUT1::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (HasAuthority()) {
		if (ToReplicate.Num() > 0) {
			int64 ChunkSize = FMath::Min(CHUNK_SIZE, ToReplicate.Num());
			TArray<FFINGPUT1BufferPixel> Chunk(ToReplicate.GetData(), ChunkSize);
			ToReplicate.RemoveAt(0, ChunkSize);
			Multicast_AddBackBufferChunk(Offset, Chunk);
			Offset += ChunkSize;

			if (ToReplicate.Num() <= 0) {
				Multicast_EndBackBufferReplication();
			}
		} else if (bShouldReplicate) {
			bShouldReplicate = false;
			Multicast_BeginBackBufferReplication(BackBuffer.GetSize());
			ToReplicate = FrontBuffer.GetData();
			Offset = 0;
		}
	}
}

void AFINComputerGPUT1::BindScreen(const FFIRTrace& screen) {
	Super::BindScreen(screen);
}

void AFINComputerGPUT1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerGPUT1, FrontBuffer);
}

TSharedPtr<SWidget> AFINComputerGPUT1:: CreateWidget() {
	boxBrush = LoadObject<USlateBrushAsset>(NULL, TEXT("SlateBrushAsset'/FicsItNetworks/Computer/UI/ComputerCaseBorder.ComputerCaseBorder'"))->Brush;
	UFINComputerRCO* RCO = Cast<UFINComputerRCO>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
	return SNew(SScaleBox)
	.Stretch(EStretch::ScaleToFit)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)[
		SAssignNew(CachedInvalidation, SInvalidationPanel)
		.Content()[
			SNew(SScreenMonitor,(UObject*)GetWorld())
			.Buffer_Lambda([this]() {
				FScopeLock Lock(&DrawingMutex);
	            return &FrontBuffer;
			})
			.Font(FSlateFontInfo(LoadObject<UObject>(NULL, TEXT("Font'/FicsItNetworks/UI/Assets/FiraCode.FiraCode'")), 12, "FiraCode-Regular"))
			.OnMouseDown_Lambda([this, RCO](int x, int y, int btn) {
				RCO->GPUMouseEvent(this, 0, x, y, btn);
				return FReply::Handled();
			})
			.OnMouseUp_Lambda([this, RCO](int x, int y, int btn) {
				RCO->GPUMouseEvent(this, 1, x, y, btn);
	            return FReply::Handled();
	        })
	        .OnMouseMove_Lambda([this, RCO](int x, int y, int btn) {
				RCO->GPUMouseEvent(this, 2, x, y, btn);
	            return FReply::Handled();
	        })
			.OnKeyDown_Lambda([this, RCO](uint32 c, uint32 key, int btn) {
				RCO->GPUKeyEvent(this, 0,  c, key, btn);
				return FReply::Handled();
			})
			.OnKeyUp_Lambda([this, RCO](uint32 c, uint32 key, int btn) {
				RCO->GPUKeyEvent(this, 1,  c, key, btn);
				return FReply::Handled();
	        })
	        .OnKeyChar_Lambda([this, RCO](TCHAR c, int btn) {
		        RCO->GPUKeyCharEvent(this, FString::Chr(c), btn);
        		return FReply::Handled();
	        })
	    ]
    ];
}

void AFINComputerGPUT1::SetScreenSize(int Width, int Height) {
	FScopeLock Lock(&DrawingMutex);

	if (!BackBuffer.SetSize(Width, Height)) return;

	if (PrimaryActorTick.bCanEverTick) netSig_ScreenSizeChanged(Width, Height);

	ForceNetUpdate();
}

void AFINComputerGPUT1::OnRep_FrontBuffer() {
	if (CachedInvalidation) {
		CachedInvalidation->InvalidateRootChildOrder();
		CachedInvalidation->InvalidateScreenPosition();
	}
}

void AFINComputerGPUT1::netSig_OnMouseDown_Implementation(int x, int y, int btn) {}
void AFINComputerGPUT1::netSig_OnMouseUp_Implementation(int x, int y, int btn) {}
void AFINComputerGPUT1::netSig_OnMouseMove_Implementation(int x, int y, int btn) {}
void AFINComputerGPUT1::netSig_ScreenSizeChanged_Implementation(int oldW, int oldH) {}
void AFINComputerGPUT1::netSig_OnKeyDown_Implementation(int64 c, int64 code, int btn) {}
void AFINComputerGPUT1::netSig_OnKeyUp_Implementation(int64 c, int64 code, int btn) {}
void AFINComputerGPUT1::netSig_OnKeyChar_Implementation(const FString& c, int btn) {}

UObject* AFINComputerGPUT1::netFunc_getScreen() {
	return Screen.Get();
}

void AFINComputerGPUT1::netFunc_setText(int x, int y, const FString& str) {
	FScopeLock Lock(&DrawingMutex);
	BackBuffer.SetText(x, y, str, CurrentForeground, CurrentBackground);
}

void AFINComputerGPUT1::netFunc_fill(int x, int y, int dx, int dy, const FString& str) {
	FString c = str;
	if (FRegexMatcher(FRegexPattern("^[[:cntrl:]]?$"), c).FindNext()) c = " ";
	BackBuffer.Fill(x, y, dx, dy, FFINGPUT1BufferPixel(c[0], CurrentForeground, CurrentBackground));
}

void AFINComputerGPUT1::netFunc_getSize(int& w, int& h) {
	BackBuffer.GetSize(w, h);
}

void AFINComputerGPUT1::netFunc_setSize(int w, int h) {
	FScopeLock Lock(&DrawingMutex);
	SetScreenSize(FMath::Clamp(w, 1, 300), FMath::Clamp(h, 1, 100));
}

void AFINComputerGPUT1::netFunc_setForeground(float r, float g, float b, float a) {
	FScopeLock Lock(&DrawingMutex);
	CurrentForeground = FLinearColor(FMath::Clamp(r, 0.0f, 1.0f), FMath::Clamp(g, 0.0f, 1.0f), FMath::Clamp(b, 0.0f, 1.0f), FMath::Clamp(a, 0.0f, 1.0f));
}

void AFINComputerGPUT1::netFunc_setBackground(float r, float g, float b, float a) {
	FScopeLock Lock(&DrawingMutex);
	CurrentBackground = FLinearColor(FMath::Clamp(r, 0.0f, 1.0f), FMath::Clamp(g, 0.0f, 1.0f), FMath::Clamp(b, 0.0f, 1.0f), FMath::Clamp(a, 0.0f, 1.0f));
}

void AFINComputerGPUT1::netFunc_setBuffer(FFINGPUT1Buffer Buffer) {
	FScopeLock Lock(&DrawingMutex);
	BackBuffer = Buffer;
}

FFINGPUT1Buffer AFINComputerGPUT1::netFunc_getBuffer() {
	FScopeLock Lock(&DrawingMutex);
	return BackBuffer;
}

void AFINComputerGPUT1::netFunc_flush() {
	FScopeLock Lock(&DrawingMutex);
	FrontBuffer = BackBuffer;
	bShouldReplicate = true;
	if (CachedInvalidation) CachedInvalidation->InvalidateRootChildOrder();
}
