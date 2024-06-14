#pragma once

// this is a terible way of doing documentation but currently only way without modifing any additional code for documentation.
inline const auto FINGenLuaSumnekoDocumentationStart = TEXT(R"(error("I don't know what your misson is. But is file is not meant to be executed in any way. It's a meta file.")
---@meta
---@diagnostic disable

---@class FIN.classes
local classes = {}

---@class FIN.structs
local structs = {}

)");

inline const auto MiscDocumentation = TEXT(R"(
-- some more FicsIt-Networks things to support more type specific things and also adds documentation for `computer`, `component`, `event` and `filesystem` libraries in FicsIt-Networks (keep in mind this is all written by hand and can maybe not represent all features available)

--- # Not in FicsIt-Networks available #
package = nil

--- # Not in FicsIt-Networks available #
os = nil

--- # Not in FicsIt-Networks available #
collectgarbage = nil

--- # Not in FicsIt-Networks available #
io = nil

--- # Not in FicsIt-Networks available #
arg = nil

--- # Not in FicsIt-Networks available #
require = nil

---@class FIN.UUID : string


-- adding alias to make more descriptive correct naming and more plausible when using `computer.getPCIDevice()`

---@alias FIN.PCIDevice FIN.FINComputerModule

---@class Engine.Object
local Object = {}

--- The network id of this component.
---
--- ## Only on objects that are network components.
--- ### Flags:
--- * ? Runtime Synchronous - Can be called/changed in Game Tick ?
--- * ? Runtime Parallel - Can be called/changed in Satisfactory Factory Tick ?
--- * Read Only - The value of this property can not be changed by code
---@type FIN.UUID
Object.id = nil

--- The nick you gave the component in the network its in.
--- If it has no nick name it returns `nil`.
---
--- ## Only on objects that are network components.
--- ### Flags:
--- * ? Runtime Synchronous - Can be called/changed in Game Tick ?
--- * ? Runtime Parallel - Can be called/changed in Satisfactory Factory Tick ?
--- * Read Only - The value of this property can not be changed by code
---@type string
Object.nick = nil

-- global functions from FicsIt-Networks

--- Tries to find the item with the provided name.
---@param name string
---@return Engine.Object
function findItem(name) end

--- Tries to find the items or item provided via name.
---@param ... string
---@return Engine.Object[]
function getItems(...) end
)");

inline const auto FutureApiDocumentation = TEXT(R"(
---@class FIN.Future
local Future = {}

--- Waits for the future to finish processing and returns the result.
--- ### Flags:
--- * Unknown
---@async
---@return any ...
function Future:await()
end

--- Gets the data.
--- ### Flags:
--- * Unknown
---@return any ...
function Future:get()
end

--- Checks if the Future is done processing.
--- ### Flags:
--- * Unknown
---@return boolean isDone
function Future:canGet()
end
)");

inline const auto EventApiDocumentation = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `event`
---
--- The Event API provides classes, functions and variables for interacting with the component network.
---@class FIN.Event.Api
event = {}

--- Adds the running lua context to the listen queue of the given component.
---@param component Engine.Object - The network component lua representation the computer should now listen to.
function event.listen(component) end

--- Returns all signal senders this computer is listening to.
---@return Engine.Object[] components - An array containing instances to all sginal senders this computer is listening too.
function event.listening() end

--- Waits for a signal in the queue. Blocks the execution until a signal got pushed to the signal queue, or the timeout is reached.
--- Returns directly if there is already a signal in the queue (the tick doesn’t get yielded).
---@param timeoutSeconds number? - The amount of time needs to pass until pull unblocks when no signal got pushed.
---@return string signalName - The name of the returned signal.
---@return Engine.Object component - The component representation of the signal sender.
---@return any ... - The parameters passed to the signal.
function event.pull(timeoutSeconds) end

--- Removes the running lua context from the listen queue of the given components. Basically the opposite of listen.
---@param component Engine.Object - The network component lua representations the computer should stop listening to.
function event.ignore(component) end

--- Stops listening to any signal sender. If afterwards there are still coming signals in, it might be the system itself or caching bug.
function event.ignoreAll() end

--- Clears every signal from the signal queue.
function event.clear() end
)");
	
inline const auto ComponentApiDocumentation = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `component`
---
--- The Component API provides structures, functions and signals for interacting with the network itself like returning network components.
---@class FIN.Component.Api
component = {}


