#include "Serial.h"

#include "util/Logging.h"

using namespace std;

namespace FicsItKernel {
	namespace FicsItFS {
		Serial::Serial(FileSystem::ListenerListRef listeners, FileSystem::SizeCheckFunc sizeCheck) : listeners(listeners), sizeCheck(sizeCheck) {}

		FileSystem::SRef<FileSystem::FileStream> Serial::open(FileSystem::FileMode m) {
			clearStreams();
			FileSystem::SRef<SerialStream> stream = new SerialStream(this, m, listeners, sizeCheck);
			inStreams.insert(stream);
			return stream;
		}

		bool Serial::isValid() const {
			return true;
		}

		void Serial::clearStreams() {
			std::unordered_set<FileSystem::WRef<FileSystem::FileStream>> removeStreams;
			for (FileSystem::WRef<FileSystem::FileStream> stream : inStreams) {
				if (!stream) removeStreams.insert(stream);
			}
			for (FileSystem::WRef<FileSystem::FileStream> stream : removeStreams) {
				inStreams.erase(stream);
			}
		}

		size_t Serial::getSize() const {
			return 0;
		}

		void Serial::write(std::string str) {
			for (auto stream = inStreams.begin(); stream != inStreams.end(); ++stream) {
				// check if stream is invalid and erase if that's the case
				if (!stream->isValid()) {
					//inStreams.erase(stream);
					//stream--;
					continue;
				}

				// write str to the input stream
				SerialStream* s = stream->get();
				if (s && s->mode & FileSystem::INPUT) s->input << str;
			}
		}

		std::string Serial::readOutput() {
			std::string str = output.str();
			output = std::stringstream();
			return str;
		}
		
		SerialStream::SerialStream(FileSystem::SRef<Serial> serial, FileSystem::FileMode mode, FileSystem::ListenerListRef& listeners, FileSystem::SizeCheckFunc sizeCheck) : FileStream(mode), serial(serial), listeners(listeners), sizeCheck(sizeCheck) {}
		
		SerialStream::~SerialStream() {}

		void SerialStream::write(std::string str) {
			if (!(mode & FileSystem::OUTPUT)) return;
			buffer.append(str);
		}

		void SerialStream::flush() {
			if (!(mode & FileSystem::OUTPUT)) return;
			serial->output << buffer;
			serial->output.flush();
			buffer = "";
		}

		std::string SerialStream::readChars(size_t chars) {
			if (!(mode & FileSystem::INPUT)) return "";
			char* buf = new char[chars];
			try {
				input.read(buf, chars);
			} catch (ios::failure e) {
				delete[] buf;
				throw e;
			}
			string s(buf);
			delete[] buf;
			input = std::stringstream(input.str().erase(0, input.tellg()));
			return s;
		}

		std::string SerialStream::readLine() {
			if (!(mode & FileSystem::INPUT)) return "";
			string s;
			std::getline(input, s);
			input = std::stringstream(input.str().erase(0, input.tellg()));
			return s;
		}

		std::string SerialStream::readAll() {
			if (!(mode & FileSystem::INPUT)) return "";
			stringstream s;
			s << input.rdbuf();
			input = std::stringstream(input.str().erase(0, input.tellg()));
			return s.str();
		}

		double SerialStream::readNumber() {
			if (!(mode & FileSystem::INPUT)) return 0.0;
			double n = 0.0;
			input >> n;
			input = std::stringstream(input.str().erase(0, input.tellg()));
			return n;
		}
		
		std::int64_t SerialStream::seek(std::string str, std::int64_t off) {
			return 0;
		}

		void SerialStream::close() {
			flush();
		}

		bool SerialStream::isEOF() {
			return input.eof();
		}

		bool SerialStream::isOpen() {
			return true;
		}
	}
}