#pragma once

#include <string>
#include <vector>

#include "FileSystem.h"
#include "NodeName.h"

namespace CodersFileSystem {
	class Path {
	private:
		std::vector<NodeName> path;

	protected:
		Path(std::vector<NodeName> path, bool absolute);

	public:
		bool absolute = false;

		Path();
		Path(const char* path);
		explicit Path(std::string path);
		explicit Path(NodeName node);
		Path(std::filesystem::path path);

		std::string getRoot() const;
		bool isFinal() const;
		bool startsWith(const Path& other) const;
		Path next() const;
		Path prev() const;
		std::string str(bool noAbsolute = false) const;
		size_t getNodeCount() const;
		Path removeFrontNodes(size_t count) const;
		std::string getFinal() const;

		bool operator==(const Path& other) const;
		bool operator<(const Path& other) const;
		Path operator/(const Path& other) const;
		Path operator/(const NodeName& node) const;
		Path& operator=(const Path& other);

		operator std::filesystem::path() const;
	};
}
