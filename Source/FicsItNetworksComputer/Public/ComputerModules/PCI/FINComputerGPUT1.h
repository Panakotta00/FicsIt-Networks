#pragma once

#include "CoreMinimal.h"
#include "FINComputerGPU.h"
#include "FINComputerGPUT1.generated.h"

DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FScreenCursorEventHandler, int, int, int);
DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FScreenKeyEventHandler, uint32, uint32, int);
DECLARE_DELEGATE_RetVal_TwoParams(FReply, FScreenKeyCharEventHandler, TCHAR, int);

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT1BufferPixel {
	GENERATED_BODY()
public:
	static const FFINGPUT1BufferPixel InvalidPixel;
	
	TCHAR Character;
	FLinearColor ForegroundColor;
	FLinearColor BackgroundColor;

	bool Serialize(FStructuredArchive::FSlot Slot) {
		FStructuredArchiveRecord Record = Slot.EnterRecord();
		Record.EnterField(SA_FIELD_NAME(TEXT("Character"))) << Character;
		Record.EnterField(SA_FIELD_NAME(TEXT("ForegroundColor"))) << ForegroundColor;
		Record.EnterField(SA_FIELD_NAME(TEXT("BackgroundColor"))) << BackgroundColor;
		return true;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) {
		bOutSuccess = true;
		Ar << Character;
		FColor Color = ForegroundColor.QuantizeRound();
		Ar << Color;
		ForegroundColor = Color;
		Color = BackgroundColor.QuantizeRound();
		Ar << Color;
		BackgroundColor = Color;
		return true;
	}
	
	FORCEINLINE explicit FFINGPUT1BufferPixel(TCHAR InCharacter = 0, FLinearColor InForeground = FLinearColor::White, FLinearColor InBackground = FLinearColor::Transparent) :
		Character(InCharacter),
		ForegroundColor(InForeground),
		BackgroundColor(InBackground) {}
	
	FORCEINLINE bool IsValid() const {
		return Character != 0;
	}
};

template<>
struct TStructOpsTypeTraits<FFINGPUT1BufferPixel> : TStructOpsTypeTraitsBase2<FFINGPUT1BufferPixel> {
	enum {
		WithStructuredSerializer = true,
		WithNetSerializer = true,
	};
};

inline FStructuredArchive::FSlot& operator<<(FStructuredArchive::FSlot& Slot, FFINGPUT1BufferPixel& Pixel) {
	Pixel.Serialize(Slot);
	return Slot;
}

enum EFINGPUT1TextBlendingMethod {
	FIN_GPUT1_TEXT_OVERWRITE,
	FIN_GPUT1_TEXT_NORMAL,
	FIN_GPUT1_TEXT_FILL,
	FIN_GPUT1_TEXT_NONE,
};

