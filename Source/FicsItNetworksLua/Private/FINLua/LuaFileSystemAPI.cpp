#include "FINLua/LuaFileSystemAPI.h"

#include "FINLuaProcessor.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	FLuaSyncCall SyncCall(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FFINKernelFSRoot* self = kernel->GetFileSystem(); \
	if (!self) return luaL_error(L, "component is invalid");
#define LuaFuncEnd() \
}

#define LuaFileFuncName(funcName) luaFile ## funcName
#define LuaFileFunc(funcName) \
int LuaFileFuncName(funcName) (lua_State* L) { \
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
		lua_getfield(L, LUA_REGISTRYINDEX, "FileStreamStorage"); \
        TSet<LuaFile>& streams = *static_cast<TSet<LuaFile>*>(lua_touserdata(L, -1)); \
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
	LuaFunc(makeFileSystem) {
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
	} LuaFuncEnd()

	LuaFunc(removeFileSystem) {
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
	} LuaFuncEnd()

	LuaFunc(initFileSystem) {
		const std::string path = luaL_checkstring(L, 1);
		lua_pushboolean(L, kernel->InitFileSystem(CodersFileSystem::Path(path)));
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(open) {
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
			luaFile(L, stream, stream ? self->persistPath(path) : "");
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(createDir) {
		const std::string path = luaL_checkstring(L, 1);
		const bool all = (bool)lua_toboolean(L, 2);
		try {
			lua_pushboolean(L, self->createDir(CodersFileSystem::Path(path), all).isValid());
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(remove) {
		const CodersFileSystem::Path path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const bool all = (bool)lua_toboolean(L, 2);
		try {
			lua_pushboolean(L, self->remove(path, all));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(move) {
		const auto from = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const auto to = CodersFileSystem::Path(luaL_checkstring(L, 2));
		try {
			lua_pushboolean(L, self->move(from, to) == 0);
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(rename) {
		const auto from = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const auto to = std::string(luaL_checkstring(L, 2));
		try {
			lua_pushboolean(L, self->rename(from, to));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(exists) {
		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, self->get(path).isValid());
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(children) {
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
	} LuaFuncEnd()

	LuaFunc(isFile) {
		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, !!dynamic_cast<CodersFileSystem::File*>(self->get(path).get()));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(isDir) {
		const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, !!dynamic_cast<CodersFileSystem::Directory*>(self->get(path).get()));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(mount) {
		const auto devPath = CodersFileSystem::Path(luaL_checkstring(L, 1));
		const auto mountPath = CodersFileSystem::Path(luaL_checkstring(L, 2));
		try {
			lua_pushboolean(L, CodersFileSystem::DeviceNode::mount(*self, devPath, mountPath));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(unmount) {
		const auto mountPath = CodersFileSystem::Path(luaL_checkstring(L, 1));
		try {
			lua_pushboolean(L, self->unmount(mountPath));
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()
	
	static int luaDoFileCont(lua_State *L, int d1, lua_KContext d2) {
		return lua_gettop(L) - 1;
	}

	LuaFunc(doFile) {
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
	} LuaFuncEnd()

	LuaFunc(loadFile) {
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
	} LuaFuncEnd()
	LuaFunc(path) {
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
	} LuaFuncEnd()
	LuaFunc(analyzePath) {
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
	} LuaFuncEnd()
	LuaFunc(isNode) {
		int args = lua_gettop(L);
		for (int i = 1; i <= args; ++i) {
			lua_pushboolean(L, CodersFileSystem::Path::isNode(lua_tostring(L, i)));
		}
		return UFINLuaProcessor::luaAPIReturn(L, args);
	} LuaFuncEnd()

	LuaFunc(meta) {
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
	} LuaFuncEnd()

	static const luaL_Reg luaFileSystemLib[] = {
		{"makeFileSystem", makeFileSystem},
		{"removeFileSystem", removeFileSystem},
		{"initFileSystem", initFileSystem},
		{"open", open},
		{"createDir", createDir},
		{"remove", remove},
		{"move", move},
		{"rename", rename},
		{"exists", exists},
		{"childs", children},
		{"children", children},
		{"isFile", isFile},
		{"isDir", isDir},
		{"mount", mount},
		{"unmount", unmount},
		{"doFile", doFile},
		{"loadFile", loadFile},
		{"path", path},
		{"analyzePath", analyzePath},
		{"isNode", isNode},
		{nullptr, nullptr}
	};

	LuaFileFunc(Close) {
		try {
			file->close();
		} CatchExceptionLua
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	} LuaFuncEnd()

	LuaFileFunc(Write) {
		const auto s = lua_gettop(L);
		for (int i = 2; i <= s; ++i) {
			size_t str_len = 0;
			const char* str = luaL_checklstring(L, i, &str_len);
			try {
				file->write(std::string(str, str_len));
			} CatchExceptionLua
		}
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	} LuaFuncEnd()

	LuaFileFunc(Read) {
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
	} LuaFuncEnd()

	LuaFileFunc(Seek) {
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
	} LuaFuncEnd()

	LuaFileFunc(String) {
		lua_pushstring(L, self->path.c_str());
		return 1;
	} LuaFuncEnd()

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
			luaFile(L, stream, path);
		} else luaFile(L, nullptr, path);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	int luaFilePersist(lua_State* L) {
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
	
	int luaFileGC(lua_State * L) {
		LuaFile& f = *static_cast<LuaFile*>(luaL_checkudata(L, 1, "File"));
		try {
			if (f->file) f->file->close();
		} CatchExceptionLua
		f.~LuaFile();
		return 0;
	}

	static const luaL_Reg luaFileLib[] = {
		{"close", luaFileClose},
		{"write", luaFileWrite},
		{"read", luaFileRead},
		{"seek", luaFileSeek},
		{"__tostring", luaFileString},
		{"__persist", luaFilePersist},
		{"__gc", luaFileGC},
		{nullptr, nullptr}
	};

	void luaFile(lua_State* L, CodersFileSystem::SRef<CodersFileSystem::FileStream> file, const std::string& path) {
		LuaFile* f = static_cast<LuaFile*>(lua_newuserdata(L, sizeof(LuaFile)));
		luaL_setmetatable(L, "File");
		new (f) LuaFile(new LuaFileContainer());
		f->get()->file = file;
		f->get()->path = path;
		lua_getfield(L, LUA_REGISTRYINDEX, "FileStreamStorage");
		if (file.isValid()) {
			TSet<LuaFile>& streams = *static_cast<TSet<LuaFile>*>(lua_touserdata(L, -1));
			streams.Add(*f);
		}
		lua_pop(L, 1);
	}

	void setupFileSystemAPI(lua_State * L) {
		PersistSetup("FileSystem", -2);
		
		luaL_newlibtable(L, luaFileSystemLib);
		luaL_setfuncs(L, luaFileSystemLib, 0);
		PersistTable("Lib", -1);
		lua_setglobal(L, "filesystem");
		lua_pushcfunction(L, (int(*)(lua_State*))luaDoFileCont);
		PersistValue("doFileCont");

		luaL_newmetatable(L, "File");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		luaL_setfuncs(L, luaFileLib, 0);
		PersistTable("File", -1);
		lua_pop(L, 1);
		lua_pushcfunction(L, luaFileUnpersist);
		PersistValue("FileUnpersist");
	}
}
