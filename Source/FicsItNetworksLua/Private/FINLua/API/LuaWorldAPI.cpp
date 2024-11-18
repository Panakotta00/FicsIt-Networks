#include "LuaWorldAPI.h"

#include "FGAttentionPingActor.h"
#include "FGGameUI.h"
#include "FGPlayerController.h"
#include "FGTimeSubsystem.h"
#include "FINLuaModule.h"
#include "FINLuaRuntime.h"
#include "FINLuaThreadedRuntime.h"
#include "LuaStruct.h"
#include "TimerManager.h"
#include "Async/Async.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		WorldModule
	 * @DisplayName		World Module
	 * @Dependency		ReflectionSystemObjectModule
	 */)", WorldModule) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		computer
		 */)", computer) {
			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		textNotification(text: string, username: string)
			 * @DisplayName		Text Notification
			 *
			 * This function allows you to prompt a user with the given username, with a text message.
			 *
			 * @parameter	text		string	Text		The Text you want to send as Notification to the user
			 * @parameter	username	string	Username	The username of the user you want to send the notification to
			 */)", textNotification) {
				FLuaSync sync(L);

				FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
				UWorld* world = luaFIN_getWorld(L);

				FString Text = luaFIN_checkFString(L, 1);
				TOptional<FString> Player;
				if (lua_isstring(L, 2)) Player = luaFIN_checkFString(L, 2);
				for (auto players = world->GetPlayerControllerIterator(); players; ++players) {
					AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
					if (Player.IsSet() && PlayerController->GetPlayerState<AFGPlayerState>()->GetUserName() != *Player) continue;
					runtime.TickActions.Enqueue([PlayerController, Text]() {
						PlayerController->GetGameUI()->ShowTextNotification(FText::FromString(Text));
					});
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
				FLuaSync sync(L);

				FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
				UWorld* world = luaFIN_getWorld(L);

				FVector Position = *luaFIN_checkStruct<FVector>(L, 1, true);
				TOptional<FString> Player;
				if (lua_isstring(L, 2)) Player = luaFIN_checkFString(L, 2);
				for (auto players = world->GetPlayerControllerIterator(); players; players++) {
					AFGPlayerController* PlayerController = Cast<AFGPlayerController>(players->Get());
					if (Player.IsSet() && PlayerController->GetPlayerState<AFGPlayerState>()->GetUserName() != *Player) continue;
					runtime.TickActions.Enqueue([PlayerController, Position]() {
						AsyncTask(ENamedThreads::GameThread, [PlayerController, Position]() {
							UClass* Class = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Player/BP_AttentionPingActor.BP_AttentionPingActor_C"));
							AFGAttentionPingActor* PingActor = PlayerController->GetWorld()->SpawnActorDeferred<AFGAttentionPingActor>(Class, FTransform(Position));
							PingActor->SetOwningPlayerState(PlayerController->GetPlayerState<AFGPlayerState>());
							PingActor->FinishSpawning(FTransform(Position));
						});
					});
					break;
				}
				return 0;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		number	time()
			 * @DisplayName		Time
			 *
			 * Returns the number of game seconds passed since the save got created. A game day consists of 24 game hours, a game hour consists of 60 game minutes, a game minute consists of 60 game seconds.
			 *
			 * @return	time	number	The current number of game seconds passed since the creation of the save.
			 */)", time) {
				FLuaSync sync(L);

				UWorld* world = luaFIN_getWorld(L);

				const AFGTimeOfDaySubsystem* Subsystem = AFGTimeOfDaySubsystem::Get(world);
				lua_pushnumber(L, Subsystem->GetPassedDays() * 86400 + Subsystem->GetDaySeconds());
				return 1;
			}
		}
	}

	void luaFIN_setWorld(lua_State* L, UWorld* world) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("world"), world);
	}

	UWorld* luaFIN_getWorld(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		UWorld** value = reinterpret_cast<UWorld**>(runtime.GlobalPointers.Find(TEXT("world")));
		fgcheck(value != nullptr);
		return *value;
	}
}
