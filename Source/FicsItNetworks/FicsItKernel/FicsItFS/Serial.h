#pragma once

#include "Library/File.h"

class FICSITNETWORKS_API FFINKernelFSSerial : public FileSystem::File {
	friend class FFINKernelSerialStream;

private:
	std::stringstream output;
	std::unordered_set<FileSystem::WRef<FFINKernelSerialStream>> inStreams;
	FileSystem::ListenerListRef listeners;
	FileSystem::SizeCheckFunc sizeCheck;

public:
	FFINKernelFSSerial(FileSystem::ListenerListRef listeners, FileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });

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

class FFINKernelSerialStream : public FileSystem::FileStream {
	friend FFINKernelFSSerial;

protected:
	FileSystem::SRef<FFINKernelFSSerial> serial;
	FileSystem::ListenerListRef& listeners;
	FileSystem::SizeCheckFunc sizeCheck;
	std::string buffer;
	std::stringstream input;

public:
	FFINKernelSerialStream(FileSystem::SRef<FFINKernelFSSerial> serial, FileSystem::FileMode mode, FileSystem::ListenerListRef& listeners, FileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
	~FFINKernelSerialStream();

	// Begin FileSystem::FileStream
	virtual void write(std::string str) override;
	virtual void flush() override;
	virtual std::string readChars(size_t chars) override;
	virtual std::string readLine() override;
	virtual std::string readAll() override;
	virtual double readNumber() override;
	virtual std::int64_t seek(std::string w, std::int64_t off) override;
	virtual void close() override;
	virtual bool isEOF() override;
	virtual bool isOpen() override;
	// End FileSystem::FileStream
};
