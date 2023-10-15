#pragma once

#include "LuaUtil.h"
#include "FicsItKernel/FicsItKernel.h"

namespace FINLua {
	struct LuaFilePersistTransfer : CodersFileSystem::ReferenceCounted {
		int pos;
		CodersFileSystem::FileMode mode;
		bool open;
	};
	
	struct LuaFileContainer : CodersFileSystem::ReferenceCounted {
		CodersFileSystem::SRef<CodersFileSystem::FileStream> file;
		std::string path;
		CodersFileSystem::SRef<LuaFilePersistTransfer> transfer;
	};

	typedef CodersFileSystem::SRef<LuaFileContainer> LuaFile;

	/**
	 * Creates a lua representation of the given file stream and pushes it to the given Lua stack.
	 *
	 * @param L		- the lua state you want to add the representation to
	 * @param file	- the file stream you want to add to the Lua stack
	 * @param path	- the path to the file opened by the filestream (needed for persistency)
	 */
	void luaFile(lua_State* L, CodersFileSystem::SRef<CodersFileSystem::FileStream> file, const std::string& path);

	/**
	* Adds the FileSystem API to the top stack entry if it is a table in the given Lua state.
	* 
	* @param[in]	filesystem	The filesystem you want to bind the api with.
	* @param[in]	ctx			The lua state you want to add the FileSystem API to. Make sure the top stack entry is a table.
	*/
	void setupFileSystemAPI(lua_State* ctx);
}
