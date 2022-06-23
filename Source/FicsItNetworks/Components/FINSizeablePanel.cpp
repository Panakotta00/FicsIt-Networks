#include "FINSizeablePanel.h"


#include "FGColoredInstanceMeshProxy.h"
#include "FINModuleBase.h"
#include "FINScreen.h"
#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FicsItNetworks/Network/FINMCPAdvConnector.h"
#include "FicsItNetworks/Network/FINNetworkCable.h"

AFINSizeablePanel::AFINSizeablePanel() {
	ModularPanel = CreateDefaultSubobject<UFINModuleSystemPanel>("Panel");
	ModularPanel->SetMobility(EComponentMobility::Movable);
	ModularPanel->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	ModularPanel->SetIsReplicated(true);
	ModularPanel->AllowedModules.AddUnique(AFINModuleBase::StaticClass());
	
	Connector = CreateDefaultSubobject<UFINMCPAdvConnector>("Connector");
	Connector->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Connector->SetIsReplicated(true);
	Connector->MaxCables = 1;

	Plane = CreateDefaultSubobject<UStaticMeshComponent>("Plane");
	Plane->AttachToComponent(ModularPanel, FAttachmentTransformRules::KeepRelativeTransform);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));  
	Plane->SetStaticMesh(PlaneMesh.Object);
	//static ConstructorHelpers::FObjectFinder<UMaterial> MeshMaterial(TEXT("/FicsItNetworks/Components/MicroControlPanels/MicroControlPanels/SizeablePanel/Temp.Temp"));
	//Plane->SetMaterial(0, MeshMaterial.Object);
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINSizeablePanel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINSizeablePanel, PanelWidth);
	DOREPLIFETIME(AFINSizeablePanel, PanelHeight);
}

void AFINSizeablePanel::BeginPlay() {
	ModularPanel->PanelWidth = abs(PanelWidth);
	ModularPanel->PanelHeight = abs(PanelHeight);
	RerunConstructionScripts();
	Super::BeginPlay();
	
	for (AFINNetworkCable* Cable : Connector->GetConnectedCables()) {
		Cable->RerunConstructionScripts();
	}
}

void AFINSizeablePanel::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AFINSizeablePanel::OnConstruction(const FTransform& transform) {
	SpawnComponents(UFGColoredInstanceMeshProxy::StaticClass(), PanelWidth, PanelHeight, PanelCornerMesh, PanelSideMesh, PanelCenterMesh, PanelConnectorMesh, this, RootComponent, Parts);
	FVector ConnectorOffset;
	if(PanelWidth < 0 && PanelHeight < 0) {
		ConnectorOffset = {3.3, 0, 8};
	}else if(PanelWidth < 0 && PanelHeight >= 0) {
		ConnectorOffset = {3.3, 0, static_cast<float>(PanelHeight * 10 - 2)};
	}else if(PanelWidth >= 0 && PanelHeight < 0) {
		ConnectorOffset = {3.3, static_cast<float>((PanelWidth - 1) * 10), 8};
	}else{
		ConnectorOffset = {3.3, static_cast<float>((PanelWidth - 1) * 10), static_cast<float>(PanelHeight * 10 - 2)};
	}
	Connector->SetMobility(EComponentMobility::Movable);
	Connector->SetRelativeLocation(ConnectorOffset);
	Connector->SetMobility(EComponentMobility::Static);

	ModularPanel->PanelWidth = abs(PanelWidth);
	ModularPanel->PanelHeight = abs(PanelHeight);

	Plane->SetMobility(EComponentMobility::Movable);
	Plane->SetVisibility(false);

	SetPanelSize(PanelWidth, PanelHeight);

	Super::OnConstruction(transform);
	
}

void AFINSizeablePanel::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
}

int32 AFINSizeablePanel::GetDismantleRefundReturnsMultiplier() const {
	return FMath::Abs(PanelWidth) * FMath::Abs(PanelHeight);
}

bool AFINSizeablePanel::ShouldSave_Implementation() const {
	return true;
}

