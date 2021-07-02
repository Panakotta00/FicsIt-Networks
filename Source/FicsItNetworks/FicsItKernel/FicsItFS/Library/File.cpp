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

unordered_set<NodeName> File::getChilds() const {
	return unordered_set<NodeName>();
}

MemFile::MemFile(ListenerListRef listeners, SizeCheckFunc sizeCheck) : File(), listeners(listeners), sizeCheck(sizeCheck) {}

SRef<FileStream> MemFile::open(FileMode m) {
	if (io.isValid() && io->isOpen()) return nullptr;
	return io = new MemFileStream(&data, m, listeners, sizeCheck);
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

MemFileStream::MemFileStream(string * data, FileMode mode, ListenerListRef& listeners, SizeCheckFunc sizeCheck) : FileStream(mode), data(data), listeners(listeners), sizeCheck(sizeCheck) {
	buf = *data;
	if ((mode & CodersFileSystem::OUTPUT) && (mode & CodersFileSystem::APPEND)) pos = buf.length();
	else if (mode & CodersFileSystem::TRUNC) *data = buf = "";
	open = true;
}

MemFileStream::~MemFileStream() {
	close();
}

void MemFileStream::write(string newData) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(buf.length(), true)) throw std::exception("out of memory");
	buf = buf.erase(pos, newData.length());
	buf.insert(pos, newData);
	pos += newData.length();
}

void MemFileStream::flush() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::OUTPUT)) return;
	*data = buf;
	listeners.onNodeChanged("", NT_File);
}

string MemFileStream::readChars(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	return buf.substr(pos, chars);
}

string MemFileStream::readLine() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	string s;
	getline(std::stringstream(buf.substr(pos)), s);
	pos += s.length();
	return s;
}

string MemFileStream::readAll() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	return buf;
}

double MemFileStream::readNumber() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	double n = 0.0;
	stringstream s(buf.substr(pos));
	s >> n;
	pos += s.tellg();
	return n;
}

int64_t MemFileStream::seek(string str, int64_t off) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (mode & APPEND) return pos;
	if (str == "set") pos = off;
	else if (str == "cur") pos += off;
	else if (str == "end") pos = buf.length() + off;
	else throw exception("no valid whence");
	if (pos > static_cast<int64_t>(buf.length())) pos = buf.length();
	else if (pos < 0) pos = 0;
	return pos;
}

void MemFileStream::close() {
	if (isOpen()) {
		flush();
		open = false;
	}
}

bool MemFileStream::isEOF() {
	if (!isOpen()) throw std::exception("filestream not open");
	return pos >= static_cast<int64_t>(buf.length());
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
}

void DiskFileStream::flush() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::OUTPUT)) return;
	stream.flush();
}

string DiskFileStream::readChars(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	string s;
	// Ensure buffer is large enough to hold characters.
	s.resize(chars);
	stream.read(s.data(), chars);
	// Shrink the string to the actual number of characters we read.
	s.resize(stream.gcount());
	return s;
}

string DiskFileStream::readLine() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	string s;
	getline(stream, s);
	return s;
}

string DiskFileStream::readAll() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");

	std::stringstream buffer;
	buffer << stream.rdbuf();
	return buffer.str();
}

double DiskFileStream::readNumber() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	double n = 0.0;
	stream >> n;
	return n;
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

bool DiskFileStream::isEOF() {
	if (!isOpen()) throw std::exception("filestream not open");
	return stream.eof();
}

bool DiskFileStream::isOpen() {
	return stream.is_open();
}
