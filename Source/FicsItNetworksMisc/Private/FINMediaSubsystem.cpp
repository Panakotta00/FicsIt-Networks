#include "FINMediaSubsystem.h"

#include "FGIconDatabaseSubsystem.h"
#include "FGIconLibrary.h"
#include "FicsItNetworksMisc.h"
#include "NativeGameplayTags.h"
#include "ReliableMessagingPlayerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
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
#include "TimerManager.h"

/** Client -> Server: hash of a sound file the client wants to download */
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_FIN_Media_SoundRequest, "FIN.Media.SoundRequest");
/** Server -> Client: hash, whether the server has the file, file content */
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_FIN_Media_SoundData, "FIN.Media.SoundData");

AFINMediaSubsystem::AFINMediaSubsystem() {
	// Clients need their own instance for their caches and to download media from the server
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnLocal;
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

	if (AFGIconDatabaseSubsystem* IconDatabase = AFGIconDatabaseSubsystem::Get(this)) {
		TArray<FIconData> Data;
		IconDatabase->GetIconData(Data);
		for (const FIconData& Icon : Data) {
			GameIconFindCache.Add(Icon.IconName.ToString(), Icon);
		}
	}

	// The reliable messaging component of a player controller may get added after the controller spawned.
	// A timer instead of Tick, the subsystem actor doesn't tick (bCanEverTick is false).
	if (GetNetMode() != NM_Standalone) {
		GetWorldTimerManager().SetTimer(SoundTransferTimer, this, &AFINMediaSubsystem::UpdateSoundTransfer, 0.5f, true, 0.f);
	}
}

void AFINMediaSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	GetWorldTimerManager().ClearTimer(SoundTransferTimer);
	Super::EndPlay(EndPlayReason);
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
#else
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINMediaSubsystem>();
#endif
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

FString AFINMediaSubsystem::HashSoundData(const TArray<uint8>& Data) {
	return FMD5::HashBytes(Data.GetData(), Data.Num());
}

bool AFINMediaSubsystem::LoadSoundFile(const FString& Sound, TArray<uint8>& OutData) {
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	const FString SaveGamesPath = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/SaveGames/"));

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	const FString SoundsFolderPath = FPaths::Combine(SaveGamesPath, TEXT("Computers/Sounds"));
	FileManager.CreateDirectoryTree(*SoundsFolderPath);

	FString FilePath = FPaths::Combine(SoundsFolderPath, Sound + TEXT(".ogg"));
	FPaths::CollapseRelativeDirectories(FilePath);
	if (!FilePath.StartsWith(SoundsFolderPath + TEXT("/"))) {
		UE_LOG(LogFicsItNetworksMisc, Warning, TEXT("Tried to load sound from '%s' but outside of sounds folder."), *FilePath);
		return false;
	}

	if (!FFileHelper::LoadFileToArray(OutData, *FilePath, FILEREAD_Silent)) {
		UE_LOG(LogFicsItNetworksMisc, Warning, TEXT("Sound file '%s' not found in sounds folder."), *FilePath);
		return false;
	}
	return true;
}

FString AFINMediaSubsystem::AddSound(TArray<uint8>&& Data) {
	FString Hash = HashSoundData(Data);
	if (!SoundCache.Contains(Hash)) {
		SoundCache.Add(Hash, MakeShared<const TArray<uint8>>(MoveTemp(Data)));
	}
	SoundCacheOrder.Remove(Hash);
	SoundCacheOrder.Add(Hash);
	TrimSoundCache();
	return Hash;
}

TSharedPtr<const TArray<uint8>> AFINMediaSubsystem::FindSound(const FString& Hash) {
	const TSharedRef<const TArray<uint8>>* Data = SoundCache.Find(Hash);
	if (!Data) return nullptr;
	SoundCacheOrder.Remove(Hash);
	SoundCacheOrder.Add(Hash);
	return *Data;
}

void AFINMediaSubsystem::TrimSoundCache() {
	int64 Size = 0;
	for (const TPair<FString, TSharedRef<const TArray<uint8>>>& Entry : SoundCache) {
		Size += Entry.Value->Num();
	}
	for (int32 i = 0; i < SoundCacheOrder.Num() && Size > MaxSoundCacheSize;) {
		const TSharedRef<const TArray<uint8>>& Data = SoundCache.FindChecked(SoundCacheOrder[i]);
		// Still referenced, e.g. by a speaker which plays it
		if (Data.GetSharedReferenceCount() > 1) {
			++i;
			continue;
		}
		Size -= Data->Num();
		SoundCache.Remove(SoundCacheOrder[i]);
		SoundCacheOrder.RemoveAt(i);
	}
}

void AFINMediaSubsystem::RequestSound(const FString& Sound, const FString& Hash, int32 Size, FFINMediaSoundLoaded Callback) {
	TSharedPtr<const TArray<uint8>> Data = FindSound(Hash);
	if (!Data.IsValid()) {
		// Use the file of the own sounds folder if it is the same as the server's
		TArray<uint8> LocalData;
		if (LoadSoundFile(Sound, LocalData) && LocalData.Num() == Size && HashSoundData(LocalData) == Hash) {
			AddSound(MoveTemp(LocalData));
			Data = FindSound(Hash);
		}
	}
	// The server only plays its own files (HasAuthority() is also true on clients, the subsystem is spawned locally)
	if (Data.IsValid() || GetNetMode() != NM_Client || Size <= 0 || Size > MaxSoundFileSize) {
		Callback.ExecuteIfBound(Data);
		return;
	}

	FSoundDownload& Download = SoundDownloads.FindOrAdd(Hash);
	Download.Size = Size;
	Download.Callbacks.Add(MoveTemp(Callback));
	SendPendingSoundRequests();
}