--- Generates and returns instances of the network component with the given UUID.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param id FIN.UUID - UUID of a network component.
---@return T? component
function component.proxy(id) end

--- Generates and returns instances of the network components with the given UUIDs.
--- You can pass any amount of parameters and each parameter will then have a corresponding return value.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param ... FIN.UUID - UUIDs
---@return T? ... - components
function component.proxy(...) end

--- Generates and returns instances of the network components with the given UUIDs.
--- You can pass any amount of parameters and each parameter will then have a corresponding return value.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param ids FIN.UUID[]
---@return T[] components
function component.proxy(ids) end

--- Generates and returns instances of the network components with the given UUIDs.
--- You can pass any amount of parameters and each parameter will then have a corresponding return value.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param ... FIN.UUID[]
---@return T[] ... - components
function component.proxy(...) end

--- Searches the component network for components with the given query.
---@param query string
---@return FIN.UUID[] UUIDs
function component.findComponent(query) end

--- Searches the component network for components with the given query.
--- You can pass multiple parameters and each parameter will be handled separately and returns a corresponding return value.
---@param ... string - querys
---@return FIN.UUID[] ... - UUIDs
function component.findComponent(...) end

--- Searches the component network for components with the given type.
---@param type Engine.Object
---@return FIN.UUID[] UUIDs
function component.findComponent(type) end

--- Searches the component network for components with the given type.
--- You can pass multiple parameters and each parameter will be handled separately and returns a corresponding return value.
---@param ... Engine.Object - classes to search for
---@return FIN.UUID[] ... - UUIDs
function component.findComponent(...) end
)");
	
inline const auto ComputerApiDocumentation = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `computer`
---
--- The Computer API provides a interface to the computer owns functionalities.
---@class FIN.Computer.Api
computer = {}

--- Media Subsystem
---@type FIN.FINMediaSubsystem
computer.media = nil

--- Returns the current memory usage
---@return integer usage
---@return integer capacity
function computer.getMemory() end

--- Returns the current computer case instance
---@return FIN.Components.ComputerCase_C
function computer.getInstance() end

--- Stops the current code execution immediately and queues the system to restart in the next tick.
function computer.reset() end

--- Stops the current code execution.
--- Basically kills the PC runtime immediately.
function computer.stop() end

--- Crashes the computer with the given error message.
---@param errorMsg string - The crash error message you want to use
function computer.panic(errorMsg) end

--- This function is mainly used to allow switching to a higher tick runtime state. Usually you use this when you want to make your code run faster when using functions that can run in asynchronous environment.
function computer.skip() end

--- Does the same as computer.skip
function computer.promote() end

--- Reverts effects of skip
function computer.demote() end

--- Returns `true` if the tick state is to higher
---@return boolean isPromoted
function computer.isPromoted() end

--- If computer state is async probably after calling computer.skip.
---@return 0 | 1 state - 0 = Sync, 1 = Async
function computer.state() end

--- Lets the computer emit a simple beep sound with the given pitch.
---@param pitch number - The pitch of the beep sound you want to play.
function computer.beep(pitch) end

--- Sets the code of the current eeprom. Doesn’t cause a system reset.
---@param code string - The code you want to place into the eeprom.
function computer.setEEPROM(code) end

--- Returns the code the current eeprom contents.
---@return string code - The code in the EEPROM
function computer.getEEPROM() end

--- Returns the number of game seconds passed since the save got created. A game day consists of 24 game hours, a game hour consists of 60 game minutes, a game minute consists of 60 game seconds.
---@return number time - The number of game seconds passed since the save got created.
function computer.time() end

--- Returns the amount of milliseconds passed since the system started.
---@return integer milliseconds - Amount of milliseconds since system start
function computer.millis() end

--- Returns some kind of strange/mysterious time data from a unknown place (the real life).
---@return integer Timestamp - Unix Timestamp
---@return string DateTimeStamp - Serverside Formatted Date-Time-Stamp
---@return string DateTimeStamp - Date-Time-Stamp after ISO 8601
function computer.magicTime() end

---@param verbosity FIN.Components.LogEntry.Verbosity
---@param format string
---@param ... any
function computer.log(verbosity, format, ...) end

--- This function allows you to get all installed PCI-Devices in a computer of a given type.
---@generic TPCIDevice : FIN.PCIDevice
---@param type TPCIDevice
---@return TPCIDevice[]
function computer.getPCIDevices(type) end

