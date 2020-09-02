#include "FINScreen.h"


#include "FGColoredInstanceMeshProxy.h"
#include "ProxyInstancedStaticMeshComponent.h"
#include "Graphics/FINGPUInterface.h"

AFINScreen::AFINScreen() {
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Connector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("Connector");
	Connector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AFINScreen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINScreen, GPU);
	DOREPLIFETIME(AFINScreen, ScreenWidth);
	DOREPLIFETIME(AFINScreen, ScreenHeight);
}

void AFINScreen::BeginPlay() {
	Super::BeginPlay();
	
	if (IsValid(GPU)) Cast<IFINGPUInterface>(GPU)->RequestNewWidget();

#if !WITH_EDITOR
	SpawnComponents(ScreenWidth, ScreenHeight, ScreenMiddle, ScreenEdge, ScreenCorner, this, RootComponent, Parts);
	FVector ConnectorOffset;
	FVector WidgetOffset;
	if (ScreenHeight < 0) {
		if (ScreenWidth < 0) {
			ConnectorOffset = {0, 50, 50};
			WidgetOffset = {0,(ScreenWidth+1.f)/2.f*100.f,(ScreenHeight+1.f)/2.f*100.f};
		} else {
			ConnectorOffset = {0, -50, 50};
			WidgetOffset = {0,(ScreenWidth-1.f)/2.f*100.f,(ScreenHeight+1.f)/2.f*100.f};
		}
	} else {
		if (ScreenWidth < 0) {
			ConnectorOffset = {0, 50, -50};
			WidgetOffset = {0,(ScreenWidth+1.f)/2.f*100.f,(ScreenHeight-1.f)/2.f*100.f};
		} else {
			ConnectorOffset = {0, -50, -50};
			WidgetOffset = {0,(ScreenWidth-1.f)/2.f*100.f,(ScreenHeight-1.f)/2.f*100.f};
		}
	}
	Connector->SetMobility(EComponentMobility::Movable);
	Connector->SetRelativeLocation(ConnectorOffset);
	Connector->SetMobility(EComponentMobility::Static);
	WidgetComponent->AddRelativeLocation(WidgetOffset);
	WidgetComponent->SetDrawSize(WidgetComponent->GetDrawSize() * FVector2D(FMath::Abs(ScreenWidth), FMath::Abs(ScreenHeight)));
#endif
}

void AFINScreen::OnConstruction(const FTransform& transform) {
#if WITH_EDITOR
	SpawnComponents(ScreenWidth, ScreenHeight, ScreenMiddle, ScreenEdge, ScreenCorner, this, RootComponent, Parts);
#endif
}

void AFINScreen::EndPlay(const EEndPlayReason::Type endPlayReason) {
	if (endPlayReason == EEndPlayReason::Destroyed) BindGPU(nullptr);
}

int32 AFINScreen::GetDismantleRefundReturnsMultiplier() const {
	return FMath::Abs(ScreenWidth) * FMath::Abs(ScreenHeight);
}

bool AFINScreen::ShouldSave_Implementation() const {
	return true;
}

void AFINScreen::BindGPU(UObject* gpu) {
	if (gpu) check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
    if (GPU != gpu) {
    	if (!gpu) SetWidget(nullptr);
    	UObject* oldGPU = GPU;
    	GPU = nullptr;
    	if (oldGPU) Cast<IFINGPUInterface>(oldGPU)->BindScreen(nullptr);
    	GPU = gpu;
    	if (gpu) {
    		Cast<IFINGPUInterface>(gpu)->BindScreen(this);
    		Cast<IFINGPUInterface>(gpu)->RequestNewWidget();
    	}
    }
	OnGPUUpdate.Broadcast();
}

UObject* AFINScreen::GetGPU() const {
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
            TSharedPtr<SScaleBox>(nullptr));
	OnWidgetUpdate.Broadcast();
}

TSharedPtr<SWidget> AFINScreen::GetWidget() const {
	return Widget;
}

void AFINScreen::netFunc_getSize(int& w, int& h) {
	w = FMath::Abs(ScreenWidth);
	h = FMath::Abs(ScreenHeight);
}

void AFINScreen::SpawnComponents(int ScreenWidth, int ScreenHeight, UStaticMesh* MiddlePartMesh, UStaticMesh* EdgePartMesh, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, TArray<UStaticMeshComponent*>& OutParts) {
	int xf = ScreenWidth/FMath::Abs(ScreenWidth);
	int yf = ScreenHeight/FMath::Abs(ScreenHeight);
	for (int x = 0; x < FMath::Abs(ScreenWidth); ++x) {
		for (int y = 0; y < FMath::Abs(ScreenHeight); ++y) {
			UStaticMeshComponent* MiddlePart = NewObject<UFGColoredInstanceMeshProxy>(Parent);
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
		SpawnEdgeComponent(x * xf, 0, 2, EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
		SpawnEdgeComponent(x * xf, ScreenHeight - 1, 0, EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	}
	for (int y = 0; y < FMath::Abs(ScreenHeight); ++y) {
		SpawnEdgeComponent(0, y * yf, -1, EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
		SpawnEdgeComponent(ScreenWidth - 1, y * yf, 1,  EdgePartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	}
	SpawnCornerComponent(0,0,0, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	SpawnCornerComponent(ScreenWidth-1,0,1, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	SpawnCornerComponent(0,ScreenHeight-1,-1, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
	SpawnCornerComponent(ScreenWidth-1,ScreenHeight-1,2, CornerPartMesh, Parent, Attach, ScreenWidth, ScreenHeight, OutParts);
}

void AFINScreen::SpawnEdgeComponent(int x, int y, int r, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* EdgePart = NewObject<UFGColoredInstanceMeshProxy>(Parent);
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

void AFINScreen::SpawnCornerComponent(int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int ScreenWidth, int ScreenHeight, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* CornerPart = NewObject<UFGColoredInstanceMeshProxy>(Parent);
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
