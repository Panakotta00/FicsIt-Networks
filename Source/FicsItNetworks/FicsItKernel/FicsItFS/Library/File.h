#pragma once

#include "Node.h"
#include "Listener.h"

#include "FileSystem.h"
#include <sstream>
#include <fstream>

namespace FileSystem {
	class MemFileStream;

	typedef std::function<bool(size_t, bool)> SizeCheckFunc;

	enum FileMode {
		READ,
		WRITE,
		APPEND,
		UPDATE_READ,
		UPDATE_WRITE,
		UPDATE_APPEND,
	};

	class File : public Node {
	public:
		File();

		virtual std::unordered_set<NodeName> getChilds() const override;
	};

	class MemFile : public File {
	private:
		std::string data;
		WRef<MemFileStream> io;
		ListenerListRef listeners;
		SizeCheckFunc sizeCheck;

	public:
		MemFile(ListenerListRef listeners, SizeCheckFunc sizeCheck = [](auto, auto) { return true; });

		virtual SRef<FileStream> open(FileMode m) override;
		virtual bool isValid() const override;


		/*
		* returns the size of the content of this file
		*
		* @return	size of the content
		*/
		size_t getSize() const;
	};

	class DiskFile : public File {
	private:
		std::filesystem::path realPath;
		SizeCheckFunc sizeCheck;

	public:
		DiskFile(const std::filesystem::path& realPath, SizeCheckFunc sizeCheck = [](auto,auto) { return true; });

		virtual SRef<FileStream> open(FileMode m) override;
		virtual bool isValid() const override;
	};

	class FileStream : public ReferenceCounted {
	protected:
		FileMode mode;
	
	public:
		FileStream(FileMode mode);

		/*
		* Writes the given string to the current output-stream at the output-stream pos
		*
		* @param[in]	str	the string you want to write to the stream
		*/
		virtual void write(std::string str) = 0;

		/*
		* "Saves" the changes of the stream to the actual file
		*/
		virtual void flush() = 0;

		/*
		* reads the given amount of characters of the input-stream at the current input-stream pos
		*
		* @param[in]	chars	the count of chars you want to read
		* @return	the read chars as string
		*/
		virtual std::string readChars(size_t chars) = 0;

		/*
		* reads one line of the input-stream at the current input-stream pos
		*
		* @return	returns the read line as string
		*/
		virtual std::string readLine() = 0;

		/*
		* reads the whole content of the input-stream
		*
		* @return	returns all the content of the input-stream
		*/
		virtual std::string readAll() = 0;

		/*
		* reads a number of the input-stream at the current input-stream pos
		*
		* @return	returns the read number as double
		*/
		virtual double readNumber() = 0;

		/*
		* sets the output-stream pos and the input stream-pos to the given position
		* 
		* @param[in]	w	a string defining if the new stream pos should get set relative to the beginning of the file ("set"), the current stream pos ("cur") or the end of the stream ("end")
		* @param[in]	off	the offset to the given relative position the new stream pos should get set to
		* @return	returns the new output-stream pos
		*/
		virtual std::int64_t seek(std::string w, std::int64_t off) = 0;

		/*
		* closes the filestream so no further I/O functions can get called
		*/
		virtual void close() = 0;

		/*
		* checks if the stream pos is at the end of the file
		*
		* @return	returns true if the stream pos is at the end of the file
		*/
		virtual bool isEOF() = 0;

		/*
		* checks if the filestream is open and I/O functions are allowed to get called
		*
		* @return	returns true if filestream is open
		*/
		virtual bool isOpen() = 0;

		/**
		 * Writes the given string to the stream.
		 *
		 * @param	str		the string you want to write to the stream
		 */
		FileStream& operator<<(const std::string& str);
	};

	class MemFileStream : public FileStream {
	protected:
		std::string* data;
		std::stringstream* stream;
		ListenerListRef& listeners;
		SizeCheckFunc sizeCheck;

	public:
		MemFileStream(std::string* data, FileMode mode, ListenerListRef& listeners, SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
		~MemFileStream();

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

	class DiskFileStream : public FileStream {
	protected:
		std::fstream stream;
		SizeCheckFunc sizeCheck;

	public:
		DiskFileStream(std::filesystem::path realPath, FileMode mode, SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
		~DiskFileStream();

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