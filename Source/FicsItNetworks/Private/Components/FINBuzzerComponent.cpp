#include "Components/FINBuzzerComponent.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace {
	/** How long the owner stays awake after the last beep */
	constexpr float AwakeDuration = 10.f;
	/** How long the first beep after waking up waits at most for the channels to the clients */
	constexpr double MaxChannelWait = 0.25;
}

UFINBuzzerComponent::UFINBuzzerComponent() {
	SetIsReplicatedByDefault(true);
}

void UFINBuzzerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	PendingCalls.Empty();
	Super::EndPlay(EndPlayReason);
}

void UFINBuzzerComponent::Beep(float Frequency, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve, float Volume) {
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	KeepAwake();
	PendingCalls.Add({false, Frequency, AttackTime, AttackCurve, DecayTime, DecayCurve, Volume});
	SendPendingCalls();
}

void UFINBuzzerComponent::StopBeep() {
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	KeepAwake();
	PendingCalls.Add({true});
	SendPendingCalls();
}

void UFINBuzzerComponent::Multicast_Beep_Implementation(float Frequency, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve, float Volume) {
	OnBeep.Broadcast(Frequency, AttackTime, AttackCurve, DecayTime, DecayCurve, Volume);
}

void UFINBuzzerComponent::Multicast_StopBeep_Implementation() {
	OnStop.Broadcast();
}

void UFINBuzzerComponent::KeepAwake() {
	AActor* Owner = GetOwner();
	if (!bKeptAwake) {
		bKeptAwake = true;
		DormancyBeforeAwake = Owner->NetDormancy;
		if (Owner->NetDormancy > DORM_Awake) {
			// The channels to the clients only open with the next replication frame
			Owner->SetNetDormancy(DORM_Awake);
			bWaitingForChannels = true;
			WaitingForChannelsSince = GetWorld()->GetTimeSeconds();
		}
	}
	GetWorld()->GetTimerManager().SetTimer(AwakeTimer, this, &UFINBuzzerComponent::RestoreDormancy, AwakeDuration, false);
}

void UFINBuzzerComponent::RestoreDormancy() {
	if (!PendingCalls.IsEmpty()) {
		GetWorld()->GetTimerManager().SetTimer(AwakeTimer, this, &UFINBuzzerComponent::RestoreDormancy, AwakeDuration, false);
		return;
	}
	bKeptAwake = false;
	if (GetOwner()->NetDormancy != DormancyBeforeAwake) {
		GetOwner()->SetNetDormancy(DormancyBeforeAwake);
	}
}

void UFINBuzzerComponent::SendPendingCalls() {
	if (PendingCalls.IsEmpty()) return;

	if (bWaitingForChannels) {
		// Clients the buzzer isn't relevant for never get a channel, so don't wait forever
		const bool bTimedOut = GetWorld()->GetTimeSeconds() - WaitingForChannelsSince > MaxChannelWait;
		if (!bTimedOut && !HasChannelsToAllClients()) {
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UFINBuzzerComponent::SendPendingCalls);
			return;
		}
		bWaitingForChannels = false;
	}

	TArray<FPendingCall> Calls = MoveTemp(PendingCalls);
	for (const FPendingCall& Call : Calls) {
		if (Call.bStop) {
			Multicast_StopBeep();
		} else {
			Multicast_Beep(Call.Frequency, Call.AttackTime, Call.AttackCurve, Call.DecayTime, Call.DecayCurve, Call.Volume);
		}
	}
}

bool UFINBuzzerComponent::HasChannelsToAllClients() const {
	const UNetDriver* Driver = GetWorld()->GetNetDriver();
	if (!Driver) return true;
	for (UNetConnection* Connection : Driver->ClientConnections) {
		// Connections still joining don't get multicasts anyway
		if (!Connection || !Connection->ViewTarget) continue;
		if (!Connection->FindActorChannelRef(GetOwner())) return false;
	}
	return true;
}