void AFINMediaSubsystem::UpdateSoundTransfer() {
	RegisterSoundHandlers();
	SendPendingSoundRequests();
}

void AFINMediaSubsystem::RegisterSoundHandlers() {
	if (GetNetMode() == NM_Standalone) return;

	const bool bServer = GetNetMode() != NM_Client;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		APlayerController* Player = It->Get();
		// The server answers requests of remote players, a client receives the files from the server
		if (!Player || bServer == Player->IsLocalController()) continue;

		UReliableMessagingPlayerComponent* Component = UReliableMessagingPlayerComponent::GetFromPlayer(Player);
		if (!Component || SoundHandlerComponents.Contains(Component)) continue;
		SoundHandlerComponents.Add(Component);

		if (bServer) {
			Component->RegisterTaggedMessageHandler(TAG_FIN_Media_SoundRequest, UReliableMessagingPlayerComponent::FOnBulkDataReplicationPayloadReceived::CreateUObject(this, &AFINMediaSubsystem::HandleSoundRequest, TWeakObjectPtr<APlayerController>(Player)));
		} else {
			Component->RegisterTaggedMessageHandler(TAG_FIN_Media_SoundData, UReliableMessagingPlayerComponent::FOnBulkDataReplicationPayloadReceived::CreateUObject(this, &AFINMediaSubsystem::HandleSoundData));
		}
	}
}

void AFINMediaSubsystem::SendPendingSoundRequests() {
	if (SoundDownloads.IsEmpty() || GetNetMode() != NM_Client) return;

	APlayerController* Player = GetWorld()->GetFirstPlayerController();
	UReliableMessagingPlayerComponent* Component = Player ? UReliableMessagingPlayerComponent::GetFromPlayer(Player) : nullptr;
	// Not available yet right after joining, retried by the timer
	if (!Component) return;
	RegisterSoundHandlers();

	for (TPair<FString, FSoundDownload>& Download : SoundDownloads) {
		if (Download.Value.bRequested) continue;
		Download.Value.bRequested = true;

		TArray<uint8> Payload;
		FMemoryWriter Writer(Payload);
		FString Hash = Download.Key;
		Writer << Hash;
		UE_LOG(LogFicsItNetworksMisc, Display, TEXT("Downloading sound '%s' (%d bytes) from the server."), *Hash, Download.Value.Size);
		Component->SendTaggedMessage(TAG_FIN_Media_SoundRequest, MoveTemp(Payload));
	}
}

void AFINMediaSubsystem::HandleSoundRequest(FGameplayTag Tag, TArray<uint8>&& Payload, TWeakObjectPtr<APlayerController> Player) {
	FString Hash;
	FMemoryReader Reader(Payload);
	Reader.ArMaxSerializeSize = 256;
	Reader << Hash;
	UReliableMessagingPlayerComponent* Component = Player.IsValid() ? UReliableMessagingPlayerComponent::GetFromPlayer(Player.Get()) : nullptr;
	if (Reader.IsError() || !Component) return;

	// Only sounds which got played are in the cache, clients can't request arbitrary files
	const TSharedPtr<const TArray<uint8>> Data = FindSound(Hash);
	bool bFound = Data.IsValid();

	TArray<uint8> Response;
	FMemoryWriter Writer(Response);
	Writer << Hash;
	Writer << bFound;
	if (bFound) {
		int32 Size = Data->Num();
		Writer << Size;
		Writer.Serialize(const_cast<uint8*>(Data->GetData()), Size);
	}
	UE_LOG(LogFicsItNetworksMisc, Display, TEXT("Sending sound '%s' to %s (%s)."), *Hash, *Player->GetName(), bFound ? TEXT("found") : TEXT("not in cache"));
	Component->SendTaggedMessage(TAG_FIN_Media_SoundData, MoveTemp(Response));
}

void AFINMediaSubsystem::HandleSoundData(FGameplayTag Tag, TArray<uint8>&& Payload) {
	FString Hash;
	bool bFound = false;
	int32 Size = 0;
	FMemoryReader Reader(Payload);
	Reader.ArMaxSerializeSize = 256;
	Reader << Hash;
	Reader << bFound;
	const FSoundDownload* Download = SoundDownloads.Find(Hash);
	if (Reader.IsError() || !Download) return;

	if (bFound) {
		Reader << Size;
	}
	if (!bFound || Reader.IsError() || Size != Download->Size || Reader.TotalSize() - Reader.Tell() != Size) {
		UE_LOG(LogFicsItNetworksMisc, Warning, TEXT("Server does not have sound '%s' (anymore) or sent invalid data."), *Hash);
		FinishSoundDownload(Hash, nullptr);
		return;
	}

	TArray<uint8> Data(Payload.GetData() + Reader.Tell(), Size);
	if (HashSoundData(Data) != Hash) {
		UE_LOG(LogFicsItNetworksMisc, Warning, TEXT("Downloaded sound '%s' is corrupted."), *Hash);
		FinishSoundDownload(Hash, nullptr);
		return;
	}

	UE_LOG(LogFicsItNetworksMisc, Display, TEXT("Downloaded sound '%s'."), *Hash);
	AddSound(MoveTemp(Data));
	FinishSoundDownload(Hash, FindSound(Hash));
}

void AFINMediaSubsystem::FinishSoundDownload(const FString& Hash, TSharedPtr<const TArray<uint8>> Data) {
	FSoundDownload Download;
	if (!SoundDownloads.RemoveAndCopyValue(Hash, Download)) return;
	for (FFINMediaSoundLoaded& Callback : Download.Callbacks) {
		Callback.ExecuteIfBound(Data);
	}
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
