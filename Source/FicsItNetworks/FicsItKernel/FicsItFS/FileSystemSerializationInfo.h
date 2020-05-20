#pragma once

#include "CoreMinimal.h"
#include "FileSystemSerializationInfo.generated.h"

struct FFileSystemNode;

USTRUCT()
struct FFileSystemNodeIndex {
	GENERATED_BODY()
	
    TSharedPtr<FFileSystemNode> Node;

	FFileSystemNodeIndex() : Node(nullptr) {}
	FFileSystemNodeIndex(TSharedPtr<FFileSystemNode> node) : Node(node) {}
	operator TSharedPtr<FFileSystemNode>() const {
		return Node;
	}

	bool Serialize(FArchive& Ar);
};

FArchive& operator<<(FArchive& Ar, FFileSystemNodeIndex& Node);

USTRUCT()
struct FFileSystemNode {
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
};

FArchive& operator<<(FArchive& Ar, FFileSystemNode& Node);

USTRUCT()
struct FFileSystemSerializationInfo {
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, FString> Mounts;

	UPROPERTY()
	TMap<FString, FFileSystemNode> FileSystemDevices;
};
