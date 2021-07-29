#include "FileSystemSerializationInfo.h"
#include "FicsItNetworks/FicsItNetworksModule.h"

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

CodersFileSystem::SRef<CodersFileSystem::Node> FFileSystemNodeIndex::Deserialize(FString name, CodersFileSystem::SRef<CodersFileSystem::Directory> parent) const {
	const std::string nodeName = TCHAR_TO_UTF8(*name);
	switch (Node->NodeType) {
	case 0: {
		CodersFileSystem::SRef<CodersFileSystem::File> file = parent->createFile(nodeName);
		if (!file) return nullptr;
		CodersFileSystem::SRef<CodersFileSystem::FileStream> stream = file->open(CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC | CodersFileSystem::BINARY);
		if (!stream) return nullptr;
		try {
			FTCHARToUTF8 Convert(*Node->Data, Node->Data.Len());
			stream->write(std::string(Convert.Get(), Convert.Length()));
			stream->close();
		} catch (...) {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Unable to deserialize VFS-File"));
			return nullptr;
		}
		return file;
	}
	case 1: {
		CodersFileSystem::SRef<CodersFileSystem::Directory> dir = parent->createSubdir(nodeName);
		if (!dir) return nullptr;
		for (TPair<FString, FFileSystemNodeIndex>& child : Node->ChildNodes) {
			CodersFileSystem::SRef<CodersFileSystem::Node> node = child.Value.Deserialize(child.Key, dir);
		}
		return dir;
	}
	default: {
		return nullptr;
	}
	}
}

FFileSystemNode& FFileSystemNode::Serialize(CodersFileSystem::SRef<CodersFileSystem::Device> device, const CodersFileSystem::Path& path) {
	const CodersFileSystem::SRef<CodersFileSystem::Node> node = device->get(path);
	if (CodersFileSystem::SRef<CodersFileSystem::File> file = node) {
		NodeType = 0;
		CodersFileSystem::SRef<CodersFileSystem::FileStream> stream = file->open(CodersFileSystem::INPUT | CodersFileSystem::BINARY);
		std::string str = CodersFileSystem::FileStream::readAll(stream);
		FUTF8ToTCHAR Convert(str.c_str(), str.length());
		Data = FString(Convert.Length(), Convert.Get());
	} else if (CodersFileSystem::SRef<CodersFileSystem::Directory> dir = node) {
		NodeType = 1;
		for (std::string child : dir->getChilds()) {
			TSharedPtr<FFileSystemNode> newNode = MakeShareable(new FFileSystemNode());
			newNode->Serialize(device, path / child);
			ChildNodes.Add(child.c_str(), newNode);
		}
	}

	return *this;
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

FFileSystemNode& FFileSystemNode::Deserialize(CodersFileSystem::SRef<CodersFileSystem::Device> Device, const std::string& deviceName) {
	if (NodeType == 2 || NodeType == 3) {
		// check the device we should deserialize
		if (!Device.isValid()) throw std::exception(("unable to find device to unpersist '" + deviceName + "'").c_str());
		// delete previously existing contents
		if (NodeType == 2) {
			for (std::string child : Device->childs("/")) {
				CodersFileSystem::Path childPath = "/";
				childPath = childPath / child;
				Device->remove(childPath, true);
			}
		}
		// deserialize children
		CodersFileSystem::SRef<CodersFileSystem::Directory> root = Device->get("/");
		if (!root.isValid()) throw std::exception(("root of device '" + deviceName + "' can not be found").c_str());
		for (TPair<FString, FFileSystemNodeIndex>& child : ChildNodes) {
			CodersFileSystem::SRef<CodersFileSystem::FileStream> node = child.Value.Deserialize(child.Key, root);
		}
	}

	return *this;
}

FArchive& operator<<(FArchive& Ar, FFileSystemNode& Node) {
	Node.Serialize(Ar);
	return Ar;
}

bool FFileSystemSerializationInfo::Serialize(FArchive& Ar) {
	Ar << Mounts;
	Ar << Devices;
	return true;
}

FArchive& operator<<(FArchive& Ar, FFileSystemSerializationInfo& Info) {
	Info.Serialize(Ar);
	return Ar;
}
