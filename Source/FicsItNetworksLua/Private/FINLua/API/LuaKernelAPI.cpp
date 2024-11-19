#include "FINLua/API/LuaKernelAPI.h"

#include "FicsItKernel.h"
#include "FicsItLogLibrary.h"
#include "FINLuaModule.h"
#include "FINLuaProcessor.h"
#include "FINLuaThreadedRuntime.h"
#include "FINMediaSubsystem.h"
#include "FINNetworkUtils.h"
#include "LuaClass.h"
#include "LuaObject.h"
#include "LuaPersistence.h"
#include "NetworkController.h"
#include "Path.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		KernelModule
	 * @DisplayName		Kernel Module
	 * @Dependency		ReflectionSystemObjectModule
	 *
	 * This Module provides an interface for other Modules to interact with the Kernel.
	 */)", KernelModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		filesystem
		 * @DisplayName		File-System Library
		 */)", filesystem) {
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
				UFINKernelSystem* kernel = FINLua::luaFIN_getKernel(L);
				FLuaSync SyncCall(L);

				const std::string path = luaL_checkstring(L, 1);
				lua_pushboolean(L, kernel->InitFileSystem(CodersFileSystem::Path(path)));
				return 1;
			}

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
				UFINKernelSystem* kernel = FINLua::luaFIN_getKernel(L);
				FINLua::FLuaSync SyncCall(L);

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
				UFINKernelSystem* kernel = FINLua::luaFIN_getKernel(L);
				FLuaSync SyncCall(L);

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
					} catch (const std::exception& ex) { \
						FDebug::DumpStackTraceToLog(ELogVerbosity::Warning); \
						return luaL_error(L, ex.what()); \
					}
				}
				lua_pushboolean(L, false);
				return 1;
			}
		}

		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		computer
		 * @DisplayName		Computer Library
		 */)", computer) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(usage: int, capacity: int)	getMemory()
			 * @DisplayName		Get Memory
			 *
			 * Returns the used memory and memory capacity the computer has.
			 *
			 * @return	usage		int		Usage		The memory usage at the current time
			 * @return	capacity	int		Capacity	The memory capacity the computer has
			 */)", getMemory) {
				FINLua::FLuaSync sync(L);

				UFINKernelSystem* kernel = FINLua::luaFIN_getKernel(L);

				int64 Usage = kernel->GetMemoryUsage();
				int64 Capacity = kernel->GetCapacity();

				lua_pushinteger(L, Usage);
				lua_pushinteger(L, Capacity);

				return 2;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ComputerCase	getInstance()
			 * @DisplayName		Get Instance
			 * @Dependency	FullReflectionModule
			 *
			 * Returns a reference to the computer case in which the current code is running.
			 *
			 * @return	case	ComputerCase	The computer case this lua runtime is running in.
			 */)", getInstance) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);

				luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(FFIRTrace(kernel->GetNetwork()->GetComponent().GetObject())));

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Object[]	getPCIDevices([type: Class])
			 * @DisplayName		Get PCI-Devices
			 *
			 * This function allows you to get all installed https://docs.ficsit.app/ficsit-networks/latest/buildings/ComputerCase/index.html#_pci_interface[PCI-Devices] in a computer of a given type.
			 * Have a look at https://docs.ficsit.app/ficsit-networks/latest/lua/examples/PCIDevices.html[this] example to fully understand how it works.
			 *
			 * @parameter	type		Class		Type		Optional type which will be used to filter all PCI-Devices. If not provided, will return all PCI-Devices.
			 * @return		objects		Object[]	Objects		An array containing instances for each PCI-Device built into the computer.
			 */)", getPCIDevices) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);

				lua_newtable(L);
				int args = lua_gettop(L);
				UFIRClass* Type = nullptr;
				if (args > 0) {
					Type = luaFIN_toFINClass(L, 1, nullptr);
					if (!Type) {
						return 1;
					}
				}
				int i = 1;
				for (TScriptInterface<IFINPciDeviceInterface> Device : kernel->GetPCIDevices()) {
					if (!Device || (Type && !Device.GetObject()->IsA(Cast<UClass>(Type->GetOuter())))) continue;
					luaFIN_pushObject(L, FFIRTrace(kernel->GetNetwork()->GetComponent().GetObject()) / Device.GetObject());
					lua_seti(L, -2, i++);
				}
				return 1;
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	media	FINMediaSubsystem
			 * @DisplayName		Media
			 *
			 * Field containing a reference to the Media Subsystem.
			 */)", media) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);
				luaFIN_pushObject(L, FIRTrace(AFINMediaSubsystem::GetMediaSubsystem(kernel)));
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		int		millis()
			 * @DisplayName		Millis
			 *
			 * Returns the amount of milliseconds passed since the system started.
			 *
			 * @return	millis	int		The amount of real milliseconds sinde the ingame-computer started.
			 */)", millis) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);

				lua_pushinteger(L, kernel->GetTimeSinceStart());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		setEEPROM(code: string)
			 * @DisplayName		Set EEPROM
			 *
			 * Sets the code of the current eeprom. Doesn’t cause a system reset.
			 *
			 * @param	code	string	The new EEPROM Code as string.
			 */)", setEEPROM) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);
				if (!kernel) return 0;

				FString code = luaFIN_checkFString(L, 1);

				if (UFINLuaProcessor* luaProcessor = Cast<UFINLuaProcessor>(kernel->GetProcessor())) {
					kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([luaProcessor, code]() {
						luaProcessor->SetEEPROM(code);
					})));
				} else {
					return luaL_error(L, "no eeprom set");
				}

				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string	getEEPROM
			 * @DisplayName		Get EEPROM
			 *
			 * Returns the current eeprom contents.
			 *
			 * @return	code	string	The EEPROM Code as string.
			 */)", getEEPROM) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);

				TOptional<FString> Code = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
				if (Code.IsSet() == false) return luaL_error(L, "no eeprom set");
				luaFIN_pushFString(L, *Code);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		beep(pitch: number)
			 * @DisplayName		Beep
			 *
			 * Lets the computer emit a simple beep sound with the given pitch.
			 *
			 * @param	pitch	number	a multiplier for the pitch adjustment of the beep sound.
			 */)", beep) {
				FLuaSync sync(L);

				UFINKernelSystem* kernel = luaFIN_getKernel(L);

				float pitch = 1;
				if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
				kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
					kernel->GetAudio()->Beep(pitch);
				})));
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		panic(error: string)
			 * @DisplayName		Panic
			 *
			 * Crashes the computer with the given error message.
			 */)", panic) {
				luaL_checkstring(L, -1);
				UFINKernelSystem* kernel = luaFIN_getKernel(L);
				FString message = luaFIN_toFString(L, -1);
				FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
				runtime.TickActions.Enqueue([kernel, message]() {
					kernel->Crash(MakeShared<FFINKernelCrash>(FString("PANIC! '") + message + "'"));
					kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
						kernel->GetAudio()->Beep(0.5);
					})));
				});
				lua_yield(L, 0);
				return 0;
			}
		}
	}

	void luaFIN_setKernel(lua_State* L, UFINKernelSystem* Kernel) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("kernel"), Kernel);
	}

	UFINKernelSystem* luaFIN_getKernel(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		UFINKernelSystem** value = reinterpret_cast<UFINKernelSystem**>(runtime.GlobalPointers.Find(TEXT("kernel")));
		fgcheck(value != nullptr);
		return *value;
	}
}
