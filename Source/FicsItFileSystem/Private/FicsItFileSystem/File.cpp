#include "FicsItFileSystem/File.h"
#include <filesystem>

#include "FileSystemException.h"

using namespace std;
using namespace CodersFileSystem;

FileMode CodersFileSystem::operator|(FileMode l, FileMode r) {
	return (FileMode)(((unsigned char)l) | ((unsigned char)r));
}

FileMode CodersFileSystem::operator&(FileMode l, FileMode r) {
	return (FileMode)(((unsigned char)l) & ((unsigned char)r));
}

FileMode CodersFileSystem::operator~(FileMode m) {
	return (FileMode)~(unsigned char)m;
}

FileStream::FileStream(FileMode mode) : mode(mode) {}

FileMode FileStream::getMode() const {
	return mode;
}

FileStream& FileStream::operator<<(const std::string& str) {
	write(str);

	return *this;
}

std::string FileStream::readAll(TSharedRef<FileStream> stream) {
	std::string str;
	do {
		str.append(stream->read(1024));
	} while (!stream->isEOF());
	return str;
}

DiskFileStream::DiskFileStream(filesystem::path realPath, FileMode mode, SizeCheckFunc sizeCheck) : FileStream(mode), path(realPath), sizeCheck(sizeCheck) {
	if (!(mode & (FileMode::OUTPUT | FileMode::INPUT))) {
		throw FileSystemException("I/O mode not set");
	}
	if ((mode & FileMode::TRUNC) && filesystem::exists(realPath)) {
		sizeCheck(-static_cast<int64_t>(std::filesystem::file_size(realPath)), true);
	}
	ios_base::openmode nativeMode = 0;
	if (mode & FileMode::OUTPUT) nativeMode |= ios::out;
	if (mode & FileMode::INPUT) nativeMode |= ios::in;
	if (mode & FileMode::APPEND) nativeMode |= ios::app;
	if (mode & FileMode::TRUNC) nativeMode |= ios::trunc;
	if (mode & FileMode::BINARY) nativeMode |= ios::binary;

	stream = std::fstream(realPath, nativeMode);
}

DiskFileStream::~DiskFileStream() {}

void DiskFileStream::write(string data) {
	if (!isOpen()) throw FileSystemException("filestream not open");
	if (!sizeCheck(data.length(), true)) throw FileSystemException("out of capacity");
	stream << data;
	stream.flush();
}

string DiskFileStream::read(size_t chars) {
	if (!isOpen()) throw FileSystemException("filestream not open");
	if (!(mode & FileMode::INPUT)) throw FileSystemException("filestream not in input mode");
	string s;
	// Ensure buffer is large enough to hold characters.
	s.resize(chars);
	stream.read(const_cast<char*>(s.data()), chars);
	// Shrink the string to the actual number of characters we read.
	s.resize(stream.gcount());
	return s;
}

bool DiskFileStream::isEOF() {
	return stream.eof();
}

int64_t DiskFileStream::seek(string str, int64_t off) {
	if (!isOpen()) throw FileSystemException("filestream not open");
	enum {
		WHENCE_INVALID,
		WHENCE_SET,
		WHENCE_CUR,
		WHENCE_END
	} whence = WHENCE_INVALID;

	if (str == "set") whence = WHENCE_SET;
	else if (str == "cur") whence = WHENCE_CUR;
	else if (str == "end") whence = WHENCE_END;

	if (whence == WHENCE_INVALID) throw FileSystemException("Invalid whence");

	if (mode & FileMode::INPUT) {
		switch (whence) {
		case WHENCE_SET:
			stream.seekg(off);
			break;
		case WHENCE_CUR:
			stream.seekg(off, ios_base::cur);
			break;
		case WHENCE_END:
			stream.seekg(off, ios_base::end);
			break;
		}
	}

	if ((mode & FileMode::OUTPUT) && !(mode & FileMode::APPEND)) {
		switch (whence) {
		case WHENCE_SET:
			stream.seekp(off);
			break;
		case WHENCE_CUR:
			stream.seekp(off, ios_base::cur);
			break;
		case WHENCE_END:
			stream.seekp(off, ios_base::end);
			break;
		}
	}

	// Default to returning read position (ie if both read and write)
	if (mode & FileMode::INPUT) {
		return stream.tellg();
	} else {
		return stream.tellp();
	}
}

void DiskFileStream::close() {
	if (isOpen()) {
		stream.close();
	}
}

bool DiskFileStream::isOpen() {
	return stream.is_open();
}
