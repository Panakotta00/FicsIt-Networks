#pragma once

#include <map>
#include <unordered_set>

#include "Directory.h"
#include "Device.h"

namespace CodersFileSystem {
	class FICSITNETWORKSCOMPUTER_API FileSystemException : public std::exception {
	public:
		FileSystemException(std::string what);
	};

	class FICSITNETWORKSCOMPUTER_API FileSystemRoot {
	protected:
		class RootListener : public Listener {
			friend FileSystemRoot;

		protected:
			FileSystemRoot* root;

		public:
			RootListener(FileSystemRoot* root);
			virtual ~RootListener();

			virtual void onMounted(Path path, SRef<Device> device) override;
			virtual void onUnmounted(Path path, SRef<Device> device) override;
			virtual void onNodeAdded(Path path, NodeType type) override;
			virtual void onNodeRemoved(Path path, NodeType type) override;
			virtual void onNodeChanged(Path path, NodeType type) override;
			virtual void onNodeRenamed(Path newPath, Path oldPath, NodeType type) override;
		};

		std::map<Path, std::pair<WRef<Device>, SRef<PathBoundListener>>> mounts;
		std::map<Path, SRef<Node>> cache;
		ListenerList listeners;
		SRef<RootListener> listener;

		/*
		* gets the device managing the given path based on mounts
		*
		* @param[in]	path	the path you want to get the device from
		* @param[out]	pending	the given path with the device path cutoff
		* @return	the device at the path
		*/
		SRef<Device> getDevice(Path path, Path& pending);

		int moveInternal(Path from, Path to);

	public:
		FileSystemRoot();
		FileSystemRoot(const FileSystemRoot&) = delete;
		FileSystemRoot(FileSystemRoot&& other);

		virtual ~FileSystemRoot();

		FileSystemRoot& operator=(const FileSystemRoot&) = delete;
		FileSystemRoot& operator=(FileSystemRoot&& other);

		/*
		* Trys to open the node at the give path with the given mode
		* 
		* @param[in]	path	the path to the node you want open
		* @param[in]	mode	the mode the filestream should be opened with
		* @return	the opened filestream
		*/
		virtual SRef<FileStream> open(Path path, FileMode mode);

		/*
		* Trys to create a directory at the given path
		*
		* @param[in]	path		the path to the directory you want to create
		* @param[in]	createTree	true if you want also to create the tree to the given directory if it doesnt exist
		* @return	the newly created directory
		*/
		virtual SRef<Directory> createDir(Path path, bool createTree = false);

		/*
		* removes the node at the given path
		* if recursive is set to false and node is a directory, the remove will fail
		*
		* @param[in]	path		path to the node you want to remove
		* @param[in]	recursive	true if you want to remove a folder and its content
		*/
		virtual bool remove(Path path, bool recursive = false);

		/*
		* rename the node at the given path to the new given name
		*
		* @param[in]	path	path to the node you want to rename
		* @param[in]	name	the new name of the node
		* @return	returns true if it was able to rename the node
		*/
		virtual bool rename(Path path, const std::string& name);

		/*
		* copies the from node to the to node
		*
		* @param[in]	from		path to the node you want to copy from
		* @param[in]	in			path to the node you want to copy to
		* @param[in]	recursive	true if you want to copy a folder and its content
		* @return	0 if copy worked, 1 if was fully not able to copy and 2 if it was able to partially copy
		*/
		virtual int copy(Path from, Path to, bool recursive = false);

		/*
		* moves the from node to the to node
		* basicaly copies from to to and removes from afterwards
		*
		* @param[in]	from		path to the node you want to move to somewere else
		* @param[in]	to			path to the node you want the from node into
		* @param[in]	recursive	true if you want to move a folder and its content
		* @return	0 if move worked, 1 if was filly not able to move and 2 if it was able to move partially
		*/
		virtual int move(Path from, Path to);

		/*
		* trys to get the node at the given path
		*
		* @param[in]	path	path to the node you want to get
		* @return	node you want to get
		*/
		virtual SRef<Node> get(Path path);

		/*
		* gets the names of direct childs of the node at the given path
		* 
		* @param[in]	path	path to the node you want to get the childs form
		* @return	returns a list of names
		*/
		virtual std::unordered_set<std::string> childs(Path path);

		/*
		* trys to mount the given device at teh given path
		*
		* @param[in]	device	the device you want to mount
		* @param[in]	path	the path were the device should get mounted to
		* @return	returns true if it was able to mount the device
		*/
		virtual bool mount(SRef<Device> device, Path path);

		/*
		* trys to umount the given mountpoint
		* if it was able to unmount the given mountpoint, also removes all listeners from the underlying device
		*
		* @param[in]	path	path tto the mointpoint
		* @return	returns true if it was able to unmount the mountpoint
		*/
		virtual bool unmount(Path path);

		/*
		* adds a new listener to the filesystem.
		* allows you to add the same listener multiple times.
		* The listener gets automatically removed when a event occurs and the reference is invalid .
		*
		* @param[in]	listener	listener you want to add
		*/
		virtual void addListener(WRef<Listener> listener);

		/*
		* removes the given listener from the filesystem.
		*
		* @param[in]	listener	listener you want to remove
		*/
		virtual void removeListener(WRef<Listener> listener);
	};
}