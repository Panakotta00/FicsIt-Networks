#include "FINModularIndicatorPoleModule.h"
#include "FGColoredInstanceMeshProxy.h"

AFINModularIndicatorPoleModule::AFINModularIndicatorPoleModule() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINModularIndicatorPoleModule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFINModularIndicatorPoleModule::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
}

void AFINModularIndicatorPoleModule::BeginPlay() {
	Super::BeginPlay();
	
	RerunConstructionScripts();
}

void AFINModularIndicatorPoleModule::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

bool AFINModularIndicatorPoleModule::ShouldSave_Implementation() const {
	return true;
}

int32 AFINModularIndicatorPoleModule::GetDismantleRefundReturnsMultiplier() const {
	return 1;
}

void AFINModularIndicatorPoleModule::GetChildDismantleActors_Implementation(TArray<AActor*>& out_ChildDismantleActors) const {
	Super::GetChildDismantleActors_Implementation(out_ChildDismantleActors);
	if(IsValid(NextChild)) {
		out_ChildDismantleActors.Add(NextChild);
		NextChild->GetChildDismantleActors_Implementation(out_ChildDismantleActors);
	}
}

AFINModularIndicatorPoleModule* AFINModularIndicatorPoleModule::netFunc_getNext() {
	if (!IsValid(NextChild)) NextChild = nullptr;
	return NextChild;
}

AFGBuildable* AFINModularIndicatorPoleModule::netFunc_getPrevious() {
	if (!IsValid(Parent)) Parent = nullptr;
	return Parent;
}
