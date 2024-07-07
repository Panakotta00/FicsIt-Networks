#include "FINLua/API/LuaFileSystemAPI.h"

#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaExtraSpace.h"
#include "FINLua/LuaPersistence.h"

#define LuaFunc() \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	FLuaSyncCall SyncCall(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FFINKernelFSRoot* self = kernel->GetFileSystem(); \
	if (!self) return luaL_error(L, "component is invalid");

#define LuaFileFunc() \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	FLuaSyncCall SyncCall(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	LuaFile* self_r = (LuaFile*)luaL_checkudata(L, 1, "File"); \
	if (!self_r) return luaL_error(L, "file is invalid"); \
	LuaFile& self = *self_r; \
	if (self->transfer) { \
		CodersFileSystem::FileMode mode; \
		if (self->transfer->open) { \
			mode = self->transfer->mode; \
			mode = mode & ~CodersFileSystem::FileMode::TRUNC; \
		} else mode = CodersFileSystem::INPUT; \
		self->file = kernel->GetFileSystem()->open(kernel->GetFileSystem()->unpersistPath(self->path), mode); \
		if (self->transfer->open) { \
			self->file->seek("set", self->transfer->pos); \
			self->transfer = nullptr; \
		} else { \
			self->file->close(); \
		} \
        TArray<LuaFile>& streams = FINLua::luaFIN_getExtraSpace(L).FileStreams; \
		streams.Add(self); \
	} \
	CodersFileSystem::SRef<CodersFileSystem::FileStream>& file = self->file; \
	if (!file) { \
		return luaL_error(L, "filestream not open"); \
	}

#define CatchExceptionLua \
	catch (const std::exception& ex) { \
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning); \
		return luaL_error(L, ex.what()); \
	}

namespace FINLua {
#define LOCTEXT_NAMESPACE "FileSystemModule"
	BeginLuaModule(FileSystem, LOCTEXT("DisplayName", "File-System Module"), LOCTEXT("Description", ""))
#define LOCTEXT_NAMESPACE "FileSystemLibrary"
	BeginLibrary(filesystem, LOCTEXT("DisplayName", "File-System Library"), LOCTEXT("Description", ""))

