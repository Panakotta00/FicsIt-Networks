#include "FINNetworkCable.h"

#include "FINNetworkConnector.h"
#include "FINNetworkAdapter.h"

#include "SML/util/Logging.h"

AFINNetworkCable::AFINNetworkCable() {
	UStaticMesh* cableSplineMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Game/FicsIt-Networks/Network/NetworkCable/Mesh_NetworkCable.Mesh_NetworkCable"));
	CableSpline = CreateDefaultSubobject<USplineMeshComponent>("CableSpline");
	CableSpline->SetupAttachment(RootComponent);
	CableSpline->SetStaticMesh(cableSplineMesh);
	CableSpline->SetForwardAxis(ESplineMeshAxis::Z);
}

AFINNetworkCable::~AFINNetworkCable() {}

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

	CableSpline->SetStartAndEnd(start, start_t, end, end_t, true);

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

void AFINNetworkCable::Dismantle_Implementation() {
	Super::Dismantle_Implementation();
}

void AFINNetworkCable::GetDismantleRefund_Implementation(TArray<FInventoryStack>& refund) const {
	Super::GetDismantleRefund_Implementation(refund);
}