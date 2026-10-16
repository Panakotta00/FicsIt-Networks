#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Subsystem/ModSubsystem.h"
#include "FGIconLibrary.h"
#include "FGSaveInterface.h"
#include "FINUtils.h"
#include "GameplayTagContainer.h"
#include "FINMediaSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINMediaTextureTimeout, FString, TexturePath, UObject*, Texture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINMediaTextureLoaded, FString, TexturePath, UObject*, Texture);

class UAsyncTaskDownloadImage;
class UReliableMessagingPlayerComponent;

/** Called with the content of a requested sound file, or nullptr if it couldn't be loaded. */
DECLARE_DELEGATE_OneParam(FFINMediaSoundLoaded, TSharedPtr<const TArray<uint8>> /*Data*/);

/**
 * The FINMediaSubsystem manages various types of Media that can be referenced anywhere in the game (world).
 * It also automatically handles replication/streaming of data if needed.
 *
 * The core is the Media reference that defines what media is requested and from where to load it.
 * For this the media subsystem gets the resources from various different sources (Engine Assets, Local Files & Internet).
 *
 * Reference Types:
 * - Raw References `raw:<UUID|Hash>`
 *		When a unknown resource gets added to the system, it will be referred to as "Raw" and the reference will contain an hash or uuid.
 *		Raw references that get replicated and requested on the client, may then get replicated automatically similar to local files.
 * - Local Files `file:<path to resource in resource folder>`
 *		Refers to an local file that has to be within the resource folder (%localappdata%/FactoryGame/FINResources).
 *		These files may replicated to clients if the file does not exist on the client with the same path.
 * - Game Icons `icon:<game icon id|game icon name>`
 *		Refers to an icon in the UFGIconLibrary from the game.
 *		Each icon is referenced with an ID that you have to provide in the media reference.
 * TODO: Support for Engine Assets
 * - Internet Resource `<URL>` (must start with `https:` or `http:`)
 *		Refers a media resource located in the internet.
 *		Automatically does an http request to fetch the resource if not cached yet.
 *		
 * Media Types are handled completely separately in regards to their API, and so the same reference may point to different resources but is the same string for different types.
 *
 * TODO: Add Media Loading Errors/Error System
 * TODO: Add Media Loading Error Re-Try timeout
 * TODO: Add Replication
 */
UCLASS()
class FICSITNETWORKSMISC_API AFINMediaSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FFINMediaTextureTimeout OnTextureTimeout;
	UPROPERTY(BlueprintAssignable)
	FFINMediaTextureLoaded OnTextureLoaded;
	
	AFINMediaSubsystem();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	virtual bool ShouldSave_Implementation() const override { return true; }

	UFUNCTION(BlueprintCallable, meta=(WorldContext))
	static AFINMediaSubsystem* GetMediaSubsystem(UObject* WorldContext);
	
	/**
	 * Checks if a texture with the given reference is already stored in cache and returns it.
	 * If it is a cache miss, then queues the resource to load and returns nullptr.
	 * Updates the cache timeout so the resource will be cached for longer.
	 * @param TextureReference the local resource reference, game resource reference or URL of the texture to load
	 * @return the texture if loaded into cache, nullptr if not loaded into cache yet.
	 */
	UObject* GetOrLoadTexture(FString TextureReference);

	/**
	 * Returns the reference of a texture.
	 * If a texture is unknown to the cache or can not be easily found by a source, the reference will be a raw reference and get cached.
	 * @param Texture 
	 * @return 
	 */
	FString GetTextureReference(UObject* Texture);

	/** Sound files larger than this are not played or transferred. */
	static constexpr int32 MaxSoundFileSize = 32 * 1024 * 1024;
	/** Sound files no one uses anymore get removed from the cache once it is larger than this. */
	static constexpr int64 MaxSoundCacheSize = 128 * 1024 * 1024;

	static FString HashSoundData(const TArray<uint8>& Data);

	/**
	 * Loads a sound file from the sounds folder (Saved/SaveGames/Computers/Sounds).
	 * @param Sound path relative to the sounds folder, without the .ogg file extension
	 */
	static bool LoadSoundFile(const FString& Sound, TArray<uint8>& OutData);

	/**
	 * Adds the given sound file content to the sound cache and returns its hash.
	 * On the server, clients can only download sounds which are in the cache.
	 */
	FString AddSound(TArray<uint8>&& Data);

	/** Returns the sound with the given hash if it is in the cache. Holding the returned pointer keeps the sound in the cache. */
	TSharedPtr<const TArray<uint8>> FindSound(const FString& Hash);

	/**
	 * Gets the content of a sound file of the server.
	 * Calls the callback right away if the sound is in the cache or the local sounds folder has the same file (same hash).
	 * Otherwise downloads it from the server and calls the callback once it arrived (or with nullptr if the server doesn't have it).
	 * @param Sound path of the sound file of the server, relative to the sounds folder
	 * @param Hash MD5 hash of the sound file of the server
	 * @param Size size of the sound file of the server in bytes
	 */
	void RequestSound(const FString& Sound, const FString& Hash, int32 Size, FFINMediaSoundLoaded Callback);

