#include "Serial.h"

FFINKernelFSSerial::FFINKernelFSSerial(FileSystem::ListenerListRef listeners, FileSystem::SizeCheckFunc sizeCheck) : listeners(listeners), sizeCheck(sizeCheck) {}

FileSystem::SRef<FileSystem::FileStream> FFINKernelFSSerial::open(FileSystem::FileMode m) {
	clearStreams();
	FileSystem::SRef<FFINKernelSerialStream> stream = new FFINKernelSerialStream(this, m, listeners, sizeCheck);
	inStreams.insert(stream);
	return stream;
}

bool FFINKernelFSSerial::isValid() const {
	return true;
}

void FFINKernelFSSerial::clearStreams() {
	std::unordered_set<FileSystem::WRef<FileSystem::FileStream>> removeStreams;
	for (FileSystem::WRef<FileSystem::FileStream> stream : inStreams) {
		if (!stream) removeStreams.insert(stream);
	}
	for (FileSystem::WRef<FileSystem::FileStream> stream : removeStreams) {
		inStreams.erase(stream);
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
size_t FFINKernelFSSerial::getSize() const {
	return 0;
}

void FFINKernelFSSerial::write(std::string str) {
	for (auto stream = inStreams.begin(); stream != inStreams.end(); ++stream) {
		// check if stream is invalid and erase if that's the case
		if (!stream->isValid()) {
			//inStreams.erase(stream);
			//stream--;
			continue;
		}

		// write str to the input stream
		FFINKernelSerialStream* s = stream->get();
		if (s && s->mode & FileSystem::INPUT) s->input << str;
	}
}

std::string FFINKernelFSSerial::readOutput() {
	std::string str = output.str();
	output = std::stringstream();
	return str;
}

FFINKernelSerialStream::FFINKernelSerialStream(FileSystem::SRef<FFINKernelFSSerial> FFINKernelFSSerial, FileSystem::FileMode mode, FileSystem::ListenerListRef& listeners, FileSystem::SizeCheckFunc sizeCheck) : FileStream(mode), serial(FFINKernelFSSerial), listeners(listeners), sizeCheck(sizeCheck) {}

FFINKernelSerialStream::~FFINKernelSerialStream() {}

void FFINKernelSerialStream::write(std::string str) {
	if (!(mode & FileSystem::OUTPUT)) return;
	buffer.append(str);
}

void FFINKernelSerialStream::flush() {
	if (!(mode & FileSystem::OUTPUT)) return;
	serial->output << buffer;
	serial->output.flush();
	buffer = "";
}

std::string FFINKernelSerialStream::readChars(size_t chars) {
	if (!(mode & FileSystem::INPUT)) return "";
	char* buf = new char[chars];
	try {
		input.read(buf, chars);
	} catch (std::ios::failure e) {
		delete[] buf;
		throw;
	}
	std::string s(buf);
	delete[] buf;
	input = std::stringstream(input.str().erase(0, input.tellg()));
	return s;
}

std::string FFINKernelSerialStream::readLine() {
	if (!(mode & FileSystem::INPUT)) return "";
	std::string s;
	std::getline(input, s);
	input = std::stringstream(input.str().erase(0, input.tellg()));
	return s;
}

std::string FFINKernelSerialStream::readAll() {
	if (!(mode & FileSystem::INPUT)) return "";
	std::stringstream s;
	s << input.rdbuf();
	input = std::stringstream(input.str().erase(0, input.tellg()));
	return s.str();
}

double FFINKernelSerialStream::readNumber() {
	if (!(mode & FileSystem::INPUT)) return 0.0;
	double n = 0.0;
	input >> n;
	input = std::stringstream(input.str().erase(0, input.tellg()));
	return n;
}

std::int64_t FFINKernelSerialStream::seek(std::string str, std::int64_t off) {
	return 0;
}

void FFINKernelSerialStream::close() {
	flush();
}

bool FFINKernelSerialStream::isEOF() {
	return input.eof();
}

bool FFINKernelSerialStream::isOpen() {
	return true;
}
