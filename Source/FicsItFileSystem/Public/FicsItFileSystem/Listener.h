#pragma once

#include "Path.h"
#include "SharedPointer.h"

#include <unordered_set>

namespace CodersFileSystem {
	class Device;

	enum NodeType {
		NT_File,
		NT_Directory,
		NT_Else
	};

	class Listener {
	public:
		virtual ~Listener() {}
		virtual void onMounted(Path path, TSharedRef<Device> device) {};
		virtual void onUnmounted(Path path, TSharedRef<Device> device) {};
		virtual void onNodeAdded(Path path, NodeType type) {};
		virtual void onNodeRemoved(Path path, NodeType type) {};
		virtual void onNodeChanged(Path  path, NodeType type) {};
		virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) {};
	};

	class PathBoundListener : public Listener {
	protected:
		Path additionalPath;
		TWeakPtr<Listener> listener;

	public:
		PathBoundListener(TWeakPtr<Listener> listener, const Path& path);

		virtual void onMounted(Path path, TSharedRef<Device> device) override;
		virtual void onUnmounted(Path path, TSharedRef<Device> device) override;
		virtual void onNodeAdded(Path path, NodeType type) override;
		virtual void onNodeRemoved(Path path, NodeType type) override;
		virtual void onNodeChanged(Path path, NodeType type) override;
		virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) override;
	};


	class ListenerList : public TArray<TWeakPtr<Listener>> {
	public:
		void onMounted(Path path, TSharedRef<Device> device);
		void onUnmounted(Path path, TSharedRef<Device> device);
		void onNodeAdded(Path path, NodeType type);
		void onNodeRemoved(Path path, NodeType type);
		void onNodeChanged(Path  path, NodeType type);
		void onNodeRenamed(Path newPath, Path oldPath, NodeType type);
	};

	class ListenerListRef {
	public:
		ListenerList& listeners;
		Path path;

		ListenerListRef(ListenerList& listeners, const Path& path);
		ListenerListRef(ListenerListRef& listenersRef, const Path& additionalPath);

		void onMounted(Path path, TSharedRef<Device> device);
		void onUnmounted(Path path, TSharedRef<Device> device);
		void onNodeAdded(Path path, NodeType type);
		void onNodeRemoved(Path path, NodeType type);
		void onNodeChanged(Path path, NodeType type);
		void onNodeRenamed(Path newPath, Path oldPath, NodeType type);
	};
}
