#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "FINBuzzerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FFINBuzzerBeep, float, Frequency, float, AttackTime, float, AttackCurve, float, DecayTime, float, DecayCurve, float, Volume);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINBuzzerStop);

/**
 * Sends the beeps of a buzzer from the server to all clients.
 * The buzzer blueprint calls Beep/StopBeep on the server and plays the sound in OnBeep/OnStop, which fire on the server and all clients.
 *
 * Buildings are net dormant, and a multicast to a dormant actor gets dropped for every client without an open actor channel.
 * So the component keeps its owner awake until a while after the last beep,
 * and the first beep after waking it up waits a moment for the channels to the clients.
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

	// Begin UActorComponent
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End UActorComponent

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

	void KeepAwake();
	void RestoreDormancy();
	void SendPendingCalls();
	bool HasChannelsToAllClients() const;

	struct FPendingCall {
		bool bStop = false;
		float Frequency = 0.f;
		float AttackTime = 0.f;
		float AttackCurve = 0.f;
		float DecayTime = 0.f;
		float DecayCurve = 0.f;
		float Volume = 0.f;
	};
	TArray<FPendingCall> PendingCalls;

	FTimerHandle AwakeTimer;
	bool bKeptAwake = false;
	TEnumAsByte<ENetDormancy> DormancyBeforeAwake = DORM_Initial;
	bool bWaitingForChannels = false;
	double WaitingForChannelsSince = 0.0;
};
