#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "FIRHypertubeSubsystem.generated.h"

class AFGBuildablePipeHyperJunction;
class AFGCharacterPlayer;
class UFGPipeConnectionComponentBase;

/**
 * A hypertube junction output forced by a script.
 */
USTRUCT()
struct FICSITREFLECTION_API FFIRHypertubeJunctionForce {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TObjectPtr<AFGBuildablePipeHyperJunction> Junction = nullptr;

	UPROPERTY(SaveGame)
	TObjectPtr<UFGPipeConnectionComponentBase> Output = nullptr;

	/** The user name of the player this output is forced for. Empty for all players. */
	UPROPERTY(SaveGame)
	FString PlayerName;
};

/**
 * Stores the forced outputs of hypertube junctions.
 * The list is replicated, because the client predicts the hypertube movement and needs the same junction outputs as the server.
 */
UCLASS()
class FICSITREFLECTION_API AFIRHypertubeSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
public:
	AFIRHypertubeSubsystem();

	/**
	 * Returns the hypertube subsystem of the given world.
	 * Can return nullptr, e.g. on a client before the subsystem got replicated.
	 */
	static AFIRHypertubeSubsystem* Get(const UObject* WorldContext);

	// Begin AActor
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override { return true; }
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	/**
	 * Forces the given junction to use the given output (if it is reachable from the input the player entered through).
	 * An empty player name forces the output for all players, otherwise only for the player with this user name.
	 * Pass nullptr as output to remove the forced output. Only works with authority.
	 */
	void SetForcedOutput(AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* Output, const FString& PlayerName = FString());

	/** Returns the forced output of the given junction for the given player name (empty = all players) or nullptr if none is set. */
	UFGPipeConnectionComponentBase* GetForcedOutput(const AFGBuildablePipeHyperJunction* Junction, const FString& PlayerName = FString()) const;

	/**
	 * Returns the forced output a player uses when entering the junction through the given input, or nullptr if the normal choice applies.
	 * An output forced for this player wins over an output forced for all players. Outputs not reachable from the input are ignored.
	 */
	UFGPipeConnectionComponentBase* GetForcedOutputForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input, const AFGCharacterPlayer* Player) const;

	/** Returns all connections of the junction that are part of at least one route. */
	static TArray<UFGPipeConnectionComponentBase*> GetConnections(const AFGBuildablePipeHyperJunction* Junction);

	/** Returns all outputs that can be reached when entering the junction through the given input. */
	static TArray<UFGPipeConnectionComponentBase*> GetOutputsForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input);

	/** Returns the end of the given route of the junction that is not the input, or nullptr if the route doesn't exist. */
	static UFGPipeConnectionComponentBase* GetRouteOutput(const AFGBuildablePipeHyperJunction* Junction, int32 RouteIndex, const UFGPipeConnectionComponentBase* Input);

	/** Returns the user name of the player controlling the given character, or an empty string. */
	static FString GetPlayerName(const AFGCharacterPlayer* Player);

private:
	void RemoveInvalidEntries();

	UPROPERTY(SaveGame, Replicated)
	TArray<FFIRHypertubeJunctionForce> ForcedOutputs;
};
