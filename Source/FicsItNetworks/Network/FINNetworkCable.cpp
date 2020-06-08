#include "FINNetworkCable.h"

#include "FINNetworkConnector.h"
#include "FINNetworkAdapter.h"

#include "SML/util/Logging.h"

AFINNetworkCable::AFINNetworkCable() {
	CableSpline = CreateDefaultSubobject<USplineMeshComponent>("CableSpline");
	CableSpline->SetupAttachment(RootComponent);
	CableSpline->SetForwardAxis(ESplineMeshAxis::Z);
	CableSpline->SetMobility(EComponentMobility::Type::Movable);
}

AFINNetworkCable::~AFINNetworkCable() {}

void AFINNetworkCable::OnConstruction(const FTransform& Transform) {

}

void AFINNetworkCable::BeginPlay() {
	Super::BeginPlay();
	
	if (!IsValid(Connector1) || !IsValid(Connector2)) return;
	FVector startPos = Connector1->GetComponentLocation();
	FVector endPos = Connector2->GetComponentLocation();
	
	CableSpline->SetVisibilitySML(true, true);

	float offset = 250.0;
	FVector start = FVector(0.0, 0.0, 0.0);
	FVector end = RootComponent->GetComponentTransform().InverseTransformPosition(endPos);
	FVector start_t = end;
	start_t.Z -= offset;
	FVector end_t = end;
	end_t.Z += offset;

	UStaticMesh* cableMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsItNetworks/Network/NetworkCable/Mesh_NetworkCable.Mesh_NetworkCable"));
	CableSpline->SetStaticMesh(cableMesh);

	CableSpline->SetStartAndEnd(start, start_t, end, end_t, true);
	CableSpline->UpdateMesh();

	CableSpline->SetMobility(EComponentMobility::Type::Static);

	Connector1->AddCable(this);
	Connector2->AddCable(this);
}

void AFINNetworkCable::EndPlay(EEndPlayReason::Type reason) {
	if (IsValid(Connector1)) {
		Connector1->RemoveCable(this);
	}
	if (IsValid(Connector2)) {
		Connector2->RemoveCable(this);
	}
}

bool AFINNetworkCable::ShouldSave_Implementation() const {
	return true;
}

int32 AFINNetworkCable::GetDismantleRefundReturnsMultiplier() const {
	if (!IsValid(Connector1) || !IsValid(Connector2)) return 0;
	FVector startPos = Connector1->GetComponentLocation();
	FVector endPos = Connector2->GetComponentLocation();
	return (startPos - endPos).Size() / 100.0;
}
