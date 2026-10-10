#include "Components/FINWwiseAudioInputComponent.h"

#include "AkAudioEvent.h"
#include "FicsItNetworksModule.h"
#include "TimerManager.h"

UFINWwiseAudioInputComponent::UFINWwiseAudioInputComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	// Keep the game object alive while the owner exists, the owner controls playback itself
	SetAutoDestroy(false);
	StopWhenOwnerDestroyed = true;
}

void UFINWwiseAudioInputComponent::EnsureEventLoaded() {
	if (AkAudioEvent && !AkAudioEvent->IsLoaded()) {
		AkAudioEvent->LoadData();
	}
}

void UFINWwiseAudioInputComponent::PostInputEvent() {
	EnsureEventLoaded();
	PostWhenLoaded(++PostRequest, 50);
}

void UFINWwiseAudioInputComponent::CancelPostInputEvent() {
	++PostRequest;
	if (UWorld* World = GetWorld()) {
		World->GetTimerManager().ClearTimer(PostRetryTimer);
	}
}

void UFINWwiseAudioInputComponent::PostWhenLoaded(uint32 ForRequest, int32 RetriesLeft) {
	if (PostRequest != ForRequest || !AkAudioEvent) return;

	if (AkAudioEvent->IsDataFullyLoaded()) {
		PostAssociatedAudioInputEvent();
		return;
	}

	UWorld* World = GetWorld();
	if (RetriesLeft <= 0 || !World) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Wwise event '%s' of '%s' is not loaded, can not play sound."), *AkAudioEvent->GetName(), *GetPathName());
		return;
	}
	World->GetTimerManager().SetTimer(PostRetryTimer, FTimerDelegate::CreateWeakLambda(this, [this, ForRequest, RetriesLeft]() {
		PostWhenLoaded(ForRequest, RetriesLeft - 1);
	}), 0.1f, false);
}
