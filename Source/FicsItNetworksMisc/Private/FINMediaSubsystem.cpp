#include "FINMediaSubsystem.h"

#include "FGIconDatabaseSubsystem.h"
#include "FGIconLibrary.h"
#include "ImageUtils.h"
#include "Engine/Engine.h"
#include "Blueprint/AsyncTaskDownloadImage.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/Paths.h"
#include "Reflection/Source/FIRSourceUObject.h"
#include "Subsystem/SubsystemActorManager.h"

AFINMediaSubsystem::AFINMediaSubsystem() {
	SetActorTickEnabled(true);
	
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (ResourcesFolderPath.IsEmpty()) {
		ResourcesFolderPath = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("FINResources"));
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ResourcesFolderPath)) PlatformFile.CreateDirectoryTree(*ResourcesFolderPath);
}

void AFINMediaSubsystem::BeginPlay() {
	Super::BeginPlay();

	TArray<FIconData> Data;
	AFGIconDatabaseSubsystem::Get(this)->GetIconData(Data);
	for (const FIconData& Icon : Data) {
		GameIconFindCache.Add(Icon.IconName.ToString(), Icon);
	}
}

void AFINMediaSubsystem::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	FDateTime Now = FDateTime::Now();
	TMap<FString, FDateTime> OldTimeouts = TextureTimeouts;
	for (const TPair<FString, FDateTime>& Timeout : OldTimeouts) {
		if (Now < Timeout.Value) continue;
		RemoveTextureFromCache(Timeout.Key);
	}
}

AFINMediaSubsystem* AFINMediaSubsystem::GetMediaSubsystem(UObject* WorldContext) {
#if WITH_EDITOR
	return nullptr;
#endif
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINMediaSubsystem>();
}

UObject* AFINMediaSubsystem::GetOrLoadTexture(FString TextureReference) {
	TextureReference.TrimStartAndEndInline();
	
	UObject** TexturePtr = Reference2Texture.Find(TextureReference);
	if (TexturePtr) return *TexturePtr;

	UObject* Texture = LoadTexture(TextureReference);
	if (Texture) return Texture;

	return nullptr;
}

FString AFINMediaSubsystem::GetTextureReference(UObject* Texture) {
	FString* Reference = Texture2Reference.Find(Texture);
	if (Reference) return *Reference;

	int32 IconID = AFGIconDatabaseSubsystem::Get(this)->GetIconIDForTexture(Texture);
	if (IconID >= 0) {
		return FString::Printf(TEXT("icon:%i"), IconID);
	}

	FString RawReference;
	do {
		RawReference = FString::Printf(TEXT("raw:%s"), *FGuid::NewGuid().ToString());
	} while (Reference2Texture.Contains(RawReference));

	AddTextureToCache(RawReference, Texture);

	return RawReference;
}

UObject* AFINMediaSubsystem::LoadTexture(const FString& TextureReference) {
	if (LoadingTextures.Contains(TextureReference)) return nullptr;
	
	if (TextureReference.StartsWith(TEXT("http:")) || TextureReference.StartsWith(TEXT("https:"))) {
		LoadHTTPTexture(TextureReference);
	} else if (TextureReference.StartsWith(TEXT("file:"))) {
		return LoadFileTexture(TextureReference);
	} else if (TextureReference.StartsWith(TEXT("icon:"))) {
		return LoadGameTexture(TextureReference);
	} else if (TextureReference.StartsWith("engine:")) {
		return LoadEngineTexture(TextureReference);
	} else if (TextureReference.StartsWith(TEXT("raw:"))) {
		
	}
	return nullptr;
}

void AFINMediaSubsystem::AddTextureToCache(const FString& TextureReference, UObject* Texture) {
	LoadingTextures.Remove(TextureReference);
	if (!Texture) return;
	Reference2Texture.Add(TextureReference, Texture);
	Texture2Reference.Add(Texture, TextureReference);
	TextureTimeouts.Add(TextureReference, FDateTime::Now() + FTimespan::FromMinutes(30));
	OnTextureLoaded.Broadcast(TextureReference, Texture);
}

UObject* AFINMediaSubsystem::RemoveTextureFromCache(const FString& TextureReference) {
	UObject* Texture;
	Reference2Texture.RemoveAndCopyValue(TextureReference, Texture);
	Texture2Reference.Remove(Texture);
	TextureTimeouts.Remove(TextureReference);
	OnTextureTimeout.Broadcast(TextureReference, Texture);
	return Texture;
}

