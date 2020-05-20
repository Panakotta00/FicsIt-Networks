#pragma once

#include "File.h"
#include "Directory.h"
#include "Listener.h"
#include "WindowsFileWatcher.h"

#include <unordered_set>

namespace FileSystem {
	class FileSystemRoot;

	typedef std::function<bool(long long, bool)> SizeCheckFunc;

	class Device : virtual public ReferenceCounted {
	protected:
		ListenerList listeners;

	public:
		virtual ~Device() {}

		/*
		* Trys to open the node at the give path with the given mode
		*
		* @param[in]	path	the path to the node you want open
		* @param[in]	mode	the mode the filestream should be opened with
		* @return	the opened filestream
		*/
		virtual SRef<FileStream> open(Path path, FileMode mode) = 0;

		/*
		* Trys to create a directory at the given path
		*
		* @param[in]	path		the path to the directory you want to create
		* @param[in]	createTree	true if you want also to create the tree to the given directory if it doesnt exist
		* @return	the newly created directory
		*/
		virtual SRef<Directory> createDir(Path path, bool createTree = false) = 0;

		/*
		* removes the node at the given path
		* if recursive is set to false and node is a directory, the remove will fail
		*
		* @param[in]	path		path to the node you want to remove
		* @param[in]	recursive	true if you want to remove a folder and its content
		*/
		virtual bool remove(Path path, bool recursive = false) = 0;

		/*
		* rename the node at the given path to the new given name
		*
		* @param[in]	path	path to the node you want to rename
		* @param[in]	name	the new name of the node
		* @return	returns true if it was able to rename the node
		*/
		virtual bool rename(Path path, const NodeName& name) = 0;

		/*
		* trys to get the node at the given path
		*
		* @param[in]	path	path to the node you want to get
		* @return	node you want to get
		*/
		virtual SRef<Node> get(Path path) = 0;

		/*
		* gets the names of direct childs of the node at the given path
		*
		* @param[in]	path	path to the node you want to get the childs form
		* @return	returns a list of names
		*/
		virtual std::unordered_set<NodeName> childs(Path path) = 0;

		/*
		* Adds the given FileSystem-Listener to the listeners list.
		* The listener gets automatically removed when a event occurs and the reference is invalid.
		* Allows you to add the same listeners multiple times.
		* Also make sure the listener knows that the provided path is relative to the device, not the filesystem the device is mounted in.
		*
		* @param[in]	listener	the listener you want to add
		*/
		virtual void addListener(WRef<Listener> listener);

		/*
		* Removes the given FileSystem-Listener from the listeners list
		*
		* @param[in]	listener	the listener you want to remove
		*/
		virtual void removeListener(WRef<Listener> listener);
	};

	class ByteCountedDevice : public Device {
	private:
		class ByteCountedDeviceListener : public Listener {
		private:
			ByteCountedDevice* root;

		public:
			ByteCountedDeviceListener(ByteCountedDevice* root);

			virtual void onMounted(Path path, SRef<Device> device) override;
			virtual void onUnmounted(Path path, SRef<Device> device) override;
			virtual void onNodeAdded(Path path, NodeType type) override;
			virtual void onNodeRemoved(Path path, NodeType type) override;
			virtual void onNodeChanged(Path path, NodeType type) override;
			virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) override;
		};

		size_t used = 0;
		bool usedValid = false;
		SRef<ByteCountedDeviceListener> byteCountedDeviceListener;

	protected:
		bool checkSizeFunc(long long size, bool addIfAble);

		unsigned char listenerMask = 0xFF;
		SizeCheckFunc checkSize;

	public:
		size_t capacity;

		ByteCountedDevice(size_t capacity = 0);

		virtual size_t getSize() const = 0;

		/*
		* returns the used space
		*
		* @return	the use space
		*/
		size_t getUsed();
	};

	class MemDevice : public ByteCountedDevice {
	protected:
		SRef<MemDirectory> root;

	public:
		MemDevice(size_t capcity = 0);

		virtual size_t getSize() const override;

		virtual SRef<FileStream> open(Path path, FileMode mode) override;
		virtual SRef<Directory> createDir(Path path, bool createTree = false) override;
		virtual bool remove(Path path, bool recursive = false) override;
		virtual bool rename(Path path, const NodeName& name) override;
		virtual SRef<Node> get(Path path) override;
		virtual std::unordered_set<NodeName> childs(Path path) override;
	};

	struct DiskDeviceWatcher;

	class DiskDevice : public ByteCountedDevice {
	private:
		std::filesystem::path realPath;
		WindowsFileWatcher watcher;

	protected:
		virtual size_t getSize() const override;

	public:
		DiskDevice(std::filesystem::path realPath, size_t capacity = 0);

		virtual SRef<FileStream> open(Path path, FileMode mode) override;
		virtual SRef<Directory> createDir(Path path, bool createTree = false) override;
		virtual bool remove(Path path, bool recursive = false) override;
		virtual bool rename(Path path, const NodeName& name) override;
		virtual SRef<Node> get(Path path) override;
		virtual std::unordered_set<NodeName> childs(Path path) override;

		/*
		* calls all event changes since device creation or last call
		*/
		void tickWatcher();
	};

	class DeviceNode : public Node {
	protected:
		SRef<Device> device;

	public:
		DeviceNode(SRef<Device> device);

		virtual SRef<FileStream> open(FileMode mode) override;
		virtual std::unordered_set<NodeName> getChilds() const override;
		virtual bool isValid() const;

		/*
		* Trys to find the device node at the given path in the given filesystem and then
		* trys to mount it at the given mountpoint in the given filesystem.
		*
		* @param[in]	fs					the filesystem you want to affect
		* @param[in]	pathToDevice		the path to the device node you want to mount
		* @param[in]	pathToMountpoint	the path to the mountpoint you want to mount the device in
		*/
		static bool mount(FileSystemRoot& fs, const Path& pathToDevice, const Path& pathToMountpoint);
	};
}