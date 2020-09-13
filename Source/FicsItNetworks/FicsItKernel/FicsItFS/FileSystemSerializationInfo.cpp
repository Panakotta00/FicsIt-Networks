#include "FileSystemSerializationInfo.h"

#include "util/Logging.h"

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

FileSystem::SRef<FileSystem::Node> FFileSystemNodeIndex::Deserialize(FString name, FileSystem::SRef<FileSystem::Directory> parent) const {
	const std::string nodeName = TCHAR_TO_UTF8(*name);
	switch (Node->NodeType) {
	case 0: {
		FileSystem::SRef<FileSystem::File> file = parent->createFile(nodeName);
		FileSystem::SRef<FileSystem::FileStream> stream = file->open(FileSystem::OUTPUT | FileSystem::TRUNC);
		try {
			stream->write(std::string(TCHAR_TO_UTF8(*Node->Data), Node->Data.Len()));
			stream->flush();
			stream->close();
		} catch (...) {
			SML::Logging::error("Unable to deserialize VFS-File");
		}
		return file;
	}
	case 1: {
		FileSystem::SRef<FileSystem::Directory> dir = parent->createSubdir(nodeName);
		for (TPair<FString, FFileSystemNodeIndex>& child : Node->ChildNodes) {
			FileSystem::SRef<FileSystem::Node> node = child.Value.Deserialize(child.Key, dir);
		}
		return dir;
	}
	default: {
		return nullptr;
	}
	}
}

FFileSystemNode& FFileSystemNode::Serialize(FileSystem::SRef<FileSystem::Device> device, const FileSystem::Path& path) {
	const FileSystem::SRef<FileSystem::Node> node = device->get(path);
	if (FileSystem::SRef<FileSystem::File> file = node) {
		NodeType = 0;
		FileSystem::SRef<FileSystem::FileStream> stream = file->open(FileSystem::INPUT);
		std::string str = stream->readAll();
		auto Convert = FUTF8ToTCHAR(str.c_str(), str.length());
		Data = FString(Convert.Get(), Convert.Length());
	} else if (FileSystem::SRef<FileSystem::Directory> dir = node) {
		NodeType = 1;
		for (FileSystem::NodeName child : dir->getChilds()) {
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

FFileSystemNode& FFileSystemNode::Deserialize(FileSystem::SRef<FileSystem::Device> Device, const std::string& deviceName) {
	if (NodeType == 2 || NodeType == 3) {
		// check the device we should deserialize
		if (!Device.isValid()) throw std::exception(("unable to find device to unpersist '" + deviceName + "'").c_str());
		// delete previously existing contents
		if (NodeType == 2) {
			for (FileSystem::NodeName child : Device->childs("/")) {
				FileSystem::Path childPath = "/";
				childPath = childPath / child;
				Device->remove(childPath, true);
			}
		}
		// deserialize children
		FileSystem::SRef<FileSystem::Directory> root = Device->get("/");
		if (!root.isValid()) throw std::exception(("root of device '" + deviceName + "' can not be found").c_str());
		for (TPair<FString, FFileSystemNodeIndex>& child : ChildNodes) {
			FileSystem::SRef<FileSystem::FileStream> node = child.Value.Deserialize(child.Key, root);
		}
	}

	return *this;
}

FArchive& operator<<(FArchive& Ar, FFileSystemNode& Node) {
	Node.Serialize(Ar);
	return Ar;
}
