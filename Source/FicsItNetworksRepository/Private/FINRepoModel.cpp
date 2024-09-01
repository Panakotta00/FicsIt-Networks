#include "FINRepoModel.h"

TOptional<FFINRepoModDependency> FFINRepoModDependency::FromJsonObject(TSharedPtr<FJsonObject> JsonObject) {
	FFINRepoModDependency dependency;

	if (!JsonObject->TryGetStringField(TEXT("id"), dependency.ID)) return {};
	JsonObject->TryGetStringField(TEXT("version"), dependency.Version);

	return dependency;
}

TOptional<FFINRepoEEPROM> FFINRepoEEPROM::FromJsonObject(TSharedPtr<FJsonObject> JsonObject) {
	FFINRepoEEPROM eeprom;

	if (!JsonObject->TryGetStringField(TEXT("name"), eeprom.Name)) return {};
	if (!JsonObject->TryGetStringField(TEXT("title"), eeprom.Title)) return {};
	if (!JsonObject->TryGetStringField(TEXT("description"), eeprom.Description)) return {};

	return eeprom;
}

TOptional<FFINRepoVersion> FFINRepoVersion::FromJsonObject(TSharedPtr<FJsonObject> JsonObject) {
	FFINRepoVersion version;

	if (!JsonObject->TryGetStringField(TEXT("version"), version.Version)) return {};
	JsonObject->TryGetStringField(TEXT("fin_version"), version.FINVersion);
	JsonObject->TryGetStringField(TEXT("game_version"), version.GameVersion);

	const TArray<TSharedPtr<FJsonValue>>* modDependencies;
	if (!JsonObject->TryGetArrayField(TEXT("mod_dependencies"), modDependencies)) return {};
	for (const TSharedPtr<FJsonValue>& modDependencyValue : *modDependencies) {
		TSharedPtr<FJsonObject>* modDependencyObject;
		if (!modDependencyValue->TryGetObject(modDependencyObject)) continue;
		TOptional<FFINRepoModDependency> modDependency = FFINRepoModDependency::FromJsonObject(*modDependencyObject);
		if (modDependency.IsSet()) {
			version.ModDependencies.Add(*modDependency);
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* eeproms;
	if (!JsonObject->TryGetArrayField(TEXT("eeprom"), eeproms)) return {};
	for (const TSharedPtr<FJsonValue>& eepromValue : *eeproms) {
		TSharedPtr<FJsonObject>* eepromObject;
		if (!eepromValue->TryGetObject(eepromObject)) continue;
		TOptional<FFINRepoEEPROM> eeprom = FFINRepoEEPROM::FromJsonObject(*eepromObject);
		if (eeprom.IsSet()) {
			version.EEPROMs.Add(*eeprom);
		}
	}

	return version;
}

TOptional<FFINRepoPackage> FFINRepoPackage::FromJsonObject(TSharedPtr<FJsonObject> JsonObject) {
	FFINRepoPackage package;

	if (!JsonObject->TryGetStringField(TEXT("id"), package.ID)) return {};
	if (!JsonObject->TryGetStringField(TEXT("name"), package.Name)) return {};
	if (!JsonObject->TryGetStringField(TEXT("short_description"), package.ShortDescription)) return {};

	const TSharedPtr<FJsonObject>* readme;
	if (!JsonObject->TryGetObjectField(TEXT("readme"), readme)) return {};
	if ((*readme)->TryGetStringField(TEXT("Markdown"), package.Readme)) {
		package.ReadmeType = EFINRepoReadmeType::FIN_Repo_Readme_Markdown;
	} else if ((*readme)->TryGetStringField(TEXT("ASCIIDOC"), package.Readme)) {
		package.ReadmeType = EFINRepoReadmeType::FIN_Repo_Readme_ASCIIDOC;
	} else {
		return {};
	}

	const TArray<TSharedPtr<FJsonValue>>* tags;
	if (!JsonObject->TryGetArrayField(TEXT("tags"), tags)) return {};
	for (const TSharedPtr<FJsonValue>& tagValue : *tags) {
		FString tag;
		if (!tagValue->TryGetString(tag)) continue;
		package.Tags.Add(tag);
	}

	const TArray<TSharedPtr<FJsonValue>>* authors;
	if (!JsonObject->TryGetArrayField(TEXT("authors"), authors)) return {};
	for (const TSharedPtr<FJsonValue>& authorValue : *authors) {
		FString author;
		if (!authorValue->TryGetString(author)) continue;
		package.Authors.Add(author);
	}

	const TArray<TSharedPtr<FJsonValue>>* versions;
	if (!JsonObject->TryGetArrayField(TEXT("versions"), versions)) return {};
	for (const TSharedPtr<FJsonValue>& versionJson : *versions) {
		TSharedPtr<FJsonObject>* versionObj;
		if (!versionJson->TryGetObject(versionObj)) continue;
		TOptional<FFINRepoVersion> version = FFINRepoVersion::FromJsonObject(*versionObj);
		if (version.IsSet()) {
			package.Versions.Add(*version);
		}
	}

	return package;
}

TOptional<FFINRepoPackageCard> FFINRepoPackageCard::FromJsonObject(TSharedPtr<FJsonObject> JsonObject) {
	FFINRepoPackageCard package;

	if (!JsonObject->TryGetStringField(TEXT("id"), package.ID)) return {};
	if (!JsonObject->TryGetStringField(TEXT("name"), package.Name)) return {};
	if (!JsonObject->TryGetStringField(TEXT("short_description"), package.ShortDescription)) return {};
	JsonObject->TryGetStringField(TEXT("version"), package.Version);

	return package;
}
