#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FINBuzzerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FFINBuzzerBeep, float, Frequency, float, AttackTime, float, AttackCurve, float, DecayTime, float, DecayCurve, float, Volume);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINBuzzerStop);

/**
 * Sends the beeps of a buzzer from the server to all clients.
 * The buzzer blueprint calls Beep/StopBeep on the server and plays the sound in OnBeep/OnStop, which fire on the server and all clients.
 *
 * The building stays net dormant. The replication graph opens a channel for the multicast to every client within the net cull distance.
 */
UCLASS(ClassGroup=(FicsItNetworks), meta=(BlueprintSpawnableComponent))
class FICSITNETWORKS_API UFINBuzzerComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Buzzer")
	FFINBuzzerBeep OnBeep;

	UPROPERTY(BlueprintAssignable, Category="Buzzer")
	FFINBuzzerStop OnStop;

	UFINBuzzerComponent();

	/** Plays a beep with the given parameters on the server and all clients. Does nothing on clients. */
	UFUNCTION(BlueprintCallable, Category="Buzzer")
	void Beep(float Frequency, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve, float Volume);

	/** Stops the beep on the server and all clients. Does nothing on clients. */
	UFUNCTION(BlueprintCallable, Category="Buzzer")
	void StopBeep();

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Beep(float Frequency, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve, float Volume);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopBeep();
};