private:
	void AddTextureToCache(const FString& TextureReference, UObject* Texture);
	UObject* RemoveTextureFromCache(const FString& TextureReference);
	UObject* LoadTexture(const FString& TextureReference);
	void LoadHTTPTexture(const FString& TextureReference);
	UObject* LoadFileTexture(const FString& TextureReference);
	UObject* LoadGameTexture(const FString& TextureReference);
	UObject* LoadEngineTexture(const FString& TextureReference);
	void DownloadNextTexture();

	UFUNCTION()
	void HandleTextureDownload(UTexture2DDynamic* Texture);

	void TrimSoundCache();
	void UpdateSoundTransfer();
	/** Registers the sound transfer message handlers on player controllers which got their reliable messaging component since the last call. */
	void RegisterSoundHandlers();
	void SendPendingSoundRequests();
	void HandleSoundRequest(FGameplayTag Tag, TArray<uint8>&& Payload, TWeakObjectPtr<APlayerController> Player);
	void HandleSoundData(FGameplayTag Tag, TArray<uint8>&& Payload);
	void FinishSoundDownload(const FString& Hash, TSharedPtr<const TArray<uint8>> Data);

	UPROPERTY()
	FString ResourcesFolderPath = TEXT("");
	UPROPERTY()
	TMap<FString, UObject*> Reference2Texture;
	UPROPERTY()
	TMap<UObject*, FString> Texture2Reference;
	UPROPERTY()
	TMap<FString, FDateTime> TextureTimeouts;
	UPROPERTY()
	TSet<FString> LoadingTextures;
	TQueue<FString> TextureDownloads;
	UPROPERTY()
	UAsyncTaskDownloadImage* TextureDownloadTask = nullptr;
	UPROPERTY()
	TMap<FString, FIconData> GameIconFindCache;

	TMap<FString, TSharedRef<const TArray<uint8>>> SoundCache;
	/** Hashes of the sound cache in order of use, least recently used first. */
	TArray<FString> SoundCacheOrder;

	struct FSoundDownload {
		int32 Size = 0;
		bool bRequested = false;
		TArray<FFINMediaSoundLoaded> Callbacks;
	};
	TMap<FString, FSoundDownload> SoundDownloads;

	TSet<TWeakObjectPtr<UReliableMessagingPlayerComponent>> SoundHandlerComponents;
	FTimerHandle SoundTransferTimer;

public:
	// Begin FIN Reflection
	UFUNCTION()
	void netClass_Meta(FFIRFunctionMeta& netFuncMeta_findGameIcon);
	
	UFUNCTION()
	FIconData netFunc_findGameIcon(const FString& IconName);

	UFUNCTION()
	TArray<FIconData> netFunc_getGameIcons(int64 PageSize, int64 Page);

	UFUNCTION()
	bool netFunc_isTextureLoading(const FString& TextureReference);

	UFUNCTION()
	bool netFunc_isTextureLoaded(const FString& TextureReference);

	UFUNCTION()
	void netFunc_loadTexture(const FString& TextureReference);
	// End FIN Reflection
};
