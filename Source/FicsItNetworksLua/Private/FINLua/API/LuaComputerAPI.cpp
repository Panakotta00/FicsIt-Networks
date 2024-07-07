#include "FGAttentionPingActor.h"
#include "FGPlayerController.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FINStateEEPROMLua.h"
#include "Network/FINNetworkUtils.h"
#include "FGTimeSubsystem.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "FINLua/Reflection/LuaStruct.h"
#include "UI/FGGameUI.h"
#include "Utils/FINMediaSubsystem.h"

#define LuaFunc() \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FLuaSyncCall SyncCall(L);

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		Computer
	 * @DisplayName		Computer Module
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
			 * @LuaFunction		getMemory
			 * @DisplayName		Get Memory
			 */)", getMemory) {
				LuaFunc()

				int64 Usage = kernel->GetMemoryUsage();
				int64 Capacity = kernel->GetCapacity();
				lua_pushinteger(L, Usage);
				lua_pushinteger(L, Capacity);
				return 2;
			}
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		getInstance
			 * @DisplayName		Get Instance
			 */)", getInstance) {
				LuaFunc()

				luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject())));
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		reset
			 * @DisplayName		Reset
			 */)", reset) {
				LuaFunc();

				processor->GetTickHelper().shouldReset();
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		stop
			 * @DisplayName		Stop
			 */)", stop) {
				LuaFunc();

				processor->GetTickHelper().shouldStop();
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		panic
			 * @DisplayName		Panic
			 */)", panic) {
				LuaFunc();

				processor->GetTickHelper().shouldCrash(MakeShared<FFINKernelCrash>(FString("PANIC! '") + luaL_checkstring(L, 1) + "'"));
				kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
					kernel->GetAudio()->Beep();
				})));
				lua_yield(L, 0);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction	skip
			 * @DisplayName		Skip
			 */)", skip) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				processor->GetTickHelper().shouldPromote();
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		promote
			 * @DisplayName		Promote
			 */)", promote) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				processor->GetTickHelper().shouldPromote();
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		demote
			 * @DisplayName		Demote
			 */)", demote) {
				FLuaSyncCall Sync(L);
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		isPromoted
			 * @DisplayName		Is Promoted
			 */)", isPromoted) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				bool bPromoted = (bool)(processor->GetTickHelper().getState() & LUA_ASYNC);
				lua_pushboolean(L, bPromoted);
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		state
			 * @DisplayName		State
			 */)", state) {
				UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
				int state = 0;
				if (processor->GetTickHelper().getState() & LUA_ASYNC) {
					state = 1;
				}
				lua_pushinteger(L, state);
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		beep
			 * @DisplayName		Beep
			 */)", beep) {
				LuaFunc();

				float pitch = 1;
				if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
				kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
					kernel->GetAudio()->Beep(pitch);
				})));
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		setEEPROM
			 * @DisplayName		Set EEPROM
			 */)", setEEPROM) {
				LuaFunc();

				AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
				if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
				size_t len;
				const char* str = luaL_checklstring(L, 1, &len);
				FUTF8ToTCHAR Conv(str, len);
				eeprom->SetCode(FString(Conv.Length(), Conv.Get()));
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		getEEPROM
			 * @DisplayName		Get EEPROM
			 */)", getEEPROM) {
				LuaFunc();

				const AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
				if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
				FString Code = eeprom->GetCode();
				FTCHARToUTF8 Conv(*Code, Code.Len());
				lua_pushlstring(L, Conv.Get(), Conv.Length());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		time
			 * @DisplayName		Time
			 */)", time) {
				LuaFunc();

				const AFGTimeOfDaySubsystem* Subsystem = AFGTimeOfDaySubsystem::Get(kernel);
				lua_pushnumber(L, Subsystem->GetPassedDays() * 86400 + Subsystem->GetDaySeconds());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		millis
			 * @DisplayName		Millis
			 */)", millis) {
				LuaFunc();

				lua_pushinteger(L, kernel->GetTimeSinceStart());
				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		log
			 * @DisplayName		Log
			 */)", log) {
				LuaFunc();

				int verbosity = luaL_checknumber(L, 1);
				FString text = luaFIN_checkFString(L, 2);
				verbosity = FMath::Clamp(verbosity, 0, EFINLogVerbosity::FIN_Log_Verbosity_Max);
				kernel->GetLog()->PushLogEntry((EFINLogVerbosity)verbosity, text);
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		textNotification
			 * @DisplayName		Text Notification
			 */)", textNotification) {
				LuaFunc();

				FString Text = luaFIN_checkFString(L, 1);
				TOptional<FString> Player;
				if (lua_isstring(L, 2)) Player = luaFIN_checkFString(L, 2);
				for (auto players = kernel->GetWorld()->GetPlayerControllerIterator(); players; ++players) {
					AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
					if (Player.IsSet() && PlayerController->GetPlayerState<AFGPlayerState>()->GetUserName() != *Player) continue;
					kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([PlayerController, Text]() {
						PlayerController->GetGameUI()->ShowTextNotification(FText::FromString(Text));
					})));
					/*kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([PlayerController, Text]() {
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
			 * @LuaFunction		attentionPing
			 * @DisplayName		Attention Ping
			 */)", attentionPing) {
				LuaFunc();

				FVector Position = luaFIN_checkStruct<FVector>(L, 1, true);
				TOptional<FString> Player;
				if (lua_isstring(L, 2)) Player = luaFIN_checkFString(L, 2);
				for (auto players = kernel->GetWorld()->GetPlayerControllerIterator(); players; players++) {
					AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
					if (Player.IsSet() && PlayerController->GetPlayerState<AFGPlayerState>()->GetUserName() != *Player) continue;
					kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([PlayerController, Position]() {
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
			 * @LuaFunction		magicTime
			 * @DisplayName		Magic Time
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
			 * @LuaFunction		getPCIDevices
			 * @DisplayName		Get PCI-Devices
			 */)", getPCIDevices) {
				LuaFunc();

				lua_newtable(L);
				int args = lua_gettop(L);
				UFINClass* Type = nullptr;
				if (args > 0) {
					Type = luaFIN_toFINClass(L, 1, nullptr);
					if (!Type) {
						return 1;
					}
				}
				int i = 1;
				for (TScriptInterface<IFINPciDeviceInterface> Device : kernel->GetPCIDevices()) {
					if (!Device || (Type && !Device.GetObject()->IsA(Cast<UClass>(Type->GetOuter())))) continue;
					luaFIN_pushObject(L, FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject()) / Device.GetObject());
					lua_seti(L, -2, i++);
				}
				return 1;
			}

			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	media
			 * @DisplayName		Media
			 */)", media) {
				UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
				luaFIN_pushObject(L, FINTrace(AFINMediaSubsystem::GetMediaSubsystem(Processor)));
			}
		}
	}
}
