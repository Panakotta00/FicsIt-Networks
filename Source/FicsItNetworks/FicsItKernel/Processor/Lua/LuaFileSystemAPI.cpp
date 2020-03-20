#include "LuaFileSystemAPI.h"

#include "FicsItKernel/FicsItFS/FileSystem.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	auto self = (FileSystem::FileSystemRoot*)lua_touserdata(L, lua_upvalueindex(1)); \
	if (!self) return luaL_error(L, "component is invalid"); \
	try {
#define LuaEndFunc \
	} catch (std::exception e) { \
		return luaL_error(L, e.what()); \
	} \
}

#define LuaFileFuncName(funcName) luaFile ## funcName
#define LuaFileFunc(funcName) \
int LuaFileFuncName(funcName) (lua_State* L) { \
	auto self = (LuaFile*)luaL_checkudata(L, 1, "File"); \
	if (!self) return luaL_error(L, "file is invalid"); \
	auto file = self->file; \
	if (!file) return luaL_error(L, "file is invalid"); \
	try {
#define LuaEndFileFunc \
	} catch (std::exception e) { \
		return luaL_error(L, e.what()); \
	} \
}

namespace FicsItKernel {
	namespace Lua {
		LuaFunc(open)
			std::string mode = "r";
			if (lua_isstring(L, 2)) mode = lua_tostring(L, 2);
			FileSystem::FileMode m;
			if (mode == "r") m = FileSystem::READ;
			else if (mode == "w") m = FileSystem::WRITE;
			else if (mode == "a") m = FileSystem::APPEND;
			else if (mode == "+r") m = FileSystem::UPDATE_READ;
			else if (mode == "+w") m = FileSystem::UPDATE_WRITE;
			else if (mode == "+a") m = FileSystem::UPDATE_APPEND;
			else luaL_argerror(L, 2, "is not valid file mode");
			luaFile(L, self->open(luaL_checkstring(L, 1), m));
			return 1;
		LuaEndFunc

		LuaFunc(createDir)
			auto path = luaL_checkstring(L, 1);
			auto all = lua_toboolean(L, 2);
			self->createDir(path, all);
			return 0;
		LuaEndFunc

		LuaFunc(remove)
			auto path = luaL_checkstring(L, 1);
			bool all = lua_toboolean(L, 2);
			self->remove(path, all);
			return 0;
		LuaEndFunc

		LuaFunc(move)
			auto from = luaL_checkstring(L, 1);
			auto to = luaL_checkstring(L, 2);
			self->move(from, to);
			return 0;
		LuaEndFunc

		LuaFunc(exists)
			auto path = luaL_checkstring(L, 1);
			lua_pushboolean(L, self->get(path).isValid());
			return 1;
		LuaEndFunc

		LuaFunc(childs)
			auto path = luaL_checkstring(L, 1);
			auto childs = self->childs(path);
			lua_newtable(L);
			int i = 1;
			for (auto& child : childs) {
				lua_pushstring(L, child.c_str());
				lua_seti(L, -2, i++);
			}
			return 1;
		LuaEndFunc

		LuaFunc(isFile)
			auto path = luaL_checkstring(L, 1);
			lua_pushboolean(L, !!dynamic_cast<FileSystem::File*>(self->get(path).get()));
			return 1;
		LuaEndFunc

		LuaFunc(isDir)
			auto path = luaL_checkstring(L, 1);
			lua_pushboolean(L, !!dynamic_cast<FileSystem::Directory*>(self->get(path).get()));
			return 1;
		LuaEndFunc

		LuaFunc(mount)
			auto devPath = luaL_checkstring(L, 1);
			auto mountPath = luaL_checkstring(L, 2);
			lua_pushboolean(L, FileSystem::DeviceNode::mount(*self, devPath, mountPath));
			return 1;
		LuaEndFunc

		LuaFunc(unmount)
			auto mountPath = luaL_checkstring(L, 1);
			lua_pushboolean(L, self->unmount(mountPath));
			return 1;
		LuaEndFunc

		LuaFunc(doFile)
			FileSystem::SRef<FileSystem::File> p = self->get(luaL_checkstring(L, 1));
			if (!p.isValid()) return luaL_argerror(L, 1, "path is no valid file");
			auto s = p->open(FileSystem::READ);
			if (!s.isValid()) return luaL_error(L, "not able to create filestream");
			int n = lua_gettop(L);
			luaL_dofile(L, s->readAll().c_str());
			s->close();
			return lua_gettop(L) - n;
		LuaEndFunc

		LuaFunc(loadFile)
			FileSystem::SRef<FileSystem::File> p = self->get(luaL_checkstring(L, 1));
			if (!p.isValid()) return luaL_argerror(L, 1, "path is no valid file");
			auto s = p->open(FileSystem::READ);
			if (!s.isValid()) return luaL_error(L, "not able to create filestream");
			luaL_loadfile(L, s->readAll().c_str());
			s->close();
			return 1;
		LuaEndFunc