--- Shows a text notification to the player. If player is `nil` to all players.
---@param text string
---@param playerName string?
function computer.textNotification(text, playerName) end

--- Creates an attentionPing at the given position to the player. If player is `nil` to all players.
---@param position Engine.Vector
---@param playerName string?
function computer.attentionPing(position, playerName) end
)");
	
inline const auto FileSystemApiDocumentationPart1 = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `filesystem`
---
--- The filesystem api provides structures, functions and variables for interacting with the virtual file systems.
---
--- You can’t access files outside the virtual filesystem. If you try to do so, the Lua runtime crashes.
---@class FIN.Filesystem.Api
filesystem = {}

---@alias FIN.Filesystem.Type
---|"tmpfs" # A temporary filesystem only existing at runtime in the memory of your computer. All data will be lost when the system stops.

--- Trys to create a new file system of the given type with the given name.
--- The created filesystem will be added to the system DevDevice.
---@param type FIN.Filesystem.Type - the type of the new filesystem
---@param name string - the name of the new filesystem you want to create
---@return boolean success - returns true if it was able to create the new filesystem
function filesystem.makeFileSystem(type, name) end

--- Trys to remove the filesystem with the given name from the system DevDevice.
--- All mounts of the device will run invalid.
---@param name string - the name of the new filesystem you want to remove
---@return boolean success - returns true if it was able to remove the new filesystem
function filesystem.removeFileSystem(name) end

--- Trys to mount the system DevDevice to the given location.
--- The DevDevice is special Device holding DeviceNodes for all filesystems added to the system. (like TmpFS and drives). It is unmountable as well as getting mounted a seccond time.
---@param path string - path to the mountpoint were the dev device should get mounted to
---@return boolean success - returns if it was able to mount the DevDevice
function filesystem.initFileSystem(path) end

---@alias FIN.Filesystem.File.Openmode
---|"r" read only -> file stream can just read from file. If file doesn’t exist, will return nil
---|"w" write -> file stream can read and write creates the file if it doesn’t exist
---|"a" end of file -> file stream can read and write cursor is set to the end of file
---|"+r" truncate -> file stream can read and write all previous data in file gets dropped
---|"+a" append -> file stream can read the full file but can only write to the end of the existing file

--- Opens a file-stream and returns it as File-table.
---@param path string
---@param mode FIN.Filesystem.File.Openmode
---@return FIN.Filesystem.File File
function filesystem.open(path, mode) end

--- Creates the folder path.
---@param path string - folder path the function should create
---@param all boolean? - if true creates all sub folders from the path
---@return boolean success - returns `true` if it was able to create the directory
function filesystem.createDir(path, all) end

--- Removes the filesystem object at the given path.
---@param path string - path to the filesystem object
---@param all boolean? - if true deletes everything
---@return boolean success - returns `true` if it was able to remove the node
function filesystem.remove(path, all) end

--- Moves the filesystem object from the given path to the other given path.
--- Function fails if it is not able to move the object.
---@param from string - path to the filesystem object you want to move
---@param to string - path to the filesystem object the target should get moved to
---@return boolean success - returns `true` if it was able to move the node
function filesystem.move(from, to) end

--- Renames the filesystem object at the given path to the given name.
---@param path string - path to the filesystem object you want to rename
---@param name string - the new name for your filesystem object
---@return boolean success - returns true if it was able to rename the node
function filesystem.rename(path, name) end

--- Checks if the given path exists.
---@param path string - path you want to check if it exists
---@return boolean exists - true if given path exists
function filesystem.exists(path) end

--- Lists all children of this node. (f.e. items in a folder)
---@param path string - path to the filesystem object you want to get the childs from
---@return string[] childs - array of string which are the names of the childs
function filesystem.childs(path) end

---@deprecated
--- Lists all children of this node. (f.e. items in a folder)
---@param path string - path to the filesystem object you want to get the childs from
---@return string[] childs - array of string which are the names of the childs
function filesystem.children(path) end

--- Checks if path refers to a file.
---@param path string - path you want to check if it refers to a file
---@return boolean isFile - true if path refers to a file
function filesystem.isFile(path) end
)");

