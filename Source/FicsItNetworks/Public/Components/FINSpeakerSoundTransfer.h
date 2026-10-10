#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FINSpeakerSoundTransfer.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FFINSpeakerSoundAvailable, const FString& /*Hash*/);

/**
 * Keeps the sound files played by speaker poles in memory, identified by the MD5 hash of their content,
 * and downloads files a client doesn't have from the server.
 * Speaker poles only play files from the server's sounds folder, so every player hears the same sound.
 */
class FICSITNETWORKS_API FFINSpeakerSoundTransfer {
public:
	/** Files larger than this are not played, so a single sound can't flood the connection. */
	static constexpr int32 MaxFileSize = 32 * 1024 * 1024;
	static constexpr int32 ChunkSize = 16 * 1024;
	/** Number of chunks requested at once, more chunks only get requested when previous ones arrived. */
	static constexpr int32 ChunkWindow = 8;
	/** Files not used by any speaker get removed from the cache once it is larger than this. */
	static constexpr int64 MaxCacheSize = 128 * 1024 * 1024;

	static FString HashData(const TArray<uint8>& Data);

	/** Adds the given file content to the cache and returns its hash. */
	static FString Add(TArray<uint8>&& Data);

	static TSharedPtr<const TArray<uint8>> Find(const FString& Hash);

	/**
	 * Starts downloading the file with the given hash from the server, if it isn't already downloading.
	 * OnSoundAvailable gets broadcast once it is in the cache.
	 * Returns false if the download can't be started yet (the remote call object of the player is not replicated yet).
	 */
	static bool Request(UWorld* World, const FString& Hash, int32 Size);

	/** Broadcast on the game thread when a downloaded file was added to the cache. */
	static FFINSpeakerSoundAvailable OnSoundAvailable;

private:
	friend class UFINSpeakerRCO;

	struct FDownload {
		TArray<uint8> Buffer;
		TBitArray<> ReceivedChunks;
		int32 NumReceived = 0;
		int32 NextChunk = 0;
		TWeakObjectPtr<class UFINSpeakerRCO> RCO;

		int32 GetNumChunks() const { return FMath::DivideAndRoundUp(Buffer.Num(), ChunkSize); }
	};

	static void ReceiveChunk(UFINSpeakerRCO* RCO, const FString& Hash, int32 ChunkIndex, const TArray<uint8>& Data);
	static void RequestNextChunk(FDownload& Download, const FString& Hash);
	static void TrimCache();

	static TMap<FString, TSharedRef<const TArray<uint8>>> Cache;
	/** Hashes of the cache in order of use, least recently used first. */
	static TArray<FString> CacheOrder;
	static TMap<FString, FDownload> Downloads;
};

/**
 * Remote call object used by clients to download speaker sound files from the server.
 */
UCLASS()
class FICSITNETWORKS_API UFINSpeakerRCO : public UFGRemoteCallObject {
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	bool bDummy = false;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestSoundChunk(const FString& Hash, int32 ChunkIndex);

	/** Sends one chunk of the file to the client. An empty chunk means the server doesn't have the file (anymore). */
	UFUNCTION(Client, Reliable)
	void ClientReceiveSoundChunk(const FString& Hash, int32 ChunkIndex, const TArray<uint8>& Data);
};
