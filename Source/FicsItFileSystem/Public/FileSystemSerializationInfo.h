#pragma once

#include "CoreMinimal.h"
#include "FicsItFileSystem.h"
#include "FicsItFileSystem/Device.h"
#include "FileSystemSerializationInfo.generated.h"

USTRUCT()
struct FICSITFILESYSTEM_API FFileSystemSerializationInfo {
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, FString> Mounts;

	bool Serialize(FArchive& Ar);
};

FArchive& operator<<(FArchive& Ar, FFileSystemSerializationInfo& Info);

template<>
struct TStructOpsTypeTraits<FFileSystemSerializationInfo> : TStructOpsTypeTraitsBase2<FFileSystemSerializationInfo> {
	enum {
		WithSerializer = true,
    };
};

namespace CodersFileSystem {
	#define FIFS_KEEP_CHANGES 1
	#define FIFS_OVERRIDE_CHANGES 0

	FICSITFILESYSTEM_API void SerializePath(TSharedRef<Device> SerializeDevice, FStructuredArchive::FRecord Record, Path Path, FString Name, int& KeepDisk, const TFunction<int(FString)>& AskForDiskOrSave);
}
