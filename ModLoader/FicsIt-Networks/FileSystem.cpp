#include "stdafx.h"
#include "FileSystem.h"

#include <sstream>

#include "LuaContext.h"

#include "LuaLib.h"
//#include "FileSystem\FileSystem.h"

using namespace SML;
using namespace SML::Objects;
namespace fs = std::filesystem;

UFileSystem* IFileSystemComponent::self() const {
	return (UFileSystem*)((size_t)this - offsetof(UFileSystem, component));
}

FGuid IFileSystemComponent::getID() const {
	return self()->id;
}

SML::Objects::FString IFileSystemComponent::getNick() const {
	return self()->nick;
}

void IFileSystemComponent::setNick(const FString& nick) {
	self()->nick = nick;
}

UObject* IFileSystemComponent::findComponent(FGuid guid) const {
	return INetworkComponent::findComponentFromCircuit(guid);
}

UNetworkCircuit * IFileSystemComponent::getCircuit() const {
	return self()->circuit;
}

void IFileSystemComponent::setCircuit(UNetworkCircuit * circuit) {
	self()->circuit = circuit;
}

void(UFileSystem::*beginPlay_f)() = nullptr;
void(UFileSystem::*tick_f)(float, ELevelTick, void*) = nullptr;
void UFileSystem::constructor() {
	// actor vtable hook
	if (!beginPlay_f) {
		auto& f = ((void(UFileSystem::**)())this->Vtable)[0x5F];
		beginPlay_f = f;
		f = &UFileSystem::beginPlay;
	}
	if (!tick_f) {
		auto& f = ((void(UFileSystem::**)(float, ELevelTick, void*))this->Vtable)[0x62];
		tick_f = f;
		f = &UFileSystem::tick;
	}

	new (&component) IFileSystemComponent();
	//save = &s_vtbl;
	new (&save) IFileSystemSaveInterface();
	new (&luaImpl) IFileSystemLua();

	new (&id) FGuid();
	new (&manager) std::unique_ptr<FileSystemManager>();
	new (&listeners) TArray<ULuaContext*>();
	idCreated = false;
	capacity = 1000;
	watcher = nullptr;
	this->PrimaryComponentTick.bCanEverTick = true;
}

void UFileSystem::destruct() {
	component.~IFileSystemComponent();
	save.~IFileSystemSaveInterface();
	luaImpl.~IFileSystemLua();
	id.~FGuid();
	manager.~unique_ptr();
	listeners.~TArray();
	if (watcher) FindCloseChangeNotification(watcher);
}

void UFileSystem::beginPlay() {
	(this->*beginPlay_f)();

	static auto nguid = (FGuid(*)()) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "FGuid::NewGuid");
	if (!idCreated) {
		id = nguid();
		idCreated = true;
	}

	// get root fs path
	static FString*(*getSavePath)(FString*) = nullptr;
	if (!getSavePath) getSavePath = (FString*(*)(FString*)) DetourFindFunction("FactoryGame-Win64-Shipping.exe", "UFGSaveSystem::GetSaveDirectoryPath");

	FString fsp;
	getSavePath(&fsp);

	std::filesystem::path root = fsp.toStr();
	root = root / "Computers" / this->component.getID().toStr();

	manager.reset((capacity < 0) ? new FileSystemManager(root) : new FileSystemManagerCapacity(root, capacity));

	watcher = ::CreateFile(root.wstring().c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
	ovl = {0};
	ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ReadDirectoryChangesW(watcher, &info, sizeof(info), true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &ovl, NULL);
}

