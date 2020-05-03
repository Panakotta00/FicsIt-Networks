#pragma once

#include "Library/File.h"

namespace FicsItKernel {
	namespace FicsItFS {
		class StdIO : public FileSystem::File {
			friend class StdIOStream;

		private:
			std::string output;
			FileSystem::ListenerListRef listeners;
			FileSystem::SizeCheckFunc sizeCheck;

		public:
			StdIO(FileSystem::ListenerListRef listeners, FileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });

			virtual FileSystem::SRef<FileSystem::FileStream> open(FileSystem::FileMode m) override;
			virtual bool isValid() const override;


			/*
			* returns the size of the content of this file
			*
			* @return	size of the content
			*/
			size_t getSize() const;
		};

		class StdIOStream : public FileSystem::FileStream {
		protected:
			FileSystem::SRef<StdIO> io;
			std::int64_t outPos;
			FileSystem::ListenerListRef& listeners;
			FileSystem::SizeCheckFunc sizeCheck;
			std::string buffer;

		public:
			StdIOStream(FileSystem::SRef<StdIO> io, FileSystem::FileMode mode, FileSystem::ListenerListRef& listeners, FileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
			~StdIOStream();

			virtual void write(std::string str);
			virtual void flush();
			virtual std::string readChars(size_t chars);
			virtual std::string readLine();
			virtual std::string readAll();
			virtual double readNumber();
			virtual std::int64_t seek(std::string w, std::int64_t off);
			virtual void close();
			virtual bool isEOF();
			virtual bool isOpen();
		};
	}
}