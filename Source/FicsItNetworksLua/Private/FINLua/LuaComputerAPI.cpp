#include "FINLua/LuaComputerAPI.h"

#include "FGAttentionPingActor.h"
#include "FGPlayerController.h"
#include "FINLua/LuaClass.h"
#include "FINLua/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FINStateEEPROMLua.h"
#include "Network/FINNetworkUtils.h"
#include "FGTimeSubsystem.h"
#include "FicsItNetworksLuaModule.h"
#include "FINLua/LuaStruct.h"
#include "UI/FGGameUI.h"
#include "UI/FINNotificationMessage.h"

#define LuaFunc(funcName) \
int funcName(lua_State* L) { \
	UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L); \
	UFINKernelSystem* kernel = processor->GetKernel(); \
	FLuaSyncCall SyncCall(L);
#define LuaFuncEnd() }

namespace FINLua {
	LuaFunc(luaComputerGetInstance) {
		luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject())));
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	} LuaFuncEnd()

	LuaFunc(luaComputerReset) {
		processor->GetTickHelper().shouldReset();
		lua_yield(L, 0);
		return 0;
	} LuaFuncEnd()

	LuaFunc(luaComputerStop) {
		processor->GetTickHelper().shouldStop();
		lua_yield(L, 0);
		return 0;
	} LuaFuncEnd()

	LuaFunc(luaComputerPanic) {
	    processor->GetTickHelper().shouldCrash(MakeShared<FFINKernelCrash>(FString("PANIC! '") + luaL_checkstring(L, 1) + "'"));
		kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
			kernel->GetAudio()->Beep();
		})));
		lua_yield(L, 0);
		return 0;
	} LuaFuncEnd()

	int luaComputerSkipContinue(lua_State* L, int status, lua_KContext ctx) {
		return 0;
	}

	int luaComputerSkip(lua_State* L) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		processor->GetTickHelper().shouldPromote();
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	int luaComputerPromote(lua_State* L) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		processor->GetTickHelper().shouldPromote();
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}
	
	int luaComputerDemote(lua_State* L) {
		FLuaSyncCall Sync(L);
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	int luaComputerIsPromoted(lua_State* L) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		bool bPromoted = (bool)(processor->GetTickHelper().getState() & LUA_ASYNC);
		lua_pushboolean(L, bPromoted);
		return 1;
	}

	int luaComputerState(lua_State* L) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		int state = 0;
		if (processor->GetTickHelper().getState() & LUA_ASYNC) {
			state = 1;
		}
		lua_pushinteger(L, state);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}
	
	LuaFunc(luaComputerBeep) {
		float pitch = 1;
		if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
		kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
		    kernel->GetAudio()->Beep(pitch);
		})));
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	} LuaFuncEnd()

	LuaFunc(luaComputerSetEEPROM) {
		AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
		if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
		size_t len;
		const char* str = luaL_checklstring(L, 1, &len);
		FUTF8ToTCHAR Conv(str, len);
		eeprom->SetCode(FString(Conv.Length(), Conv.Get()));
		return 0;
	} LuaFuncEnd()

	LuaFunc(luaComputerGetEEPROM) {
        const AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
		if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
		FString Code = eeprom->GetCode();
		FTCHARToUTF8 Conv(*Code, Code.Len());
		lua_pushlstring(L, Conv.Get(), Conv.Length());
		return 1;
	} LuaFuncEnd()

	LuaFunc(luaComputerTime) {
		const AFGTimeOfDaySubsystem* Subsystem = AFGTimeOfDaySubsystem::Get(kernel);
		lua_pushnumber(L, Subsystem->GetPassedDays() * 86400 + Subsystem->GetDaySeconds());
		return 1;
	} LuaFuncEnd()
	
	LuaFunc(luaComputerMillis) {
		lua_pushinteger(L, kernel->GetTimeSinceStart());
		return 1;
	} LuaFuncEnd()

	LuaFunc(luaComputerTextNotification) {
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
	} LuaFuncEnd()
	
	LuaFunc(luaComputerAttentionPing) {
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
	} LuaFuncEnd()

	LuaFunc(luaComputerMagicTime) {
		FDateTime Now = FDateTime::UtcNow();
		lua_pushinteger(L, Now.ToUnixTimestamp());
		FTCHARToUTF8 ConvertStr(*Now.ToString());
		lua_pushlstring(L, ConvertStr.Get(), ConvertStr.Length());
		FTCHARToUTF8 ConvertIso(*Now.ToIso8601());
		lua_pushlstring(L, ConvertIso.Get(), ConvertIso.Length());
		return 3;
	} LuaFuncEnd()
	
	LuaFunc(luaComputerPCIDevices) {
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
	} LuaFuncEnd()

	LuaFunc(luaComputerMemory) {
		int64 Usage = kernel->GetMemoryUsage();
		int64 Capacity = kernel->GetCapacity();
		lua_pushinteger(L, Usage);
		lua_pushinteger(L, Capacity);
		return 2;
	} LuaFuncEnd()

	static const luaL_Reg luaComputerLib[] = {
		{"getMemory", luaComputerMemory},
		{"getInstance", luaComputerGetInstance},
		{"reset", luaComputerReset},
		{"stop", luaComputerStop},
		{"panic", luaComputerPanic},
		{"skip", luaComputerSkip},
		{"promote", luaComputerPromote},
		{"demote", luaComputerDemote},
		{"isPromoted", luaComputerIsPromoted},
		{"state", luaComputerState},
		{"beep", luaComputerBeep},
		{"setEEPROM", luaComputerSetEEPROM},
		{"getEEPROM", luaComputerGetEEPROM},
		{"time", luaComputerTime},
		{"millis", luaComputerMillis},
		{"magicTime", luaComputerMagicTime},
		{"getPCIDevices", luaComputerPCIDevices},
		{"textNotification", luaComputerTextNotification},
		{"attentionPing", luaComputerAttentionPing},
		{nullptr, nullptr}
	};
	
	void setupComputerAPI(lua_State* L) {
		PersistSetup("Computer", -2);
		luaL_newlibtable(L, luaComputerLib);
		luaL_setfuncs(L, luaComputerLib, 0);
		PersistTable("Lib", -1);
		lua_setglobal(L, "computer");
	}
}
