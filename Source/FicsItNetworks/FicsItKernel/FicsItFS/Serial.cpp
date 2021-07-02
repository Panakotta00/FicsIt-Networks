#include "Serial.h"

FFINKernelFSSerial::FFINKernelFSSerial(CodersFileSystem::ListenerListRef listeners, CodersFileSystem::SizeCheckFunc sizeCheck) : listeners(listeners), sizeCheck(sizeCheck) {}

CodersFileSystem::SRef<CodersFileSystem::FileStream> FFINKernelFSSerial::open(CodersFileSystem::FileMode m) {
	FScopeLock Lock(&Mutex);
	clearStreams();
	CodersFileSystem::SRef<FFINKernelSerialStream> stream = new FFINKernelSerialStream(this, m, listeners, sizeCheck);
	inStreams.insert(stream);
	return stream;
}

bool FFINKernelFSSerial::isValid() const {
	return true;
}

void FFINKernelFSSerial::clearStreams() {
	FScopeLock Lock(&Mutex);
	std::unordered_set<CodersFileSystem::WRef<CodersFileSystem::FileStream>> removeStreams;
	for (CodersFileSystem::WRef<CodersFileSystem::FileStream> stream : inStreams) {
		if (!stream) removeStreams.insert(stream);
	}
	for (CodersFileSystem::WRef<CodersFileSystem::FileStream> stream : removeStreams) {
		inStreams.erase(stream);
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
size_t FFINKernelFSSerial::getSize() const {
	return 0;
}

void FFINKernelFSSerial::write(std::string str) {
	FScopeLock Lock(&Mutex);
	for (auto stream = inStreams.begin(); stream != inStreams.end(); ++stream) {
		// check if stream is invalid and erase if that's the case
		if (!stream->isValid()) {
			//inStreams.erase(stream);
			//stream--;
			continue;
		}

		// write str to the input stream
		FFINKernelSerialStream* s = stream->get();
		if (s && s->mode & CodersFileSystem::INPUT) s->input << str;
	}
}

std::string FFINKernelFSSerial::readOutput() {
	FScopeLock Lock(&Mutex);
	std::string str = output.str();
	output = std::stringstream();
	return str;
}

FFINKernelSerialStream::FFINKernelSerialStream(CodersFileSystem::SRef<FFINKernelFSSerial> FFINKernelFSSerial, CodersFileSystem::FileMode mode, CodersFileSystem::ListenerListRef& listeners, CodersFileSystem::SizeCheckFunc sizeCheck) : FileStream(mode), serial(FFINKernelFSSerial), listeners(listeners), sizeCheck(sizeCheck) {}

FFINKernelSerialStream::~FFINKernelSerialStream() {}

void FFINKernelSerialStream::write(std::string str) {
	if (!(mode & CodersFileSystem::OUTPUT)) return;
	buffer.append(str);
}

void FFINKernelSerialStream::flush() {
	if (!(mode & CodersFileSystem::OUTPUT)) return;
	FScopeLock Lock(&serial->Mutex);
	serial->output << buffer;
	serial->output.flush();
	buffer = "";
}

std::string FFINKernelSerialStream::readChars(size_t chars) {
	if (!(mode & CodersFileSystem::INPUT)) return "";
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
	if (!(mode & CodersFileSystem::INPUT)) return "";
	std::string s;
	std::getline(input, s);
	input = std::stringstream(input.str().erase(0, input.tellg()));
	return s;
}

std::string FFINKernelSerialStream::readAll() {
	if (!(mode & CodersFileSystem::INPUT)) return "";
	std::stringstream s;
	s << input.rdbuf();
	input = std::stringstream(input.str().erase(0, input.tellg()));
	return s.str();
}

double FFINKernelSerialStream::readNumber() {
	if (!(mode & CodersFileSystem::INPUT)) return 0.0;
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
