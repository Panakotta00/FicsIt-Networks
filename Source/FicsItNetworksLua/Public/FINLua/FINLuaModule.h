#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"
#include "FINLuaModule.generated.h"

USTRUCT()
struct FFINLuaMetatable {
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	TMap<FString, lua_CFunction> Functions;
};

/**
 * A FINLuaModule is collection of global functions, tables, metatables and more.
 * These modules can be dynamically discovered and loaded at runtime.
 * They allow the easy extension of the Lua Runtime through external sources (like extension mods).
 * Another main purpose is to provide documentation for the different functions etc. to be usable within the game.
 * A module can also be just a wrapper around a simple Lua library, but providing documentation.
 */
UCLASS(Abstract)
class UFINLuaModule : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FString InternalName;

	UPROPERTY()
	FText DisplayName;

	UPROPERTY()
	FText Description;

	UPROPERTY()
	TArray<FFINLuaMetatable> Metatables;

	virtual void SetupModule(lua_State* L);
};
