#pragma once

#include "FINLua/LuaUtil.h"
#include "FicsItKernel/FicsItKernel.h"

namespace FINLua {
	struct LuaFilePersistTransfer {
		int pos;
		CodersFileSystem::FileMode mode;
		bool open;
	};
	
	struct LuaFileContainer {
		TSharedPtr<CodersFileSystem::FileStream> file;
		std::string path;
		TSharedPtr<LuaFilePersistTransfer> transfer;
	};

	typedef TSharedRef<LuaFileContainer> LuaFile;

	/**
	 * Creates a lua representation of the given file stream and pushes it to the given Lua stack.
	 *
	 * @param L		- the lua state you want to add the representation to
	 * @param file	- the file stream you want to add to the Lua stack
	 * @param path	- the path to the file opened by the filestream (needed for persistency)
	 */
	void luaFIN_pushFile(lua_State* L, TSharedPtr<CodersFileSystem::FileStream> file, const std::string& path);
}
