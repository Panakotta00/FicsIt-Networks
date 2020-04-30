#pragma once

#include <string>

namespace FileSystem {
	class NodeName : public std::string {
	public:
		NodeName(const char* str);
		NodeName(const std::string& str);
		NodeName(std::string&& str);

		NodeName& operator=(const char* str);
		NodeName& operator=(const std::string& str);
		NodeName& operator=(std::string&& str);
	};
}

namespace std {
    template <>
    struct hash<FileSystem::NodeName> {
        std::size_t operator()(const FileSystem::NodeName& k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            return hash<string>()((const std::string&)k);
        }
    };

}