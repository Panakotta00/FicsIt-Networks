#pragma once

#include <exception>
#include <string>

namespace CodersFileSystem {
	class FICSITFILESYSTEM_API FileSystemException : public std::exception {
	public:
		std::string message;

		FileSystemException(const std::string& message) : message(message) {}

		virtual const char* what() const noexcept override {
			return message.c_str();
		}
	};
}