inline const auto FileSystemApiDocumentationPart2 = TEXT(R"(
--- Checks if given path refers to a directory.
---@param path string - path you want to check if it refers to a directory
---@return boolean isDir - returns true if path refers to a directory
function filesystem.isDir(path) end

--- This function mounts the device referenced by the the path to a device node to the given mount point.
---@param device string - the path to the device you want to mount
---@param mountPoint string - the path to the point were the device should get mounted to
function filesystem.mount(device, mountPoint) end

--- This function unmounts the device referenced to the the given mount point.
---@param mountPoint string - the path to the point were the device is referenced to
function filesystem.unmount(mountPoint) end

--- Executes Lua code in the file referd by the given path.
--- Function fails if path doesn’t exist or path doesn’t refer to a file.
---@param path string - path to file you want to execute as Lua code
---@return any ... - Returned values from executed file.
function filesystem.doFile(path) end

--- Loads the file refered by the given path as a Lua function and returns it.
--- Functions fails if path doesn’t exist or path doesn’t reger to a file.
---@param path string - path to the file you want to load as Lua function
---@return function loadedFunction - the file compiled as Lua function
function filesystem.loadFile(path) end

---@alias FIN.Filesystem.PathParameters
---|0 Normalize the path. -> /my/../weird/./path → /weird/path
---|1 Normalizes and converts the path to an absolute path. -> my/abs/path → /my/abs/path
---|2 Normalizes and converts the path to an relative path. -> /my/relative/path → my/relative/path
---|3 Returns the whole file/folder name. -> /path/to/file.txt → file.txt
---|4 Returns the stem of the filename. -> /path/to/file.txt → file || /path/to/.file → .file
---|5 Returns the file-extension of the filename. -> /path/to/file.txt → .txt || /path/to/.file → empty-str || /path/to/file. → .

--- Combines a variable amount of strings as paths together.
---@param ... string - paths to be combined
---@return string path - the final combined path
function filesystem.path(...) end

--- Combines a variable amount of strings as paths together to one big path.
--- Additionally, applies given conversion.
---@param parameter FIN.Filesystem.PathParameters - defines a conversion that should get applied to the output path.
---@param ... string - paths to be combined
---@return string path - the final combined and converted output path
function filesystem.path(parameter, ...) end

---@alias FIN.Filesystem.PathRegister
---|1 Is filesystem root
---|2 Is Empty (includes if it is root-path)
---|4 Is absolute path
---|8 Is only a file/folder name
---|16 Filename has extension
---|32 Ends with a / → refers a directory

--- Will be checked for lexical features.
--- Return value which is a bit-flag-register describing those lexical features.
---@param path string - filesystem-path you want to get lexical features from. 
---@return FIN.Filesystem.PathRegister BitRegister - bit-register describing the features of each path
function filesystem.analyzePath(path) end

--- Each string will be viewed as one filesystem-path and will be checked for lexical features.
--- Each of those string will then have a integer return value which is a bit-flag-register describing those lexical features.
---@param ... string - filesystem-paths you want to get lexical features from.
---@return FIN.Filesystem.PathRegister ... - bit-registers describing the features of each path
function filesystem.analyzePath(...) end

--- For given string, returns a bool to tell if string is a valid node (file/folder) name.
---@param node string - node-name you want to check.
---@return boolean isNode - True if node is a valid node-name.
function filesystem.isNode(node) end

--- For each given string, returns a bool to tell if string is a valid node (file/folder) name.
---@param ... string - node-names you want to check.
---@return boolean ... - True if the corresponding string is a valid node-name.
function filesystem.isNode(...) end
)");

inline const auto FileApiDocumentation = TEXT(R"(
---@class FIN.Filesystem.File
local File = {}

---@param data string
function File:write(data) end

---@param length integer
function File:read(length) end

---@alias FIN.Filesystem.File.SeekMode
---|"set" # Base is beginning of the file.
---|"cur" # Base is current position.
---|"end" # Base is end of file.

---@param mode FIN.Filesystem.File.SeekMode
---@param offset integer
---@return integer offset
function File:seek(mode, offset) end

function File:close() end
)");

const FString FINGenLuaSumnekoDocumentationEnd = FString::Printf(TEXT("%s\n%s\n%s\n%s\n%s\n%s%s\n%s"),
	MiscDocumentation, FutureApiDocumentation, EventApiDocumentation, ComponentApiDocumentation, ComputerApiDocumentation, FileSystemApiDocumentationPart1, FileSystemApiDocumentationPart2, FileApiDocumentation);
