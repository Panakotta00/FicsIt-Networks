#include "NodeName.h"

namespace CodersFileSystem {
	std::string&& checkNodeName(std::string&& str) {
		if (str.find("/") != std::string::npos) throw std::invalid_argument("name is not allowed to contain '/'");
		return std::move(str);
	}

	NodeName::NodeName(const char* str) : std::string(checkNodeName(str)) {}

	NodeName::NodeName(const std::string& str) : std::string(checkNodeName(std::move(std::string(str)))) {}

	NodeName::NodeName(std::string&& str) : std::string(checkNodeName(std::move(str))) {}

	NodeName& CodersFileSystem::NodeName::operator=(const char* str) {
		*this = std::move(checkNodeName(str));
		return *this;
	}

	NodeName& CodersFileSystem::NodeName::operator=(const std::string& str) {
		*this = std::move(checkNodeName(std::move(std::string(str))));
		return *this;
	}

	NodeName& CodersFileSystem::NodeName::operator=(std::string&& str) {
		*(std::string*)this = std::move(checkNodeName(std::move(str)));
		return *this;
	}
}