#pragma once

#include "LuaProcessorStateStorage.h"
#include "FicsItKernel/Processor/Processor.h"
#include "FicsItKernel/FicsItKernel.h"
#include "LuaFileSystemAPI.h"
#include "LuaProcessor.generated.h"

class AFINStateEEPROMLua;
struct lua_State;
struct lua_Debug;
namespace FicsItKernel {
	namespace Lua {
		int luaPull(lua_State*);
	}
}

class LuaFileSystemListener : public CodersFileSystem::Listener {
private:
	class UFINLuaProcessor* Parent = nullptr;
public:
	LuaFileSystemListener(class UFINLuaProcessor* InParent) : Parent(InParent) {}
	
	virtual void onUnmounted(CodersFileSystem::Path path, CodersFileSystem::SRef<CodersFileSystem::Device> device) override;
	virtual void onNodeRemoved(CodersFileSystem::Path path, CodersFileSystem::NodeType type) override;
};

enum LuaTickState {
	LUA_SYNC		= 0b00001,
	LUA_ASYNC		= 0b00010,
	LUA_ERROR		= 0b00100,
	LUA_END			= 0b01000,
	LUA_BEGIN		= 0b10000,
	LUA_SYNC_ERROR	= LUA_SYNC | LUA_ERROR,
	LUA_SYNC_END	= LUA_SYNC | LUA_END,
	LUA_ASYNC_BEGIN	= LUA_ASYNC | LUA_BEGIN,
	LUA_ASYNC_ERROR	= LUA_ASYNC | LUA_ERROR,
	LUA_ASYNC_END	= LUA_ASYNC | LUA_END,
};
ENUM_CLASS_FLAGS(LuaTickState);

struct FLuaTickRunnable : public FNonAbandonableTask {
private:
	class FFINLuaProcessorTick* Tick;
	
public:	
	FLuaTickRunnable(class FFINLuaProcessorTick* Tick) : Tick(Tick) {}
	
	void DoWork();

	// ReSharper disable once CppMemberFunctionMayBeStatic
	FORCEINLINE TStatId GetStatId() const {
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};

class FFINLuaProcessorTick {
	// Lua Tick state lua step lengths
	int SyncLen = 2500;
	int SyncErrorLen = 1250;
	int SyncEndLen = 500;
	int AsyncLen = 2500;
	int AsyncErrorLen = 1200;
	int AsyncEndLen = 500;
	
private:
	class UFINLuaProcessor* Processor = nullptr;
	LuaTickState State = LUA_SYNC;
	FLuaTickRunnable Runnable;
	TSharedPtr<FAsyncTask<FLuaTickRunnable>> asyncTask;
	FCriticalSection StateMutex;
	FCriticalSection TickMutex;
	bool bShouldPromote = false;
	bool bShouldDemote = false;
	bool bShouldStop = false;
	bool bShouldReset = false;
	bool bShouldCrash = false;
	bool bDoSync = false;
	bool bWaitForSignal = false;
	TSharedPtr<FFINKernelCrash> ToCrash;
	TPromise<void> AsyncSync;
	TPromise<void> SyncAsync;
	FCriticalSection AsyncSyncMutex;
	

public:

	FFINLuaProcessorTick();
	FFINLuaProcessorTick(class UFINLuaProcessor* Processor);

	~FFINLuaProcessorTick();

	void reset();
	void stop();
	void promote();
	void demote();
	void demoteInAsync();
	void shouldStop();
	void shouldReset();
	void shouldPromote();
	void shouldDemote();
	void shouldWaitForSignal();
	void signalFound();
	void shouldCrash(const TSharedRef<FFINKernelCrash>& Crash);
	int steps() const;
	
	void syncTick();
	bool asyncTick();
	bool postTick();

	void tickHook(lua_State* L);
	int apiReturn(lua_State* L, int args);

	LuaTickState getState() { return State; }
};

UCLASS()
class UFINLuaProcessor : public UFINKernelProcessor {
	GENERATED_BODY()

