#pragma once

#include "Path.h"
#include "ReferenceCount.h"

#include <unordered_set>

namespace FileSystem {
	class Node;
	class Device;

	enum NodeType {
		NT_File,
		NT_Directory,
		NT_Else
	};

	extern NodeType getTypeFromRef(SRef<Node> node);

	class Listener : virtual public ReferenceCounted {
	public:
		virtual ~Listener() {}
		virtual void onMounted(Path path, SRef<Device> device) {};
		virtual void onUnmounted(Path path, SRef<Device> device) {};
		virtual void onNodeAdded(Path path, NodeType type) {};
		virtual void onNodeRemoved(Path path, NodeType type) {};
		virtual void onNodeChanged(Path  path, NodeType type) {};
		virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) {};
	};

	class PathBoundListener : public Listener {
	protected:
		Path additionalPath;
		WRef<Listener> listener;

	public:
		PathBoundListener(WRef<Listener> listener, const Path& path);

		virtual void onMounted(Path path, SRef<Device> device) override;
		virtual void onUnmounted(Path path, SRef<Device> device) override;
		virtual void onNodeAdded(Path path, NodeType type) override;
		virtual void onNodeRemoved(Path path, NodeType type) override;
		virtual void onNodeChanged(Path path, NodeType type) override;
		virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) override;
	};


	class ListenerList : public std::unordered_set<WRef<Listener>> {
	public:
		void onMounted(Path path, SRef<Device> device);
		void onUnmounted(Path path, SRef<Device> device);
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

		void onMounted(Path path, SRef<Device> device);
		void onUnmounted(Path path, SRef<Device> device);
		void onNodeAdded(Path path, NodeType type);
		void onNodeRemoved(Path path, NodeType type);
		void onNodeChanged(Path path, NodeType type);
		void onNodeRenamed(Path newPath, Path oldPath, NodeType type);
	};
}