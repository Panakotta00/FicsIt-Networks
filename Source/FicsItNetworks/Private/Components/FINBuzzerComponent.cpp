#include "Components/FINBuzzerComponent.h"
#include "Async/Async.h"

UFINBuzzerComponent::UFINBuzzerComponent() {
	SetIsReplicatedByDefault(true);
}

void UFINBuzzerComponent::Beep(float Frequency, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve, float Volume) {
	if (!IsInGameThread()) {
		// RPCs may only be sent from the game thread, Lua can call this from its own thread
		AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<UFINBuzzerComponent>(this), Frequency, AttackTime, AttackCurve, DecayTime, DecayCurve, Volume]() {
			if (UFINBuzzerComponent* This = WeakThis.Get()) This->Beep(Frequency, AttackTime, AttackCurve, DecayTime, DecayCurve, Volume);
		});
		return;
	}
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	Multicast_Beep(Frequency, AttackTime, AttackCurve, DecayTime, DecayCurve, Volume);
}

void UFINBuzzerComponent::StopBeep() {
	if (!IsInGameThread()) {
		AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<UFINBuzzerComponent>(this)]() {
			if (UFINBuzzerComponent* This = WeakThis.Get()) This->StopBeep();
		});
		return;
	}
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	Multicast_StopBeep();
}

void UFINBuzzerComponent::Multicast_Beep_Implementation(float Frequency, float AttackTime, float AttackCurve, float DecayTime, float DecayCurve, float Volume) {
	OnBeep.Broadcast(Frequency, AttackTime, AttackCurve, DecayTime, DecayCurve, Volume);
}

void UFINBuzzerComponent::Multicast_StopBeep_Implementation() {
	OnStop.Broadcast();
}
