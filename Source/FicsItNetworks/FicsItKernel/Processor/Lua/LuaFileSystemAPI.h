#pragma once

extern "C" {
	#include "ThirdParty/lua.h"
	#include "ThirdParty/lauxlib.h"
	#include "ThirdParty/lualib.h"
}

#include "Lua.h"
#include "FicsItKernel/FicsItKernel.h"

namespace FicsItKernel {
	namespace Lua {
		class LuaProcessor;

		struct LuaFilePersistTransfer : FileSystem::ReferenceCounted {
			int pos;
			FileSystem::FileMode mode;
			bool open;
		};
		
		struct LuaFileContainer : FileSystem::ReferenceCounted {
			FileSystem::SRef<FileSystem::FileStream> file;
			std::string path;
			FileSystem::SRef<LuaFilePersistTransfer> transfer;
		};

		typedef FileSystem::SRef<LuaFileContainer> LuaFile;

		/**
		 * Creates a lua representation of the given file stream and pushes it to the given Lua stack.
		 *
		 * @param L		- the lua state you want to add the representation to
		 * @param file	- the file stream you want to add to the Lua stack
		 * @param path	- the path to the file opened by the filestream (needed for persistency)
		 */
		void luaFile(lua_State* L, FileSystem::SRef<FileSystem::FileStream> file, const std::string& path);

		/**
		* Adds the FileSystem API to the top stack entry if it is a table in the given Lua state.
		* 
		* @param[in]	filesystem	The filesystem you want to bind the api with.
		* @param[in]	ctx			The lua state you want to add the FileSystem API to. Make sure the top stack entry is a table.
		*/
		void setupFileSystemAPI(lua_State* ctx);
	}
}