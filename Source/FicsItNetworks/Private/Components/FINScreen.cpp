#include "Components/FINScreen.h"

#include "FGColoredInstanceMeshProxy.h"
#include "Graphics/FINGPUInterface.h"
#include "Network/FINNetworkCable.h"

AFINScreen::AFINScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
	Connector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("Connector");
	Connector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Connector->SetIsReplicated(true);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINScreen, GPUPtr);
	DOREPLIFETIME(AFINScreen, GPU);
	DOREPLIFETIME(AFINScreen, ScreenWidth);
	DOREPLIFETIME(AFINScreen, ScreenHeight);
}

void AFINScreen::BeginPlay() {
	RerunConstructionScripts();

	Super::BeginPlay();

	FVector WidgetOffset;
	if (ScreenHeight < 0) {
		if (ScreenWidth < 0) {
			WidgetOffset = {0,(ScreenWidth+1.f)/2.f*100.f,(ScreenHeight+1.f)/2.f*100.f};
		} else {
			WidgetOffset = {0,(ScreenWidth-1.f)/2.f*100.f,(ScreenHeight+1.f)/2.f*100.f};
		}
	} else {
		if (ScreenWidth < 0) {
			WidgetOffset = {0,(ScreenWidth+1.f)/2.f*100.f,(ScreenHeight-1.f)/2.f*100.f};
		} else {
			WidgetOffset = {0,(ScreenWidth-1.f)/2.f*100.f,(ScreenHeight-1.f)/2.f*100.f};
		}
	}
	WidgetComponent->AddRelativeLocation(WidgetOffset);
	WidgetComponent->SetDrawSize(WidgetComponent->GetDrawSize() * FVector2D(FMath::Abs(ScreenWidth), FMath::Abs(ScreenHeight)));

	if (HasAuthority()) GPUPtr = GPU.Get();
	if (GPUPtr) Cast<IFINGPUInterface>(GPUPtr)->RequestNewWidget();

	for (AFINNetworkCable* Cable : Connector->GetConnectedCables()) Cable->RerunConstructionScripts();
}

void AFINScreen::OnConstruction(const FTransform& transform) {
	SpawnComponents(UFGColoredInstanceMeshProxy::StaticClass(), ScreenWidth, ScreenHeight, ScreenMiddle, ScreenEdge, ScreenCorner, this, RootComponent, Parts);
	FVector ConnectorOffset;
	if (ScreenHeight < 0) {
		if (ScreenWidth < 0) {
			ConnectorOffset = {0, 50, 50};
		} else {
			ConnectorOffset = {0, -50, 50};
		}
	} else {
		if (ScreenWidth < 0) {
			ConnectorOffset = {0, 50, -50};
		} else {
			ConnectorOffset = {0, -50, -50};
		}
	}
	Connector->SetMobility(EComponentMobility::Movable);
	Connector->SetRelativeLocation(ConnectorOffset);
	Connector->SetMobility(EComponentMobility::Static);

	Super::OnConstruction(transform);
}

void AFINScreen::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (bGPUChanged) {
		bGPUChanged = false;
		
		if (GPUPtr) {
			Cast<IFINGPUInterface>(GPUPtr)->RequestNewWidget();
		} else {
			SetWidget(SNew(SScaleBox));
		}

		OnGPUUpdate.Broadcast();

		NetMulti_OnGPUUpdate();
	}
	if (HasAuthority() && (((bool)GPUPtr) != GPU.IsValid())) {
		if (!GPUPtr) GPUPtr = GPU.Get();
		OnGPUValidChanged(GPU.IsValid(), GPUPtr);
		GPUPtr = GPU.Get();
	}
	if (!Widget && GPUPtr) Cast<IFINGPUInterface>(GPUPtr)->RequestNewWidget();
}

void AFINScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(FFINNetworkTrace());
}

