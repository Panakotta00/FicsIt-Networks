#include "FINComponentUtility.h"

#include "WindowsPlatformApplicationMisc.h"
#include "EngineGlobals.h"
#include "VorbisAudioInfo.h"
//#include "Interfaces/IAudioFormat.h"

#include "FGSaveSystem.h"
#include "FGPlayerController.h"

#include "Network/FINNetworkAdapter.h"

#include <filesystem>
#include <fstream>
#include <sstream>

bool UFINComponentUtility::bAllowUsing = true;

UFINNetworkConnector* UFINComponentUtility::GetNetworkConnectorFromHit(FHitResult hit) {
	if (!hit.bBlockingHit) return nullptr;

	UFINNetworkConnector* connector = nullptr;
	FVector pos;

	if (!hit.Actor.IsValid()) return nullptr;
	
	TArray<UActorComponent*> connectors = hit.Actor->GetComponentsByClass(UFINNetworkConnector::StaticClass());

	for (auto con : connectors) {
		if (!Cast<USceneComponent>(con)) continue;

		FVector npos = Cast<USceneComponent>(con)->GetComponentLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = (UFINNetworkConnector*)con;
		}
	}

	if (connector) return connector;
	
	TArray<UActorComponent*> adapters = hit.Actor->GetComponentsByClass(UFINNetworkAdapterReference::StaticClass());
	
	for (auto adapterref : adapters) {
		if (!adapterref || !((UFINNetworkAdapterReference*)adapterref)->Ref) continue;

		FVector npos = ((UFINNetworkAdapterReference*)adapterref)->Ref->GetActorLocation();
		if (!connector || (pos - hit.ImpactPoint).Size() > (npos - hit.ImpactPoint).Size()) {
			pos = npos;
			connector = ((UFINNetworkAdapterReference*)adapterref)->Ref->Connector;
		}
	}

	return connector;
}

void UFINComponentUtility::ClipboardCopy(FString str) {
	FWindowsPlatformApplicationMisc::ClipboardCopy(*str);
}

void UFINComponentUtility::SetAllowUsing(UObject* WorldContextObject, bool newUsing) {
	if (!newUsing) {
		// Cast<AFGCharacterPlayer>(WorldContextObject->GetWorld()->GetFirstPlayerController()->GetCharacter())->SetBestUableActor(nullptr);
		// TODO: Find workaround if needed
	}

	bAllowUsing = newUsing;
}
