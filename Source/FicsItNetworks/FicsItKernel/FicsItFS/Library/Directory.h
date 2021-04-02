#pragma once

#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "File.h"

namespace CodersFileSystem {
	typedef std::function<bool(long long, bool)> SizeCheckFunc;

	class Directory : public Node {
	public:
		Directory();
		virtual ~Directory();

		/*
		* Creates a subdirectory with the given name in itself. If the dir already exists, it works like getChild.
		*
		* @param[in]	subdir	name of the subdir you want to create
		* @return	Returns the created Directory. Nullptr if it was not able to create the directory.
		*/
		virtual WRef<Directory> createSubdir(const NodeName& subdir) = 0;

		/*
		* Creates a file with the given name in itself.
		*
		* @param[in]	name	name of the new file
		* @return	Returns the created file. Nullptr if it was not able to create the file.
		*/
		virtual WRef<File> createFile(const NodeName& name) = 0;

		/*
		* Removes the entry in the directory with the given name.
		* Is not able to remove entries with childs if recursive is not set.
		*
		* @param[in]	entry		name of the entry you want to remove
		* @param[in]	recursive	true if you want to remove a directory
		* @return	returns	true if it was able to remove the entry
		*/
		virtual bool remove(const NodeName& entry, bool recursive) = 0;

		/*
		* Trys to rename the entry with the given name to the given new name
		*
		* @param[in]	entry	name of the entry you want to rename
		* @param[in]	name	the new name for the entry
		* @return	returns true if it was able to rename the entry
		*/
		virtual bool rename(const NodeName& entry, const NodeName& name) = 0;
	};

	class MemDirectory : public Directory {
	protected:
		std::unordered_map<NodeName, SRef<Node>> entries;
		ListenerListRef listeners;
		SizeCheckFunc checkSize;

	public:
		MemDirectory(ListenerListRef listeners, SizeCheckFunc checkSize);
		virtual ~MemDirectory();
		
		/* Begin Directory-Interface-Implementation */
		virtual std::unordered_set<NodeName> getChilds() const override;
		virtual SRef<FileStream> open(FileMode mode) override;
		virtual bool isValid() const override;
		
		virtual WRef<Directory> createSubdir(const NodeName& subdir) override;
		virtual WRef<File> createFile(const NodeName& name) override;
		virtual bool remove(const NodeName& subdir, bool recursive) override;
		virtual bool rename(const NodeName& entry, const NodeName& name) override;
		/* End Directory-Interface-Implementation */

		/*
		* trys to find and return the entry with the given entry
		*
		* @param[in]	name	the name of the entry you want to find
		* @return	returns the found entry, nullptr if not found
		*/
		SRef<Node> get(const NodeName& name);

		/*
		* Adds any given node to the directory tree.
		* As entry name the given nodes name is used.
		* Fails to add if the nodes name is already used in the directory tree.
		* 
		* @param[in]	node	node wich should get added to the directory tree
		* @return	returns true if it was able to add the node to the directory tree
		*/
		bool add(const SRef<Node>& node, const NodeName& name);
	};

	class DiskDirectory : public Directory {
	protected:
		std::filesystem::path realPath;
		SizeCheckFunc checkSize;

		/* Begin Directory-Interface-Implementation */
		virtual std::unordered_set<NodeName> getChilds() const override;
		virtual SRef<FileStream> open(FileMode mode) override;
		virtual bool isValid() const override;

		virtual WRef<Directory> createSubdir(const NodeName& subdir) override;
		virtual WRef<File> createFile(const NodeName& name) override;
		virtual bool remove(const NodeName& subdir, bool recursive) override;
		virtual bool rename(const NodeName& entry, const NodeName& name) override;
		/* End Directory-Interface-Implementation */

	public:
		DiskDirectory(const std::filesystem::path& realpath, SizeCheckFunc checkSize);
		virtual ~DiskDirectory();
	};
}