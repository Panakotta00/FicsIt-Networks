#include "Listener.h"

#include "Device.h"

using namespace FileSystem;
using namespace std;

#define ListenerLoop(listenerVarName) \
	for (auto i = begin(); i != end(); i++) { \
		if (!i->isValid()) { \
			erase(i--); \
			continue; \
		} \
		Listener& listenerVarName = *i->get(); \

void ListenerList::onMounted(Path path, SRef<Device> device) {
	ListenerLoop(listener)
		listener.onMounted(path, device);
}
}

void ListenerList::onUnmounted(Path path, SRef<Device> device) {
	ListenerLoop(listener)
		listener.onUnmounted(path, device);
	}
}

void ListenerList::onNodeAdded(Path path, NodeType type) {
	ListenerLoop(listener)
		listener.onNodeAdded(path, type);
	}
}

void ListenerList::onNodeRemoved(Path path, NodeType type) {
	ListenerLoop(listener)
		listener.onNodeRemoved(path, type);
	}
}

void ListenerList::onNodeChanged(Path path, NodeType type) {
	ListenerLoop(listener)
		listener.onNodeChanged(path, type);
	}
}

void FileSystem::ListenerList::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	ListenerLoop(listener)
		listener.onNodeRenamed(newPath, oldPath, type);
	}
}

ListenerListRef::ListenerListRef(ListenerList & listeners, const Path & path) : listeners(listeners), path(path) {}

FileSystem::ListenerListRef::ListenerListRef(ListenerListRef & listenersRef, const Path & path) : listeners(listenersRef.listeners), path(listenersRef.path / path) {}

void ListenerListRef::onMounted(Path path, SRef<Device> device) {
	listeners.onMounted(this->path / path, device);
}

void ListenerListRef::onUnmounted(Path path, SRef<Device> device) {
	listeners.onUnmounted(this->path / path, device);
}

void ListenerListRef::onNodeAdded(Path path, NodeType type) {
	listeners.onNodeAdded(this->path / path, type);
}

void ListenerListRef::onNodeRemoved(Path path, NodeType type) {
	listeners.onNodeRemoved(this->path / path, type);
}

void ListenerListRef::onNodeChanged(Path path, NodeType type) {
	listeners.onNodeChanged(this->path / path, type);
}

void FileSystem::ListenerListRef::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	listeners.onNodeRenamed(this->path / newPath, this->path / oldPath, type);
}

PathBoundListener::PathBoundListener(WRef<Listener> listener, const Path & path) : additionalPath(path), listener(listener) {}

void PathBoundListener::onMounted(Path path, SRef<Device> device) {
	listener->onMounted(additionalPath / path, device);
}

void PathBoundListener::onUnmounted(Path path, SRef<Device> device) {
	listener->onUnmounted(additionalPath / path, device);
}

void PathBoundListener::onNodeAdded(Path path, NodeType type) {
	listener->onNodeAdded(additionalPath / path, type);
}

void PathBoundListener::onNodeRemoved(Path path, NodeType type) {
	listener->onNodeRemoved(additionalPath / path, type);
}

void PathBoundListener::onNodeChanged(Path path, NodeType type) {
	listener->onNodeChanged(additionalPath / path, type);
}

void PathBoundListener::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	listener->onNodeRenamed(additionalPath / newPath, additionalPath / oldPath, type);
}

NodeType FileSystem::getTypeFromRef(SRef<Node> node) {
	auto p = node.get();
	if (dynamic_cast<Directory*>(p)) return NT_Directory;
	else if (dynamic_cast<File*>(p)) return NT_File;
	else return NT_Else;
}
