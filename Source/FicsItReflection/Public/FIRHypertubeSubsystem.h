#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "FIRHypertubeSubsystem.generated.h"

class AFGBuildablePipeHyperJunction;
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
	 * Forces the given junction to always use the given output (if it is reachable from the input the player entered through).
	 * Pass nullptr as output to remove the forced output. Only works with authority.
	 */
	void SetForcedOutput(AFGBuildablePipeHyperJunction* Junction, UFGPipeConnectionComponentBase* Output);

	/** Returns the forced output of the given junction or nullptr if none is set. */
	UFGPipeConnectionComponentBase* GetForcedOutput(const AFGBuildablePipeHyperJunction* Junction) const;

	/** Returns the forced output of the given junction, but only if it can be reached from the given input. */
	UFGPipeConnectionComponentBase* GetForcedOutputForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input) const;

	/** Returns all connections of the junction that are part of at least one route. */
	static TArray<UFGPipeConnectionComponentBase*> GetConnections(const AFGBuildablePipeHyperJunction* Junction);

	/** Returns all outputs that can be reached when entering the junction through the given input. */
	static TArray<UFGPipeConnectionComponentBase*> GetOutputsForInput(const AFGBuildablePipeHyperJunction* Junction, const UFGPipeConnectionComponentBase* Input);

private:
	void RemoveInvalidEntries();

	UPROPERTY(SaveGame, Replicated)
	TArray<FFIRHypertubeJunctionForce> ForcedOutputs;
};