	friend int FicsItKernel::Lua::luaPull(lua_State* L);
	friend int luaComputerSkip(lua_State* L);
	friend FLuaTickRunnable;
	friend struct FLuaSyncCall;
	friend FFINLuaProcessorTick;

private:
	// Processor cache
	UPROPERTY()
	TWeakObjectPtr<AFINStateEEPROMLua> EEPROM;

	// Lua runtime
	lua_State* luaState = nullptr;
	lua_State* luaThread = nullptr;
	int luaThreadIndex = 0;
	FFINLuaProcessorTick tickHelper;

	// signal pulling
	UPROPERTY(SaveGame)
	int PullState = 0; // 0 = not pulling, 1 = pulling with timeout, 2 = pull indefinitely
	UPROPERTY(SaveGame)
	double Timeout = 0.0;
	UPROPERTY(SaveGame)
	uint64 PullStart = 0;

	// filesystem handling
	TSet<FicsItKernel::Lua::LuaFile> FileStreams;
	CodersFileSystem::SRef<LuaFileSystemListener> FileSystemListener;
	
public:
	UPROPERTY(SaveGame)
	FFINLuaProcessorStateStorage StateStorage;
	
	static UFINLuaProcessor* luaGetProcessor(lua_State* L);
	
	UFINLuaProcessor();
	~UFINLuaProcessor();

	// Begin UObject
	virtual void Serialize(FArchive& Ar) override;
	virtual void BeginDestroy() override;
	// End UObject
	
	// Begin IFGSaveInterface
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	// Begin Processor
	virtual void SetKernel(UFINKernelSystem* InKernel) override;
	virtual void Tick(float InDelta) override;
	virtual void Stop(bool bInIsCrash) override;
	virtual void Reset() override;
	virtual int64 GetMemoryUsage(bool bInRecalc = false) override;
	virtual void SetEEPROM(AFINStateEEPROM* InEEPROM) override;
	// End Processor

	/**
	 * returns the tick helper
	 */
	FFINLuaProcessorTick& GetTickHelper();

	/**
	 * Checks if the pull timeout has been reached
	 */
	bool PullTimeoutReached();
	
	/**
	 * Executes one lua tick sync or async.
	 * Might set after tick cache which should get handled by tick.
	 */
	void LuaTick();

	/**
	 * Sets up the lua environment.
	 * Adds the Computer API in global namespace and adds the FileSystem API in global namespace.
	 *
	 * @param[in]	L	the lua context you want to setup
	 */
	static void LuaSetup(lua_State* L);

	/**
	 * Allows to access the the eeprom used by the processor.
	 * Nullptr if no eeprom is currently set.
	 *
	 * @return	the eeprom used by the processor.
	 */
	AFINStateEEPROMLua* GetEEPROM() const;

	/**
	 * Tries to pop a signal from the signal queue in the network controller
	 * and pushes the resulting values to the given lua stack.
	 *
	 * @param[in]	L	the stack were the values should get pushed to.
	 * @return	the count of values we have pushed.
	 */
	int DoSignal(lua_State* L);
	
	void ClearFileStreams();
	TSet<FicsItKernel::Lua::LuaFile> GetFileStreams() const;

	static void luaHook(lua_State* L, lua_Debug* ar);

	/**
	 * Access the lua processor in the given state registry and checks
	 * if the tick process is in the second stage of execution.
	 * If this is the case yields the runtime with a continuation function
	 * which will return values of the count of args on top of the stack prior
	 * to the yield.
	 * If the tick is in the first stage, we just return.
	 *
	 * @param[in]	L		the lua state all of this should occur
	 * @param[in]	args	the count of arguments we should copy and the continuation should return
	 * @return	if it even returns, returns the same as args
	 */
	static int luaAPIReturn(lua_State* L, int args);

	/**
	 * Returns the lua state
	 */
	lua_State* GetLuaState() const;

	/**
	 * Returns the current lua thread
	 */
	lua_State* GetLuaThread() const;
};

struct FLuaSyncCall {
	UFINLuaProcessor* Processor;

	FLuaSyncCall(lua_State* L) {
		Processor = UFINLuaProcessor::luaGetProcessor(L);
		Processor->tickHelper.demoteInAsync();
	}
};
