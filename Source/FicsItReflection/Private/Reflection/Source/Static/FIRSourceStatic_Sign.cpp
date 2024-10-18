#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "FGSignLibrary.h"
#include "Buildables/FGBuildableSignBase.h"
#include "Buildables/FGBuildableWidgetSign.h"

BeginClass(AFGBuildableSignBase, "SignBase", "Sign Base", "The base class for all signs in the game.")
	BeginFunc(getSignType, "Get Sign Type", "Returns the sign type descriptor") {
	OutVal(0, RClass<UFGSignTypeDescriptor>, descriptor, "Descriptor", "The sign type descriptor")
	Body()
	descriptor = (FIRClass)IFGSignInterface::Execute_GetSignTypeDescriptor(self);
} EndFunc()
EndClass()

BeginClass(AFGBuildableWidgetSign, "WidgetSign", "Widget Sign", "The type of sign that allows you to define layouts, images, texts and colors manually.")
BeginFunc(setPrefabSignData, "Set Prefab Sign Data", "Sets the prefabg sign data e.g. the user settings like colo and more to define the signs content.", 0) {
	InVal(0, RStruct<FPrefabSignData>, prefabSignData, "Prefab Sign Data", "The new prefab sign data for this sign.")
	Body()
	self->SetPrefabSignData(prefabSignData);
} EndFunc()
BeginFunc(getPrefabSignData, "Get Prefab Sign Data", "Returns the prefabg sign data e.g. the user settings like colo and more to define the signs content.") {
	OutVal(0, RStruct<FPrefabSignData>, prefabSignData, "Prefab Sign Data", "The new prefab sign data for this sign.")
	Body()
	FPrefabSignData SignData;
	self->GetSignPrefabData(SignData);
	prefabSignData = (FIRStruct)SignData;
} EndFunc()
EndClass()

BeginClass(UFGSignTypeDescriptor, "SignType", "Sign Type", "Describes the type of a sign.")
BeginClassProp(RStruct<FVector2D>, dimensions, "Dimensions", "The canvas dimensions of this sign.") {
	FVector2D dimensions;
	UFGSignLibrary::GetCanvasDimensionsFromSignDescriptor(self, dimensions);
	FIRReturn dimensions;
} EndProp()
BeginClassFunc(getColors, "Get Colors", "Returns the default foreground/background/auxiliary colors of this sign type.", false) {
	OutVal(0, RStruct<FLinearColor>, foreground, "Foreground", "The foreground color")
	OutVal(1, RStruct<FLinearColor>, background, "Background", "The background color")
	OutVal(2, RStruct<FLinearColor>, auxiliary, "Auxiliary", "The auxiliary color")
	Body()
	FLinearColor fg, bg, au;
	UFGSignLibrary::GetDefaultColorsFromSignDescriptor(self, fg, bg, au);
	foreground = (FIRStruct)fg;
	background = (FIRStruct)bg;
	auxiliary = (FIRStruct)au;
} EndFunc()
BeginClassFunc(getPrefabs, "Get Prefabs", "Returns a list of all sign prefabs this sign can use.", false) {
	OutVal(0, RArray<RClass<UFGSignPrefabWidget>>, prefabs, "Prefabs", "The sign prefabs this sign can use")
	Body()
	TArray<FIRAny> PrefabsArray;
	TArray<TSoftClassPtr<UFGSignPrefabWidget>> PrefabList;
	UFGSignLibrary::GetPrefabLayoutsFromSignDescriptor(self, PrefabList);
	for (TSoftClassPtr<UFGSignPrefabWidget> Prefab : PrefabList) {
		PrefabsArray.Add((FIRClass)Prefab.Get());
	}
	prefabs = PrefabsArray;
} EndFunc()
BeginClassFunc(getTextElements, "Get Text Elements", "Returns a list of element names and their default text values.", false) {
	OutVal(0, RArray<RString>, textElements, "Text Elements", "A list of text element names of this type.")
	OutVal(0, RArray<RString>, textElementsDefaultValues, "Text Elements Default Values", "A list of default values for the text elements of this type.")
	Body()
	TArray<FIRAny> TextElements, TextElementsDefaultValues;
	TMap<FString, FString> Elements;
	UFGSignLibrary::GetTextElementNameMapFromSignDescriptor(self, Elements);
	for (const TPair<FString, FString>& Element : Elements) {
		TextElements.Add(Element.Key);
		TextElementsDefaultValues.Add(Element.Value);
	}
	textElements = TextElements;
	textElementsDefaultValues = TextElementsDefaultValues;
} EndFunc()
BeginClassFunc(getIconElements, "Get Icon Elements", "Returns a list of element names and their default icon values.", false) {
	OutVal(0, RArray<RString>, iconElements, "Icon Elements", "A list of icon element names of this type.")
	OutVal(0, RArray<RObject<UTexture2D>>, iconElementsDefaultValues, "Icon Elements Default Values", "A list of default values for the icon elements of this type.")
	Body()
	TArray<FIRAny> IconElements, IconElementsDefaultValues;
	TMap<FString, UObject*> Elements;
	UFGSignLibrary::GetIconElementNameMapFromSignDescriptor(self, Elements);
	for (const TPair<FString, UObject*>& Element : Elements) {
		IconElements.Add(Element.Key);
		IconElementsDefaultValues.Add((FIRObj)Element.Value);
	}
	iconElements = IconElements;
	iconElementsDefaultValues = IconElementsDefaultValues;
} EndFunc()
EndClass()

