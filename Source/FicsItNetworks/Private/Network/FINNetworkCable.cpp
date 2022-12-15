#include "Network/FINNetworkCable.h"
#include "Network/FINNetworkConnectionComponent.h"

AFINNetworkCable::AFINNetworkCable() {
	CableSpline = CreateDefaultSubobject<USplineMeshComponent>("CableSpline");
	CableSpline->SetupAttachment(RootComponent);
	CableSpline->SetForwardAxis(ESplineMeshAxis::Z);
	CableSpline->SetMobility(EComponentMobility::Type::Movable);
}

AFINNetworkCable::~AFINNetworkCable() {}

void AFINNetworkCable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINNetworkCable, Connector1);
	DOREPLIFETIME(AFINNetworkCable, Connector2);
}

void AFINNetworkCable::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	if (!IsValid(Connector1) || !IsValid(Connector2)) return;
	const FVector StartPos = Connector1->GetComponentLocation();
	const FVector EndPos = Connector2->GetComponentLocation();
	const int Length = FVector::Distance(StartPos, EndPos);
	
	CableSpline->SetVisibility(true, true);

	float Offset = 250.0;
	const FVector Start = FVector(0.0, 0.0, 0.0);
	FVector End = RootComponent->GetComponentTransform().InverseTransformPosition(EndPos);
	FVector Start_T = End;
	End = End + 0.0001;
	if ((FMath::Abs(Start_T.X) < 10 || FMath::Abs(Start_T.Y) < 10) && FMath::Abs(Start_T.Z) <= Offset) Offset = 1;
	Offset = FMath::Min(Length * SlackLengthFactor , MaxCableSlack);
	Start_T.Z -= Offset;
	FVector End_T = End;
	End_T.Z += Offset;

	CableSpline->SetStartAndEnd(Start, Start_T, End, End_T, true);
	CableSpline->UpdateMesh();

	CableSpline->SetMobility(EComponentMobility::Type::Static);
}

void AFINNetworkCable::BeginPlay() {
	Super::BeginPlay();

	AFGBuildable* Buildable = Cast<AFGBuildable>(GetOwner());
	if (!Buildable || !Buildable->GetBlueprintDesigner()) {
		ConnectConnectors();
	}
}

void AFINNetworkCable::EndPlay(EEndPlayReason::Type reason) {
	if (HasAuthority() && IsValid(this) && (reason == EEndPlayReason::Destroyed)) {
		if (IsValid(Connector1)) {
			Connector1->RemoveConnectedCable(this);
		}
		if (IsValid(Connector2)) {
			Connector2->RemoveConnectedCable(this);
		}
	}
}

bool AFINNetworkCable::ShouldSave_Implementation() const {
	return true;
}

void AFINNetworkCable::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(Connector1);
	out_dependentObjects.Add(Connector2);
}

int32 AFINNetworkCable::GetDismantleRefundReturnsMultiplier() const {
	if (!IsValid(Connector1) || !IsValid(Connector2)) return 0;
	FVector startPos = Connector1->GetComponentLocation();
	FVector endPos = Connector2->GetComponentLocation();
	return (startPos - endPos).Size() / 1000.0;
}

void AFINNetworkCable::OnConnectorUpdate() {
	RerunConstructionScripts();
}

void AFINNetworkCable::ConnectConnectors_Implementation() {
	if (!IsValid(Connector1) || !IsValid(Connector2)) return;
	if (HasAuthority()) {
		Connector1->AddConnectedCable(this);
		Connector2->AddConnectedCable(this);
	}
		
	RerunConstructionScripts();
}
