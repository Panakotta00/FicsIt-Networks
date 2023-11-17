#pragma once

#include "CoreMinimal.h"
#include "Library/Device.h"
#include "FileSystemSerializationInfo.generated.h"

struct FFileSystemNode;

USTRUCT()
struct FICSITNETWORKS_API FFileSystemNodeIndex {
	GENERATED_BODY()
	
    TSharedPtr<FFileSystemNode> Node;

	FFileSystemNodeIndex() : Node(nullptr) {}
	FFileSystemNodeIndex(TSharedPtr<FFileSystemNode> node) : Node(node) {}
	operator TSharedPtr<FFileSystemNode>() const {
		return Node;
	}

	bool Serialize(FArchive& Ar);

	CodersFileSystem::SRef<CodersFileSystem::Node> Deserialize(FString name, CodersFileSystem::SRef<CodersFileSystem::Directory> parent) const;
};

FArchive& operator<<(FArchive& Ar, FFileSystemNodeIndex& Node);

template<>
struct TStructOpsTypeTraits<FFileSystemNodeIndex> : TStructOpsTypeTraitsBase2<FFileSystemNodeIndex> {
	enum {
		WithSerializer = true,
    };
};

USTRUCT()
struct FICSITNETWORKS_API FFileSystemNode {
	GENERATED_BODY()

	/**
	 * Multi purpose data of the node used to store the nodes data.
	 * f.e. file contents
	 */
	UPROPERTY()
	FString Data = "";

	/**
	 * Used to identify the type of the node.
	 * -1 = Invalid
	 * 0 = File
	 * 1 = Directory
	 * 2 = DiskFS
	 * 3 = TempFS
	 */
	UPROPERTY()
	int NodeType = -1;

	UPROPERTY()
	TMap<FString, FFileSystemNodeIndex> ChildNodes;

	bool Serialize(FArchive& Ar);

	FFileSystemNode& Deserialize(CodersFileSystem::SRef<CodersFileSystem::Device> device, const std::string& deviceName);
	FFileSystemNode& Serialize(CodersFileSystem::SRef<CodersFileSystem::Device> device, const CodersFileSystem::Path& path);
};

FArchive& operator<<(FArchive& Ar, FFileSystemNode& Node);

template<>
struct TStructOpsTypeTraits<FFileSystemNode> : TStructOpsTypeTraitsBase2<FFileSystemNode> {
	enum {
		WithSerializer = true,
    };
};

USTRUCT()
struct FICSITNETWORKS_API FFileSystemSerializationInfo {
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, FString> Mounts;

	UPROPERTY()
	TMap<FString, FFileSystemNode> Devices;

	bool Serialize(FArchive& Ar);
};

FArchive& operator<<(FArchive& Ar, FFileSystemSerializationInfo& Info);

template<>
struct TStructOpsTypeTraits<FFileSystemSerializationInfo> : TStructOpsTypeTraitsBase2<FFileSystemSerializationInfo> {
	enum {
		WithSerializer = true,
    };
};