int32 AFINScreen::GetDismantleRefundReturnsMultiplier() const {
	return FMath::Abs(ScreenWidth) * FMath::Abs(ScreenHeight);
}

bool AFINScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINScreen::BindGPU(const FFINNetworkTrace& gpu) {
	if (IsValid(gpu.GetUnderlyingPtr())) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
	if (GPU != gpu) {
		FFINNetworkTrace oldGPU = GPU;
		GPU = FFINNetworkTrace();
		if (IsValid(oldGPU.GetUnderlyingPtr())) Cast<IFINGPUInterface>(oldGPU.GetUnderlyingPtr())->BindScreen(FFINNetworkTrace());
		GPU = gpu;
		if (IsValid(gpu.GetUnderlyingPtr())) {
			Cast<IFINGPUInterface>(gpu.GetUnderlyingPtr())->BindScreen(gpu / this);
		}
		bGPUChanged = true;
		GPUPtr = GPU.Get();
	}
}

FFINNetworkTrace AFINScreen::GetGPU() const {
	return GPU;
}

void AFINScreen::SetWidget(TSharedPtr<SWidget> widget) {
	if (Widget != widget) Widget = widget;
	WidgetComponent->SetSlateWidget(
		Widget.IsValid() ?
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.Content()[
				Widget.ToSharedRef()
			]
		:
			SNew(SScaleBox)
		);
	WidgetComponent->RequestRedraw();
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINScreen::GetWidget() const {
	return Widget;
}

void AFINScreen::RequestNewWidget() {
	if (GPUPtr) Cast<IFINGPUInterface>(GPUPtr)->RequestNewWidget();
}

void AFINScreen::OnGPUValidChanged_Implementation(bool bValid, UObject* newGPU) {
	if (!bValid) {
		if (newGPU) Cast<IFINGPUInterface>(newGPU)->DropWidget();
	} else {
		if (newGPU) Cast<IFINGPUInterface>(newGPU)->RequestNewWidget();
	}
}

void AFINScreen::netFunc_getSize(int& w, int& h) {
	w = FMath::Abs(ScreenWidth);
	h = FMath::Abs(ScreenHeight);
}

void AFINScreen::NetMulti_OnGPUUpdate_Implementation() {}

void AFINScreen::SpawnComponents(TSubclassOf<UStaticMeshComponent> Class, int ScreenWidth, int ScreenHeight, UStaticMesh* MiddlePartMesh, UStaticMesh* EdgePartMesh, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, TArray<UStaticMeshComponent*>& OutParts) {
	int xf = ScreenWidth/FMath::Abs(ScreenWidth);
	int yf = ScreenHeight/FMath::Abs(ScreenHeight);
	for (int x = 0; x < FMath::Abs(ScreenWidth); ++x) {
		for (int y = 0; y < FMath::Abs(ScreenHeight); ++y) {
			UStaticMeshComponent* MiddlePart = NewObject<UStaticMeshComponent>(Parent, Class);
			MiddlePart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
			MiddlePart->SetRelativeLocation(FVector(0, x * 100 * xf - 50, y * 100 * yf - 50));
			MiddlePart->RegisterComponent();
			MiddlePart->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			MiddlePart->SetStaticMesh(MiddlePartMesh);
			MiddlePart->SetMobility(EComponentMobility::Static);
			OutParts.Add(MiddlePart);
		}
	}
	for (int x = 0; x < FMath::Abs(ScreenWidth); ++x) {
		SpawnEdgeComponent(Class, x * xf, 0, 2, EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
		SpawnEdgeComponent(Class, x * xf, ScreenHeight - 1, 0, EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	}
	for (int y = 0; y < FMath::Abs(ScreenHeight); ++y) {
		SpawnEdgeComponent(Class, 0, y * yf, -1, EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
		SpawnEdgeComponent(Class, ScreenWidth - 1, y * yf, 1,  EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	}
	SpawnCornerComponent(Class, 0,0,0, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	SpawnCornerComponent(Class, ScreenWidth-1,0,1, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	SpawnCornerComponent(Class, 0,ScreenHeight-1,-1, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	SpawnCornerComponent(Class, ScreenWidth-1,ScreenHeight-1,2, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
}

void AFINScreen::SpawnEdgeComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* EdgePart = NewObject<UStaticMeshComponent>(Parent, Class);
	EdgePart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);

	if (ScreenWidth < 0) {
		if (r == 1) r = -1;
		else if (r == -1) r = 1;
	}
	if (ScreenHeight < 0) {
		if (r == 0) r = 2;
		else if (r == 2) r = 0;
	}
	int fx, fy;
	switch(r) {
	case 0:
		EdgePart->SetRelativeLocation(FVector(0, x * 100 + 50, y * 100 + 50));
		EdgePart->SetRelativeRotation(FRotator(0, 0, 180));
		break;
	case -1:
		fx = ScreenWidth < 0 ? -3 : 1;
		EdgePart->SetRelativeLocation(FVector(0, x * 100 - 50*fx, y * 100 + 50));
		EdgePart->SetRelativeRotation(FRotator(0, 0, 90));
		break;
	case 1:
		EdgePart->SetRelativeLocation(FVector(0, x * 100 + 50, y * 100 - 50));
		EdgePart->SetRelativeRotation(FRotator(0, 0, -90));
		break;
	case 2:
		fy = ScreenHeight < 0 ? -3 : 1;
		EdgePart->SetRelativeLocation(FVector(0, x * 100 - 50, y * 100 - 50*fy));
		break;
	default:
		break;
	}
	EdgePart->RegisterComponent();
	EdgePart->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	EdgePart->SetStaticMesh(EdgePartMesh);
	EdgePart->SetMobility(EComponentMobility::Static);
	OutParts.Add(EdgePart);
}

void AFINScreen::SpawnCornerComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* CornerPart = NewObject<UStaticMeshComponent>(Parent, Class);
	CornerPart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);

	if (ScreenWidth < 0) {
		if (ScreenHeight < 0) {
			if (r == 0) r = 2;
			else if (r == 2) r = 0;
			else if (r == 1) r = -1;
			else if (r == -1) r = 1;
		} else {
			if (r == 0) r = 1;
			else if (r == 2) r = -1;
			else if (r == 1) r = 0;
			else if (r == -1) r = 2;
		}
	} else {
		if (ScreenHeight < 0) {
			if (r == 0) r = -1;
			else if (r == 2) r = 1;
			else if (r == 1) r = 2;
			else if (r == -1) r = 0;
		}
	}
	
	int fx = ScreenWidth < 0 ? -3 : 1;
	int fy = ScreenHeight < 0 ? -3 : 1;
	switch(r) {
	case 0:
		CornerPart->SetRelativeLocation(FVector(0, x * 100 - 50*fx, y * 100 - 50*fy));
		CornerPart->SetRelativeRotation(FRotator(0, 0, 0));
		break;
	case -1:
		CornerPart->SetRelativeLocation(FVector(0, x * 100 - 50*fx, y * 100 + 50));
		CornerPart->SetRelativeRotation(FRotator(0, 0, 90));
		break;
	case 1:
		CornerPart->SetRelativeLocation(FVector(0, x * 100 + 50, y * 100 - 50*fy));
		CornerPart->SetRelativeRotation(FRotator(0, 0, -90));
		break;
	case 2:
		CornerPart->SetRelativeLocation(FVector(0, x * 100 + 50, y * 100 + 50));
		CornerPart->SetRelativeRotation(FRotator(0, 0, 180));
		break;
	default:
		break;
	}
	CornerPart->RegisterComponent();
	CornerPart->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	CornerPart->SetStaticMesh(CornerPartMesh);
	CornerPart->SetMobility(EComponentMobility::Static);
	OutParts.Add(CornerPart);
}
