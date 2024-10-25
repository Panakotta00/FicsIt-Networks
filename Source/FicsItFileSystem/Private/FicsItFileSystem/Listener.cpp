#include "FicsItFileSystem/Listener.h"

using namespace CodersFileSystem;
using namespace std;

#define ListenerLoop(listenerVarName) \
	for (const auto& i : *this) { \
		if (!i.IsValid()) { \
			continue; \
		} \
		Listener& listenerVarName = *i.Pin(); \

void ListenerList::onMounted(Path path, TSharedRef<Device> device) {
	ListenerLoop(listener)
		listener.onMounted(path, device);
	}
}

void ListenerList::onUnmounted(Path path, TSharedRef<Device> device) {
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

void ListenerListRef::onMounted(Path newPath, TSharedRef<Device> device) {
	listeners.onMounted(path / newPath, device);
}

void ListenerListRef::onUnmounted(Path newPath, TSharedRef<Device> device) {
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

PathBoundListener::PathBoundListener(TWeakPtr<Listener> listener, const Path & path) : additionalPath(path), listener(listener) {}

void PathBoundListener::onMounted(Path path, TSharedRef<Device> device) {
	listener.Pin()->onMounted(additionalPath / path, device);
}

void PathBoundListener::onUnmounted(Path path, TSharedRef<Device> device) {
	listener.Pin()->onUnmounted(additionalPath / path, device);
}

void PathBoundListener::onNodeAdded(Path path, NodeType type) {
	listener.Pin()->onNodeAdded(additionalPath / path, type);
}

void PathBoundListener::onNodeRemoved(Path path, NodeType type) {
	listener.Pin()->onNodeRemoved(additionalPath / path, type);
}

void PathBoundListener::onNodeChanged(Path path, NodeType type) {
	listener.Pin()->onNodeChanged(additionalPath / path, type);
}

void PathBoundListener::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	listener.Pin()->onNodeRenamed(additionalPath / newPath, additionalPath / oldPath, type);
}
