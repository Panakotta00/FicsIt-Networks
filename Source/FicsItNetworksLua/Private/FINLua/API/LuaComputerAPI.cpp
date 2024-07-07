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
#define LOCTEXT_NAMESPACE "ComputerModule"
	BeginLuaModule(Computer, LOCTEXT("DisplayName", "Computer Module"), LOCTEXT("Description", "The Computer Module provides the Computer Library."))
#define LOCTEXT_NAMESPACE "ComputerLibrary"
	BeginLibrary(computer, LOCTEXT("DisplayName", "Computer Library"), LOCTEXT("Description", "The Computer Library provides functions for interaction with the computer and especially the Lua Runtime."))

	FieldFunction(getMemory, LOCTEXT("getMemory_DisplayName", "Get Memory"), LOCTEXT("getMemory_Description", "")) {
		LuaFunc()

		int64 Usage = kernel->GetMemoryUsage();
		int64 Capacity = kernel->GetCapacity();
		lua_pushinteger(L, Usage);
		lua_pushinteger(L, Capacity);
		return 2;
	}

	FieldFunction(getInstance, LOCTEXT("getInstance_DisplayName", "Get Instance"), LOCTEXT("getInstance_Description", "")) {
		LuaFunc()

		luaFIN_pushObject(L, UFINNetworkUtils::RedirectIfPossible(FFINNetworkTrace(kernel->GetNetwork()->GetComponent().GetObject())));
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(reset, LOCTEXT("reset_DisplayName", "Reset"), LOCTEXT("reset_Description", "")) {
		LuaFunc();

		processor->GetTickHelper().shouldReset();
		lua_yield(L, 0);
		return 0;
	}

	FieldFunction(stop, LOCTEXT("stop_DisplayName", "Stop"), LOCTEXT("stop_Description", "")) {
		LuaFunc();

		processor->GetTickHelper().shouldStop();
		lua_yield(L, 0);
		return 0;
	}

	FieldFunction(panic, LOCTEXT("panic_DisplayName", "Panic"), LOCTEXT("panic_Description", "")) {
		LuaFunc();

	    processor->GetTickHelper().shouldCrash(MakeShared<FFINKernelCrash>(FString("PANIC! '") + luaL_checkstring(L, 1) + "'"));
		kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel]() {
			kernel->GetAudio()->Beep();
		})));
		lua_yield(L, 0);
		return 0;
	}

	FieldFunction(skip, LOCTEXT("skip_DisplayName", "Skip"), LOCTEXT("skip_Description", "")) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		processor->GetTickHelper().shouldPromote();
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(promote, LOCTEXT("promote_DisplayName", "Promote"), LOCTEXT("promote_Description", "")) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		processor->GetTickHelper().shouldPromote();
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(demote, LOCTEXT("demote_DisplayName", "Demote"), LOCTEXT("demote_Description", "")) {
		FLuaSyncCall Sync(L);
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(isPromoted, LOCTEXT("isPromoted_DisplayName", "Is Promoted"), LOCTEXT("isPromoted_Description", "")) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		bool bPromoted = (bool)(processor->GetTickHelper().getState() & LUA_ASYNC);
		lua_pushboolean(L, bPromoted);
		return 1;
	}

	FieldFunction(state, LOCTEXT("state_DisplayName", "State"), LOCTEXT("state_Description", "")) {
		UFINLuaProcessor* processor = UFINLuaProcessor::luaGetProcessor(L);
		int state = 0;
		if (processor->GetTickHelper().getState() & LUA_ASYNC) {
			state = 1;
		}
		lua_pushinteger(L, state);
		return UFINLuaProcessor::luaAPIReturn(L, 1);
	}

	FieldFunction(beep, LOCTEXT("beep_DisplayName", "Beep"), LOCTEXT("beep_Description", "")) {
		LuaFunc();

		float pitch = 1;
		if (lua_isnumber(L, 1)) pitch = lua_tonumber(L, 1);
		kernel->PushFuture(MakeShared<TFINDynamicStruct<FFINFuture>>(FFINFunctionFuture([kernel, pitch]() {
		    kernel->GetAudio()->Beep(pitch);
		})));
		return UFINLuaProcessor::luaAPIReturn(L, 0);
	}

	FieldFunction(setEEPROM, LOCTEXT("setEEPROM_DisplayName", "Set EEPROM"), LOCTEXT("setEEPROM_Description", "")) {
		LuaFunc();

		AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
		if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
		size_t len;
		const char* str = luaL_checklstring(L, 1, &len);
		FUTF8ToTCHAR Conv(str, len);
		eeprom->SetCode(FString(Conv.Length(), Conv.Get()));
		return 0;
	}

	FieldFunction(getEEPROM, LOCTEXT("getEEPROM_DisplayName", "Get EEPROM"), LOCTEXT("getEEPROM_Description", "")) {
		LuaFunc();

        const AFINStateEEPROMLua* eeprom = Cast<UFINLuaProcessor>(kernel->GetProcessor())->GetEEPROM();
		if (!IsValid(eeprom)) return luaL_error(L, "no eeprom set");
		FString Code = eeprom->GetCode();
		FTCHARToUTF8 Conv(*Code, Code.Len());
		lua_pushlstring(L, Conv.Get(), Conv.Length());
		return 1;
	}

	FieldFunction(time, LOCTEXT("time_DisplayName", "Time"), LOCTEXT("time_Description", "")) {
		LuaFunc();

		const AFGTimeOfDaySubsystem* Subsystem = AFGTimeOfDaySubsystem::Get(kernel);
		lua_pushnumber(L, Subsystem->GetPassedDays() * 86400 + Subsystem->GetDaySeconds());
		return 1;
	}

	FieldFunction(millis, LOCTEXT("millis_DisplayName", "Millis"), LOCTEXT("millis_Description", "")) {
		LuaFunc();

		lua_pushinteger(L, kernel->GetTimeSinceStart());
		return 1;
	}

	FieldFunction(log, LOCTEXT("log_DisplayName", "Log"), LOCTEXT("log_Description", "")) {
		LuaFunc();

		int verbosity = luaL_checknumber(L, 1);
		FString text = luaFIN_checkFString(L, 2);
		verbosity = FMath::Clamp(verbosity, 0, EFINLogVerbosity::FIN_Log_Verbosity_Max);
		kernel->GetLog()->PushLogEntry((EFINLogVerbosity)verbosity, text);
		return 0;
	}

	FieldFunction(textNotification, LOCTEXT("textNotification_DisplayName", "Text Notification"), LOCTEXT("textNotification_Description", "")) {
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

	FieldFunction(attentionPing, LOCTEXT("attentionPing_DisplayName", "Attention Ping"), LOCTEXT("attentionPing_Description", "")) {
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

	FieldFunction(magicTime, LOCTEXT("magicTime_DisplayName", "Magic Time"), LOCTEXT("magicTime_Description", "")) {
		FDateTime Now = FDateTime::UtcNow();
		lua_pushinteger(L, Now.ToUnixTimestamp());
		FTCHARToUTF8 ConvertStr(*Now.ToString());
		lua_pushlstring(L, ConvertStr.Get(), ConvertStr.Length());
		FTCHARToUTF8 ConvertIso(*Now.ToIso8601());
		lua_pushlstring(L, ConvertIso.Get(), ConvertIso.Length());
		return 3;
	}

	FieldFunction(getPCIDevices, LOCTEXT("getPCIDevices_DisplayName", "Get PCI-Devices"), LOCTEXT("getPCIDevices_Description", "")) {
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

	FieldBare(media, LOCTEXT("media_DisplayName", "Media"), LOCTEXT("media_Description", "")) {
		UFINLuaProcessor* Processor = UFINLuaProcessor::luaGetProcessor(L);
		luaFIN_pushObject(L, FINTrace(AFINMediaSubsystem::GetMediaSubsystem(Processor)));
	}

	EndLibrary()

	EndLuaModule()
}
