#pragma once

#include "Library/File.h"

class FICSITNETWORKS_API FFINKernelFSSerial : public CodersFileSystem::File {
	friend class FFINKernelSerialStream;

private:
	std::stringstream output;
	std::unordered_set<CodersFileSystem::WRef<FFINKernelSerialStream>> inStreams;
	CodersFileSystem::ListenerListRef listeners;
	CodersFileSystem::SizeCheckFunc sizeCheck;
	FCriticalSection Mutex;

public:
	FFINKernelFSSerial(CodersFileSystem::ListenerListRef listeners, CodersFileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });

	// Begin FileSystem::Node
	virtual CodersFileSystem::SRef<CodersFileSystem::FileStream> open(CodersFileSystem::FileMode m) override;
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

class FFINKernelSerialStream : public CodersFileSystem::FileStream {
	friend FFINKernelFSSerial;

protected:
	CodersFileSystem::SRef<FFINKernelFSSerial> serial;
	CodersFileSystem::ListenerListRef& listeners;
	CodersFileSystem::SizeCheckFunc sizeCheck;
	std::stringstream input;

public:
	FFINKernelSerialStream(CodersFileSystem::SRef<FFINKernelFSSerial> serial, CodersFileSystem::FileMode mode, CodersFileSystem::ListenerListRef& listeners, CodersFileSystem::SizeCheckFunc sizeCheck = [](auto, auto) { return true; });
	~FFINKernelSerialStream();

	// Begin FileSystem::FileStream
	virtual void write(std::string str) override;
	virtual std::string read(size_t chars) override;
	virtual std::int64_t seek(std::string w, std::int64_t off) override;
	virtual void close() override;
	virtual bool isEOF() override;
	virtual bool isOpen() override;
	// End FileSystem::FileStream
};
