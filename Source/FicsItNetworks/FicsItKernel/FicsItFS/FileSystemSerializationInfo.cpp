#include "FileSystemSerializationInfo.h"

bool FFileSystemNodeIndex::Serialize(FArchive& Ar) {
	bool valid = Node.IsValid();
	Ar << valid;
	if (!Node.IsValid() && valid) Node = MakeShareable(new FFileSystemNode());
	if (valid) {
		FFileSystemNode& node = *Node;
		Ar << node;
	}
	return true;
}

FArchive& operator<<(FArchive& Ar, FFileSystemNodeIndex& Node) {
	Node.Serialize(Ar);
	return Ar;
}

bool FFileSystemNode::Serialize(FArchive& Ar) {
	Ar << Data;
	Ar << NodeType;
	Ar << ChildNodes;
	return true;
}

FArchive& operator<<(FArchive& Ar, FFileSystemNode& Node) {
	Node.Serialize(Ar);
	return Ar;
}
