#include "FGAttentionPingActor.h"
#include "FGPlayerController.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FINItemStateEEPROMText.h"
#include "FGTimeSubsystem.h"
#include "FILLogContainer.h"
#include "FILLogEntry.h"
#include "FINMediaSubsystem.h"
#include "FINNetworkUtils.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "FINLua/Reflection/LuaStruct.h"
#include "UI/FGGameUI.h"

#define LuaFunc() \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FLuaSyncCall SyncCall(L);

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		Computer
	 * @DisplayName		Computer Module
	 * @Dependency		ReflectionSystemObjectModule
	 *
	 * The Computer Module provides the Computer Library.
	 */)", Computer) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		computer
		 * @DisplayName		Computer Library
		 *
		 * The Computer Library provides functions for interaction with the computer and especially the Lua Runtime.
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
				LuaFunc()

				int64 Usage = kernel->GetMemoryUsage();
				int64 Capacity = kernel->GetCapacity();
				lua_pushinteger(L, Usage);
				lua_pushinteger(L, Capacity);
				return 2;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ComputerCase	getInstance()
			 * @DisplayName		Get Instance
			 *
			 * Returns a reference to the computer case in which the current code is running.
			 *
			 * @return	case	ComputerCase	The computer case this lua runtime is running in.
			 */)", getInstance) {
				LuaFunc()

				luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(FFIRTrace(kernel->GetNetwork()->GetComponent().GetObject())));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		reset()
			 * @DisplayName		Reset
			 *
			 * Stops the current code execution immediately and queues the system to restart in the next tick.
			 */)", reset) {
				LuaFunc();

				processor->GetTickHelper().shouldReset();
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		stop()
			 * @DisplayName		Stop
			 *
			 * Stops the current code execution. +
			 * Basically kills the PC runtime immediately.
			 */)", stop) {
				LuaFunc();

				processor->GetTickHelper().shouldStop();
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		panic(error: string)
			 * @DisplayName		Panic
			 *
			 * Crashes the computer with the given error message.
			 */)", panic) {
				LuaFunc();

				processor->GetTickHelper().shouldCrash(MakeShared<FFINKernelCrash>(FString("PANIC! '") + luaL_checkstring(L, 1) + "'"));
				kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
					kernel->GetAudio()->Beep();
				})));
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		skip()
			 * @DisplayName		Skip
			 *
			 * This function can be used to skip the current lua tick prematurely.
			 * Mostly for people who want to optimize their games runtime performance.
			 */)", skip) {
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		promote()
			 * @DisplayName		Promote
			 *
			 * This function is mainly used to allow switching to a higher tick runtime state.
			 * Usually you use this when you want to make your code run faster when using functions that can run in asynchronous environment.
			 */)", promote) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				processor->GetTickHelper().shouldPromote();
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		demote()
			 * @DisplayName		Demote
			 *
			 * This function is used to allow switching back to the normal tick rate.
			 */)", demote) {
				FLuaSyncCall Sync(L);
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		bool	isPromoted()
			 * @DisplayName		Is Promoted
			 *
			 * Returns true if the Lua runtime is currently promoted/elevated.
			 * Which means its running in an seperate game thread allowing for fast bulk calculations.
			 *
			 * @return	isPromoted	bool	True if the currenty runtime is running in promoted/elevated tick state.
			 */)", isPromoted) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				bool bPromoted = (bool)(processor->GetTickHelper().getState() & LUA_ASYNC);
				lua_pushboolean(L, bPromoted);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		int		state()
			 * @DisplayName		State
			 *
			 * DEPRECATED! Please use `isPromoted()` instead
			 */)", state) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				int state = 0;
				if (processor->GetTickHelper().getState() & LUA_ASYNC) {
					state = 1;
				}
				lua_pushinteger(L, state);
				luaFIN_warning(L, "Deprecated function call 'state()', use 'isPromoted()' instead", false);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		beep(pitch: number)
			 * @DisplayName		Beep
			 *
			 * Lets the computer emit a simple beep sound with the given pitch.
			 *
			 * @param	pitch	number	a multiplier for the pitch adjustment of the beep sound.
			 */)", beep) {
				LuaFunc();

				float pitch = 1;
				if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
				kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
					kernel->GetAudio()->Beep(pitch);
				})));
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		setEEPROM(code: string)
			 * @DisplayName		Set EEPROM
			 *
			 * Sets the code of the current eeprom. Doesn’t cause a system reset.
			 *
			 * @param	code	string	The new EEPROM Code as string.
			 */)", setEEPROM) {
				LuaFunc();

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
				LuaFunc();

				TOptional<FString> Code = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
				if (Code.IsSet() == false) return luaL_error(L, "no eeprom set");
				luaFIN_pushFString(L, *Code);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		number	time()
			 * @DisplayName		Time
			 *
			 * Returns the number of game seconds passed since the save got created. A game day consists of 24 game hours, a game hour consists of 60 game minutes, a game minute consists of 60 game seconds.
			 *
			 * @return	time	number	The current number of game seconds passed since the creation of the save.
			 */)", time) {
				LuaFunc();

				const AFGTimeOfDaySubsystem* Subsystem = AFGTimeOfDaySubsystem::Get(kernel);
				lua_pushnumber(L, Subsystem->GetPassedDays() * 86400 + Subsystem->GetDaySeconds());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		int		millis()
			 * @DisplayName		Millis
			 *
			 * Returns the amount of milliseconds passed since the system started.
			 *
			 * @return	millis	int		The amount of real milliseconds sinde the ingame-computer started.
			 */)", millis) {
				LuaFunc();

				lua_pushinteger(L, kernel->GetTimeSinceStart());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		log(message: string, verbosity: int)
			 * @DisplayName		Log
			 *
			 * Allows you to print a log message to the computers log with the given log verbosity.
			 *
			 * @parameter	message		string	Message		The log message you want to print
			 * @parameter	verbosity	int		Verbosity	The log-level/verbosity of the message you want to log. 0 = Debug, 1 = Info, 2 = Warning, 3 = Error & 4 = Fatal
			 */)", log) {
				LuaFunc();

				int verbosity = luaL_checknumber(L, 1);
				FString text = luaFIN_checkFString(L, 2);
				verbosity = FMath::Clamp(verbosity, 0, FIL_Verbosity_Max);
				kernel->GetLog()->PushLogEntry((EFILLogVerbosity)verbosity, text);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		textNotification(text: string, username: string)
			 * @DisplayName		Text Notification
			 *
			 * This function allows you to prompt a user with the given username, with a text message.
			 *
			 * @parameter	text		string	Text		The Text you want to send as Notification to the user
			 * @parameter	username	string	Username	The username of the user you want to send the notification to
			 */)", textNotification) {
				LuaFunc();

				FString Text = luaFIN_checkFString(L, 1);
				TOptional<FString> Player;
				if (lua_isstring(L, 2)) Player = luaFIN_checkFString(L, 2);
				for (auto players = kernel->GetWorld()->GetPlayerControllerIterator(); players; ++players) {
					AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
					if (Player.IsSet() && PlayerController->GetPlayerState<AFGPlayerState>()->GetUserName() != *Player) continue;
					kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([PlayerController, Text]() {
						PlayerController->GetGameUI()->ShowTextNotification(FText::FromString(Text));
					})));
					/*kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([PlayerController, Text]() {
						UFINNotificationMessage* Message = NewObject<UFINNotificationMessage>();
						Message->NotificationText = FText::FromString(Text);
						PlayerController->GetGameUI()->AddPendingMessage(Message);
						UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Pending Messages? %i"), PlayerController->GetGameUI()->CanReceiveMessageQueue());
						UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Pending Message? %i"), PlayerController->GetGameUI()->CanReceiveMessage(UFINNotificationMessage::StaticClass()));
					})));*/
					break;
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		attentionPing(position: Vector, [username: string])
			 * @DisplayName		Attention Ping
			 *
			 * Allows to send a World Marker/Attention Ping for all or the given user.
			 *
			 * @parameter	position	Struct<FVector>		Position	The position in the world where the ping should occur
			 * @parameter	username	string				Username	The username of the user you want to ping.
			 */)", attentionPing) {
				LuaFunc();

				FVector Position = *luaFIN_checkStruct<FVector>(L, 1, true);
				TOptional<FString> Player;
				if (lua_isstring(L, 2)) Player = luaFIN_checkFString(L, 2);
				for (auto players = kernel->GetWorld()->GetPlayerControllerIterator(); players; players++) {
					AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
					if (Player.IsSet() && PlayerController->GetPlayerState<AFGPlayerState>()->GetUserName() != *Player) continue;
					kernel->PushFuture(MakeShared<TFIRInstancedStruct<FFINFuture>>(FFINFunctionFuture([PlayerController, Position]() {
						UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Player/BP_AttentionPingActor.BP_AttentionPingActor_C"));
						AFGAttentionPingActor* PingActor = PlayerController->GetWorld()->SpawnActorDeferred<AFGAttentionPingActor>(Class, FTransform(Position));
						PingActor->SetOwningPlayerState(PlayerController->GetPlayerState<AFGPlayerState>());
						PingActor->FinishSpawning(FTransform(Position));
					})));
					break;
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		(int, string, string)		magicTime()
			 * @DisplayName		Magic Time
			 *
			 * Returns some kind of strange/mysterious time data from a unknown place (the real life).
			 *
			 * @return	unix			int		Unix			Unix Timestamp
			 * @return	cultureTime		string	Culture Time	The time as text with the culture format used by the Host
			 * @return	iso8601			string	ISO 8601		The time as a Date-Time-Stamp after ISO 8601
			 */)", magicTime) {
				FDateTime Now = FDateTime::UtcNow();
				lua_pushinteger(L, Now.ToUnixTimestamp());
				FTCHARToUTF8 ConvertStr(*Now.ToString());
				lua_pushlstring(L, ConvertStr.Get(), ConvertStr.Length());
				FTCHARToUTF8 ConvertIso(*Now.ToIso8601());
				lua_pushlstring(L, ConvertIso.Get(), ConvertIso.Length());
				return 3;
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
				LuaFunc();

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
				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				luaFIN_pushObject(L, FIRTrace(AFINMediaSubsystem::GetMediaSubsystem(Processor)));
				//luaFIN_persistValue(L, -1, PersistName);
			}
		}
	}
}
