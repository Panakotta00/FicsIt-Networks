#include "File.h"

#include <experimental/filesystem>

using namespace std;
using namespace FileSystem;

FileMode FileSystem::operator|(FileMode l, FileMode r) {
	return (FileMode)(((unsigned char)l) | ((unsigned char)r));
}

FileMode FileSystem::operator&(FileMode l, FileMode r) {
	return (FileMode)(((unsigned char)l) & ((unsigned char)r));
}

FileMode FileSystem::operator~(FileMode m) {
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

bool FileSystem::MemFile::isValid() const {
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
	if ((mode & FileSystem::OUTPUT) && (mode & FileSystem::APPEND)) pos = buf.length();
	else if (mode & FileSystem::TRUNC) *data = buf = "";
	open = true;
}

MemFileStream::~MemFileStream() {
	close();
}

void MemFileStream::write(string data) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(buf.length(), true)) throw std::exception("out of memory");
	buf = buf.erase(pos, data.length());
	buf.insert(pos, data);
	pos += data.length();
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
	if (mode & FileMode::OUTPUT) std::ofstream(realPath).close();
	stream = std::fstream(realPath, std::ios::in);
	if (!stream.is_open()) return;
	if (!(mode & TRUNC)) {
		stringstream s;
		s << stream.rdbuf();
		buf = s.str();
	} else sizeCheck(-static_cast<int64_t>(std::filesystem::file_size(realPath)), true); 
	stream.close();
	stream = std::fstream(realPath, std::ios::out | std::ios::trunc);
	stream << buf;
	stream.flush();
}

DiskFileStream::~DiskFileStream() {}

void DiskFileStream::write(string data) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(data.length(), true)) throw std::exception("out of capacity");
	buf = buf.erase(pos, data.length());
	buf.insert(pos, data);
	pos += data.length();
}

void DiskFileStream::flush() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::OUTPUT)) return;
	stream.close();
	stream = std::fstream(path, std::ios::out | std::ios::trunc);
	stream << buf;
	stream.flush();
}

string DiskFileStream::readChars(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	return buf.substr(pos, chars);
}

string DiskFileStream::readLine() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	string s;
	getline(std::stringstream(buf.substr(pos)), s);
	pos += s.length();
	return s;
}

string DiskFileStream::readAll() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	return buf;
}

double DiskFileStream::readNumber() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::INPUT)) throw std::exception("filestream not in input mode");
	double n = 0.0;
	stringstream s(buf.substr(pos));
	s >> n;
	pos += s.tellg();
	return n;
}

int64_t DiskFileStream::seek(string str, int64_t off) {
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

void DiskFileStream::close() {
	if (isOpen()) {
		flush();
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
