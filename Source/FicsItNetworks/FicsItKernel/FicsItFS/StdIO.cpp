#include "StdIO.h"

using namespace std;

namespace FicsItKernel {
	namespace FicsItFS {
		StdIO::StdIO(FileSystem::ListenerListRef listeners, FileSystem::SizeCheckFunc sizeCheck) : listeners(listeners), sizeCheck(sizeCheck) {}

		FileSystem::SRef<FileSystem::FileStream> StdIO::open(FileSystem::FileMode m) {
			return new StdIOStream(this, m, listeners, sizeCheck);
		}

		bool StdIO::isValid() const {
			return true;
		}

		size_t StdIO::getSize() const {
			return output.length();
		}
		
		StdIOStream::StdIOStream(FileSystem::SRef<StdIO> io, FileSystem::FileMode mode, FileSystem::ListenerListRef& listeners, FileSystem::SizeCheckFunc sizeCheck) : FileStream(mode), io(io), outPos(io->output.length()), listeners(listeners), sizeCheck(sizeCheck) {}
		StdIOStream::~StdIOStream() {}

		void StdIOStream::write(std::string str) {
			if (mode != FileSystem::READ) buffer.append(str);
		}

		void StdIOStream::flush() {
			if (mode != FileSystem::READ) {
				io->output.append(buffer);
				io->output = io->output.substr(*(io->output.end() - 2000));
			}
		}

		std::string StdIOStream::readChars(size_t chars) {
			if (mode == FileSystem::WRITE || mode == FileSystem::APPEND) return "";
			return io->output.substr(outPos, chars);
		}

		std::string StdIOStream::readLine() {
			if (mode == FileSystem::WRITE || mode == FileSystem::APPEND) return "";
			std::stringstream strS(io->output);
			strS.seekg(outPos);
			std::string s;
			std::getline(strS, s);
			return s;
		}
		std::string StdIOStream::readAll() {
			if (mode == FileSystem::WRITE || mode == FileSystem::APPEND) return "";
			return io->output;
		}
		double StdIOStream::readNumber() {
			std::stringstream strS(io->output);
			strS.seekg(outPos);
			double n;
			strS >> n;
			return n;
		}
		
		std::int64_t StdIOStream::seek(std::string str, std::int64_t off) {
			if (!isOpen()) throw std::exception("filestream not open");
			if (str == "set") outPos = off;
			else if (str == "cur") outPos += off;
			else if (str == "end") outPos = io->output.length() - off;
			else throw exception("no valid whence");
			return outPos;
		}

		void StdIOStream::close() {
			flush();
		}
		bool StdIOStream::isEOF() {
			return io->output.length() <= (size_t)outPos;
		}

		bool StdIOStream::isOpen() {
			return true;
		}
	}
}