void UFileSystem::tick(float deltaSeconds, ELevelTick tick, void* thisFunc) {
	(this->*tick_f)(deltaSeconds, tick, thisFunc);

	DWORD status = WaitForSingleObject(ovl.hEvent, 0);
	if (status != WAIT_OBJECT_0) return;
	
	FILE_NOTIFY_INFORMATION* current = &info[0];
	std::wstring bufStr;
	while (current) {
		std::wstring fname = std::wstring((const wchar_t*)&current->FileName, current->FileNameLength);
		std::replace(fname.begin(), fname.end(), L'\\', L'/');
		bool isDir = fs::is_directory(manager->getRoot() / fname);
		Utility::warning((manager->getRoot() / fname).string());
		fname = std::wstring(L"/") + fname;
		switch (current->Action) {

		case FILE_ACTION_ADDED:
			if (isDir) signalFileSystemChange(4, fname, fname);
			else signalFileSystemChange(0, fname, fname);
			Utility::warning("add! ", isDir);
			break;
		case FILE_ACTION_REMOVED:
			if (isDir) signalFileSystemChange(5, fname, fname);
			else signalFileSystemChange(1, fname, fname);
			Utility::warning("remove!");
			break;
		case FILE_ACTION_MODIFIED:
			if (isDir) Utility::warning("DirChange!");
			else signalFileSystemChange(2, fname, fname);
			Utility::warning("modify!");
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			Utility::warning("new!");
			signalFileSystemChange(3, fname, bufStr);
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			Utility::warning("old!");
			bufStr = fname;
			break;
		}
		if (current->NextEntryOffset <= 0) break;
		current = (FILE_NOTIFY_INFORMATION*)((size_t)current + current->NextEntryOffset);
	}
	
	ReadDirectoryChangesW(watcher, &info, sizeof(info), true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &ovl, NULL);
}

void UFileSystem::signalFileSystemChange(int type, std::wstring npath, std::wstring opath) {
	auto o = (Objects::UObject*)this;
	SigFileSystemChange_Params params{type, opath.c_str(), npath.c_str()};
	o->findFunction(L"luaSig_FileSystemChange")->invoke(o, &params);
	Utility::error(sizeof(params));
}

void UFileSystem::execSigFSC(FFrame & stack, void *) {
	FString s1, s2;
	int i;
	stack.stepCompIn(&i);
	stack.stepCompIn(&s1);
	stack.stepCompIn(&s2);
	stack.code += !!stack.code;
}

UClass* UFileSystem::staticClass() {
	return Paks::ClassBuilder<UFileSystem>::staticClass();
}

FileSystemManager::FileSystemManager(fs::path root) : root(fs::absolute(root)) {
	fs::create_directories(this->root);
}

fs::path FileSystemManager::getPath(std::string path, bool ec, bool pc, int arg) {
	if (path.length() < 1) throw LuaExceptionArg(arg, "path out of filesystem");
	if (path[1] == '/' || path[1] == '\\') path = path.substr(1);
	fs::path p = fs::absolute(root / path);
	auto ps = p.string();
	if (ps.rfind(root.string(), 0) != 0) throw LuaExceptionArg(arg, "path out of filesystem");
	if (!fs::is_regular_file(p) && !fs::is_directory(p) && fs::exists(p)) throw LuaExceptionArg(arg, "path exists but is no file or directory");
	if ((ec && !fs::exists(p)) || (pc && !fs::exists(p.parent_path()))) throw LuaExceptionArg(arg, "path does not exist");
	return p;
}

std::filesystem::path FileSystemManager::getRoot() const {
	return root;
}

std::unique_ptr<FileSystemFileStream> FileSystemManager::open(std::string path, std::string mode) {
	auto p = getPath(path, false, true, 1);
	auto fsfs = std::unique_ptr<FileSystemFileStream>(new FileSystemFileStream(this));
	fsfs->open(p, mode);
	return fsfs;
}

void FileSystemManager::createDir(std::string path, bool all) {
	auto p = getPath(path, false, !all, 1);
	if (fs::exists(p)) throw LuaExceptionArg(1, "path already exists");
	if (all) fs::create_directories(p);
	else fs::create_directory(p);
}

void FileSystemManager::remove(std::string path, bool all) {
	auto p = getPath(path, true, false, 1);
	if (all) {
		fs::remove_all(p);
	} else if (all) if (!fs::remove(p)) throw LuaException("not able to remove");
}

void FileSystemManager::move(std::string from, std::string to) {
	auto f = getPath(from, true, false, 1);
	auto t = getPath(to, false, true, 2);
	if (!fs::is_directory(t) && fs::exists(t)) throw LuaExceptionArg(2, "path exists but is no directory");
	fs::rename(f, t);
}

bool FileSystemManager::exists(std::string path) {
	return fs::exists(getPath(path, false, false, 0));
}

bool FileSystemManager::isFile(std::string path) {
	return fs::is_regular_file(getPath(path, false, false, 0));
}

bool FileSystemManager::isDir(std::string path) {
	return fs::is_directory(getPath(path, false, false, 0));
}

