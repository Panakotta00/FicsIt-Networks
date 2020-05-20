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
	ios::openmode m = 0x0;
	if (mode & FileSystem::INPUT) m |= ios::in;
	if (mode & FileSystem::OUTPUT) m |= ios::out;
	if (mode & FileSystem::TRUNC) m |= ios::trunc;
	if (mode & FileSystem::APPEND) m |= ios::app;
	stream = new stringstream(*data, m);
}

MemFileStream::~MemFileStream() {
	close();
}

void MemFileStream::write(string buf) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(buf.length(), true)) throw std::exception("out of memory");
	stream->write(buf.c_str(), buf.length());
}

void MemFileStream::flush() {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!(mode & FileMode::OUTPUT)) return;
	stream->flush();
	*data = stream->str();
	listeners.onNodeChanged("", NT_File);
}

string MemFileStream::readChars(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	char* buf = new char[chars];
	try {
		stream->read(buf, chars);
	} catch (ios::failure e) {
		delete[] buf;
		throw e;
	}
	string s(buf);
	delete[] buf;
	return s;
}

string MemFileStream::readLine() {
	if (!isOpen()) throw std::exception("filestream not open");
	string s;
	getline(*stream, s);
	return s;
}

string MemFileStream::readAll() {
	if (!isOpen()) throw std::exception("filestream not open");
	stringstream s;
	s << stream->rdbuf();
	return s.str();
}

double MemFileStream::readNumber() {
	if (!isOpen()) throw std::exception("filestream not open");
	double n = 0.0;
	*stream >> n;
	return n;
}

int64_t MemFileStream::seek(string str, int64_t off) {
	if (!isOpen()) throw std::exception("filestream not open");
	auto w = stringstream::cur;
	if (str == "set") w = stringstream::beg;
	else if (str == "cur") w = stringstream::cur;
	else if (str == "end") w = stringstream::end;
	else throw exception("no valid whence");
	stream->seekg(off, w);
	return stream->seekp(off, w).tellp();
}

void MemFileStream::close() {
	if (isOpen()) {
		flush();
		delete stream;
		stream = nullptr;
	}
}

bool MemFileStream::isEOF() {
	if (!isOpen()) throw std::exception("filestream not open");
	return stream->eof();
}

bool MemFileStream::isOpen() {
	return stream != nullptr;
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

DiskFileStream::DiskFileStream(filesystem::path realPath, FileMode mode, SizeCheckFunc sizeCheck) : FileStream(mode), sizeCheck(sizeCheck) {
	ios::openmode m = 0x0;
	if (mode & FileSystem::INPUT) m |= ios::in;
	if (mode & FileSystem::OUTPUT) m |= ios::out;
	if (mode & FileSystem::TRUNC) m |= ios::trunc;
	if (mode & FileSystem::APPEND) m |= ios::app;
	long long data = std::filesystem::exists(realPath) ? std::filesystem::file_size(realPath) : 0;
	stream.open(realPath, m);
	if (stream.is_open() && mode & FileSystem::TRUNC) bool b = sizeCheck(-data, true); 
}

DiskFileStream::~DiskFileStream() {}

void DiskFileStream::write(string str) {
	if (!isOpen()) throw std::exception("filestream not open");
	if (!sizeCheck(str.length(), true)) throw std::exception("out of diskspace");
	stream.write(str.c_str(), str.length());
}

void DiskFileStream::flush() {
	if (!isOpen()) throw std::exception("filestream not open");
	stream.flush();
}

string DiskFileStream::readChars(size_t chars) {
	if (!isOpen()) throw std::exception("filestream not open");
	char* buf = new char[chars];
	try {
		stream.read(buf, chars);
	} catch (ios::failure e) {
		delete[] buf;
		throw e;
	}
	string s(buf);
	delete[] buf;
	return s;
}

string DiskFileStream::readLine() {
	if (!isOpen()) throw std::exception("filestream not open");
	string s;
	getline(stream, s);
	return s;
}

string DiskFileStream::readAll() {
	if (!isOpen()) throw std::exception("filestream not open");
	stringstream s;
	s << stream.rdbuf();
	return s.str();
}

double DiskFileStream::readNumber() {
	if (!isOpen()) throw std::exception("filestream not open");
	double n = 0.0;
	stream >> n;
	return n;
}

int64_t DiskFileStream::seek(string str, int64_t off) {
	if (!isOpen()) throw std::exception("filestream not open");
	auto w = stringstream::cur;
	if (str == "set") w = stringstream::beg;
	else if (str == "cur") w = stringstream::cur;
	else if (str == "end") w = stringstream::end;
	else throw exception("no valid whence");
	stream.seekg(off, w);
	return stream.seekp(off, w).tellp();
}

void DiskFileStream::close() {
	stream.close();
}

bool DiskFileStream::isEOF() {
	if (!isOpen()) throw std::exception("filestream not open");
	return stream.eof();
}

bool DiskFileStream::isOpen() {
	return stream.is_open();
}
