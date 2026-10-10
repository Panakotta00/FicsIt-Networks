#include "Components/FINSpeakerSoundTransfer.h"

#include "FGPlayerController.h"
#include "FicsItNetworksModule.h"
#include "Misc/SecureHash.h"
#include "Net/UnrealNetwork.h"

FFINSpeakerSoundAvailable FFINSpeakerSoundTransfer::OnSoundAvailable;
TMap<FString, TSharedRef<const TArray<uint8>>> FFINSpeakerSoundTransfer::Cache;
TArray<FString> FFINSpeakerSoundTransfer::CacheOrder;
TMap<FString, FFINSpeakerSoundTransfer::FDownload> FFINSpeakerSoundTransfer::Downloads;

FString FFINSpeakerSoundTransfer::HashData(const TArray<uint8>& Data) {
	return FMD5::HashBytes(Data.GetData(), Data.Num());
}

FString FFINSpeakerSoundTransfer::Add(TArray<uint8>&& Data) {
	FString Hash = HashData(Data);
	if (!Cache.Contains(Hash)) {
		Cache.Add(Hash, MakeShared<const TArray<uint8>>(MoveTemp(Data)));
	}
	CacheOrder.Remove(Hash);
	CacheOrder.Add(Hash);
	TrimCache();
	return Hash;
}

TSharedPtr<const TArray<uint8>> FFINSpeakerSoundTransfer::Find(const FString& Hash) {
	const TSharedRef<const TArray<uint8>>* Data = Cache.Find(Hash);
	if (!Data) return nullptr;
	CacheOrder.Remove(Hash);
	CacheOrder.Add(Hash);
	return *Data;
}

void FFINSpeakerSoundTransfer::TrimCache() {
	int64 Size = 0;
	for (const TPair<FString, TSharedRef<const TArray<uint8>>>& Entry : Cache) {
		Size += Entry.Value->Num();
	}
	for (int32 i = 0; i < CacheOrder.Num() && Size > MaxCacheSize;) {
		const TSharedRef<const TArray<uint8>>& Data = Cache.FindChecked(CacheOrder[i]);
		// Still referenced by a speaker which plays it
		if (Data.GetSharedReferenceCount() > 1) {
			++i;
			continue;
		}
		Size -= Data->Num();
		Cache.Remove(CacheOrder[i]);
		CacheOrder.RemoveAt(i);
	}
}

bool FFINSpeakerSoundTransfer::Request(UWorld* World, const FString& Hash, int32 Size) {
	if (Size <= 0 || Size > MaxFileSize) return false;

	FDownload* Download = Downloads.Find(Hash);
	if (Download && Download->RCO.IsValid()) return true;

	AFGPlayerController* Controller = World ? Cast<AFGPlayerController>(World->GetFirstPlayerController()) : nullptr;
	UFINSpeakerRCO* RCO = Controller ? Controller->GetRemoteCallObjectOfClass<UFINSpeakerRCO>() : nullptr;
	if (!RCO) return false;

	// A download of a previous session (or one without a valid connection anymore) gets restarted
	Download = &Downloads.Add(Hash);
	Download->Buffer.SetNumUninitialized(Size);
	Download->ReceivedChunks.Init(false, Download->GetNumChunks());
	Download->RCO = RCO;

	UE_LOG(LogFicsItNetworks, Display, TEXT("Downloading speaker sound '%s' (%d bytes) from the server."), *Hash, Size);
	for (int32 i = 0; i < ChunkWindow; ++i) {
		RequestNextChunk(*Download, Hash);
	}
	return true;
}

void FFINSpeakerSoundTransfer::RequestNextChunk(FDownload& Download, const FString& Hash) {
	if (Download.NextChunk >= Download.GetNumChunks() || !Download.RCO.IsValid()) return;
	Download.RCO->ServerRequestSoundChunk(Hash, Download.NextChunk++);
}

void FFINSpeakerSoundTransfer::ReceiveChunk(UFINSpeakerRCO* RCO, const FString& Hash, int32 ChunkIndex, const TArray<uint8>& Data) {
	FDownload* Download = Downloads.Find(Hash);
	if (!Download || Download->RCO.Get() != RCO) return;

	if (Data.Num() == 0) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Server does not have speaker sound '%s' anymore, download canceled."), *Hash);
		Downloads.Remove(Hash);
		return;
	}

	const int32 Offset = ChunkIndex * ChunkSize;
	const int32 ExpectedSize = FMath::Min(ChunkSize, Download->Buffer.Num() - Offset);
	if (ChunkIndex < 0 || ChunkIndex >= Download->GetNumChunks() || Data.Num() != ExpectedSize || Download->ReceivedChunks[ChunkIndex]) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Received invalid chunk %d of speaker sound '%s', download canceled."), ChunkIndex, *Hash);
		Downloads.Remove(Hash);
		return;
	}

	FMemory::Memcpy(Download->Buffer.GetData() + Offset, Data.GetData(), Data.Num());
	Download->ReceivedChunks[ChunkIndex] = true;
	++Download->NumReceived;

	if (Download->NumReceived < Download->GetNumChunks()) {
		RequestNextChunk(*Download, Hash);
		return;
	}

	TArray<uint8> Buffer = MoveTemp(Download->Buffer);
	Downloads.Remove(Hash);
	if (HashData(Buffer) != Hash) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Downloaded speaker sound '%s' is corrupted."), *Hash);
		return;
	}
	UE_LOG(LogFicsItNetworks, Display, TEXT("Downloaded speaker sound '%s'."), *Hash);
	Add(MoveTemp(Buffer));
	OnSoundAvailable.Broadcast(Hash);
}

void UFINSpeakerRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFINSpeakerRCO, bDummy);
}

void UFINSpeakerRCO::ServerRequestSoundChunk_Implementation(const FString& Hash, int32 ChunkIndex) {
	// Only files a speaker played are in the cache, clients can't request arbitrary files
	const TSharedPtr<const TArray<uint8>> Data = FFINSpeakerSoundTransfer::Find(Hash);
	const int32 Offset = ChunkIndex * FFINSpeakerSoundTransfer::ChunkSize;
	if (!Data.IsValid() || Offset >= Data->Num()) {
		ClientReceiveSoundChunk(Hash, ChunkIndex, TArray<uint8>());
		return;
	}
	const int32 Size = FMath::Min(FFINSpeakerSoundTransfer::ChunkSize, Data->Num() - Offset);
	ClientReceiveSoundChunk(Hash, ChunkIndex, TArray<uint8>(Data->GetData() + Offset, Size));
}

bool UFINSpeakerRCO::ServerRequestSoundChunk_Validate(const FString& Hash, int32 ChunkIndex) {
	return Hash.Len() <= 64 && ChunkIndex >= 0 && ChunkIndex < FMath::DivideAndRoundUp(FFINSpeakerSoundTransfer::MaxFileSize, FFINSpeakerSoundTransfer::ChunkSize);
}

void UFINSpeakerRCO::ClientReceiveSoundChunk_Implementation(const FString& Hash, int32 ChunkIndex, const TArray<uint8>& Data) {
	FFINSpeakerSoundTransfer::ReceiveChunk(this, Hash, ChunkIndex, Data);
}
