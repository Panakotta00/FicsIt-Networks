#pragma once

#include "FINComputerGPU.h"
#include "Async/ParallelFor.h"

#include "FINComputerGPUT1.generated.h"

DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FScreenCursorEventHandler, int, int, int);
DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FScreenKeyEventHandler, uint32, uint32, int);
DECLARE_DELEGATE_RetVal_TwoParams(FReply, FScreenKeyCharEventHandler, TCHAR, int);

USTRUCT()
struct FFINGPUT1BufferPixel {
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
		WithStructuredSerializer = true
	};
};

USTRUCT()
struct FFINGPUT1Buffer {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	int Width = 0;

	UPROPERTY(SaveGame)
	int Height = 0;

	UPROPERTY(SaveGame)
	TArray<FFINGPUT1BufferPixel> Pixels;

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
				Pixels.AddUninitialized(CopyWidth);
				FMemory::Memcpy(&Pixels[i * Width], &CopyFrom->Pixels[i * CopyFrom->Width], CopyWidth * sizeof(FFINGPUT1BufferPixel));
				Pixels.AddDefaulted(EmptyWidth);
			}
		}
		
		Pixels.AddDefaulted(EmptyHeight * Width);
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

	/**
	 * Allows to resize the buffer, data that was already inside the buffer will be positioned the at the same location.
	 *
	 * @param	InWidth		the new width of the buffer
	 * @param	InHeight	the new height of the buffer
	 */
	FORCEINLINE void SetSize(int InWidth, int InHeight) {
		if (InWidth < 0) InWidth = 0;
		if (InHeight < 0) InHeight = 0;
		*this = FFINGPUT1Buffer(InWidth, InHeight, this);
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
		if (Index < 0) return FFINGPUT1BufferPixel::InvalidPixel;
		return Pixels[Index];
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
		if (Index < 0) return false;
		Pixels[Index] = Pixel;
		return true;
	}

	/**
	 * Allows to copy a given buffer to this buffer at the given location
	 *
	 * @param	X		the X position of the upper left corner at witch it should get copied to
	 * @param	Y		the Y position of the upper left corner at witch it should get copied to
	 * @param	From	the buffer which you want to copy from
	 */
	FORCEINLINE void Copy(int X, int Y, const FFINGPUT1Buffer& From) {
		const int CopyWidth = X < 0 ? FMath::Min(Width, From.Width - X) : FMath::Min(Width - X, From.Width);
		const int CopyHeight = Y < 0 ? FMath::Min(Height, From.Height - Y) : FMath::Min(Height - Y, From.Height);
		const int OffsetX = FMath::Clamp(X, 0, TNumericLimits<int>::Max());
		const int OffsetY = FMath::Clamp(Y, 0, TNumericLimits<int>::Max());
		const int FOffsetX = FMath::Clamp(-X, 0, TNumericLimits<int>::Max());
		const int FOffsetY = FMath::Clamp(-Y, 0, TNumericLimits<int>::Max());
		if (OffsetX >= Width || OffsetY >= Height) return;
		if (FOffsetX >= From.Width || FOffsetY >= From.Height) return;

		for (int i = 0; i < CopyHeight; ++i) {
			const int Offset = OffsetX + (OffsetY + i) * Width;
			const int FOffset = FOffsetX + (FOffsetY + i) * From.Width;
			FMemory::Memcpy(&Pixels[Offset], &From.Pixels[FOffset], CopyWidth * sizeof(FFINGPUT1BufferPixel));
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
		const int CopyWidth = InX < 0 ? FMath::Min(Width, InWidth - InX) : FMath::Min(Width - InX, InWidth);
		const int CopyHeight = InX < 0 ? FMath::Min(Height, InHeight - InY) : FMath::Min(Height - InY, InHeight);
		const int OffsetX = FMath::Clamp(InX, 0, TNumericLimits<int>::Max());
		const int OffsetY = FMath::Clamp(InY, 0, TNumericLimits<int>::Max());
		if (OffsetX >= Width || OffsetY >= Height) return;
		
		for (int X = 0; X < CopyWidth; ++X) {
			for (int Y = 0; Y < CopyHeight; ++Y) {
				const int Offset = OffsetX + X + (OffsetY + Y) * Width;
				Pixels[Offset] = InPixel;
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
		ParallelFor(Length, [this, &InCharacters, &InForeground, &InBackground, Width, Height](int i) {
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
				Line += Pixels[LineOffset + X].Character;
			}
			Out += Line.TrimEnd() + '\n';
		}
		return Out;
	}
};

class FICSITNETWORKS_API SScreenMonitor : public SLeafWidget {
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
class AFINComputerGPUT1 : public AFINComputerGPU {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame, Replicated)
	FFINGPUT1Buffer FrontBuffer;

	UPROPERTY(SaveGame)
	FLinearColor CurrentForeground = FLinearColor(1,1,1,1);

	UPROPERTY(SaveGame)
	FLinearColor CurrentBackground = FLinearColor(0,0,0,0);

	UPROPERTY(SaveGame)
	FFINGPUT1Buffer BackBuffer;
	
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
	* @param[in]	Width	the new width of the screen
	* @param[in]	Height	the new height of the screen
	*/
	void SetScreenSize(int Width, int Height);

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
	FFINGPUT1Buffer netFunc_getBuffer();

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
