#include "FicsItKernel/FicsItFS/Library/Listener.h"

using namespace CodersFileSystem;
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

void CodersFileSystem::ListenerList::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	ListenerLoop(listener)
		listener.onNodeRenamed(newPath, oldPath, type);
	}
}

ListenerListRef::ListenerListRef(ListenerList & listeners, const Path & path) : listeners(listeners), path(path) {}

CodersFileSystem::ListenerListRef::ListenerListRef(ListenerListRef & listenersRef, const Path & path) : listeners(listenersRef.listeners), path(listenersRef.path / path) {}

void ListenerListRef::onMounted(Path newPath, SRef<Device> device) {
	listeners.onMounted(path / newPath, device);
}

void ListenerListRef::onUnmounted(Path newPath, SRef<Device> device) {
	listeners.onUnmounted(path / newPath, device);
}

void ListenerListRef::onNodeAdded(Path newPath, NodeType type) {
	listeners.onNodeAdded(path / newPath, type);
}

void ListenerListRef::onNodeRemoved(Path newPath, NodeType type) {
	listeners.onNodeRemoved(path / newPath, type);
}

void ListenerListRef::onNodeChanged(Path newPath, NodeType type) {
	listeners.onNodeChanged(path / newPath, type);
}

void CodersFileSystem::ListenerListRef::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	listeners.onNodeRenamed(path / newPath, path / oldPath, type);
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

NodeType CodersFileSystem::getTypeFromRef(SRef<Node> node) {
	auto p = node.get();
	if (dynamic_cast<Directory*>(p)) return NT_Directory;
	else if (dynamic_cast<File*>(p)) return NT_File;
	else return NT_Else;
}
