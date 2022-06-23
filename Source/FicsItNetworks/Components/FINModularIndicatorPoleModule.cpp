#include "FINModularIndicatorPoleModule.h"
#include "FGColoredInstanceMeshProxy.h"

AFINModularIndicatorPoleModule::AFINModularIndicatorPoleModule() {
	Indicator = CreateDefaultSubobject<UStaticMeshComponent>("Indicator");
	Indicator->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINModularIndicatorPoleModule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINModularIndicatorPoleModule, IndicatorColor);
	DOREPLIFETIME(AFINModularIndicatorPoleModule, EmissiveStrength);
}

void AFINModularIndicatorPoleModule::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
}

void AFINModularIndicatorPoleModule::BeginPlay() {
	Super::BeginPlay();
	
	RerunConstructionScripts();

	if (Indicator->GetMaterials().Num() > 0) {
		UMaterialInterface* Material = Indicator->GetMaterial(1);
		IndicatorMaterialInstance = UMaterialInstanceDynamic::Create(Cast<UMaterialInstance>(Material)->Parent, nullptr);
		Indicator->SetMaterial(1, IndicatorMaterialInstance);
	}
}

void AFINModularIndicatorPoleModule::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (bHasChanged) {
		bHasChanged = false;
		UpdateEmessive();
		ForceNetUpdate();
	}
}

bool AFINModularIndicatorPoleModule::ShouldSave_Implementation() const {
	return true;
}

int32 AFINModularIndicatorPoleModule::GetDismantleRefundReturnsMultiplier() const {
	return 1;
}

void AFINModularIndicatorPoleModule::UpdateEmessive_Implementation() {
	if (IndicatorMaterialInstance) {
		IndicatorMaterialInstance->SetVectorParameterValue("BtnColor", IndicatorColor);
		IndicatorMaterialInstance->SetScalarParameterValue("Emit", EmissiveStrength);
	}
}

void AFINModularIndicatorPoleModule::SetColor_Implementation(float r, float g, float b, float e) {
	FLinearColor oldColor = IndicatorColor;
	float oldEmissive = EmissiveStrength;
	IndicatorColor.R = FMath::Clamp(r, 0.0f, 1.0f);
	IndicatorColor.G = FMath::Clamp(g, 0.0f, 1.0f);
	IndicatorColor.B = FMath::Clamp(b, 0.0f, 1.0f);
	EmissiveStrength = FMath::Clamp(e, 0.0f, 5.0f);
	netSig_ColorChanged(oldColor.R, oldColor.G, oldColor.B, oldEmissive);
	bHasChanged = true;
}

void AFINModularIndicatorPoleModule::netFunc_setColor(float r, float g, float b, float e) {
	SetColor(r,g,b,e);
}


void AFINModularIndicatorPoleModule::GetChildDismantleActors_Implementation(TArray<AActor*>& out_ChildDismantleActors) const {
	Super::GetChildDismantleActors_Implementation(out_ChildDismantleActors);
	if(IsValid(NextChild)) {
		out_ChildDismantleActors.Add(NextChild);
		NextChild->GetChildDismantleActors_Implementation(out_ChildDismantleActors);
	}
}

void AFINModularIndicatorPoleModule::netFunc_getColor(float& r, float& g, float& b, float& e) {
	r = IndicatorColor.R;
	g = IndicatorColor.G;
	b = IndicatorColor.B;
	e = EmissiveStrength;
}

void AFINModularIndicatorPoleModule::netSig_ColorChanged(float r, float g, float b, float e) {}

AFINModularIndicatorPoleModule* AFINModularIndicatorPoleModule::netFunc_getNext() {
	if (!IsValid(NextChild)) NextChild = nullptr;
	return NextChild;
}

AFGBuildable* AFINModularIndicatorPoleModule::netFunc_getPrevious() {
	if (!IsValid(Parent)) Parent = nullptr;
	return Parent;
}