enum EFINGPUT1ColorBlendingMethod {
	FIN_GPUT1_OVERWRITE,
	FIN_GPUT1_NORMAL,
	FIN_GPUT1_MULTIPLY,
	FIN_GPUT1_DIVIDE,
	FIN_GPUT1_ADDITION,
	FIN_GPUT1_SUBTRACT,
	FIN_GPUT1_DIFFERENCE,
	FIN_GPUT1_DARKEN_ONLY,
	FIN_GPUT1_LIGHTEN_ONLY,
	FIN_GPUT1_NONE
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINGPUT1Buffer {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	int Width = 0;

	UPROPERTY(SaveGame)
	int Height = 0;

	UPROPERTY(SaveGame, NotReplicated)
	TArray<FFINGPUT1BufferPixel> Items;

	FORCEINLINE int PosToIndex(int X, int Y) const {
		if (!FMath::IsWithinInclusive(X, 0, Width)
			|| !FMath::IsWithinInclusive(Y, 0, Height)) return -1;
		return Y * Width + X;
	}

	FORCEINLINE bool IndexToPos(int Index, int& X, int& Y) const {
		if (!FMath::IsWithinInclusive(Index, 0, Width*Height - 1)) return false;
		X = Index % Width;
		Y = Index / Width;
		return true;
	}

public:
	FFINGPUT1Buffer() = default;
	
	void SetChunk(int InOffset, const TArray<FFINGPUT1BufferPixel>& InPixels) {
		int Count = InOffset + InPixels.Num() - Items.Num();
		if (Count > 0) Items.AddDefaulted(Count);
		FMemory::Memcpy(Items.GetData() + InOffset, InPixels.GetData(), InPixels.Num() * Items.GetTypeSize());
	}

	TArray<FFINGPUT1BufferPixel>& GetData() {
		return Items;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) {
		bOutSuccess = true;
		Ar << Width << Height;
		int Count = Items.Num();
		Ar << Count;
		if (Ar.IsLoading()) Items.SetNumUninitialized(Count);
		for (int i = 0; i < Count && bOutSuccess; ++i) {
			Items[i].NetSerialize(Ar, Map, bOutSuccess);
		}
		return true;
	}

	/**
	 * Creates and fills a new buffer with the default pixels.
	 * If CopyFrom is given, copies the buffer into the upper left corner of the new buffer.
	 */
	FORCEINLINE FFINGPUT1Buffer(int InWidth, int InHeight, const FFINGPUT1Buffer* CopyFrom = nullptr) : Width(InWidth), Height(InHeight) {
		check(InWidth >= 0 && InHeight >= 0);
		
		int EmptyHeight = Height;

		if (CopyFrom) {
			const int CopyWidth = FMath::Min(Width, CopyFrom->Width);
			const int CopyHeight = FMath::Min(Height, CopyFrom->Height);
			const int EmptyWidth = Width - CopyWidth;
			EmptyHeight = Height - CopyHeight;

			for (int i = 0; i < CopyHeight; ++i) {
				Items.AddUninitialized(CopyWidth);
				FMemory::Memcpy(&Items[i * Width], &CopyFrom->Items[i * CopyFrom->Width], CopyWidth * sizeof(FFINGPUT1BufferPixel));
				Items.AddDefaulted(EmptyWidth);
			}
		}
		
		Items.AddDefaulted(EmptyHeight * Width);
	}
	
	/**
	 * Allows to get the dimensions of the buffer
	 *
	 * @param	OutWidth	the width of the buffer
	 * @param	OutHeight	the height of the buffer
	 */
	FORCEINLINE void GetSize(int& OutWidth, int& OutHeight) const {
		OutWidth = Width;
		OutHeight = Height;
	}
	FORCEINLINE FIntPoint GetSize() const {
		return FIntPoint(Width, Height);
	}

	/**
	 * Allows to resize the buffer, data that was already inside the buffer will be positioned the at the same location.
	 *
	 * @param	InWidth		the new width of the buffer
	 * @param	InHeight	the new height of the buffer
	 * @retrun	True if the size has changed (f.e. false if the size was the same as the current size)
	 */
	FORCEINLINE bool SetSize(int InWidth, int InHeight) {
		if (InWidth < 0) InWidth = 0;
		if (InHeight < 0) InHeight = 0;
		if (InWidth == Width && InHeight == Height) return false;
		*this = FFINGPUT1Buffer(InWidth, InHeight, this);
		return true;
	}
	
	/**
	 * Allows to get the character on the given position in the buffer.
	 *
	 * @param	X				X Position of the character you want to get
	 * @param	Y				Y Position of the character you want to get
	 * @retrun	The pixel at the given location. Invalid Pixel if invalid position.
	 */
	FORCEINLINE const FFINGPUT1BufferPixel& Get(int X, int Y) const {
		const int Index = PosToIndex(X, Y);
		if (Index < 0 || Index >= Items.Num()) return FFINGPUT1BufferPixel::InvalidPixel;
		return Items[Index];
	}

	/**
	 * Allows to set the character on the given position in the buffer.
	 *
	 * @param	X			X Position of the character you want to set
	 * @param	Y			Y Position of the character you want to set
	 * @param	Pixel	The pixel you want to store at the given location.
	 * @retrun	True if valid position and able to set pixel.
	 */
	FORCEINLINE bool Set(int X, int Y, FFINGPUT1BufferPixel Pixel) {
		const int Index = PosToIndex(X, Y);
		if (Index < 0 || Index >= Items.Num()) return false;
		Items[Index] = Pixel;
		return true;
	}

	FORCEINLINE void BlendPixelRow(FFINGPUT1BufferPixel* Source, const FFINGPUT1BufferPixel* From, int Count, EFINGPUT1TextBlendingMethod BlendMode) {
		FFINGPUT1BufferPixel* SourceEnd = Source + Count;
		switch (BlendMode) {
		case FIN_GPUT1_TEXT_OVERWRITE:
			while (Source < SourceEnd) {
				Source->Character = From->Character;
				Source += 1;
				From += 1;
			}
			break;
		case FIN_GPUT1_TEXT_NORMAL:
			while (Source < SourceEnd) {
				if (From->Character != ' ') Source->Character = From->Character;
				Source += 1;
				From += 1;
			}
			break;
		case FIN_GPUT1_TEXT_FILL:
			while (Source < SourceEnd) {
				if (Source->Character != ' ') Source->Character = From->Character;
				Source += 1;
				From += 1;
			}
			break;
		case FIN_GPUT1_TEXT_NONE:
		default: ;
		}
	}
	
	FORCEINLINE void BlendPixelRow(FFINGPUT1BufferPixel* Source, const FFINGPUT1BufferPixel* From, int Count, EFINGPUT1ColorBlendingMethod BlendMode, int Offset) {
		FLinearColor* CSource = (FLinearColor*)(((uint8*)Source) + Offset);
		const FLinearColor* CFrom = (const FLinearColor*)(((const uint8*)From) + Offset);
		FLinearColor* SourceEnd = (FLinearColor*)((uint8*)CSource + Count*sizeof(FFINGPUT1BufferPixel));
		switch (BlendMode) {
		case FIN_GPUT1_OVERWRITE:
			while (CSource < SourceEnd) {
				*CSource = *CFrom;
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_NORMAL:
			while (CSource < SourceEnd) {
				float alpha = CFrom->A + CSource->A * (1 - CFrom->A);
				CSource->R = alpha != 0.0f ? (CFrom->R * CFrom->A + CSource->R * CSource->A * (1 - CFrom->A)) / alpha : 0.0f;
				CSource->G = alpha != 0.0f ? (CFrom->G * CFrom->A + CSource->G * CSource->A * (1 - CFrom->A)) / alpha : 0.0f;
				CSource->B = alpha != 0.0f ? (CFrom->B * CFrom->A + CSource->B * CSource->A * (1 - CFrom->A)) / alpha : 0.0f;
				CSource->A = alpha;
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_MULTIPLY:
			while (CSource < SourceEnd) {
				*CSource *= *CFrom;
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_DIVIDE:
			while (CSource < SourceEnd) {
				CSource->R = CFrom->R != 0.0f ? CSource->R / CFrom->R : 0.0f;
				CSource->G = CFrom->G != 0.0f ? CSource->G / CFrom->G : 0.0f;
				CSource->B = CFrom->B != 0.0f ? CSource->B / CFrom->B : 0.0f;
				CSource->A = CFrom->A != 0.0f ? CSource->A / CFrom->A : 0.0f;
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_ADDITION:
			while (CSource < SourceEnd) {
				*CSource += *CFrom;
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_SUBTRACT:
			while (CSource < SourceEnd) {
				*CSource -= *CFrom;
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_DIFFERENCE:
			while (CSource < SourceEnd) {
				CSource->R = FMath::Abs(CFrom->R - CSource->R);
				CSource->G = FMath::Abs(CFrom->G - CSource->G);
				CSource->B = FMath::Abs(CFrom->B - CSource->B);
				CSource->A = FMath::Abs(CFrom->A - CSource->A);
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_DARKEN_ONLY:
			while (CSource < SourceEnd) {
				CSource->R = FMath::Min(CFrom->R, CSource->R);
				CSource->G = FMath::Min(CFrom->G, CSource->G);
				CSource->B = FMath::Min(CFrom->B, CSource->B);
				CSource->A = FMath::Min(CFrom->A, CSource->A);
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		case FIN_GPUT1_LIGHTEN_ONLY:
			while (CSource < SourceEnd) {
				CSource->R = FMath::Max(CFrom->R, CSource->R);
				CSource->G = FMath::Max(CFrom->G, CSource->G);
				CSource->B = FMath::Max(CFrom->B, CSource->B);
				CSource->A = FMath::Max(CFrom->A, CSource->A);
				CSource = (FLinearColor*)((uint8*)CSource + sizeof(FFINGPUT1BufferPixel));
				CFrom = (FLinearColor*)((uint8*)CFrom + sizeof(FFINGPUT1BufferPixel));
			}
			break;
		default: ;
		}
	}

	/**
	 * Allows to copy a given buffer to this buffer at the given location
	 *
	 * @param	X					the X position of the upper left corner at witch it should get copied to
	 * @param	Y					the Y position of the upper left corner at witch it should get copied to
	 * @param	From				the buffer which you want to copy from
	 * @param	TextBlendMode		the blend mode that should be used to combine the text of the two buffers
	 * @param	ForegroundBlendMode	the blend mode that should be used to combine the two buffers foreground
	 * @param	BackgroundBlendMode	the blend mode that should be used to combine the two buffers background
	 */
	FORCEINLINE void Copy(int X, int Y, const FFINGPUT1Buffer& From, EFINGPUT1TextBlendingMethod TextBlendMode, EFINGPUT1ColorBlendingMethod ForegroundBlendMode, EFINGPUT1ColorBlendingMethod BackgroundBlendMode) {
		const int CopyWidth = X < 0 ? FMath::Min(Width, From.Width + X) : FMath::Min(Width - X, From.Width);
		const int CopyHeight = Y < 0 ? FMath::Min(Height, From.Height + Y) : FMath::Min(Height - Y, From.Height);
		const int OffsetX = FMath::Clamp(X, 0, TNumericLimits<int>::Max());
		const int OffsetY = FMath::Clamp(Y, 0, TNumericLimits<int>::Max());
		const int FOffsetX = FMath::Clamp(-X, 0, TNumericLimits<int>::Max());
		const int FOffsetY = FMath::Clamp(-Y, 0, TNumericLimits<int>::Max());
		if (OffsetX >= Width || OffsetY >= Height) return;
		if (FOffsetX >= From.Width || FOffsetY >= From.Height) return;

		for (int i = 0; i < CopyHeight; ++i) {
			const int Offset = OffsetX + (OffsetY + i) * Width;
			const int FOffset = FOffsetX + (FOffsetY + i) * From.Width;
			if (ForegroundBlendMode == FIN_GPUT1_OVERWRITE && BackgroundBlendMode == FIN_GPUT1_OVERWRITE && TextBlendMode == FIN_GPUT1_TEXT_OVERWRITE) {
				FMemory::Memcpy(&Items[Offset], &From.Items[FOffset], CopyWidth * sizeof(FFINGPUT1BufferPixel));
			} else {
				BlendPixelRow(&Items[Offset], &From.Items[FOffset], CopyWidth, TextBlendMode);
				BlendPixelRow(&Items[Offset], &From.Items[FOffset], CopyWidth, ForegroundBlendMode, offsetof(FFINGPUT1BufferPixel, ForegroundColor));
				BlendPixelRow(&Items[Offset], &From.Items[FOffset], CopyWidth, BackgroundBlendMode, offsetof(FFINGPUT1BufferPixel, BackgroundColor));
			}
		}
	}

	/**
	 * Allows to fill this buffer in the given range with the given pixel
	 *
	 * @param	InX			the x position were the upper left corner of the range should be
	 * @param	InY			the y position were the upper left corner of the range should be
	 * @param	InWidth		the width of the range
	 * @param	InHeight	the height of the range
	 * @param	InPixel		the pixel you want to place on each pixel of the range
	 */
	FORCEINLINE void Fill(int InX, int InY, int InWidth, int InHeight, const FFINGPUT1BufferPixel& InPixel) {
		const int CopyWidth = InX < 0 ? FMath::Min(Width, InWidth + InX) : FMath::Min(Width - InX, InWidth);
		const int CopyHeight = InX < 0 ? FMath::Min(Height, InHeight + InY) : FMath::Min(Height - InY, InHeight);
		const int OffsetX = FMath::Clamp(InX, 0, TNumericLimits<int>::Max());
		const int OffsetY = FMath::Clamp(InY, 0, TNumericLimits<int>::Max());
		if (OffsetX >= Width || OffsetY >= Height) return;
		
		for (int X = 0; X < CopyWidth; ++X) {
			for (int Y = 0; Y < CopyHeight; ++Y) {
				const int Offset = OffsetX + X + (OffsetY + Y) * Width;
				if (Items.Num() <= Offset) return;
				Items[Offset] = InPixel;
			}
		}
	}

	/**
	 * Allows to write the given text onto the buffer and with the given offset.
	 *
	 * @param	InX				the X Position at which the text should begin to get written
	 * @param	InY				the Y Position at which the text should begin to get written
	 * @param	InText			the text that should get written
	 * @param	InForeground	the foreground color which will be used to write the text
	 * @param	InBackground	the background color which will be used to write the text
	 */
	FORCEINLINE void SetText(int InX, int InY, const FString& InText, const FLinearColor& InForeground, const FLinearColor& InBackground) {
		FString toSet = InText;
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
				int oldX = InX + inLine.Len();
				if (InY >= 0 && InX < Width && InY < Height) {
					if (InX < 0) {
						if (inLine.Len() < FMath::Abs(InX)) {
							InX = -1;
						} else {
							inLine.RemoveAt(0, FMath::Abs(InX));
							InX = 0;
						}
					}
					if (InX >= 0) {
						int replace = FMath::Clamp(inLine.Len(), 0, static_cast<int>(Width)-InX);
						if (replace > 0) {
							int64 CharIndex = InY * Width + InX;
							for (int dx = 0; dx < replace; ++dx) {
								Set(InX + dx, InY, FFINGPUT1BufferPixel(inLine[dx], InForeground, InBackground));
							}
						}
					}
				}
				InX = oldX;
				if (returned) InX = 0;
			}
			if (newLine) ++InY;
		}
	}

	/**
	 * Sets the internal data based on the given data.
	 *
	 * @param	InCharacters	the characters you want to draw with a length of exactly width*height
	 * @param	InForeground	the values of the foreground color slots for each character were a group of four values give one color. so the length has to be exactly width*height*4
	 * @param	InBackground	the values of the background color slots for each character were a group of four values give one color. so the length has to be exactly width*height*4
	 * @return	True when successfully able to set the raw data of this buffer
	 */
	FORCEINLINE bool SetRaw(const FString& InCharacters, const TArray<float>& InForeground, const TArray<float>& InBackground) {
		const int Length = Width * Height;
		if (InCharacters.Len() != Length) return false;
		if (InForeground.Num() != Length*4) return false;
		if (InBackground.Num() != Length*4) return false;
		ParallelFor(Length, [this, &InCharacters, &InForeground, &InBackground](int i) {
			int Offset = i * 4;
			const FLinearColor ForegroundColor(
					InForeground[Offset],
				InForeground[Offset+1],
				InForeground[Offset+2],
				InForeground[Offset+3]);
			const FLinearColor BackgroundColor(
				InBackground[Offset],
				InBackground[Offset+1],
				InBackground[Offset+2],
				InBackground[Offset+3]);
			Set(i % Width, i / Width, FFINGPUT1BufferPixel(InCharacters[i], ForegroundColor, BackgroundColor));
		});
		return true;
	}

	/**
	 * Returns the buffer as String with no ending whitespace.
	 */
	FORCEINLINE FString GetAsText() const {
		FString Out;
		for (int Y = 0; Y < Height; ++Y) {
			const int LineOffset = Y * Width;
			FString Line;
			for (int X = 0; X < Width; ++X) {
				Line += Items[LineOffset + X].Character;
			}
			Out += Line.TrimEnd() + '\n';
		}
		return Out;
	}
};

template<>
struct TStructOpsTypeTraits<FFINGPUT1Buffer> : TStructOpsTypeTraitsBase2<FFINGPUT1Buffer> {
	enum {
		WithNetSerializer = true,
	};
};

class FICSITNETWORKSCOMPUTER_API SScreenMonitor : public SLeafWidget {
	SLATE_BEGIN_ARGS(SScreenMonitor) : _Font()
		{
			_Clipping = EWidgetClipping::OnDemand;
		}
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)
		SLATE_ATTRIBUTE(const FFINGPUT1Buffer*, Buffer)

		SLATE_EVENT(FScreenCursorEventHandler, OnMouseDown)
		SLATE_EVENT(FScreenCursorEventHandler, OnMouseUp)
		SLATE_EVENT(FScreenCursorEventHandler, OnMouseMove)
		SLATE_EVENT(FScreenKeyEventHandler, OnKeyDown)
		SLATE_EVENT(FScreenKeyEventHandler, OnKeyUp)
		SLATE_EVENT(FScreenKeyCharEventHandler, OnKeyChar)
	SLATE_END_ARGS()

public:
    void Construct( const FArguments& InArgs, UObject* Context);

	/**
	 * Returns the currently displayed buffer.
	 *
	 * @return	the currently displayed buffer.
	 */
	FFINGPUT1Buffer GetBuffer() const;

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
	
private:
    TAttribute<const FFINGPUT1Buffer*> Buffer;
	TAttribute<FSlateFontInfo> Font;
	FScreenCursorEventHandler OnMouseDownEvent;
	FScreenCursorEventHandler OnMouseUpEvent;
	FScreenCursorEventHandler OnMouseMoveEvent;
	FScreenKeyEventHandler OnKeyDownEvent;
	FScreenKeyEventHandler OnKeyUpEvent;
	FScreenKeyCharEventHandler OnKeyCharEvent;
	UObject* WorldContext;

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
	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	// End SWidget

	bool HandleShortCut(const FKeyEvent& InKeyEvent);
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINComputerGPUT1 : public AFINComputerGPU {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame, ReplicatedUsing=OnRep_FrontBuffer)
	FFINGPUT1Buffer FrontBuffer;

	UPROPERTY(SaveGame)
	FLinearColor CurrentForeground = FLinearColor(1,1,1,1);

	UPROPERTY(SaveGame)
	FLinearColor CurrentBackground = FLinearColor(0,0,0,0);

	UPROPERTY(SaveGame)
	FFINGPUT1Buffer BackBuffer;
	
	UPROPERTY()
	FSlateBrush boxBrush;

	FCriticalSection DrawingMutex;

	TSharedPtr<SInvalidationPanel> CachedInvalidation;

	const int64 CHUNK_SIZE = 100;
	int64 Offset = 0;
	TArray<FFINGPUT1BufferPixel> ToReplicate;
	bool bShouldReplicate = false;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BeginBackBufferReplication(FIntPoint Size);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddBackBufferChunk(int64 InOffset, const TArray<FFINGPUT1BufferPixel>& Chunk);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndBackBufferReplication();
	
public:
	AFINComputerGPUT1();

	// Begin AActor
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin IFINGraphicsPorcessingUnit
	virtual void BindScreen(const FFIRTrace& Screen) override;
	// End IFINGraphicsProcessingUnit

	virtual TSharedPtr<SWidget> CreateWidget() override;

	/**
	* Reallocates the TextGrid for the new given screen size.
	*
	* @param[in]	Width	the new width of the screen
	* @param[in]	Height	the new height of the screen
	*/
	void SetScreenSize(int Width, int Height);

	UFUNCTION()
	void OnRep_FrontBuffer();
	
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
	void netSig_OnKeyChar(const FString& c, int btn);
	UFUNCTION()
	void netSigMeta_OnKeyChar(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "OnKeyChar";
		DisplayName = FText::FromString("On Key Character");
		Description = FText::FromString("Triggers when a character key got 'clicked' and essentially a character got typed in, usful for text input.");
		ParameterInternalNames.Add("c");
		ParameterDisplayNames.Add(FText::FromString("Character"));
		ParameterDescriptions.Add(FText::FromString("The character that got typed in as string."));
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
		InternalName = "setBackground";
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
	void netFunc_setBuffer(FFINGPUT1Buffer Buffer);
	UFUNCTION()
	void netFuncMeta_setBuffer(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "setBuffer";
		DisplayName = FText::FromString("Set Buffer");
		Description = FText::FromString("Allows to change the back buffer of the GPU to the given buffer.");
		ParameterInternalNames.Add("buffer");
		ParameterDisplayNames.Add(FText::FromString("Buffer"));
		ParameterDescriptions.Add(FText::FromString("The Buffer you want to now use as back buffer."));
		Runtime = 2;
	}
	
	UFUNCTION()
	FFINGPUT1Buffer netFunc_getBuffer();
	UFUNCTION()
    void netFuncMeta_getBuffer(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
    	InternalName = "getBuffer";
    	DisplayName = FText::FromString("Get Buffer");
    	Description = FText::FromString("Returns the back buffer as struct to be able to use advanced buffer handling functions. (struct is a copy)");
    	ParameterInternalNames.Add("buffer");
    	ParameterDisplayNames.Add(FText::FromString("Buffer"));
    	ParameterDescriptions.Add(FText::FromString("The Buffer that is currently the back buffer."));
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
