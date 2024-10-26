#include "FINLua/API/LuaFileSystemAPI.h"

#include "FINLuaProcessor.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaExtraSpace.h"
#include "FINLua/LuaPersistence.h"

#undef LuaFunc
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
		self->file = kernel->GetFileSystem()->open(self->path, mode); \
		if (self->transfer->open) { \
			self->file->seek("set", self->transfer->pos); \
			self->transfer = nullptr; \
		} else { \
			self->file->close(); \
		} \
        TArray<LuaFile>& streams = FINLua::luaFIN_getExtraSpace(L).FileStreams; \
		streams.Add(self); \
	} \
	TSharedPtr<CodersFileSystem::FileStream>& file = self->file; \
	if (!file) { \
		return luaL_error(L, "filestream not open"); \
	}

#define CatchExceptionLua \
	catch (const std::exception& ex) { \
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning); \
		return luaL_error(L, ex.what()); \
	}

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		FileSystem
	 * @DisplayName		File-System Module
	 *
	 * FicsIt-Networks implements it’s own virtual filesystem for UNIX like experience using the FileSystem.
	 * The file system consists of nodes. Such a node can be a File, a Folder or something else. Each nodes is able to have child nodes and might be able to get opened with a file stream. A folder also delivers an abstract interface for adding and removing nodes. A Device implements lookup functions for searching for nodes in the filesystem structure. Such a device can be a temporary filesystem (tmpfs) which exists only in memory and which will get purged on a system reboot, or f.e. a disk filesystem (drives) which store the nodes on the real virtual filesystem in a folder. You can then mount such a device to a location in the root filesystem tree so when you now access a path, the filesystem looks first for the device and uses then the remaining path to get the node data from the device.
	 * There is also a DriveNode which simply holds a reference to a drive so you can access a drive via the filesystem. You can use then the mount function to mount the Drive of a DriveNode to the given location in the filesystem tree.
	 * Because you need at least one drive mounted to even be able to access any node, we provide the initFileSystem function which you can call only once in a system session. This functions will then mount the DevDevice to the given location.
	 * The DevDevice is a sepcial Device which holds all the DeviceNodes representing Device attached to the computer session, like hard drives, tempfs and the serial I/O.
	 */)", FileSystem) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		filesystem
		 * @DisplayName		File-System Library
		 *
		 * The filesystem api provides structures, functions and variables for interacting with the virtual file systems.
		 * You can't access files outside the virtual filesystem. If you try to do so, the Lua runtime crashes.
		 */)", filesystem) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	makeFileSystem(type: string, name: string)
			 * @DisplayName		Make File-System
			 *
			 * Trys to create a new file system of the given type with the given name.
			 * The created filesystem will be added to the system DevDevice.
			 *
			 * .Possible Types:
			 * [%collapsible]
			 * ====
			 * * `tmpfs`
			 * +
			 * A temporary filesystem only existing at runtime in the memory of your computer. All data will be lost when the system stops.
			 * ====
			 *
			 * @parameter	type		string	Type		the type of the new filesystem
			 * @parameter	name		string	Name		the name of the new filesystem you want to create
			 * @return		success		bool	Success		returns true if it was able to create the new filesystem
			 */)", makeFileSystem) {
				LuaFunc()

				const std::string type = luaL_checkstring(L, 1);
				const std::string name = luaL_checkstring(L, 2);
				TSharedPtr<CodersFileSystem::Device> device;
				if (type == "tmpfs") {
					return luaL_argerror(L, 1, "Temp Filesystems are not possible anymore till a complete FileSystem Overhaul.");
				} else return luaL_argerror(L, 1, "No valid FileSystem Type");
				/*if (!device.IsValid()) luaL_argerror(L, 1, "Invalid Device");
				TSharedPtr<FFINKernelFSDevDevice> dev = kernel->GetDevDevice();
				try {
					lua_pushboolean(L, dev.IsValid() ? dev->addDevice(device, name) : false);
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);*/
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	removeFileSystem(name: string)
			 * @DisplayName		Remove File-System
			 *
			 * Tries to remove the filesystem with the given name from the system DevDevice.
			 * All mounts of the device will run invalid.
			 *
			 * @parameter	name		string	Name		the name of the new filesystem you want to remove
			 * @return		success		bool	Success		returns true if it was able to remove the new filesystem
			 */)", removeFileSystem) {
				LuaFunc();

				luaL_error(L, "Other FileSystems are not supported anymore until a complete FileSystem Overhaul");

				const std::string name = luaL_checkstring(L, 1);
				TSharedPtr<FFINKernelFSDevDevice> dev = kernel->GetDevDevice();
				if (dev.IsValid()) {
					try {
						const auto devices = dev->getDevices();
						const auto device = devices.find(name);
						/*if (device != devices.end() && dynamic_cast<CodersFileSystem::MemDevice*>(device->second.get())) {
							lua_pushboolean(L, dev->removeDevice(device->second));
							return UFINLuaProcessor::luaAPIReturn(L, 1);
						}*/
					} CatchExceptionLua
				}
				lua_pushboolean(L, false);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	initFileSystem(path: string)
			 * @DisplayName		Init File-System
			 *
			 * Trys to mount the system DevDevice to the given location.
			 * The DevDevice is special Device holding DeviceNodes for all filesystems added to the system. (like TmpFS and drives). It is unmountable as well as getting mounted a seccond time.
			 *
			 * @parameter	path		string	Path		path to the mountpoint were the dev device should get mounted to
			 * @return		success		bool	Success		returns if it was able to mount the DevDevice
			 */)", initFileSystem) {
				LuaFunc();

				const std::string path = luaL_checkstring(L, 1);
				lua_pushboolean(L, kernel->InitFileSystem(CodersFileSystem::Path(path)));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		File	open(path: string, mode: string)
			 * @DisplayName		Open
			 *
			 * Opens a file-stream and returns it as File-table.
			 *
			 * .Possible Modes
			 * [%collapsible]
			 * ====
			 * [cols="1,1,4a"]
			 * |===
			 * | `r` | read only
			 * | File-Stream can just read from file. +
			 *   If file doesn’t exist, open will return nil
			 * | `w` | write
			 * | File-Stream can read and write. +
			 *   Creates the file if it doesn’t exist
			 * | `a` | end of file
			 * | File-Stream can read and write. +
			 *   Cursor is set to the end of file.
			 * | `+r` | truncate
			 * | File-Stream can read and write. +
			 *   All previous data in file gets dropped
			 * | `+a` | append
			 * | File-Stream can read the full file, +
			 *   but can only write to the end of the existing file.
			 * |===
			 * ====
			 *
			 * @parameter	path	string	Path	the path to the file you want to open a file-stream for
			 * @parameter	mode	string	Mode	The mode for the file stream
			 * @return		file	File	File	The File table of the file stream. Nil if not able to open file in read only.
			 */)", open) {
				LuaFunc();

				FString Mode = "r";
				if (lua_isstring(L, 2)) Mode = luaFIN_toFString(L, 2);
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
					const TSharedPtr<CodersFileSystem::FileStream> stream = self->open(path, m);
					luaFIN_pushFile(L, stream, path);
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	createDir(path: string, recursive: bool)
			 * @DisplayName		Create Directory
			 *
			 * Creates the folder path.
			 *
			 * @parameter	path		string	Path		folder path the function should create
			 * @parameter	recursive	bool	Recursive	If false creates only the last folder of the path. If true creates all folders in the path.
			 * @return		success		bool	Success		Returns true if it was able to create the directory.
			 */)", createDir) {
				LuaFunc();

				const std::string path = luaL_checkstring(L, 1);
				const bool all = (bool)lua_toboolean(L, 2);
				try {
					lua_pushboolean(L, self->createDir(CodersFileSystem::Path(path), all));
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	remove(path: string, recursive: bool)
			 * @DisplayName		Remove
			 *
			 * Removes the filesystem object at the given path.
			 *
			 * @parameter	path		string	Path		path to the filesystem object
			 * @parameter	recusive	bool	Recursive	If false only removes the given filesystem object. If true removes all childs of the filesystem object.
			 * @return		success		bool	Success		Returns true if it was able to remove the node
			 */)", remove) {
				LuaFunc();

				const CodersFileSystem::Path path = CodersFileSystem::Path(luaL_checkstring(L, 1));
				const bool all = (bool)lua_toboolean(L, 2);
				try {
					lua_pushboolean(L, self->remove(path, all));
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	move(from: string, to: string)
			 * @DisplayName		Move
			 *
			 * Moves the filesystem object from the given path to the other given path.
			 *
			 * Function fails if it is not able to move the object.
			 *
			 * @parameter	from		string	From		path to the filesystem object you want to move
			 * @parameter	to			string	To			path to the filesystem object the target should get moved to
			 * @return		success		bool	Success		returns true if it was able to move the node
			 */)", move) {
				LuaFunc();

				const auto from = CodersFileSystem::Path(luaL_checkstring(L, 1));
				const auto to = CodersFileSystem::Path(luaL_checkstring(L, 2));
				try {
					lua_pushboolean(L, self->move(from, to) == 0);
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	rename(path: string, name: string)
			 * @DisplayName		Rename
			 *
			 * Renames the filesystem object at the given path to the given name.
			 *
			 * @parameter	path		string	Path		path to the filesystem object you want to rename
			 * @parameter	name		string	Name		the new name for your filesystem object
			 * @return		success		bool	Success		returns true if it was able to rename the node
			 */)", rename) {
				LuaFunc();

				const auto from = CodersFileSystem::Path(luaL_checkstring(L, 1));
				const auto to = std::string(luaL_checkstring(L, 2));
				try {
					lua_pushboolean(L, self->rename(from, to));
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	exists(path: string)
			 * @DisplayName		Exists
			 *
			 * Returns true if the given path exists.
			 */)", exists) {
				LuaFunc();

				const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
				try {
					lua_pushboolean(L, self->fileType(path).IsSet());
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string[]	children(path: string)
			 * @DisplayName		Children
			 *
			 * Lists all children names of the node with the given path. (f.e. items in a folder)
			 */)", children) {
				LuaFunc();

				const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
				std::unordered_set<std::string> childs;
				try {
					childs = self->children(path);
				} CatchExceptionLua
				lua_newtable(L);
				int i = 1;
				for (const auto& child : childs) {
					lua_pushstring(L, child.c_str());
					lua_seti(L, -2, i++);
				}
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	isFile(path: string)
			 * @DisplayName		Is File
			 *
			 * Returns true if the given path refers to a file.
			 */)", isFile) {
				LuaFunc();
				const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
				try {
					lua_pushboolean(L, self->fileType(path) == CodersFileSystem::File_Regular);
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	isDir(path: string)
			 * @DisplayName		Is Dir
			 *
			 * Returns true if the given path refers to directory.
			 */)", isDir) {
				LuaFunc();
				const auto path = CodersFileSystem::Path(luaL_checkstring(L, 1));
				try {
					lua_pushboolean(L, self->fileType(path) == CodersFileSystem::File_Directory);
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	mount(device: string, mountPoint: string)
			 * @DisplayName		Mount
			 *
			 * This function mounts the device referenced by the the path to a device node to the given mount point.
			 *
			 * @parameter	device		string	Device			the path to the device you want to mount
			 * @parameter	mountPoint	string	Mount Point		the path to the point were the device should get mounted to
			 * @return		success		bool	Success			true if the mount was executed successfully
			 */)", mount) {
				LuaFunc();
				const auto devPath = CodersFileSystem::Path(luaL_checkstring(L, 1));
				const auto mountPath = CodersFileSystem::Path(luaL_checkstring(L, 2));
				try {
					TSharedPtr<CodersFileSystem::Device> device =  self->getDevice(devPath);
					if (device) {
						lua_pushboolean(L, self->mount(device.ToSharedRef(), mountPath));
					} else {
						lua_pushboolean(L, false);
					}
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	unmount(mountPoint: string)
			 * @DisplayName		Un-Mount
			 *
			 * This function unmounts if the device at the given mount point.
			 *
			 * @parameter	mountPoint	string	Mount Point		the path the device is mounted to
			 * @return		success		bool	Success			returns true if it was able to unmount the device located at the mount point
			 */)", unmount) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		...		doFile(path: string)
			 * @DisplayName		Do File
			 *
			 * Executes Lua code in the file referd by the given path.
			 *
			 * Function fails if path doesn’t exist or path doesn’t refer to a file.
			 *
			 * Returns the result of the execute function or what ever it yielded
			 */)", doFile) {
				LuaFunc();

				const CodersFileSystem::Path path(luaL_checkstring(L, 1));
				TSharedPtr<CodersFileSystem::FileStream> file;
				try {
					file = self->open(path, CodersFileSystem::INPUT);
				} CatchExceptionLua
				if (!file.IsValid()) return luaL_error(L, "not able to create filestream");
				std::string code;
				try {
					code = CodersFileSystem::FileStream::readAll(file.ToSharedRef());
				} CatchExceptionLua
				try {
					file->close();
				} CatchExceptionLua
				luaL_loadbufferx(L, code.c_str(), code.size(), ("@" + path.str()).c_str(), "t");
				lua_callk(L, 0, LUA_MULTRET, 0, luaDoFileCont);
				return luaDoFileCont(L, 0, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		function	loadFile(path: string)
			 * @DisplayName		Load File
			 *
			 * Loads the file refered by the given path as a Lua function and returns it.
			 * Functions fails if path doesn’t exist or path doesn’t reger to a file.
			 */)", loadFile) {
				LuaFunc();

				const CodersFileSystem::Path path = luaL_checkstring(L, 1);
				TSharedPtr<CodersFileSystem::FileStream> file;
				try {
					file = self->open(path, CodersFileSystem::INPUT);
				} CatchExceptionLua
				if (!file.IsValid()) return luaL_error(L, "not able to create filestream");
				std::string code;
				try {
					code = CodersFileSystem::FileStream::readAll(file.ToSharedRef());
				} CatchExceptionLua
				try {
					file->close();
				} CatchExceptionLua

				luaL_loadbufferx(L, code.c_str(), code.size(), ("@" + path.str()).c_str(), "t");
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string	path([int,] string...)
			 * @DisplayName		Path
			 *
			 * Combines a variable amount of strings as paths together to one big path.
			 *
			 * Additionally, applies given conversion. Defined by the optionally given integer.
			 *
			 * .Possible Conversions
			 * [%collapsible]
			 * ====
			 * [cols="1,10a"]
			 * |===
			 * | 0 | Normalize the path. +
			 *       `/my/../weird/./path` -> `/weird/path`
			 * | 1 | Normalizes and converts the path to an absolute path. +
			 *       `my/abs/path` -> `/my/abs/path`
			 * | 2 | Normalizes and converts the path to an relative path. +
			 *       `/my/relative/path` -> `my/relative/path`
			 * | 3 | Returns the whole file/folder name. +
			 *       `/path/to/file.txt` -> `file.txt`
			 * | 4 | Returns the stem of the filename. +
			 *       `/path/to/file.txt` -> `file` +
			 *       `/path/to/.file` -> `.file`
			 * | 5 | Returns the file-extension of the filename. +
			 *       `/path/to/file.txt` -> `.txt` +
			 *       `/path/to/.file` -> empty-str +
			 *       `/path/to/file.` -> `.`
			 * |===
			 * ====
			 */)", path) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		int...	analyzePath(string...)
			 * @DisplayName		Analyze Path
			 *
			 * Each provided string will be viewed as one filesystem-path and will be checked for lexical features. +
			 * Each of those string will then have a integer return value which is a bit-flag-register describing those lexical features.
			 *
			 * .Bit-Flags
			 * [%collapsible]
			 * ====
			 * [cols="1,10a"]
			 * |===
			 * | 1 | Is filesystem root
			 * | 2 | Is Empty (includes if it is root-path)
			 * | 3 | Is absolute path
			 * | 4 | Is only a file/folder name
			 * | 5 | Filename has extension
			 * | 6 | Ends with a `/` -> refers a directory
			 * |===
			 * ====
			 */)", analyzePath) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool...		isNode(string...)
			 * @DisplayName		Is Node
			 *
			 * For each given string, returns a bool to tell if string is a valid node (file/folder) name.
			 */)", isNode) {
				LuaFunc();
				int args = lua_gettop(L);
				for (int i = 1; i <= args; ++i) {
					lua_pushboolean(L, CodersFileSystem::Path::isNode(lua_tostring(L, i)));
				}
				return UFINLuaProcessor::luaAPIReturn(L, args);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string...	meta(string...)
			 * @DisplayName		Meta
			 *
			 * Returns for each given string path, a table that defines contains some meta information about node the string references.
			 *
			 * .Possible `type`-Field strings
			 * [%collapsible]
			 * ====
			 * [cols="1,10a"]
			 * | ===
			 * | `File`			| A normal File
			 * | `Directory`	| A directory or folder that can hold multiple nodes.
			 * | `Device`		| A special type of Node that represents a filesystem and can be mounted.
			 * | `Unknown`		| The node type is not known to this utility function.
			 * | ===
			 * ====
			 */)", meta) {
				LuaFunc();

				int args = lua_gettop(L);
				for (int i = 1; i <= args; ++i) {
					CodersFileSystem::Path Path = lua_tostring(L, i);
					TOptional<CodersFileSystem::FileType> FileType = self->fileType(Path);
					lua_newtable(L);
					if (FileType.IsSet()) {
						switch (*FileType) {
							case CodersFileSystem::File_Regular:
								lua_pushstring(L, "File");
							break;
							case CodersFileSystem::File_Directory:
								lua_pushstring(L, "Directory");
							break;
							case CodersFileSystem::File_Device:
								lua_pushstring(L, "Device");
							break;
							default:
								lua_pushstring(L, "Unknown");
						}
					} else {
						lua_pushstring(L, "Unknown");
					}
					lua_setfield(L, -2, "type");
				}
				return UFINLuaProcessor::luaAPIReturn(L, args);
			}
		}

		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	File
		 * @DisplayName		File
		 */)", File) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		close()
			 * @DisplayName		Close
			 *
			 * Closes the File-Stream.
			 */)", close) {
				LuaFileFunc();

				try {
					file->close();
				} CatchExceptionLua
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		write(string...)
			 * @DisplayName		Write
			 *
			 * Writes the given strings to the File-Stream.
			 */)", write) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string...	read(int...)
			 * @DisplayName		Read
			 *
			 * Reads up to the given amount of bytes from the file.
			 * Strings may be smaller than the given amount of bytes due to f.e. reaching the End-Of-File.
			 */)", read) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		int		seek(where: string, offset: int)
			 * @DisplayName		Seek
			 *
			 * Moves the File-Streams pointer to a position defined by the offset and from what starting location.
			 *
			 * .Possble `where` values
			 * [%collapsible]
			 * ====
			 * * `cur` Offset is relative to the current location
			 * * `set` Offset is relative to the beginning of the file
			 * * `end` Offset is relative to the end of the file
			 * ====
			 */)", seek) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
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
					TSharedPtr<CodersFileSystem::FileStream> stream;
					if (open) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__persist
			 * @DisplayName		Persist
			 */)", __persist) {
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

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__gc
			 * @DisplayName		GC
			 */)", __gc) {
				LuaFile& f = *static_cast<LuaFile*>(luaL_checkudata(L, 1, "File"));
				try {
					if (f->file) f->file->close();
				} CatchExceptionLua
				f.~LuaFile();
				return 0;
			}
		}

		LuaModulePostSetup() {
			PersistenceNamespace("FileSystem");

			lua_pushcfunction(L, static_cast<lua_CFunction>(reinterpret_cast<void*>(filesystem::luaDoFileCont)));
			PersistValue("doFileCont");

			luaL_getmetatable(L, "File");
			lua_pushvalue(L, -1);
			lua_setfield(L, -2, "__index");
			lua_pop(L, 1);

			lua_pushcfunction(L, File::luaFileUnpersist);
			PersistValue("FileUnpersist");
		}
	}

	void luaFIN_pushFile(lua_State* L, TSharedPtr<CodersFileSystem::FileStream> file, const std::string& path) {
		LuaFile* f = static_cast<LuaFile*>(lua_newuserdata(L, sizeof(LuaFile)));
		luaL_setmetatable(L, "File");
		new (f) LuaFile(new LuaFileContainer());
		f->Get().file = file;
		f->Get().path = path;
		if (file.IsValid()) {
			TArray<LuaFile>& streams = FINLua::luaFIN_getExtraSpace(L).FileStreams;
			streams.Add(*f);
		}
	}
}