FileSystemManagerCapacity::FileSystemManagerCapacity(std::filesystem::path root, std::uint64_t capacity) : FileSystemManager(root), capacity(capacity) {
	updateSize();
}

std::uint64_t FileSystemManagerCapacity::getSize(std::string path, int arg) {
	return getSize(getPath(path, true, false, 1));
}

std::uint64_t FileSystemManagerCapacity::getSize(std::filesystem::path path) {
	return getSize(std::filesystem::directory_entry(path));
}

std::uint64_t FileSystemManagerCapacity::getSize(std::filesystem::directory_entry e) {
	std::uint64_t count = 0;
	if (e.is_directory()) {
		count += e.path().filename().string().length();
		for (auto& i : std::filesystem::directory_iterator(e)) {
			count += getSize(i);
		}
	} else if (e.is_regular_file()) {
		count += e.path().filename().string().length();
		count += e.file_size();
	}
	return count;
}

std::uint64_t FileSystemManagerCapacity::updateSize() {
	return used = getSize(root) - root.filename().string().length();
}

void FileSystemManagerCapacity::checkNewSpace(std::uint64_t size) {
	if (used + size > capacity) throw LuaException("out of storage capacity");
}

std::unique_ptr<FileSystemFileStream> FileSystemManagerCapacity::open(std::string path, std::string mode) {
	auto p = getPath(path, false, true, 1);
	auto fsfs = std::unique_ptr<FileSystemFileStream>(new FileSystemFileStreamCapacity(this));
	fsfs->open(p, mode);
	return fsfs;
}

void FileSystemManagerCapacity::createDir(std::string path, bool all) {
	auto p = getPath(path, false, !all, 1);
	std::uint64_t space = 0;
	if (all) {
		auto np = p;
		while (!fs::exists(np)) {
			space += np.filename().string().length();
			np = np.parent_path();
		}
	} else {
		space = p.filename().string().length();
	}
	updateSize();
	checkNewSpace(space);
	FileSystemManager::createDir(path, all);
	used += space;
}

void FileSystemManagerCapacity::remove(std::string path, bool all) {
	FileSystemManager::remove(path, all);
	updateSize();
}


void FileSystemManagerCapacity::move(std::string from, std::string to) {
	auto f = getPath(from, true, false, 1);
	auto t = getPath(to, false, true, 2);
	auto fl = t.filename().string().length();
	auto tl = f.filename().string().length();
	updateSize();
	if (tl > fl) checkNewSpace(tl - fl);
	FileSystemManager::move(from, to);
	used += tl - fl;
}

FileSystemFileStream::FileSystemFileStream(FileSystemManager * owner) : owner(owner) {}

void FileSystemFileStream::checkOpen() {
	if (!stream.is_open()) throw LuaException("file is closed");
}

std::fstream::openmode FileSystemFileStream::getMode(std::string mode) {
	std::fstream::openmode m = std::fstream::in;
	if (mode.length() > 0) {
		if (mode == "r") m = std::fstream::in;
		else if (mode == "w") m = std::fstream::out;
		else if (mode == "a") m = std::fstream::out | std::fstream::ate;
		//else if (mode == "r+") m = std::fstream::out;
		else if (mode == "w+") m = std::fstream::out | std::fstream::trunc;
		else if (mode == "a+") m = std::fstream::out | std::fstream::app;
		else throw LuaExceptionArg(3, "invalid mode");
	}
	return m;
}

void FileSystemFileStream::open(std::filesystem::path path, std::string mode) {
	this->path = path;
	
	auto m = getMode(mode);
	auto e = fs::exists(path);
	if (e && !fs::is_regular_file(path)) throw LuaExceptionArg(1, "path is not valid file");
	stream.open(path, m);
	this->mode = m;
}

void FileSystemFileStream::write(std::string str) {
	checkOpen();
	stream << str;
}

void FileSystemFileStream::flush() {
	checkOpen();
	stream.flush();
}

std::string FileSystemFileStream::readChars(size_t chars) {
	checkOpen();
	char* buf = new char[chars];
	try {
		stream.read(buf, chars);
	} catch (std::ios::failure e) {
		delete[] buf;
		throw e;
	}
	std::string s(buf);
	delete[] buf;
	return s;
}

