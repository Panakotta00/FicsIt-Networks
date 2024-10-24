#pragma once

#include "File.h"
#include "Listener.h"
#include "FileWatcher.h"

#include <unordered_set>
#include <cstdint>

#include "SharedPointer.h"

namespace CodersFileSystem {
	class FileSystemRoot;

	typedef std::function<bool(long long, bool)> SizeCheckFunc;

	class FICSITFILESYSTEM_API Device {
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
		virtual TSharedPtr<FileStream> open(Path path, FileMode mode) = 0;

		/*
		* Trys to create a directory at the given path
		*
		* @param[in]	path		the path to the directory you want to create
		* @param[in]	createTree	true if you want also to create the tree to the given directory if it doesnt exist
		* @return	the newly created directory
		*/
		virtual bool createDir(Path path, bool createTree = false) = 0;

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
		virtual bool rename(Path path, const std::string& name) = 0;

		/*
		* gets the names of direct childs of the node at the given path
		*
		* @param[in]	path	path to the node you want to get the childs form
		* @return	returns a list of names
		*/
		virtual std::unordered_set<std::string> children(Path path) = 0;

		/*
		 * Returns the type of a file
		 */
		virtual TOptional<FileType> fileType(Path path) = 0;

		virtual TSharedPtr<Device> getDevice(Path path) { return nullptr; }

		/*
		* Adds the given FileSystem-Listener to the listeners list.
		* The listener gets automatically removed when a event occurs and the reference is invalid.
		* Allows you to add the same listeners multiple times.
		* Also make sure the listener knows that the provided path is relative to the device, not the filesystem the device is mounted in.
		*
		* @param[in]	listener	the listener you want to add
		*/
		virtual void addListener(TWeakPtr<Listener> listener);

		/*
		* Removes the given FileSystem-Listener from the listeners list
		*
		* @param[in]	listener	the listener you want to remove
		*/
		virtual void removeListener(TWeakPtr<Listener> listener);

		virtual void tickListeners() {}
	};

	class FICSITFILESYSTEM_API ByteCountedDevice : public Device {
	private:
		class ByteCountedDeviceListener : public Listener {
		private:
			ByteCountedDevice* root;

		public:
			ByteCountedDeviceListener(ByteCountedDevice* root);

			virtual void onMounted(Path path, TSharedRef<Device> device) override;
			virtual void onUnmounted(Path path, TSharedRef<Device> device) override;
			virtual void onNodeAdded(Path path, NodeType type) override;
			virtual void onNodeRemoved(Path path, NodeType type) override;
			virtual void onNodeChanged(Path path, NodeType type) override;
			virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) override;
		};

		size_t used = 0;
		bool usedValid = false;
		TSharedPtr<ByteCountedDeviceListener> byteCountedDeviceListener;

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

	struct DiskDeviceWatcher;

	class FICSITFILESYSTEM_API DiskDevice : public ByteCountedDevice {
	private:
		std::filesystem::path realPath;
		FileWatcher watcher;

	protected:
		virtual size_t getSize() const override;

	public:
		DiskDevice(std::filesystem::path realPath, size_t capacity = 0);

		virtual TSharedPtr<FileStream> open(Path path, FileMode mode) override;
		virtual bool createDir(Path path, bool createTree = false) override;
		virtual bool remove(Path path, bool recursive = false) override;
		virtual bool rename(Path path, const std::string& name) override;
		virtual std::unordered_set<std::string> children(Path path) override;
		virtual TOptional<FileType> fileType(Path path) override;
		virtual void tickListeners() override;

		/**
		 * Gets the real path mapped to this disk device.
		 *
		 * @return the real path mapped
		 */
		std::filesystem::path getRealPath() const;
	};
}