BeginClass(UFGSignPrefabWidget, "SignPrefab", "Sign Prefab", "Descibes a layout of a sign.")
EndClass()

BeginStructConstructable(FPrefabSignData, "PrefabSignData", "Prefab Sign Data", "This structure stores all data that defines what a sign displays.")
BeginProp(RClass<UObject>, layout, "Layout", "The object that actually displayes the layout") {
	FIRReturn (FIRClass)self->PrefabLayout.Get();
} PropSet() {
	self->PrefabLayout = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, foreground, "Foreground", "The foreground Color.") {
	FIRReturn (FIRStruct)self->ForegroundColor;
} PropSet() {
	self->ForegroundColor = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, background, "Background", "The background Color.") {
	FIRReturn (FIRStruct)self->BackgroundColor;
} PropSet() {
	self->BackgroundColor = Val;
} EndProp()
BeginProp(RFloat, emissive, "Emissive", "The emissiveness of the sign.") {
	FIRReturn self->Emissive;
} PropSet() {
	self->Emissive = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, auxiliary, "Auxiliary", "The auxiliary Color.") {
	FIRReturn (FIRStruct)self->AuxiliaryColor;
} PropSet() {
	self->AuxiliaryColor = Val;
} EndProp()
BeginProp(RClass<UFGSignTypeDescriptor>, signType, "Sign Type", "The type of sign this prefab fits to.") {
	FIRReturn (FIRClass)self->SignTypeDesc;
} PropSet() {
	self->SignTypeDesc = Val;
} EndProp()
BeginFunc(getTextElements, "Get Text Elements", "Returns all text elements and their values.") {
	OutVal(0, RArray<RString>, textElements, "Text Elements", "The element names for all text elements.")
	OutVal(1, RArray<RString>, textElementValues, "Text Element Values", "The values for all text elements.")
	Body()
	TArray<FIRAny> TextElements, TextElementValues;
	for (const TPair<FString, FString>& Element : self->TextElementData) {
		TextElements.Add(Element.Key);
		TextElementValues.Add(Element.Value);
	}
	textElements = (FIRArray)TextElements;
	textElementValues = (FIRArray)TextElementValues;
} EndFunc()
BeginFunc(getIconElements, "Get Icon Elements", "Returns all icon elements and their values.") {
	OutVal(0, RArray<RString>, iconElements, "Icon Elements", "The element names for all icon elements.")
	OutVal(1, RArray<RInt>, iconElementValues, "Icon Element Values", "The values for all icon elements.")
	Body()
	TArray<FIRAny> IconElements, IconElementValues;
	for (const TPair<FString, int32>& Element : self->IconElementData) {
		IconElements.Add(Element.Key);
		IconElementValues.Add((FIRInt)Element.Value);
	}
	iconElements = IconElements;
	iconElementValues = IconElementValues;
} EndFunc()
BeginFunc(setTextElements, "Set Text Elements", "Sets all text elements and their values.") {
	InVal(0, RArray<RString>, textElements, "Text Elements", "The element names for all text elements.")
	InVal(1, RArray<RString>, textElementValues, "Text Element Values", "The values for all text elements.")
	Body()
	if (textElements.Num() != textElementValues.Num()) throw FFIRException(TEXT("Count of element names and element values are not the same."));
	self->TextElementData.Empty();
	for (int i = 0; i < textElements.Num(); ++i) {
		self->TextElementData.Add(textElements[i].GetString(), textElementValues[i].GetString());
	}
} EndFunc()
BeginFunc(setIconElements, "Set Icon Elements", "Sets all icon elements and their values.") {
	InVal(0, RArray<RString>, iconElements, "Icon Elements", "The element names for all icon elements.")
	InVal(1, RArray<RInt>, iconElementValues, "Icon Element Values", "The values for all icon elements.")
	Body()
	if (iconElements.Num() != iconElementValues.Num()) throw FFIRException(TEXT("Count of element names and element values are not the same."));
	self->IconElementData.Empty();
	for (int i = 0; i < iconElements.Num(); ++i) {
		self->IconElementData.Add(iconElements[i].GetString(), iconElementValues[i].GetInt());
	}
} EndFunc()
BeginFunc(setTextElement, "Set Text Element", "Sets a text element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the text element")
	InVal(1, RString, value, "Value", "The value of the text element")
	Body()
	self->TextElementData.Add(elementName, value);
} EndFunc()
BeginFunc(setIconElement, "Set Icon Element", "Sets a icon element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the icon element")
	InVal(1, RInt, value, "Value", "The value of the icon element")
	Body()
	self->IconElementData.Add(elementName, value);
} EndFunc()
BeginFunc(getTextElement, "Get Text Element", "Gets a text element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the text element")
	OutVal(1, RInt, value, "Value", "The value of the text element")
	Body()
	FString* Element = self->TextElementData.Find(elementName);
	if (!Element) throw FFIRException(TEXT("No element with the given name found"));
	value = *Element;
} EndFunc()
BeginFunc(getIconElement, "Get Icon Element", "Gets a icon element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the icon element")
	OutVal(1, RInt, value, "Value", "The value of the icon element")
	Body()
	int* Element = self->IconElementData.Find(elementName);
	if (!Element) throw FFIRException(TEXT("No element with the given name found"));
	value = (FIRInt)*Element;
} EndFunc()
EndStruct()
