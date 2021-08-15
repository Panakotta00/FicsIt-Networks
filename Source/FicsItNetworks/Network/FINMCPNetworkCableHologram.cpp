#include "FINMCPNetworkCableHologram.h"


#include "FINMCPAdvConnector.h"
#include "FINMCPConnector.h"
#include "FINMCPNetworkCable.h"
#include "FicsItNetworks/FINComponentUtility.h"


AFINMCPNetworkCableHologram::AFINMCPNetworkCableHologram() {
	UStaticMesh* cableMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Network/ThinNetworkCable/SM_ThinNetworkCable2.SM_ThinNetworkCable2"));
	//UStaticMesh* adapterMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Network/Mesh_Adapter.Mesh_Adapter"));

	Cable->SetStaticMesh(cableMesh);
}


bool AFINMCPNetworkCableHologram::TrySnapToActor(const FHitResult& hitResult) {
	UStaticMeshComponent* Adapter = IsInSecondStep() ? Adapter1 : Adapter2;
	AFGBuildableHologram* PlugHolo = IsInSecondStep() ? PlugHologram1 : PlugHologram2;
	//AFGBuildableHologram* PoleHolo = IsInSecondStep() ? PoleHologram1 : PoleHologram2;
	Adapter->SetVisibility(false, true);
	//PoleHolo->SetDisabled(true);
	PlugHolo->SetDisabled(true);
	
	// Check if hit actor is valid
	auto actor = hitResult.Actor.Get();
	if (!actor) {
		Snapped = {FIN_NOT_SNAPPED};
		OnInvalidHitResult();
		return false;
	}

	//FStringClassReference RecipeClassReference = FStringClassReference(TEXT("/<ModReference>/<FolderWhereThe BPis>/<BPname>.<BPname>_C"));
		
	// Try to find network connector
	UFINNetworkConnectionComponent* Connector = UFINComponentUtility::GetNetworkConnectorFromHit(hitResult);
	UFINMCPAdvConnector* MCPAdvConnector = dynamic_cast<UFINMCPAdvConnector*>(Connector);
	UFINMCPConnector* MCPConnector = dynamic_cast<UFINMCPConnector*>(Connector);
	auto CS = actor->GetClass()->GetPathName();
	if (CS.Equals(FString("/FicsItNetworks/Components/SmallNetworkWallPlug/Item_SmallNetworkWallPlug.Item_SmallNetworkWallPlug_C")) || MCPAdvConnector || MCPConnector) {
		// Use networks connector as snapping point
		Snapped = {FIN_CONNECTOR, Connector};
		SetHologramLocationAndRotation(hitResult);
		return true;
	}else if(actor ) {
	}


	Snapped = { FIN_NOT_SNAPPED };

	return false;
}

FVector AFINMCPNetworkCableHologram::GetConnectorPos(const FFINCablePlacementStepInfo* Info) const {
	if(Info->SnapType == FIN_PLUG) {
		return Cast<AActor>(Info->SnappedObj)->GetActorLocation() + Cast<AActor>(Info->SnappedObj)->GetActorRotation().RotateVector(FVector(7.5,0, 0));
	}
	return Info->GetConnectorPos();
}

FRotator AFINMCPNetworkCableHologram::GetConnectorRot(const FFINCablePlacementStepInfo* Info) const {
	return Info->GetConnectorRot();
}

int32 AFINMCPNetworkCableHologram::GetBaseCostMultiplier() const {
	if (Snapped.SnapType == FIN_NOT_SNAPPED || From.SnapType == FIN_NOT_SNAPPED) return 0.0;
	return (GetConnectorPos(&Snapped) - GetConnectorPos(&Snapped)).Size()/1000.0;
}

