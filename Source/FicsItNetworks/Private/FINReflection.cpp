#include "FILLogEntry.h"
#include "Computer/FINComputerGPUT1.h"
#include "Computer/FINComputerGPUT2.h"
#include "GameFramework/Actor.h"
#include "Network/FINFuture.h"
#include "Reflection/Source/FIRSourceStaticMacros.h"

BeginStruct(FFINFuture, "Future", "Future", "A Future struct MAY BE HANDLED BY CPU IMPLEMENTATION differently, generaly, this is used to make resources available on a later point in time. Like if data won't be avaialble right away and you have to wait for it to process first. Like when you do a HTTP Request, then it takes some time to get the data from the web server. And since we don't want to halt the game and wait for the data, you can use a future to check if the data is available, or let just the Lua Code wait, till the data becomes available.")
EndStruct()

BeginStructConstructable(FFINGPUT1Buffer, "GPUT1Buffer", "GPU T1 Buffer", "A structure that can hold a buffer of characters and colors that can be displayed with a gpu")
BeginFunc(getSize, "Get Size", "Allows to get the dimensions of the buffer.", 2) {
	OutVal(0, RFloat, width, "Width", "The width of this buffer")
	OutVal(1, RFloat, height, "Height", "The height of this buffer")
	Body()
	int Width, Height;
	self->GetSize(Width, Height);
	width = (FIRInt)Width;
	height = (FIRInt)Height;
} EndFunc()
BeginFunc(setSize, "Set Size", "Allows to set the dimensions of the buffer.", 2) {
	InVal(0, RFloat, width, "Width", "The width this buffer should now have")
	InVal(1, RFloat, height, "Height", "The height this buffer now have")
	Body()
	self->SetSize(width, height);
} EndFunc()
BeginFunc(get, "Get", "Allows to get a single pixel from the buffer at the given position", 2) {
	InVal(0, RInt, x, "X", "The x position of the character you want to get")
	InVal(1, RInt, y, "Y", "The y position of the character you want to get")
	OutVal(2, RString, c, "Char", "The character at the given position")
	OutVal(3, RStruct<FLinearColor>, foreground, "Foreground Color", "The foreground color of the pixel at the given position")
	OutVal(4, RStruct<FLinearColor>, background, "Background Color", "The background color of the pixel at the given position")
	Body()
	const FFINGPUT1BufferPixel& Pixel = self->Get(x, y);
	c = FString::Chr(Pixel.Character);
	foreground = (FIRStruct) Pixel.ForegroundColor;
	background = (FIRStruct) Pixel.BackgroundColor;
} EndFunc()
BeginFunc(set, "Set", "Allows to set a single pixel of the buffer at the given position", 2) {
	InVal(0, RInt, x, "X", "The x position of the character you want to set")
	InVal(1, RInt, y, "Y", "The y position of the character you want to set")
	InVal(2, RString, c, "Char", "The character the pixel should have")
	InVal(3, RStruct<FLinearColor>, foreground, "Foreground Color", "The foreground color the pixel at the given position should have")
	InVal(4, RStruct<FLinearColor>, background, "Background Color", "The background color the pixel at the given position should have")
	OutVal(5, RBool, done, "Done", "True if the pixel got set successfully")
	Body()
	if (c.Len() < 1) return;
	done = self->Set(x, y, FFINGPUT1BufferPixel(c[0], foreground, background));
} EndFunc()
BeginFunc(copy, "Copy", "Copies the given buffer at the given offset of the upper left corner into this buffer.", 2) {
	InVal(0, RInt, x, "X", "The x offset of the upper left corner of the buffer relative to this buffer")
	InVal(1, RInt, y, "Y", "The y offset of the upper left corener of the buffer relative to this buffer")
	InVal(2, RStruct<FFINGPUT1Buffer>, buffer, "Buffer", "The buffer from wich you want to copy from")
	InVal(3, RInt, textBlendMode, "Text Blend Mode", "The blend mode that is used for the text.\n0 = Overwrite this with the content of the given buffer\n1 = Overwrite with only characters that are not ' '\n2 = Overwrite only were this characters are ' '\n3 = Keep this buffer")
	InVal(4, RInt, foregroundBlendMode, "Foreground Color Blend Mode", "The blend mode that is used for the foreground color.\n0 = Overwrite with the given color\n1 = Normal alpha composition\n2 = Multiply\n3 = Divide\n4 = Addition\n5 = Subtraction\n6 = Difference\n7 = Darken Only\n8 = Lighten Only\n9 = None")
	InVal(5, RInt, backgroundBlendMode, "Background Color Blend Mode", "The blend mode that is used for the background color.\n0 = Overwrite with the given color\n1 = Normal alpha composition\n2 = Multiply\n3 = Divide\n4 = Addition\n5 = Subtraction\n6 = Difference\n7 = Darken Only\n8 = Lighten Only\n9 = None")
	Body()
	self->Copy(x, y, buffer, (EFINGPUT1TextBlendingMethod)textBlendMode, (EFINGPUT1ColorBlendingMethod)foregroundBlendMode, (EFINGPUT1ColorBlendingMethod)backgroundBlendMode);
} EndFunc()
BeginFunc(setText, "Set Text", "Allows to write the given text onto the buffer and with the given offset.", 2) {
	InVal(0, RInt, x, "X", "The X Position at which the text should begin to get written.")
	InVal(1, RInt, y, "Y", "The Y Position at which the text should begin to get written.")
	InVal(2, RString, text, "Text", "The text that should get written.")
	InVal(3, RStruct<FLinearColor>, foreground, "Foreground", "The foreground color which will be used to write the text.")
	InVal(4, RStruct<FLinearColor>, background, "Background", "The background color which will be used to write the text.")
	Body()
	self->SetText(x, y, text, foreground, background);
} EndFunc()
BeginFunc(fill, "Fill", "Draws the given character at all given positions in the given rectangle on-to the hidden screen buffer.", 2) {
	InVal(0, RInt, x, "X", "The x coordinate at which the rectangle should get drawn. (upper-left corner)")
	InVal(1, RInt, y, "Y", "The y coordinate at which the rectangle should get drawn. (upper-left corner)")
	InVal(2, RInt, width, "Width", "The width of the rectangle.")
	InVal(3, RInt, height, "Height", "The height of the rectangle.")
	InVal(4, RString, character, "Character", "A string with a single character that will be used for each pixel in the range you want to fill.")
	InVal(5, RStruct<FLinearColor>, foreground, "Foreground", "The foreground color which will be used to fill the rectangle.")
	InVal(6, RStruct<FLinearColor>, background, "Background", "The background color which will be used to fill the rectangle.")
	Body()
	if (character.Len() < 1) character = " ";
	self->Fill(x, y, width, height, FFINGPUT1BufferPixel(character[0], foreground, background));
} EndFunc()
BeginFunc(setRaw, "Set Raw", "Allows to set the internal data of the buffer more directly.", 2) {
	InVal(0, RString, characters, "Characters", "The characters you want to draw with a length of exactly width*height.")
	InVal(1, RArray<RFloat>, foreground, "Foreground Color", "The values of the foreground color slots for each character were a group of four values give one color. so the length has to be exactly width*height*4.")
	InVal(2, RArray<RFloat>, background, "Background Color", "The values of the background color slots for each character were a group of four values give one color. so the length has to be exactly width*height*4.")
	OutVal(3, RBool, success, "Success", "True if the raw data was successfully written")
	Body()
	TArray<float> Foreground, Background;
	if (foreground.Num() != background.Num()) success = false;
	else {
		//Foreground.AddUninitialized(foreground.Num());
		//Background.AddUninitialized(background.Num());
		//ParallelFor(foreground.Num(), [&Foreground, &foreground, &Background, &background](int i) {
		for (int i = 0; i < foreground.Num(); ++i) {
			Foreground.Add(foreground[i].GetFloat());
			Background.Add(background[i].GetFloat());
		//});
		}
		int Width, Height;
		self->GetSize(Width, Height);
		const int Length = Width * Height;
		if (characters.Len() != Length
			|| foreground.Num() != Length*4
			|| background.Num() != Length*4) {
			success = false;
		}
		ParallelFor(Length, [self, Width, &characters, &foreground, &background](int i) {
			int Offset = i * 4;
			const FLinearColor ForegroundColor(
				foreground[Offset].GetFloat(),
				foreground[Offset+1].GetFloat(),
				foreground[Offset+2].GetFloat(),
				foreground[Offset+3].GetFloat());
			const FLinearColor BackgroundColor(
				background[Offset].GetFloat(),
				background[Offset+1].GetFloat(),
				background[Offset+2].GetFloat(),
				background[Offset+3].GetFloat());
			self->Set(i % Width, i / Width, FFINGPUT1BufferPixel(characters[i], ForegroundColor, BackgroundColor));
		});
		success = true;
	}
} EndFunc()
BeginFunc(clone, "Clone", "Clones this buffer into a new struct") {
	OutVal(0, RStruct<FFINGPUT1Buffer>, buffer, "Buffer", "The clone of this buffer")
	Body()
	buffer = (FIRStruct) *self;
} EndFunc()
EndStruct()

