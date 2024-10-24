#pragma once

#include "Listener.h"

#include <sstream>
#include <fstream>
#include <functional>
#include <filesystem>

namespace CodersFileSystem {
	typedef std::function<bool(long long, bool)> SizeCheckFunc;

	enum FileType : uint8_t {
		File_Regular,
		File_Directory,
		File_Device,
	};

	enum FileMode : unsigned char {
		INPUT	= 0b00001,
		OUTPUT	= 0b00010,
		APPEND	= 0b00100,
		TRUNC	= 0b01000,
		BINARY	= 0b10000,
	};

	FICSITFILESYSTEM_API FileMode operator |(FileMode l, FileMode r);
	FICSITFILESYSTEM_API FileMode operator &(FileMode l, FileMode r);
	FICSITFILESYSTEM_API FileMode operator ~(FileMode m);

	class FICSITFILESYSTEM_API FileStream {
	protected:
		FileMode mode;
	
	public:
		FileStream(FileMode mode);
		virtual ~FileStream() = default;

		/**
		 * Returns the open mode of the file stream
		 *
		 * @return	used file mode
		 */
		virtual FileMode getMode() const;

		/*
		* Writes the given string to the current output-stream at the output-stream pos
		*
		* @param[in]	str	the string you want to write to the stream
		*/
		virtual void write(std::string str) = 0;

		/*
		 * reads the given amount of characters of the input-stream at the current input-stream pos.
		 * might return less characters than requested, but stream may still have characters available later.
		 *
		 * If no further characters are available in filestream, EOF flag will be set and can be checked with the isEOF function.
		 *
		 * @param[in]	chars	the count of chars you want to read
		 * @return	the read chars as string
		 */
		virtual std::string read(size_t chars) = 0;

		/**
		 * Returns true if the end-of-file (EOF) flag was set.
		 * Flag will be overwritten by further successful reads or any seek operations.
		 *
		 * @return true if the stream doesn't have any more characters available to read.
		 */
		virtual bool isEOF() = 0;

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

		/**
		 * Reads the given stream till EOF
		 *
		 * @param[in]	stream	the stream you want to read till EOF
		 * @return all the content of the stream read til EOF
		 */
		static std::string readAll(TSharedRef<FileStream> stream);
	};

	class FICSITFILESYSTEM_API DiskFileStream : public FileStream {
	protected:
		std::filesystem::path path;
		SizeCheckFunc sizeCheck;
		std::fstream stream;

	public:
		DiskFileStream(std::filesystem::path realPath, FileMode mode, SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
		~DiskFileStream();

		virtual void write(std::string str) override;
		virtual std::string read(size_t chars) override;
		virtual bool isEOF() override;
		virtual std::int64_t seek(std::string w, std::int64_t off) override;
		virtual void close() override;
		virtual bool isOpen() override;
	};
}