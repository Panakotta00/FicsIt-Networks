#pragma once

#include "CoreMinimal.h"
#include "FINRepoModel.generated.h"

UENUM(BlueprintType)
enum EFINRepoReadmeType {
	FIN_Repo_Readme_Markdown,
	FIN_Repo_Readme_ASCIIDOC,
};

USTRUCT(BlueprintType)
struct FFINRepoModDependency {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ID;

	UPROPERTY(BlueprintReadWrite)
	FString Version;

	static TOptional<FFINRepoModDependency> FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};

USTRUCT(BlueprintType)
struct FFINRepoEEPROM {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Title;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	static TOptional<FFINRepoEEPROM> FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};

USTRUCT(BlueprintType)
struct FFINRepoVersion {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Version;

	UPROPERTY(BlueprintReadWrite)
	FString FINVersion;

	UPROPERTY(BlueprintReadWrite)
	FString GameVersion;

	UPROPERTY(BlueprintReadWrite)
	TArray<FFINRepoModDependency> ModDependencies;

	UPROPERTY(BlueprintReadWrite)
	TArray<FFINRepoEEPROM> EEPROMs;

	static TOptional<FFINRepoVersion> FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};

USTRUCT(BlueprintType)
struct FFINRepoPackage {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ID;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString ShortDescription;

	UPROPERTY(BlueprintReadWrite)
	FString Readme;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EFINRepoReadmeType> ReadmeType;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Tags;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Authors;

	UPROPERTY(BlueprintReadWrite)
	TArray<FFINRepoVersion> Versions;

	static TOptional<FFINRepoPackage> FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};

USTRUCT(BlueprintType)
struct FFINRepoPackageCard {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ID;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString ShortDescription;

	UPROPERTY(BlueprintReadWrite)
	FString Version;

	static TOptional<FFINRepoPackageCard> FromJsonObject(TSharedPtr<FJsonObject> JsonObject);
};
