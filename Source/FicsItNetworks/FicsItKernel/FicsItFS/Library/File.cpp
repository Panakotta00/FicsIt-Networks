#include "File.h"

#include <filesystem>

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

File::File() {}

unordered_set<std::string> File::getChilds() const {
	return unordered_set<std::string>();
}

MemFile::MemFile(ListenerListRef listeners, SizeCheckFunc sizeCheck) : File(), listeners(listeners), sizeCheck(sizeCheck) {}

SRef<FileStream> MemFile::open(FileMode m) {
	if (io.isValid() && io->isOpen()) return nullptr;
	SRef<MemFileStream> FS = new MemFileStream(&data, m, listeners, sizeCheck);
	io = FS;
	return FS;
}

bool CodersFileSystem::MemFile::isValid() const {
	return true;
}

size_t MemFile::getSize() const {
	return data.length();
}

FileStream::FileStream(FileMode mode) : mode(mode) {}

FileMode FileStream::getMode() const {
	return mode;
}

FileStream& FileStream::operator<<(const std::string& str) {
	write(str);

	return *this;
}

std::string FileStream::readAll(SRef<FileStream> stream) {
	std::string str;
	do {
		str.append(stream->read(1024));
	} while (!stream->isEOF());
	return str;
}

MemFileStream::MemFileStream(string * data, FileMode mode, ListenerListRef& listeners, SizeCheckFunc sizeCheck) : FileStream(mode), data(data), listeners(listeners), sizeCheck(sizeCheck) {
	if ((mode & CodersFileSystem::OUTPUT) && (mode & CodersFileSystem::APPEND)) pos = data->length();
	else if (mode & CodersFileSystem::TRUNC) *data = "";
	open = true;
}

MemFileStream::~MemFileStream() {
	close();
}

void MemFileStream::write(string newData) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(data->length(), true)) throw std::exception("out of memory");
	data->erase(pos, newData.length());
	data->insert(pos, newData);
	pos += newData.length();
}

string MemFileStream::read(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	if (pos >= data->size()) {
		flagEOF = true;
		return "";
	}
	flagEOF = false;
	string buf = data->substr(pos, chars);
	pos += buf.length();
	return buf;
}

bool MemFileStream::isEOF() {
	return flagEOF;
}

int64_t MemFileStream::seek(string str, int64_t off) {
	if (!isOpen()) throw std::exception("filestream not open");
	flagEOF = false;
	if (mode & APPEND) return pos;
	if (str == "set") pos = off;
	else if (str == "cur") pos += off;
	else if (str == "end") pos = data->length() + off;
	else throw exception("no valid whence");
	if (pos > static_cast<uint64_t>(data->length())) pos = data->length();
	else if (pos < 0) pos = 0;
	return pos;
}

void MemFileStream::close() {
	if (isOpen()) {
		open = false;
	}
}

bool MemFileStream::isOpen() {
	return open;
}

DiskFile::DiskFile(const filesystem::path& realPath, SizeCheckFunc sizeCheck) : File(), realPath(realPath), sizeCheck(sizeCheck) {}

SRef<FileStream> DiskFile::open(FileMode m) {
	SRef<FileStream> s = new DiskFileStream(realPath, m, sizeCheck);
	if (s->isOpen()) return s;
	return nullptr;
}

bool DiskFile::isValid() const {
	return filesystem::is_regular_file(realPath);
}

DiskFileStream::DiskFileStream(filesystem::path realPath, FileMode mode, SizeCheckFunc sizeCheck) : FileStream(mode), path(realPath), sizeCheck(sizeCheck) {
	if (!(mode & (FileMode::OUTPUT | FileMode::INPUT))) {
		throw std::exception("I/O mode not set");
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
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(data.length(), true)) throw std::exception("out of capacity");
	stream << data;
	stream.flush();
}

string DiskFileStream::read(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
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
	if (!isOpen()) throw std::exception("filestream not open");
	enum {
		WHENCE_INVALID,
		WHENCE_SET,
		WHENCE_CUR,
		WHENCE_END
	} whence = WHENCE_INVALID;

	if (str == "set") whence = WHENCE_SET;
	else if (str == "cur") whence = WHENCE_CUR;
	else if (str == "end") whence = WHENCE_END;

	if (whence == WHENCE_INVALID) throw std::exception("Invalid whence");

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