void AFINMediaSubsystem::LoadHTTPTexture(const FString& TextureReference) {
	LoadingTextures.Add(TextureReference);
	TextureDownloads.Enqueue(TextureReference);
	DownloadNextTexture();
}

UObject* AFINMediaSubsystem::LoadFileTexture(const FString& TextureReference) {
	FString FilePath = TextureReference.RightChop(FString(TEXT("file:")).Len());
	FilePath = FPaths::Combine(ResourcesFolderPath, FilePath);
	FilePath = FPaths::ConvertRelativePathToFull(FilePath);
	if (!FPaths::IsUnderDirectory(FilePath, ResourcesFolderPath)) return nullptr;
	UObject* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
	AddTextureToCache(TextureReference, Texture);
	return Texture;
}

UObject* AFINMediaSubsystem::LoadGameTexture(const FString& TextureReference) {
	FString IconName = TextureReference.RightChop(FString(TEXT("game:")).Len());
	int32 ID;
	UObject* Texture = nullptr;
	if (FDefaultValueHelper::ParseInt(IconName, ID)) {
		Texture = AFGIconDatabaseSubsystem::Get(this)->GetIconTextureFromIconID(ID);
	} else {
		FIconData* IconData = GameIconFindCache.Find(IconName);
		if (IconData) Texture = IconData->Texture.Get();
	}
	AddTextureToCache(TextureReference, Texture);
	return Texture;
}

UObject* AFINMediaSubsystem::LoadEngineTexture(const FString& TextureReference) {
	FString AssetPath = TextureReference.RightChop(FString(TEXT("engine:")).Len());
	FSoftObjectPath Object(AssetPath);
	UObject* Texture = Object.TryLoad();
	AddTextureToCache(TextureReference, Texture);
	return Texture;
}

void AFINMediaSubsystem::DownloadNextTexture() {
	if (TextureDownloadTask) return;
	FString Reference;
	if (!TextureDownloads.Peek(Reference)) return;
	TextureDownloadTask = UAsyncTaskDownloadImage::DownloadImage(Reference);
	TextureDownloadTask->OnFail.AddDynamic(this, &AFINMediaSubsystem::HandleTextureDownload);
	TextureDownloadTask->OnSuccess.AddDynamic(this, &AFINMediaSubsystem::HandleTextureDownload);
}

void AFINMediaSubsystem::HandleTextureDownload(UTexture2DDynamic* Texture) {
	FString Reference;
	if (TextureDownloads.Dequeue(Reference)) {
		AddTextureToCache(Reference, Texture);
	}
	TextureDownloadTask = nullptr;
	DownloadNextTexture();
}

void AFINMediaSubsystem::netClass_Meta(FFIRFunctionMeta& netFuncMeta_findGameIcon) {
	netFuncMeta_findGameIcon.InternalName = TEXT("findGameIcon");
	netFuncMeta_findGameIcon.DisplayName = FText::FromString(TEXT("Find Game Icon"));
	netFuncMeta_findGameIcon.Description = FText::FromString(TEXT("Tries to find an game icon like the ones you use for signs."));
}

FIconData AFINMediaSubsystem::netFunc_findGameIcon(const FString& IconName) {
	FIconData* Icon = GameIconFindCache.Find(IconName);
	if (!Icon) return FIconData();
	return *Icon;
}

TArray<FIconData> AFINMediaSubsystem::netFunc_getGameIcons(int64 PageSize, int64 Page) {
	TArray<FIconData> Icons;
	AFGIconDatabaseSubsystem::Get(this)->GetIconData(Icons);
	TArray<FIconData> Data;
	Data = UFINUtils::PaginateArray(TArrayView<const FIconData>(Icons), PageSize, Page);
	if (Page < 0) Algo::Reverse(Data);
	return Data;
}

bool AFINMediaSubsystem::netFunc_isTextureLoading(const FString& TextureReference) {
	return LoadingTextures.Contains(TextureReference);
}

bool AFINMediaSubsystem::netFunc_isTextureLoaded(const FString& TextureReference) {
	return Reference2Texture.Contains(TextureReference);
}

void AFINMediaSubsystem::netFunc_loadTexture(const FString& TextureReference) {
	GetOrLoadTexture(TextureReference);
}
