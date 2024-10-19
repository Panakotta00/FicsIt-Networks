#pragma once

#include <regex>
#include <string>

namespace CodersFileSystem {
	class FICSITNETWORKSCOMPUTER_API Path {
	private:
		static std::regex sepperatorPattern;
		static std::regex nodePattern;
		std::string path;
	
	public:
		static bool isNode(const std::string& str) {
			return std::regex_match(str, nodePattern);
		}
		
		Path() = default;
		Path(const char* path) : Path(std::string(path)) {}
		Path(std::string str) {
			if (str.size() < 1) return;
			if (str[0] == '/') path = "/";
			std::smatch match;
			while (std::regex_search(str, match, sepperatorPattern)) {
				if (0 != match.position()) {
					std::string node = str.substr(0, match.position());
					append(node);
				} else {
					path = "/";
				}
				str = match.suffix();
			}
			if (str.length() < 1 && path.size() > 0 && path[path.size()-1] != '/') path += "/";
			else append(str);
		}

		Path& append(std::string node) {
			size_t pos = path.find_last_of("/");
			if (node == ".") {
				if (pos != path.size()-1) path.append("/");
				path.append(node);
			} else if (node == "..") {
				if (pos != path.size()-1) path.append("/");
				path.append(node);
			} else if (node == "/") {
				path.append(node);
			} else if (isNode(node)) {
				if (pos != path.size()-1) path.append("/");
				path.append(node);
			}
			return *this;
		}
		
		std::string getRoot() const {
			std::string str = relative();
			size_t slash = str.find_first_of("/");
			return str.substr(0, slash);
		}
		
		bool isSingle() const {
			size_t pos = path.find('/', 1);
			return (pos == std::string::npos && path.size() > 0 && path != "/");
		}

		bool isAbsolute() const {
			return path.substr(0, 1) == "/";
		}

		bool isEmpty() const {
			return (path.size() == 1 && isAbsolute()) || path.size() == 0;
		}

		bool isRoot() const {
			return path == "/";
		}

		bool isDir() const {
			return path[path.size()-1] == '/';
		}
		
		bool startsWith(const Path& other) const {
			Path o = isAbsolute() ? other.absolute() : other.relative();
			return path.substr(0, o.path.size()) == o.str();
		}

		Path removeFrontNodes(size_t count) const {
			size_t pos = 0;
			for (int i = 0; i < count; ++i) {
				pos = path.find('/', pos);
				if (pos == std::string::npos) return "";
				pos = pos + 1;
			}
			return path.substr(pos);
		}

		std::string fileName() const {
			size_t slash = path.find_last_of("/");
			if (slash == std::string::npos) return path;
			return path.substr(slash+1);
		}

		std::string fileExtension() const {
			std::string name = fileName();
			size_t pos = name.find_last_of(".");
			if (pos == std::string::npos || pos == 0) return "";
			return name.substr(pos);
		}

		std::string fileStem() const {
			std::string name = fileName();
			size_t pos = name.find_last_of(".");
			return name.substr(0, pos < 1 ? std::string::npos : pos);
		}

		Path normalize() const {
			Path newPath;
			if (isAbsolute()) newPath.path = "/";
			size_t posEnd, posStart = 0;
			do {
				posEnd = path.find_first_of("/", posStart);
				std::string node = path.substr(posStart, posEnd-posStart);
				posStart = posEnd+1;
				if (node == ".") {
				} else if (node == "..") {
					size_t pos = newPath.path.find_last_of("/");
					if (pos == std::string::npos) {
						newPath.path = "";
					} else {
						newPath.path.erase(pos);
					}
					if (newPath.path.size() < 1 && isAbsolute()) newPath.path = "/";
				} else if (isNode(node)) {
					if (newPath.path.size() > 0 && newPath.path.back() != '/') {
						newPath.path.append("/");
					}
					newPath.path.append(node);
				}
			} while (posEnd != std::string::npos);
			return newPath;
		}
		
		Path absolute() const {
			if (isAbsolute()) return normalize();
			return "/" + normalize().path;
		}
		
		Path relative() const {
			if (isAbsolute()) return normalize().path.substr(1);
			return normalize();
		}
		
		Path operator/(const Path& other) const {
			if (other.isAbsolute()) return other;
			Path newPath = *this;
			size_t posEnd, posStart = 0;
			do {
				posEnd = other.path.find_first_of("/", posStart);
				newPath.append(other.path.substr(posStart, posEnd-posStart));
				posStart = posEnd+1;
			} while (posEnd != std::string::npos);
			return newPath;
		}

		std::string str() const {
			return path;
		}

		bool operator==(const Path& other) const {
			return relative().str() == other.relative().str();
		}

		bool operator<(const Path& other) const {
			return str() < other.str();
		}
		
		operator std::string() const {
			return path;
		}
	};
}