BeginStruct(FFILEntry, "LogEntry", "Log Entry", "An entry in the Computer Log.")
BeginProp(RString, content, "Content", "The Message-Content contained within the log entry.") {
	Return self->Content;
} EndProp()
BeginProp(RString, timestamp, "Timestamp", "The timestamp at which the log entry got logged.") {
	Return self->Timestamp.ToString();
} EndProp()
BeginProp(RInt, verbosity, "Verbosity", "The verbosity of the log entry.") {
	Return (FIRInt)self->Verbosity;
} EndProp()
BeginFunc(format, "Format", "Creates a formatted string representation of this log entry.") {
	OutVal(0, RString, result, "Result", "The resulting formatted string")
	Body()
	result = self->ToClipboardText();
} EndFunc()
EndStruct()

BeginStructConstructable(FFINGPUT2DC_Box, "GPUT2DrawCallBox", "GPU T2 Box Draw Call", "This struct contains the necessary information to draw a box onto the GPU T2.")
BeginProp(RStruct<FVector2D>, position, "Position", "The drawn local position of the rectangle.") {
	Return FIRStruct(self->Position);
} PropSet() {
	self->Position = Val;
} EndProp()
BeginProp(RStruct<FVector2D>, size, "Size", "The drawn size of the rectangle.") {
	Return FIRStruct(self->Size);
} PropSet() {
	self->Size = Val;
} EndProp()
BeginProp(RFloat, rotation, "Rotation", "The draw rotation of the rectangle.") {
	Return FIRFloat(self->Rotation);
} PropSet() {
	self->Rotation = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, color, "Color", "The fill color of the rectangle, or the tint of the image drawn.") {
	Return FIRStruct(self->Color.ReinterpretAsLinear());
} PropSet() {
	self->Color = Val.QuantizeRound();
} EndProp()
BeginProp(RString, image, "Image", "If not empty, should be a image reference to the image that should be drawn inside the rectangle.") {
	Return FIRStr(self->Image);
} PropSet() {
	self->Image = Val;
} EndProp()
BeginProp(RStruct<FVector2D>, imageSize, "Image Size", "The size of the internal image drawn, necessary for proper scaling, antialising and tiling.") {
	Return FIRStruct(self->ImageSize);
} PropSet() {
	self->ImageSize = Val;
} EndProp()
BeginProp(RBool, hasCenteredOrigin, "Has Centered Origin", "If set to false, the position will give the left upper corner of the box and rotation will happen around this point. If set to true, the position will give the center point of box and the rotation will happen around this center point.") {
	Return FIRBool(self->bHasCenteredOrigin);
} PropSet() {
	self->bHasCenteredOrigin = Val;
} EndProp()
BeginProp(RBool, horizontalTiling, "Horizontal Tiling", "True if the image should be tiled horizontally.") {
	Return FIRBool(self->bHorizontalTiling);
} PropSet() {
	self->bHorizontalTiling = Val;
} EndProp()
BeginProp(RBool, verticalTiling, "Vertical Tiling", "True if the image should be tiled vertically.") {
	Return FIRBool(self->bVerticalTiling);
} PropSet() {
	self->bVerticalTiling = Val;
} EndProp()
BeginProp(RBool, isBorder, "Is Border", "If true, the margin values provide a way to specify a fixed sized border thicknesses the boxes images will use (use the image as 9-patch).") {
	Return FIRBool(self->bIsBorder);
} PropSet() {
	self->bIsBorder = Val;
} EndProp()
BeginProp(RStruct<FMargin>, margin, "Margin", "The margin values of the 9-patch (border).") {
	Return FIRStruct(self->Margin);
} PropSet() {
	self->Margin = FVector4(Val.Left, Val.Top, Val.Right, Val.Bottom);
} EndProp()
BeginProp(RBool, isRounded, "Is Rounded", "True if the box can have rounded borders.") {
	Return FIRBool(self->bIsRounded);
} PropSet() {
	self->bIsRounded = Val;
} EndProp()
BeginProp(RStruct<FVector4>, radii, "Radii", "The rounded border radii used if isRounded is set to true.\nThe Vector4 corner mapping in order: Top Left, Top Right, Bottom Right & Bottom Left.") {
	Return FIRStruct(self->BorderRadii);
} PropSet() {
	self->BorderRadii = Val;
} EndProp()
BeginProp(RBool, hasOutline, "Has Outline", "True if the box has a colorful (inward) outline.") {
	Return FIRBool(self->bHasOutline);
} PropSet() {
	self->bHasOutline = Val;
} EndProp()
BeginProp(RFloat, outlineThickness, "Outline Thickness", "The uniform thickness of the outline around the box.") {
	Return FIRFloat(self->OutlineThickness);
} PropSet() {
	self->OutlineThickness = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, outlineColor, "Outline Color", "The color of the outline around the box.") {
	Return FIRStruct(self->OutlineColor.ReinterpretAsLinear());
} PropSet() {
	self->OutlineColor = Val.QuantizeRound();
} EndProp()
EndStruct()
