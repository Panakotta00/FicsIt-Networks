#pragma once

#include <memory>
#include <filesystem>
#include <fstream>

#include <assets/BPInterface.h>
#include <util/Objects/TArray.h>

#include <SDK.hpp>

#include "LuaException.h"
#include "NetworkComponent.h"
#include "LuaImplementation.h"

enum ELevelTick {
	LEVELTICK_TimeOnly = 0x0,
	LEVELTICK_ViewportsOnly = 0x1,
	LEVELTICK_All = 0x2,
	LEVELTICK_PauseTick = 0x3,
};

class UFileSystem;
struct lua_State;

class IFileSystemComponent : public INetworkComponent {
private:
	UFileSystem* self() const;

public:
	virtual SML::Objects::FGuid getID() const;
	virtual SML::Objects::UObject* findComponent(SML::Objects::FGuid guid) const;
	virtual UNetworkCircuit* getCircuit() const override;
	virtual void setCircuit(UNetworkCircuit * circuit) override;
};

class IFileSystemLua : public ILuaImplementation {
private:
	UFileSystem* self() const;

public:
	virtual void luaAddSignalListener(ULuaContext* ctx) override;
	virtual void luaRemoveSignalListener(ULuaContext* ctx) override;
	virtual SML::Objects::TArray<ULuaContext*> luaGetSignalListeners() override;
	virtual bool luaIsReachableFrom(SML::Objects::UObject* listener) override;
};

struct IFileSystemSaveInterface {
	virtual bool NeedTransform();
	virtual bool ShouldSave();
	virtual void gatherDeps(SML::Objects::TArray<SML::Objects::UObject*>*);
	virtual void postLoad(int, int);
	virtual void preLoad(int, int);
	virtual void postSave(int, int);
	virtual void preSave(int, int);
	virtual SML::Objects::UObject* _getUObject() const;
};

class FileSystemManager;

class UFileSystem : public SDK::UActorComponent {
public:
	SML::Objects::FGuid id;
	bool idCreated;

	int capacity;
	SML::Objects::TArray<ULuaContext*> listeners;
	UNetworkCircuit* circuit;
	
	std::unique_ptr<FileSystemManager> manager;
	HANDLE watcher;
	OVERLAPPED ovl;
	FILE_NOTIFY_INFORMATION info[16];

	IFileSystemSaveInterface save;
	IFileSystemComponent component;
	IFileSystemLua luaImpl;

	void constructor();
	void destruct();

	void beginPlay();
	void tick(float deltaSeconds, ELevelTick tick, void* thisFunc);

	/**
	* 0 fileCreated, 1 fileDeleted, 2 fileContentChanged, 3 fileRenamed, 4 dirCreated, 5 dirDeleted, 6 dirRenamed
	*/
	void signalFileSystemChange(int type, std::wstring npath, std::wstring opath);
	void execSigFSC(SML::Objects::FFrame& stack, void*);

	static SML::Objects::UClass* staticClass();
};

class FileSystemFileStream;

class FileSystemManager {
protected:
	std::filesystem::path root;

public:
	FileSystemManager(std::filesystem::path root);

	std::filesystem::path getPath(std::string path, bool hasExistenceCheck , bool hasParentCheck, int arg);

	std::filesystem::path getRoot() const;
	virtual std::unique_ptr<FileSystemFileStream> open(std::string path, std::string mode);
	virtual void createDir(std::string path, bool all = false);
	virtual void remove(std::string path, bool all = false);
	virtual void move(std::string from, std::string to);
	virtual bool exists(std::string path);
	virtual bool isFile(std::string path);
	virtual bool isDir(std::string path);
};

class FileSystemManagerCapacity : public FileSystemManager {
protected:
	std::uint64_t capacity;
	std::uint64_t used;

public:
	FileSystemManagerCapacity(std::filesystem::path root, std::uint64_t capacity);

	std::uint64_t getSize(std::string path, int arg);
	std::uint64_t getSize(std::filesystem::path path);
	std::uint64_t getSize(std::filesystem::directory_entry entry);
	std::uint64_t updateSize();
	void checkNewSpace(std::uint64_t size);

	virtual std::unique_ptr<FileSystemFileStream> open(std::string path, std::string mode) override;
	virtual void createDir(std::string path, bool all = false) override;
	virtual void remove(std::string path, bool all = false) override;
	virtual void move(std::string from, std::string to) override;
};

class FileSystemFileStream {
	friend FileSystemManager;

protected:
	FileSystemManager* owner;
	std::filesystem::path path;
	std::fstream stream;
	std::ios_base::openmode mode;

	FileSystemFileStream(FileSystemManager* owner);

public:
	void checkOpen();
	std::fstream::openmode getMode(std::string mode);

	virtual void open(std::filesystem::path path, std::string mode);
	virtual void write(std::string str);
	virtual void flush();
	virtual std::string readChars(size_t chars);
	virtual std::string readLine();
	virtual std::string readAll();
	virtual double readNumber();
	virtual std::int64_t seek(std::string w, std::int64_t off);
	virtual void close();
	virtual bool isEOF();
};

class FileSystemFileStreamCapacity : public FileSystemFileStream {
	friend FileSystemManagerCapacity;

protected:
	FileSystemFileStreamCapacity(FileSystemManagerCapacity* owner);

public:
	virtual void open(std::filesystem::path path, std::string mode);
	virtual void write(std::string str) override;
};