#include "Path.h"

using namespace std;
using namespace FileSystem;

Path::Path(std::vector<NodeName> path, bool absolute) : path(path), absolute(absolute) {}

Path::Path(std::filesystem::path path) : Path(path.string()) {}

Path::Path() {}

Path::Path(const char * p) : Path((std::string)p) {}

Path::Path(string oPath) {
	if (oPath.length() < 1) return;
	if (oPath[0] == '/' || oPath[0] == '\\') {
		absolute = true;
		oPath.erase(0, 1);
	}
	if (oPath.length() > 0) do {
		size_t sp = oPath.find("/");
		size_t sp2 = oPath.find("\\");
		if (sp2 != string::npos && (sp == string::npos || sp2 < sp)) sp = sp2;
		string s;
		if (sp != string::npos) {
			s = oPath.substr(0, sp);
			oPath = oPath.substr(sp + 1);
		} else {
			s = oPath;
			oPath = "";
		}
		if (s == "..") {
			if (path.size() > 0) path.pop_back();
		} else if (s != ".") path.push_back(s);
	} while (oPath.length() > 0);
}

Path::Path(NodeName node) {
	path.push_back(node);
}

string Path::getRoot() const {
	if (path.size() > 0) return path[0];
	else if (absolute) return "";
	else return "";
}

bool Path::isFinal() const {
	return path.size() <= 1;
}

bool FileSystem::Path::startsWith(const Path & other) const {
	if (path.size() < other.path.size()) return false;
	for (int i = 0; i < other.path.size(); ++i) if (other.path[i] != path[i]) return false;
	return true;
}

Path Path::next() const {
	auto nPath = path;
	if (nPath.size() > 0) nPath.erase(nPath.begin());
	return Path(nPath, absolute);
}

Path FileSystem::Path::prev() const {
	auto nPath = path;
	if (nPath.size() > 0) nPath.erase(nPath.end() - 1);
	return Path(nPath, absolute);
}

std::string FileSystem::Path::str() const {
	std::string p = (absolute) ? "/" : "";
	for (auto n : path) {
		p += n + "/";
	}
	if (path.size() > 0) p.erase(p.end() - 1);
	return p;
}

size_t FileSystem::Path::getNodeCount() const {
	return path.size();
}

Path FileSystem::Path::removeFrontNodes(size_t count) const {
	Path p = *this;
	if (p.path.size() < count) count = p.path.size();
	p.path.erase(p.path.begin(), p.path.begin() + count);
	return p;
}

string Path::getFinal() const {
	if (path.size() < 1) return "";
	return path[path.size()-1];
}

bool FileSystem::Path::operator==(const Path & other) const {
	if (other.absolute != absolute || other.path.size() != path.size()) return false;
	for (int i = 0; i < other.path.size(); ++i) if (other.path[i] != path[i]) return false;
	return true;
}

bool FileSystem::Path::operator<(const Path & other) const {
	return path < other.path;
}

Path FileSystem::Path::operator/(const Path & other) const {
	Path np(path, absolute);
	for (auto& n : other.path) {
		np.path.push_back(n);
	}
	return np;
}

Path & FileSystem::Path::operator=(const Path & other) {
	absolute = other.absolute;
	path = other.path;
	return *this;
}

FileSystem::Path::operator std::string() const {
	return str();
}

FileSystem::Path::operator std::filesystem::path() const {
	return str();
}