std::string FileSystemFileStream::readLine() {
	checkOpen();
	std::string s;
	std::getline(stream, s);
	return s;
}

std::string FileSystemFileStream::readAll() {
	checkOpen();
	std::stringstream s;
	s << stream.rdbuf();
	return s.str();
}

double FileSystemFileStream::readNumber() {
	checkOpen();
	double n = 0.0;
	stream >> n;
	return n;
}

std::int64_t FileSystemFileStream::seek(std::string str, std::int64_t off) {
	checkOpen();
	auto w = std::fstream::cur;
	if (str == "set") w = std::fstream::beg;
	else if (str == "cur") w = std::fstream::cur;
	else if (str == "end") w = std::fstream::end;
	else throw LuaExceptionArg(2, "no valid whence");
	return stream.seekg(off, w).tellg();
}

void FileSystemFileStream::close() {
	checkOpen();
	stream.close();
}

bool FileSystemFileStream::isEOF() {
	return stream.eof();
}

FileSystemFileStreamCapacity::FileSystemFileStreamCapacity(FileSystemManagerCapacity * owner) : FileSystemFileStream((FileSystemManager*)owner) {}

void FileSystemFileStreamCapacity::open(fs::path path, std::string mode) {
	((FileSystemManagerCapacity*)owner)->updateSize();
	auto m = getMode(mode);
	if (m & std::fstream::out && !fs::exists(path)) ((FileSystemManagerCapacity*)owner)->checkNewSpace(path.filename().string().length());
	FileSystemFileStream::open(path, mode);
	((FileSystemManagerCapacity*)owner)->updateSize();
}

void FileSystemFileStreamCapacity::write(std::string str) {
	std::int64_t space = 0;
	if (mode & std::fstream::app) {
		space = str.length();
	} else {
		auto p = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto e = stream.tellg();
		stream.seekg(p);
		auto o = e - p;
		auto s = std::string(str).length();
		space = s - o;
	}
	if (space > 0) ((FileSystemManagerCapacity*)owner)->checkNewSpace(space);
	FileSystemFileStream::write(str);
}

UFileSystem * IFileSystemLua::self() const {
	return (UFileSystem*)((size_t)this - offsetof(UFileSystem, luaImpl));;
}

void IFileSystemLua::luaAddSignalListener(ULuaContext * ctx) {
	auto s = self();
	if (!(std::find(s->listeners.begin(), s->listeners.end(), ctx) != s->listeners.end())) {
		self()->listeners.add(ctx);
	}
}

void IFileSystemLua::luaRemoveSignalListener(ULuaContext * ctx) {
	auto s = self();
	int i = 0;
	for (auto e : s->listeners) {
		if (e == ctx) {
			s->listeners.remove(i);
			break;
		}
	}
}

SML::Objects::TArray<ULuaContext*> IFileSystemLua::luaGetSignalListeners() {
	return self()->listeners;
}

bool IFileSystemLua::luaIsReachableFrom(SML::Objects::UObject * listener) {
	return true;
	INetworkComponent* comp;
	try {
		comp = ((INetworkComponent*)((size_t)self() + ((SML::Objects::UObject*)self())->clazz->getImplementation(UNetworkComponent::staticClass()).off));
	} catch (...) {
		return false;
	}
	INetworkComponent* lcomp;
	try {
		auto c = (SML::Objects::UObject*) ((ULuaContext*)listener)->component;
		lcomp = ((INetworkComponent*)((size_t)c + (c)->clazz->getImplementation(UNetworkComponent::staticClass()).off));
	} catch (...) {
		return false;
	}
	volatile bool found = comp->findComponent(lcomp->getID());
	return found;
}

bool IFileSystemSaveInterface::NeedTransform() {
	return false;
}

bool IFileSystemSaveInterface::ShouldSave() {
	return true;
}

void IFileSystemSaveInterface::gatherDeps(SML::Objects::TArray<SML::Objects::UObject*>*) {}

void IFileSystemSaveInterface::postLoad(int, int) {}

void IFileSystemSaveInterface::preLoad(int, int) {}

void IFileSystemSaveInterface::postSave(int, int) {}

void IFileSystemSaveInterface::preSave(int, int) {}

SML::Objects::UObject * IFileSystemSaveInterface::_getUObject() const { return nullptr; }

