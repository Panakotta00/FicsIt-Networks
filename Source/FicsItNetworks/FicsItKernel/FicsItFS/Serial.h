#pragma once

#include "Library/File.h"

namespace FicsItKernel {
	namespace FicsItFS {
		class Serial : public FileSystem::File {
			friend class SerialStream;

		private:
			std::stringstream output;
			std::unordered_set<FileSystem::WRef<SerialStream>> inStreams;
			FileSystem::ListenerListRef listeners;
			FileSystem::SizeCheckFunc sizeCheck;

		public:
			Serial(FileSystem::ListenerListRef listeners, FileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });

			// Begin FileSystem::Node
			virtual FileSystem::SRef<FileSystem::FileStream> open(FileSystem::FileMode m) override;
			virtual bool isValid() const override;
			// End FileSystem::Node

			/**
			 * Clears the stream list from unused streams
			 */
			void clearStreams();

			/*
			* returns the size of the content of this file
			*
			* @return	size of the content
			*/
			size_t getSize() const;

			/*
			 * Writes the given string to all open input streams input buffers
			 *
			 * @param	str		string to write to input
			 */
			void write(std::string str);

			/*
			 * Reads all from the output stream, causes the output stream to get cleared
			 *
			 * @return	the contents of the output stream
			 */
			std::string readOutput();
		};

		class SerialStream : public FileSystem::FileStream {
			friend Serial;

		protected:
			FileSystem::SRef<Serial> serial;
			FileSystem::ListenerListRef& listeners;
			FileSystem::SizeCheckFunc sizeCheck;
			std::string buffer;
			std::stringstream input;

		public:
			SerialStream(FileSystem::SRef<Serial> serial, FileSystem::FileMode mode, FileSystem::ListenerListRef& listeners, FileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
			~SerialStream();

			// Begin FileSystem::FileStream
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
			// End FileSystem::FileStream
		};
	}
}