#pragma once

#include "Path.h"
#include "ReferenceCount.h"

#include <unordered_set>
#include <unordered_map>

namespace CodersFileSystem {
	class FileStream;
	enum FileMode;

	class Node : virtual public ReferenceCounted {
	public:
		Node();
		virtual ~Node();

		/*
		* returns a list of direct childs of this node.
		*
		* @return	the list of direct childs
		*/
		virtual std::unordered_set<NodeName> getChilds() const = 0;

		/*
		* trys to open the node. Returns the Ref to the FileStream for IO which is now in charge of managing the files content.
		*
		* @param[in]	the open mod with which the file should get opened
		* @return	returns the created and opened filestream, nullptr if not able to open the node
		*/
		virtual SRef<FileStream> open(FileMode mode) = 0;

		/*
		* checks if the node is still valid for use
		* primary use for node cache
		*
		* @return	returns true if node is still valid
		*/
		virtual bool isValid() const = 0;
	};
}