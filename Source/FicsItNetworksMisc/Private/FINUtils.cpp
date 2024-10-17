#include "FINUtils.h"

#include "FGSaveSession.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Dom/JsonObject.h"
#include "Internationalization/Regex.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

const FRegexPattern UFINUtils::VariablePattern(TEXT("\\$\\{(\\w+)(\\|(\\w+))?\\}"));

#pragma optimize("", off)
FString UFINUtils::InterpolateString(const FString& Text, const TMap<FString, FString>& Variables, bool bEmptyInvalidVariables) {
	FString OutText;
	FRegexMatcher Matcher(VariablePattern, Text);
	int CurrentIndex = 0;
	while (Matcher.FindNext()) {
		FString Variable = Matcher.GetCaptureGroup(1);
		const FString* Value = Variables.Find(Variable);
		if (Value || bEmptyInvalidVariables) {
			OutText.Append(TextRange(Text, FTextRange(CurrentIndex, Matcher.GetMatchBeginning())));
			if (Value) OutText.Append(*Value);
		} else {
			OutText.Append(TextRange(Text, FTextRange(CurrentIndex, Matcher.GetMatchEnding())));
		} 
		CurrentIndex = Matcher.GetMatchEnding();
	}
	OutText.Append(Text.RightChop(CurrentIndex));
	return OutText;
}
#pragma optimize("", on)

void UFINUtils::VariablesFormString(const FString& Text, TMap<FString, FString>& OutVariables) {
	FRegexMatcher Matcher(VariablePattern, Text);
	while (Matcher.FindNext()) {
		FString Variable = Matcher.GetCaptureGroup(1);
		FString Default = Matcher.GetCaptureGroup(3);
		OutVariables.Add(Variable, Default);
	}
}

FVersion UFINUtils::GetFINSaveVersion(UObject* WorldContext) {
	UFGSaveSession* Session = UFGSaveSession::Get(WorldContext);
	if (!IsValid(Session)) return FVersion();
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Session->GetModMetadata());
	TSharedPtr<FJsonObject> Metadata = nullptr;
	FJsonSerializer::Deserialize(Reader, Metadata);

	if (!Metadata.IsValid()) {
		return FVersion();
	}

	const TArray<TSharedPtr<FJsonValue>>* Mods;
	if (Metadata->TryGetArrayField(TEXT("Mods"), Mods)) {
		for (const TSharedPtr<FJsonValue>& Mod : *Mods) {
			TSharedPtr<FJsonObject>* ModObj;
			if (!Mod->TryGetObject(ModObj)) continue;
			
			FString Reference;
			if (!ModObj->Get()->TryGetStringField(TEXT("Reference"), Reference) || Reference != TEXT("FicsItNetworks")) continue;
			FString VersionString;
			if (!ModObj->Get()->TryGetStringField(TEXT("Version"), VersionString)) continue;

			FVersion VersionStruct;
			FString ParseError;
			if(!VersionStruct.ParseVersion(VersionString, ParseError)) continue;

			return VersionStruct;
		}
	}

	return FVersion();
}