void AFINMCPNetworkCableHologram::SetHologramLocationAndRotation(const FHitResult& hit) {
	UStaticMeshComponent* Adapter = IsInSecondStep() ? Adapter1 : Adapter2;
	AFGBuildableHologram* PlugHolo = IsInSecondStep() ? PlugHologram1 : PlugHologram2;
	//AFGBuildableHologram* PoleHolo = IsInSecondStep() ? PoleHologram1 : PoleHologram2;
	if (Snapped.SnapType == FIN_NOT_SNAPPED) {
		AActor* Actor = hit.GetActor();
		if (Actor) {
			if ((Actor->IsA<AFGBuildableFoundation>() || Actor->IsA<AFGBuildableWall>()) && !hit.Normal.Equals(FVector(0, 0, 1), 0.1)) {
				Snapped = { FIN_PLUG, PlugHolo };
				PlugHolo->SetHologramLocationAndRotation(hit);
			}// else {
			//	Snapped = { FIN_POLE, PoleHolo };
			//	PoleHolo->SetHologramLocationAndRotation(hit);
			//}
		}
	}
	
	// place to snap start
	if (!IsInSecondStep()) this->RootComponent->SetWorldLocation(GetConnectorPos(&Snapped));
	else this->RootComponent->SetWorldLocation(GetConnectorPos(&From));

	// update snapp data and sub holos
	UpdateSnapped();
	
	bool validSnap = IsSnappedValid();
	UpdateMeshValidity(validSnap);

	// update cable mesh
	float offset = 250.0;
	FVector start;
	start.X = start.Y = start.Z = 0;
	FVector end = RootComponent->GetComponentToWorld().InverseTransformPosition(GetConnectorPos(&Snapped));
	FVector start_t = end;
	end = end + 0.0001;
	if ((FMath::Abs(start_t.X) < 10 || FMath::Abs(start_t.Y) < 10) && FMath::Abs(start_t.Z) <= offset) offset = 1;
	start_t.Z -= offset;
	FVector end_t = end;
	end_t.Z += offset;
	Cable->SetStartAndEnd(start, start_t, end, end_t, true);

	// update snap visibilty to
	Adapter->SetVisibility(false, false);
	//PoleHolo->SetDisabled(true);
	PlugHolo->SetDisabled(true);
	switch (Snapped.SnapType) {
	case FIN_SETTINGS:
		Adapter->SetVisibility(true, true);
		Adapter->SetRelativeLocation(end);
		Adapter->SetWorldRotation(GetConnectorRot(&Snapped));
		break;
	case FIN_POLE:
	case FIN_PLUG:
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetDisabled(false);
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetHologramLocationAndRotation(hit);
		Cast<AFGBuildableHologram>(Snapped.SnappedObj)->SetScrollRotateValue(GetScrollRotateValue());
		break;
	default: ;
	}

	UpdateMeshValidity(validSnap);
}

#pragma optimize("", off)
void AFINMCPNetworkCableHologram::SpawnChildren(AActor* hologramOwner, FVector spawnLocation, APawn* hologramInstigator) {
	//TSubclassOf<UFGRecipe> RecipePole = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Network/NetworkPole/Recipe_NetworkPole.Recipe_NetworkPole_C"));
	TSubclassOf<UFGRecipe> RecipePlug = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Components/SmallNetworkWallPlug/Recipe_SmallNetworkWallPlug.Recipe_SmallNetworkWallPlug_C"));
	//PoleHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePole, hologramOwner, spawnLocation, hologramInstigator));
	//PoleHologram1->SetDisabled(true);
	PlugHologram1 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePlug, hologramOwner, spawnLocation, hologramInstigator));
	PlugHologram1->SetDisabled(true);
	//PoleHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePole, hologramOwner, spawnLocation, hologramInstigator));
	//PoleHologram2->SetDisabled(true);
	PlugHologram2 = Cast<AFGBuildableHologram>(AFGHologram::SpawnChildHologramFromRecipe(this, RecipePlug, hologramOwner, spawnLocation, hologramInstigator));
	PlugHologram2->SetDisabled(true);
}
#pragma optimize("", on)


void AFINMCPNetworkCableHologram::UpdateMeshValidity(bool bValid) {
	UMaterialInstance* Material = bValid ? mValidPlacementMaterial : mInvalidPlacementMaterial;
	Cable->SetMaterial(0, Material);
	Adapter1->SetMaterial(0, Material);
	Adapter2->SetMaterial(0, Material);
	//for (UActorComponent* Comp : PoleHologram1->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
	//for (UActorComponent* Comp : PoleHologram2->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
	for (UActorComponent* Comp : PlugHologram1->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
	for (UActorComponent* Comp : PlugHologram2->GetComponentsByClass(UStaticMeshComponent::StaticClass())) Cast<UStaticMeshComponent>(Comp)->SetMaterial(0, Material);
}