		static const luaL_Reg luaFileSystemLib[] = {
			{"open", open},
			{"createDir", createDir},
			{"remove", remove},
			{"move", move},
			{"exists", exists},
			{"childs", childs},
			{"isFile", isFile},
			{"isDir", isDir},
			{"mount", mount},
			{"unmount", unmount},
			{"doFile", doFile},
			{"loadFile", loadFile},
			{NULL,NULL}
		};

		LuaFileFunc(Close)
			file->close();
			return 0;
		LuaEndFileFunc

		LuaFileFunc(Write)
			auto s = lua_gettop(L);
			for (int i = 2; i <= s; ++i) {
				std::string str = luaL_checkstring(L, i);
				file->write(str);
			}
			return 0;
		LuaEndFileFunc

		LuaFileFunc(Flush)
			file->flush();
			return 0;
		LuaEndFileFunc

		LuaFileFunc(Read)
			auto args = lua_gettop(L);
			for (int i = 2; i <= args; ++i) {
				if (lua_isnumber(L, i)) {
					if (file->isEOF()) lua_pushnil(L);
					auto n = lua_tointeger(L, i);
					lua_pushstring(L, file->readChars(n).c_str());
				} else {
					char fo = 'l';
					if (lua_isstring(L, i)) {
						std::string s = lua_tostring(L, i);
						if (s.size() != 2 || s[0] != '*') fo = '\0';
						fo = s[1];
					}
					switch (fo) {
					case 'n':
					{
						lua_pushnumber(L, file->readNumber());
						break;
					} case 'a':
					{
						lua_pushstring(L, file->readAll().c_str());
						break;
					} case 'l':
					{
						if (!file->isEOF()) lua_pushstring(L, file->readLine().c_str());
						else lua_pushnil(L);
						break;
					}
					default:
						luaL_argerror(L, i, "no valid format");
					}
				}
			}
			return args - 1;
		LuaEndFileFunc

		LuaFileFunc(ReadLine)
			if (file->isEOF()) lua_pushnil(L);
			else lua_pushstring(L, file->readLine().c_str());
			return 1;
		LuaEndFileFunc

		int luaFileLines(lua_State * L) {
			auto f = (LuaFile*)luaL_checkudata(L, 1, "File");
			lua_pushcclosure(L, luaFileReadLine, 1);
			return 1;
		}

		int luaFileSeek(lua_State * L) {
			auto f = (LuaFile*)luaL_checkudata(L, 1, "File");
			std::string w = "cur";
			std::int64_t off = 0;
			if (lua_isstring(L, 2)) w = lua_tostring(L, 2);
			if (lua_isinteger(L, 3)) off = lua_tointeger(L, 3);
			try {
				lua_pushinteger(L, f->file->seek(w, off));
			} catch (LuaException & e) { return e.lua(L); }
			return 1;
		}

		int luaFileString(lua_State * L) {
			auto f = (LuaFile*)luaL_checkudata(L, 1, "File");
			try {
				lua_pushstring(L, f->file->readAll().c_str());
			} catch (LuaException & e) { return e.lua(L); }
			return 1;
		}

		int luaFileGC(lua_State * L) {
			auto f = (LuaFile*)luaL_checkudata(L, 1, "File");
			try {
				f->file->close();
			} catch (LuaException & e) { return e.lua(L); }
			f->~LuaFile();
			return 0;
		}

		static const luaL_Reg luaFileLib[] = {
			{"close", luaFileClose},
			{"write", luaFileWrite},
			{"flush", luaFileFlush},
			{"read", luaFileRead},
			{"lines", luaFileLines},
			{"seek", luaFileSeek},
			{"__tostring", luaFileString},
			{"__gc", luaFileGC},
			{NULL, NULL}
		};

		void luaFile(lua_State* L, FileSystem::SRef<FileSystem::FileStream> file) {
			if (!file.isValid()) {
				lua_pushnil(L);
				return;
			}
			auto f = (LuaFile*)lua_newuserdata(L, sizeof(LuaFile));
			luaL_setmetatable(L, "File");
			new (f) LuaFile{std::move(file)};
		}

		void setupFileSystemAPI(FileSystem::FileSystemRoot* filesystem, lua_State * L) {
			auto& fs_ud = *(FileSystem::FileSystemRoot**)lua_newuserdata(L, sizeof(void*));
			fs_ud = filesystem;
			luaL_setfuncs(L, luaFileSystemLib, 1);

			luaL_newmetatable(L, "File");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			luaL_setfuncs(L, luaFileLib, 0);
			lua_pop(L, 1);
		}
	}
}
