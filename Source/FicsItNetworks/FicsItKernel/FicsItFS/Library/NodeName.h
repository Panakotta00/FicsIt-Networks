#pragma once

#include <string>

namespace CodersFileSystem {
	class NodeName : public std::string {
	public:
		NodeName(const char* str);
		NodeName(const std::string& str);
		NodeName(std::string&& str);

		NodeName& operator=(const char* str);
		NodeName& operator=(const std::string& str);
		NodeName& operator=(std::string&& str);

		operator std::string() const {
			return *this;
		}
	};
}

inline std::filesystem::path operator/(const std::filesystem::path& LeftPath, const CodersFileSystem::NodeName& RightNode) {
	return LeftPath / std::filesystem::path(std::string(RightNode));
}

namespace std {
    template <>
    struct hash<CodersFileSystem::NodeName> {
        std::size_t operator()(const CodersFileSystem::NodeName& k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            return hash<string>()((const std::string&)k);
        }
    };

}