void AFINSizeablePanel::SpawnComponents(TSubclassOf<UStaticMeshComponent> Class, int PanelWidth, int PanelHeight,
                                        UStaticMesh* ULMesh,
                                        UStaticMesh* UCMesh,
                                        UStaticMesh* CCMesh,
                                        UStaticMesh* ConnectorMesh,
                                        AActor* Parent, USceneComponent* Attach,
                                        TArray<UStaticMeshComponent*>& OutParts)
{
	int xf = PanelWidth/FMath::Abs(PanelWidth);
	int yf = PanelHeight/FMath::Abs(PanelHeight);
	for (int x = 0; x < FMath::Abs(PanelWidth); ++x) {
		for (int y = 0; y < FMath::Abs(PanelHeight); ++y) {
			UStaticMeshComponent* MiddlePart = NewObject<UStaticMeshComponent>(Parent, Class);
			MiddlePart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
			MiddlePart->SetRelativeLocation(FVector(0, x * 10 * xf, y * 10 * yf));
			MiddlePart->RegisterComponent();
			MiddlePart->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			MiddlePart->SetStaticMesh(CCMesh);
			MiddlePart->SetMobility(EComponentMobility::Static);
			OutParts.Add(MiddlePart);
		}
	}
	if(FMath::Abs(PanelWidth) > 1) {
		SpawnEdgeComponent(Class, 0, 0, 2, PanelWidth, 1, UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //DC
		SpawnEdgeComponent(Class, 0, PanelHeight - 1, 0, PanelWidth, 1, UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);   //UC
	}
	if(FMath::Abs(PanelHeight) > 1) {
		SpawnEdgeComponent(Class, 0, 0, -1, PanelHeight, 1, UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //CR
		SpawnEdgeComponent(Class, PanelWidth - 1, 0, 1, PanelHeight, 1,  UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //CL
	}
	//for (int x = 1; x < FMath::Abs(PanelWidth); ++x) {
	//	SpawnEdgeComponent(Class, -0.5+x * xf, 0, 2, UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //DC
	//	SpawnEdgeComponent(Class, -0.5+x * xf, PanelHeight - 1, 0, UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);   //UC
	//}
	//for (int y = 1; y < FMath::Abs(PanelHeight); ++y) {
	//	SpawnEdgeComponent(Class, 0, -0.5+y * yf, -1, UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //CR
	//	SpawnEdgeComponent(Class, PanelWidth - 1, -0.5+y * yf, 1,  UCMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //CL
	//}
	SpawnCornerComponent(Class, 0,0,0, ULMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts); //DL
	SpawnCornerComponent(Class, PanelWidth-1,0,1, ULMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts); //DR
	SpawnCornerComponent(Class, 0,PanelHeight-1,-1, ULMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts);  //UL
	SpawnCornerComponent(Class, PanelWidth-1,PanelHeight-1,2, ULMesh, Parent, Attach, PanelWidth, PanelHeight, OutParts); //UR

	UStaticMeshComponent* ConnectorMeshComponent = NewObject<UStaticMeshComponent>(Parent, Class);
	ConnectorMeshComponent->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	if(PanelWidth < 0 && PanelHeight < 0) {
		ConnectorMeshComponent->SetRelativeLocation(FVector(0, 0, 5));
		ConnectorMeshComponent->SetRelativeRotation(FRotator(0, 0, 0));
	}else if(PanelWidth < 0 && PanelHeight >= 0) {
		ConnectorMeshComponent->SetRelativeLocation(FVector(0, 0, PanelHeight * 10 - 5));
		ConnectorMeshComponent->SetRelativeRotation(FRotator(0, 0, 0));
	}else if(PanelWidth >= 0 && PanelHeight < 0) {
		ConnectorMeshComponent->SetRelativeLocation(FVector(0, (PanelWidth - 1) * 10, 5));
		ConnectorMeshComponent->SetRelativeRotation(FRotator(0, 0, 0));
	}else{
		ConnectorMeshComponent->SetRelativeLocation(FVector(0, (PanelWidth - 1) * 10, PanelHeight * 10 - 5));
		ConnectorMeshComponent->SetRelativeRotation(FRotator(0, 0, 0));
	}
	ConnectorMeshComponent->RegisterComponent();
	ConnectorMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	ConnectorMeshComponent->SetStaticMesh(ConnectorMesh);
	ConnectorMeshComponent->SetMobility(EComponentMobility::Static);
	OutParts.Add(ConnectorMeshComponent);
}

void AFINSizeablePanel::SpawnEdgeComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, int scaleX, int scaleY, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int PanelWidth, int PanelHeight, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* EdgePart = NewObject<UStaticMeshComponent>(Parent, Class);
	EdgePart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);

	if (PanelWidth < 0) {
		if (r == 1) r = -1;
		else if (r == -1) r = 1;
	}
	if (PanelHeight < 0) {
		if (r == 0) r = 2;
		else if (r == 2) r = 0;
	}
	int fx, fy;
	switch(r) {
		case 0: //Upper Center
			EdgePart->SetRelativeLocation(FVector(0, x + (PanelWidth * 10) / 2 - 5 * (PanelWidth < 0?-1:1), y * 10 + 5));
			EdgePart->SetRelativeScale3D(FVector(1,FMath::Abs(PanelWidth) - 1,1));
			EdgePart->SetRelativeRotation(FRotator(0, 0, 0));
			break;
		case -1: //Center Right
			fx = PanelWidth < 0 ? -3 : 1;
			EdgePart->SetRelativeLocation(FVector(0, x * 10 - 5*fx, y + (PanelHeight * 10) / 2 - 5 * (PanelHeight < 0?-1:1)));
			EdgePart->SetRelativeScale3D(FVector(1,FMath::Abs(PanelHeight) - 1, 1));
			EdgePart->SetRelativeRotation(FRotator(0, 0, -90));
			break;
		case 1: //Center Left
			EdgePart->SetRelativeLocation(FVector(0, x * 10 + 5, y + (PanelHeight * 10) / 2 - 5 * (PanelHeight < 0?-1:1)));
			EdgePart->SetRelativeScale3D(FVector(1,FMath::Abs(PanelHeight) - 1, 1));
			EdgePart->SetRelativeRotation(FRotator(0, 0, 90));
			break;
		case 2: //Down Center
			fy = PanelHeight < 0 ? -3 : 1;
			EdgePart->SetRelativeLocation(FVector(0, x + (PanelWidth * 10) / 2 - 5 * (PanelWidth < 0?-1:1), y * 10 - 5*fy));
			EdgePart->SetRelativeScale3D(FVector(1,FMath::Abs(PanelWidth) - 1,1));
			EdgePart->SetRelativeRotation(FRotator(0, 0, 180));
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

void AFINSizeablePanel::SpawnCornerComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int PanelWidth, int PanelHeight, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* CornerPart = NewObject<UStaticMeshComponent>(Parent, Class);
	CornerPart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);

	if (PanelWidth < 0) {
		if (PanelHeight < 0) {
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
		if (PanelHeight < 0) {
			if (r == 0) r = -1;
			else if (r == 2) r = 1;
			else if (r == 1) r = 2;
			else if (r == -1) r = 0;
		}
	}
	
	int fx = PanelWidth < 0 ? -3 : 1;
	int fy = PanelHeight < 0 ? -3 : 1;
	switch(r) {
	case 0:
		CornerPart->SetRelativeLocation(FVector(0, x * 10 - 5*fx, y * 10 - 5*fy));
		CornerPart->SetRelativeRotation(FRotator(0, 0, 180));
		break;
	case -1:
		CornerPart->SetRelativeLocation(FVector(0, x * 10 - 5*fx, y * 10 + 5));
		CornerPart->SetRelativeRotation(FRotator(0, 0, -90));
		break;
	case 1:
		CornerPart->SetRelativeLocation(FVector(0, x * 10 + 5, y * 10 - 5*fy));
		CornerPart->SetRelativeRotation(FRotator(0, 0, 90));
		break;
	case 2:
		CornerPart->SetRelativeLocation(FVector(0, x * 10 + 5, y * 10 + 5));
		CornerPart->SetRelativeRotation(FRotator(0, 0, 0));
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

void AFINSizeablePanel::SetPanelSize(int width, int height) {
	ModularPanel->PanelWidth = abs(PanelWidth = width);
	ModularPanel->PanelHeight = abs(PanelHeight = height);

	
	int x = 5;
	int y = -5;
	auto InY = 5 + (PanelWidth < 0?0:FMath::Abs(PanelWidth) - 1) * 10;
	auto InZ = -5 + (PanelHeight < 0?(PanelHeight + 1) * 10:0);
	ModularPanel->SetMobility(EComponentMobility::Movable);
	ModularPanel->SetRelativeLocation(FVector(7.05, InY, InZ));
	ModularPanel->SetRelativeRotation(FRotator(90, 180, 0));
	float plx;
	float ply;
	if(PanelWidth < 0) {
		plx = -PanelWidth * 10 / 2;
	}else {
		plx = PanelWidth * 10 / 2;
	}
	if(PanelHeight < 0) {
		ply = -PanelHeight * 10 / 2;
	}else{
		ply = PanelHeight * 10 / 2;
	}
	ply = FMath::Abs(PanelHeight * 10 / 2);
	plx = FMath::Abs(PanelWidth * 10 / 2);
	auto loc = ModularPanel->GetRelativeLocation();
	auto rot = ModularPanel->GetRelativeRotation();
	UE_LOG(LogFicsItNetworks, Display, TEXT("PanelSize: %d, %d, %f, %f, %f, %f, %f, %f"), InY, InZ, loc.X, loc.Y, loc.Z, rot.Pitch, rot.Yaw, rot.Roll);
	Plane->SetRelativeLocation(FVector(ply, plx, 0));
	Plane->SetRelativeScale3D(FVector(static_cast<float>(PanelHeight) / 10, static_cast<float>(PanelWidth) / 10, 1));
}