	FieldFunction(makeFileSystem, LOCTEXT("makeFileSystem_DisplayName", "Make File-System"), LOCTEXT("makeFileSystem_Description", "")) {
		LuaFunc()

		const std::string type = luaL_checkstring(L, 1);
		const std::string name = luaL_checkstring(L, 2);
		CodersFileSystem::SRef<CodersFileSystem::Device> device;
		if (type == "tmpfs") {
			device = new CodersFileSystem::MemDevice();
		} else return luaL_argerror(L, 1, "No valid FileSystem Type");
		if (!device.isValid()) luaL_argerror(L, 1, "Invalid Device");
		CodersFileSystem::SRef<FFINKernelFSDevDevice> dev = self->getDevDevice();
		try {
			lua_pushboolean(L, dev.isValid() ? dev->addDevice(device, name) : false);
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(removeFileSystem, LOCTEXT("removeFileSystem_DisplayName", "Remove File-System"), LOCTEXT("removeFileSystem_Description", "")) {
		LuaFunc();

		const std::string name = luaL_checkstring(L, 1);
		const CodersFileSystem::SRef<FFINKernelFSDevDevice> dev = self->getDevDevice();
		if (dev.isValid()) {
			try {
				const auto devices = dev->getDevices();
				const auto device = devices.find(name);
				if (device != devices.end() && dynamic_cast<CodersFileSystem::MemDevice*>(device->second.get())) {
					lua_pushboolean(L, dev->removeDevice(device->second));
					return UFINLuaProcessor::luaAPIReturn(L, 1);
				}
			} CatchExceptionLua
		}
		lua_pushboolean(L, false);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(initFileSystem, LOCTEXT("initFileSystem_DisplayName", "Init File-System"), LOCTEXT("initFileSystem_Description", "")) {
		LuaFunc();

		const std::string path = luaL_checkstring(L, 1);
		lua_pushboolean(L, kernel->InitFileSystem(CodersFileSystem::Path(path)));
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(open, LOCTEXT("open_DisplayName", "Open"), LOCTEXT("open_Description", "")) {
		LuaFunc();

		FString Mode = "r";
		if (lua_isstring(L, 2)) Mode = FString(lua_tostring(L, 2));
		CodersFileSystem::FileMode m;
		if (Mode.Contains("+r")) m = CodersFileSystem::INPUT | CodersFileSystem::OUTPUT;
		else if (Mode.Contains("+w")) m = CodersFileSystem::INPUT | CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC;
		else if (Mode.Contains("+a")) m = CodersFileSystem::INPUT | CodersFileSystem::OUTPUT | CodersFileSystem::APPEND;
		else if (Mode.Contains("r")) m = CodersFileSystem::INPUT;
		else if (Mode.Contains("w")) m = CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC;
		else if (Mode.Contains("a")) m = CodersFileSystem::OUTPUT | CodersFileSystem::APPEND;
		else return luaL_argerror(L, 2, "is not valid file mode");
		if (Mode.Contains("b")) m = m | CodersFileSystem::BINARY;
		try {
			const CodersFileSystem::Path path = CodersFileSystem::Path(luaL_checkstring(L, 1));
			const CodersFileSystem::SRef<CodersFileSystem::FileStream> stream = self->open(path, m);
			luaFIN_pushFile(L, stream, stream ? self->persistPath(path) : "");
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(createDir, LOCTEXT("createDir_DisplayName", ""), LOCTEXT("createDir_Description", "")) {
		LuaFunc();

		const std::string path = luaL_checkstring(L, 1);
		const bool all = (bool)lua_toboolean(L, 2);
		try {
			lua_pushboolean(L, self->createDir(CodersFileSystem::Path(path), all).isValid());
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(remove, LOCTEXT("remove_DisplayName", "Remove"), LOCTEXT("remove_Description", "")) {
		LuaFunc();

		const CodersFileSystem::Path path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const bool all = (bool)lua_toboolean(L, 2);
		try {
			lua_pushboolean(L, self->remove(path, all));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(move, LOCTEXT("move_DisplayName", "Move"), LOCTEXT("move_Description", "")) {
		LuaFunc();

		const auto from = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const auto to = CodersFileSystem::Path(luaL_checkstring(L, 2));
		try {
			lua_pushboolean(L, self->move(from, to) == 0);
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(rename, LOCTEXT("rename_DisplayName", "Rename"), LOCTEXT("rename_Description", "")) {
		LuaFunc();

		const auto from = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const auto to = std::string(luaL_checkstring(L, 2));
		try {
			lua_pushboolean(L, self->rename(from, to));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(exists, LOCTEXT("exists_DisplayName", "Exists"), LOCTEXT("exists_Description", "")) {
		LuaFunc();

		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, self->get(path).isValid());
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(children, LOCTEXT("children_DisplayName", "Children"), LOCTEXT("children_Description", "")) {
		LuaFunc();

		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		std::unordered_set<std::string> childs;
		try {
			childs = self->childs(path);
		} CatchExceptionLua
		lua_newtable(L);
		int i = 1;
		for (const auto& child : childs) {
			lua_pushstring(L, child.c_str());
			lua_seti(L, -2, i++);
		}
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(isFile, LOCTEXT("isFile_DisplayName", "Is File"), LOCTEXT("isFile_Description", "")) {
		LuaFunc();
		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, !!dynamic_cast<CodersFileSystem::File*>(self->get(path).get()));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(isDir, LOCTEXT("isDir_DisplayName", "Is Dir"), LOCTEXT("isDir_Description", "")) {
		LuaFunc();
		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, !!dynamic_cast<CodersFileSystem::Directory*>(self->get(path).get()));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(mount, LOCTEXT("mount_DisplayName", "Mount"), LOCTEXT("mount_Description", "")) {
		LuaFunc();
		const auto devPath = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const auto mountPath = CodersFileSystem::Path(luaL_checkstring(L, 2));
		try {
			lua_pushboolean(L, CodersFileSystem::DeviceNode::mount(*self, devPath, mountPath));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(unmount, LOCTEXT("unmount_DisplayName", "Un-Mount"), LOCTEXT("unmount_Description", "")) {
		LuaFunc();
		const auto mountPath = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, self->unmount(mountPath));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}
	
	static int luaDoFileCont(lua_State *L, int d1, lua_KContext d2) {
		return lua_gettop(L) - 1;
	}

	FieldFunction(doFile, LOCTEXT("doFile_DisplayName", "Do File"), LOCTEXT("doFile_Description", "")) {
		LuaFunc();

		const CodersFileSystem::Path path(luaL_checkstring(L, 1));
		CodersFileSystem::SRef<CodersFileSystem::FileStream> file;
		try {
			file = self->open(path, CodersFileSystem::INPUT);
		} CatchExceptionLua
		if (!file.isValid()) return luaL_error(L, "not able to create filestream");
		std::string code;
		try {
			code = CodersFileSystem::FileStream::readAll(file);
		} CatchExceptionLua
		try {
			file->close();
		} CatchExceptionLua
		luaL_loadbufferx(L, code.c_str(), code.size(), ("@" + path.str()).c_str(), "t");
		lua_callk(L, 0, LUA_MULTRET, 0, luaDoFileCont);
		return luaDoFileCont(L, 0, 0);
	}

	FieldFunction(loadFile, LOCTEXT("loadFile_DisplayName", "Load File"), LOCTEXT("loadFile_Description", "")) {
		LuaFunc();

		const CodersFileSystem::Path path = luaL_checkstring(L, 1);
		CodersFileSystem::SRef<CodersFileSystem::FileStream> file;
		try {
			file = self->open(path, CodersFileSystem::INPUT);
		} CatchExceptionLua
		if (!file.isValid()) return luaL_error(L, "not able to create filestream");
		std::string code;
		try {
			code = CodersFileSystem::FileStream::readAll(file);
		} CatchExceptionLua
		try {
			file->close();
		} CatchExceptionLua
		
		luaL_loadbufferx(L, code.c_str(), code.size(), ("@" + path.str()).c_str(), "t");
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(path, LOCTEXT("path_DisplayName", "Path"), LOCTEXT("path_Description", "")) {
		LuaFunc();

		int args = lua_gettop(L);
		int start = 1;
		int conversion = -1;
		if (lua_isinteger(L, start)) conversion = lua_tointeger(L, start++);
		CodersFileSystem::Path path;
		for (int i = start; i <= args; ++i) path = path / CodersFileSystem::Path(luaL_checkstring(L, i));
		std::string out;
		int retargs = 1;
		bool custom = false;
		switch (conversion) {
		case 0:
			out = path.normalize().str();
			break;
		case 1:
			out = path.absolute().str();
			break;
		case 2:
			out = path.relative().str();
			break;
		case 3:
			out = path.fileName();
			break;
		case 4:
			out = path.fileStem();
			break;
		case 5:
			out = path.fileExtension();
			break;
		default:
			out = path.str();
		}
		if (!custom) lua_pushstring(L,out.c_str());
		return UFINLuaProcessor::luaAPIReturn(L, retargs);
	}

	FieldFunction(analyzePath, LOCTEXT("analyzePath_DisplayName", "Analyze Path"), LOCTEXT("analyzePath_Description", "")) {
		LuaFunc();

		int args = lua_gettop(L);
		for (int i = 1; i <= args; ++i) {
			CodersFileSystem::Path path = lua_tostring(L, i);
			int flags = 0;
			if (path.isRoot())						flags |= 0b000001;
			if (path.isEmpty())						flags |= 0b000010;
			if (path.isAbsolute())					flags |= 0b000100;
			if (path.isSingle())					flags |= 0b001000;
			if (path.fileExtension().size() > 0)	flags |= 0b010000;
			if (path.isDir())						flags |= 0b100000;
			lua_pushinteger(L, flags);
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}

	FieldFunction(isNode, LOCTEXT("isNode_DisplayName", "Is Node"), LOCTEXT("isNode_Description", "")) {
		LuaFunc();
		int args = lua_gettop(L);
		for (int i = 1; i <= args; ++i) {
			lua_pushboolean(L, CodersFileSystem::Path::isNode(lua_tostring(L, i)));
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}

	FieldFunction(meta, LOCTEXT("meta_DisplayName", "Meta"), LOCTEXT("meta_Description", "")) {
		LuaFunc();

		int args = lua_gettop(L);
		for (int i = 1; i <= args; ++i) {
			CodersFileSystem::Path Path = lua_tostring(L, i);
			CodersFileSystem::SRef<CodersFileSystem::Node> Node = self->get(Path);
			if (!Node.isValid()) {
				lua_pushnil(L);
				continue;
			}
			lua_newtable(L);
			if (dynamic_cast<CodersFileSystem::File*>(Node.get())) {
				lua_pushstring(L, "File");
			} else if (dynamic_cast<CodersFileSystem::Directory*>(Node.get())) {
				lua_pushstring(L, "Directory");
			} else if (dynamic_cast<CodersFileSystem::DeviceNode*>(Node.get())) {
				lua_pushstring(L, "Device");
			} else {
				lua_pushstring(L, "Unknown");
			}
			lua_setfield(L, -2, "type");
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	}

	EndLibrary()

#define LOCTEXT_NAMESPACE "FileSystem-File"
	BeginMetatable(File, LOCTEXT("DisplayName", "File"), LOCTEXT("Description", ""))

	FieldFunction(close, LOCTEXT("close_DisplayName", "Close"), LOCTEXT("close_Description", "")) {
		LuaFileFunc();

		try {
			file->close();
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(write, LOCTEXT("write_DisplayName", "Write"), LOCTEXT("write_Description", "")) {
		LuaFileFunc();

		const auto s = lua_gettop(L);
		for (int i = 2; i <= s; ++i) {
			size_t str_len = 0;
			const char* str = luaL_checklstring(L, i, &str_len);
			try {
				file->write(std::string(str, str_len));
			} CatchExceptionLua
		}
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(read, LOCTEXT("read_DisplayName", "Read"), LOCTEXT("read_Description", "")) {
		LuaFileFunc();

		const auto args = lua_gettop(L);
		for (int i = 2; i <= args; ++i) {
			try {
				const auto n = lua_tointeger(L, i);
				std::string s = file->read(n);
				if (s.size() == 0 && file->isEOF()) lua_pushnil(L);
				else lua_pushlstring(L, s.c_str(), s.size());
			} catch (const std::exception& ex) {
				luaL_error(L, ex.what());
			}
		}
		return UFINLuaProcessor::luaAPIReturn(L, args - 1);
	}

	FieldFunction(seek, LOCTEXT("seek_DisplayName", "Seek"), LOCTEXT("seek_Description", "")) {
		LuaFileFunc();

		LuaFile& f = *static_cast<LuaFile*>(luaL_checkudata(L, 1, "File"));
		std::string w = "cur";
		std::int64_t off = 0;
		if (lua_isstring(L, 2)) w = lua_tostring(L, 2);
		if (lua_isinteger(L, 3)) off = lua_tointeger(L, 3);
		int64_t seek;
		try {
			seek = f->file->seek(w, off);
		} CatchExceptionLua
		lua_pushinteger(L, seek);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(__tostring, LOCTEXT("string_DisplayName", "To String"), LOCTEXT("string_Description", "")) {
		LuaFileFunc();

		lua_pushstring(L, self->path.c_str());
		return 1;
	}

	int luaFileUnpersist(lua_State* L) {
		UFINKernelSystem* kernel = UFINLuaProcessor::luaGetProcessor(L)->GetKernel();
		const bool valid = (bool)lua_toboolean(L, lua_upvalueindex(1));
		std::string path = "";
		if (valid) {
			path = lua_tostring(L, lua_upvalueindex(2));
			const bool open = (bool)lua_toboolean(L, lua_upvalueindex(3));
			CodersFileSystem::SRef<CodersFileSystem::FileStream> stream;
			if (open) {
				path = kernel->GetFileSystem()->unpersistPath(path).str();
				CodersFileSystem::FileMode mode = static_cast<CodersFileSystem::FileMode>(static_cast<int>(lua_tonumber(L, lua_upvalueindex(4))));
				mode = mode & ~CodersFileSystem::FileMode::TRUNC;
				stream = kernel->GetFileSystem()->open(CodersFileSystem::Path(path), mode);
				stream->seek("set", static_cast<int>(lua_tonumber(L, lua_upvalueindex(5))));
			} else {
				path = "";
				stream = nullptr;
			}
			luaFIN_pushFile(L, stream, path);
		} else luaFIN_pushFile(L, nullptr, path);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(__persist, LOCTEXT("persist_DisplayName", "Persist"), LOCTEXT("persist_Description", "")) {
		LuaFile& f = *static_cast<LuaFile*>(luaL_checkudata(L, -1, "File"));
		if (f->transfer) {
			lua_pushboolean(L, true);
			lua_pushstring(L, f->path.c_str());
			lua_pushboolean(L, f->transfer->open);
			lua_pushinteger(L, f->transfer->mode);
			lua_pushinteger(L, f->transfer->pos);
			lua_pushcclosure(L, luaFileUnpersist, 5);
		} else {
			lua_pushboolean(L, false);
			lua_pushstring(L, f->path.c_str());
			lua_pushcclosure(L, luaFileUnpersist, 2);
		}
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}
	
	FieldFunction(__gc, LOCTEXT("gc_DisplayName", "GC"), LOCTEXT("gc_Description", "")) {
		LuaFile& f = *static_cast<LuaFile*>(luaL_checkudata(L, 1, "File"));
		try {
			if (f->file) f->file->close();
		} CatchExceptionLua
		f.~LuaFile();
		return 0;
	}

	EndMetatable()

	ModulePostSetup() {
		PersistenceNamespace("FileSystem");

		lua_pushcfunction(L, reinterpret_cast<int(*)(lua_State*)>(filesystemLibrary::luaDoFileCont));
		PersistValue("doFileCont");

		luaL_getmetatable(L, "File");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);

		lua_pushcfunction(L, FileMetatable::luaFileUnpersist);
		PersistValue("FileUnpersist");
	}

	EndLuaModule()

	void luaFIN_pushFile(lua_State* L, CodersFileSystem::SRef<CodersFileSystem::FileStream> file, const std::string& path) {
		LuaFile* f = static_cast<LuaFile*>(lua_newuserdata(L, sizeof(LuaFile)));
		luaL_setmetatable(L, "File");
		new (f) LuaFile(new LuaFileContainer());
		f->get()->file = file;
		f->get()->path = path;
		if (file.isValid()) {
			TArray<LuaFile>& streams = FINLua::luaFIN_getExtraSpace(L).FileStreams;
			streams.Add(*f);
		}
		lua_pop(L, 1);
	}
}
