#include "FINIndicatorPole.h"
#include "FGColoredInstanceMeshProxy.h"

AFINIndicatorPole::AFINIndicatorPole() {
	Indicator = CreateDefaultSubobject<UFGColoredInstanceMeshProxy>("Indicator");
	Indicator->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Indicator->SetInstanced(true);
	
	Connector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("Connector");
	Connector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Connector->SetIsReplicated(true);

	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINIndicatorPole::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINIndicatorPole, Height);
	DOREPLIFETIME(AFINIndicatorPole, IndicatorColor);
	DOREPLIFETIME(AFINIndicatorPole, EmessiveStrength);
}

void AFINIndicatorPole::OnConstruction(const FTransform& transform) {
	CreatePole();

	Super::OnConstruction(transform);

	UpdateEmessive();
}

void AFINIndicatorPole::BeginPlay() {
	Super::BeginPlay();
	
	CreatePole();

	UpdateEmessive();

	bHasChanged = true;
}

void AFINIndicatorPole::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (bHasChanged) {
		bHasChanged = false;
		UpdateEmessive_Net();
		ForceNetUpdate();
	}
}

bool AFINIndicatorPole::ShouldSave_Implementation() const {
	return true;
}

int32 AFINIndicatorPole::GetDismantleRefundReturnsMultiplier() const {
	return Height + 6;
}

void AFINIndicatorPole::OnBuildEffectFinished() {
	Super::OnBuildEffectFinished();

	UpdateEmessive();
	
	bHasChanged = true;
}

void AFINIndicatorPole::CreatePole() {
	// Clean up
	Poles.Empty();

	// Construction
	for (int i = 0; i < Height; ++i) {
		UFGColoredInstanceMeshProxy* Pole = NewObject<UFGColoredInstanceMeshProxy>(this);
		check(Pole);
		Pole->AttachToComponent(Indicator, FAttachmentTransformRules::KeepRelativeTransform);
		Pole->SetRelativeLocation(FVector(0,0, -(i) * 100.0));
		Pole->RegisterComponent();
		Pole->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		Pole->SetStaticMesh(LongPole);
		Pole->SetMobility(EComponentMobility::Static);
		Pole->mInstanceHandle.SetColorIndex(mColorSlot);
		Poles.Add(Pole);
	}
}

void AFINIndicatorPole::UpdateEmessive_Net_Implementation() {
	UpdateEmessive();
}

void AFINIndicatorPole::UpdateEmessive() {
	Indicator->SetUserDefinedData(TArray<float>{0, IndicatorColor.R, IndicatorColor.G, IndicatorColor.B});
	const bool bIsInstanced = Indicator->mInstanceHandle.IsInstanced();
	if (bIsInstanced) {
		Indicator->mInstanceHandle.SetCustomDataById(0, 0);
		Indicator->mInstanceHandle.SetCustomDataById(1, IndicatorColor.R);
		Indicator->mInstanceHandle.SetCustomDataById(2, IndicatorColor.G);
		Indicator->mInstanceHandle.SetCustomDataById(3, IndicatorColor.B);
		Indicator->mInstanceManager->UpdateColorForInstanceFromDataArray(Indicator->mInstanceHandle);
	} else {
		Indicator->SetCustomPrimitiveDataFloat(0, 0);
		Indicator->SetCustomPrimitiveDataFloat(1, IndicatorColor.R);
		Indicator->SetCustomPrimitiveDataFloat(2, IndicatorColor.G);
		Indicator->SetCustomPrimitiveDataFloat(3, IndicatorColor.B);
	}
}

void AFINIndicatorPole::netFunc_setColor(float r, float g, float b, float e) {
	FLinearColor oldColor = IndicatorColor;
	float oldEmissive = EmessiveStrength;
	IndicatorColor.R = FMath::Clamp(r, 0.0f, 1.0f);
	IndicatorColor.G = FMath::Clamp(g, 0.0f, 1.0f);
	IndicatorColor.B = FMath::Clamp(b, 0.0f, 1.0f);
	EmessiveStrength = FMath::Clamp(e, 0.0f, 5.0f);
	netSig_ColorChanged(oldColor.R, oldColor.G, oldColor.B, oldEmissive);
	bHasChanged = true;
}

void AFINIndicatorPole::netFunc_getColor(float& r, float& g, float& b, float& e) {
	r = IndicatorColor.R;
	g = IndicatorColor.G;
	b = IndicatorColor.B;
	e = EmessiveStrength;
}

void AFINIndicatorPole::netSig_ColorChanged(float r, float g, float b, float e) {}

AFINIndicatorPole* AFINIndicatorPole::netFunc_getTopPole() {
	if (!IsValid(TopConnected)) TopConnected = nullptr;
	return TopConnected;
}

AFINIndicatorPole* AFINIndicatorPole::netFunc_getBottomPole() {
	if (!IsValid(BottomConnected)) BottomConnected = nullptr;
	return BottomConnected;
}
