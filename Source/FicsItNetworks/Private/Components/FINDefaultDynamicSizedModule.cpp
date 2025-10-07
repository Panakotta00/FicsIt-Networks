#include "Components/FINDefaultDynamicSizedModule.h"

#include "FicsItNetworksModule.h"
#include "FINModularIndicatorPoleHolo.h"
#include "UnrealNetwork.h"

AFINDefaultDynamicSizedModule::AFINDefaultDynamicSizedModule() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINDefaultDynamicSizedModule::BeginPlay() {
	ConstructParts();
	Super::BeginPlay();
	
}

void AFINDefaultDynamicSizedModule::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
}

void AFINDefaultDynamicSizedModule::OnConstruction(const FTransform& transform) {
	ConstructParts();

	Super::OnConstruction(transform);
	
}

#pragma optimize( "", off )
void AFINDefaultDynamicSizedModule::ConstructParts() {
	// Clear Components
	for (USceneComponent* comp : Parts) {
		if(IsValid(comp)) {
			comp->UnregisterComponent();
			comp->SetActive(false);
			comp->DestroyComponent();
		}
	}
	Parts.Empty();

	// Create Components
	Execute_SpawnComponents(this, DynamicWidth, DynamicHeight, this, RootComponent, Parts);

}
#pragma optimize( "", on )

void AFINDefaultDynamicSizedModule::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
}

int32 AFINDefaultDynamicSizedModule::GetDismantleRefundReturnsMultiplier() const {
	//return FMath::Abs(PanelWidth) * FMath::Abs(PanelHeight);
	return FMath::Max((FMath::Abs(DynamicWidth) * FMath::Abs(DynamicHeight)) / 10, 1);
}

bool AFINDefaultDynamicSizedModule::ShouldSave_Implementation() const {
	return true;
}

#pragma optimize( "", off )
void AFINDefaultDynamicSizedModule::SpawnComponents_Implementation(int Width, int Height, AActor* Parent,
                                                                   USceneComponent* Attach, TArray<USceneComponent*>& OutParts) {
	//Width = -3;
	//Height = -5;
	int absWidth = FMath::Abs(Width);
	int absHeight = FMath::Abs(Height);
	if (absWidth <= 0 || absHeight <= 0) {
		return;
	}
	TSubclassOf<UStaticMeshComponent> Class = UStaticMeshComponent::StaticClass(); 
	int xf = Width/absWidth;
	int yf = Height/absHeight;
	absWidth = std::max(absWidth, MinWidth);
	absHeight = std::max(absHeight, MinHeight);
	//Width = absWidth * xf;
	//Height = absHeight * yf;
	int cWidth = std::ceil((absWidth + CenterSpawnSizeModifier) / CenterMeshSizeMultiplierX);
	int cHeight = std::ceil((absHeight + CenterSpawnSizeModifier) / CenterMeshSizeMultiplierY);
	USceneComponent *MeshCollection = NewObject<USceneComponent>(Parent, USceneComponent::StaticClass());;
	MeshCollection->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	FVector meshOffset = this->MeshOffset;
	if (xf < 0) {
		meshOffset += FVector(-(absWidth - 1) * 10 , 0, 0);
	}
	if (yf < 0) {
		meshOffset += FVector(0, -(absHeight - 1) * 10, 0);
	}
	MeshCollection->SetRelativeLocation(meshOffset);
	MeshCollection->SetRelativeRotation(MeshRotation);
	MeshCollection->RegisterComponent();
	MeshCollection->SetMobility(EComponentMobility::Static);
	OutParts.Add(MeshCollection);
	Attach = MeshCollection;
	int RepeatX = cWidth;
	int RepeatY = cHeight;
	if (CenterTilingMode == EFINDynamicMeshSpawnMode_None || absWidth < CenterSpawnMinimumWidth || absWidth < CenterSpawnMinimumHeight) {
		RepeatX = 0;
		RepeatY = 0;
	}else if (CenterTilingMode == EFINDynamicMeshSpawnMode_Stretch){
		RepeatX = 1;
		RepeatY = 1;
	}
	UE_LOG(LogFicsItNetworks_DebugRoze, Warning, L"SpawnComponents_Repeat(%d, %d)", RepeatX, RepeatY);
	for (int CntY = 0; CntY < RepeatY; CntY++) {
		for (int CntX = 0; CntX < RepeatX; CntX++) {
			UStaticMeshComponent* MiddlePart = NewObject<UStaticMeshComponent>(Parent, Class);
			MiddlePart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
			if (CenterTilingMode == EFINDynamicMeshSpawnMode_Stretch) {
				MiddlePart->SetRelativeScale3D(FVector(1, cWidth / CenterMeshSizeMultiplierX, cHeight / CenterMeshSizeMultiplierY));
				MiddlePart->SetRelativeLocation(FVector(0, ((absWidth) * 10 * CenterMeshSizeMultiplierX / 2 - 5 * CenterMeshSizeMultiplierX), ((absHeight) * 10 * CenterMeshSizeMultiplierY)/ 2 - 5 * CenterMeshSizeMultiplierY));
			}else {
				MiddlePart->SetRelativeScale3D(FVector(1, 1, 1));
				MiddlePart->SetRelativeLocation(FVector(0, CntX * 10 * CenterMeshSizeMultiplierX + 5 * CenterMeshSizeMultiplierX, CntY * 10 * CenterMeshSizeMultiplierY + 5 * CenterMeshSizeMultiplierY));
			}
			MiddlePart->RegisterComponent();
			MiddlePart->SetStaticMesh(MeshE);
			MiddlePart->SetMobility(EComponentMobility::Static);
			MiddlePart->SetVisibility(true);
			OutParts.Add(MiddlePart);
		}
	}
	if(absWidth >= EdgeSpawnMinimumWidth) {
		SpawnEdgeComponent(Class, 0, 0, 2, MeshH, Parent, Attach, absWidth, absHeight, CenterSpawnSizeModifier, bRotateMeshes, CenterMeshSizeMultiplierX, EdgeTilingMode, OutParts);  //DC
		SpawnEdgeComponent(Class, 0, absHeight - 1, 0, MeshB, Parent, Attach, absWidth, absHeight, CenterSpawnSizeModifier, bRotateMeshes, CenterMeshSizeMultiplierX, EdgeTilingMode, OutParts);   //UC
	}
	if(absHeight >= EdgeSpawnMinimumHeight) {
		SpawnEdgeComponent(Class, 0, 0, -1, MeshF, Parent, Attach, absWidth, absHeight, CenterSpawnSizeModifier, bRotateMeshes, CenterMeshSizeMultiplierY, EdgeTilingMode, OutParts);  //CR
		SpawnEdgeComponent(Class, absWidth - 1, 0, 1, MeshD, Parent, Attach, absWidth, absHeight, CenterSpawnSizeModifier, bRotateMeshes, CenterMeshSizeMultiplierY, EdgeTilingMode, OutParts);  //CL
	}
	SpawnCornerComponent(Class, 0,0, 0, MeshI, Parent, Attach, absWidth, absHeight, bRotateMeshes, OutParts); //DL
	SpawnCornerComponent(Class, absWidth-1,0,1,MeshG, Parent, Attach, absWidth, absHeight, bRotateMeshes, OutParts); //DR
	SpawnCornerComponent(Class, 0,absHeight-1,-1, MeshC, Parent, Attach, absWidth, absHeight, bRotateMeshes, OutParts);  //UL
	SpawnCornerComponent(Class, absWidth-1,absHeight-1,2, MeshA, Parent, Attach, absWidth, absHeight, bRotateMeshes, OutParts); //UR
	
	SpawnAdditionalComponents(Width, Height, Parent, MeshCollection, OutParts);
}

void AFINDefaultDynamicSizedModule::SpawnAdditionalComponents_Implementation(int Width, int Height, AActor* Parent,
	USceneComponent* Attach, TArray<USceneComponent*>& OutParts) {
	
}

#pragma optimize( "", on )

void AFINDefaultDynamicSizedModule::SetDynamicSize_Implementation(int Width, int Height) {
	this->DynamicWidth = std::max(Width, MinWidth);
	this->DynamicHeight = std::max(Height, MinHeight);
	UE_LOG(LogFicsItNetworks, Warning, L"AFINDefaultDynamicSizedModule::SetDynamicSize_Implementation(PanelWidth=%d, PanelHeight=%d)", DynamicWidth, DynamicHeight)
	this->ModuleSize = FVector2D(this->DynamicWidth, this->DynamicHeight);
}

void AFINDefaultDynamicSizedModule::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINDefaultDynamicSizedModule, DynamicWidth);
	DOREPLIFETIME(AFINDefaultDynamicSizedModule, DynamicHeight);
}

#pragma optimize( "", off )
void AFINDefaultDynamicSizedModule::SpawnEdgeComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int Width, int Height, int SizeModifier, bool Rotate, double MeshScale, EFINDynamicMeshSpawnMode TilingMode, TArray<USceneComponent*>& OutParts) {
	if (Rotate) {
		if (Width < 0) {
			if (r == 1) r = -1;
			else if (r == -1) r = 1;
		}
		if (Height < 0) {
			if (r == 0) r = 2;
			else if (r == 2) r = 0;
		}
	}
	int fx, fy;
	//int Scale; = FMath::Abs(Width) + SizeModifier;
	int Scale;
	int Shift = 0;
	int LocationX = 0;
	int LocationY = 0;
	if (r == 0 || r == 2) {
		Scale = std::ceil((FMath::Abs(Width) + SizeModifier) / MeshScale);
	}else{
		Scale = std::ceil((FMath::Abs(Height) + SizeModifier) / MeshScale);
	}
	int Repeat = 0;
	switch (TilingMode) {
		case EFINDynamicMeshSpawnMode_Stretch: {
			Repeat = 1;
			break;
		}
		case EFINDynamicMeshSpawnMode_Tile: {
			Repeat = Scale;
			Scale = 1;
			break;
		}
		case EFINDynamicMeshSpawnMode_None: {
			Repeat = 0;
			break;
		}
	}
	for (int cnt = 0; cnt < Repeat; cnt++) {
		UStaticMeshComponent* EdgePart = NewObject<UStaticMeshComponent>(Parent, Class);
		EdgePart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
		switch(r) {
			case 0: //Upper Center
				if (Repeat > 1) {
					EdgePart->SetRelativeLocation(FVector(0, x + cnt * 10 * MeshScale + 5 * (Height < 0?-1:1), y * 10 + 5));
				}else{
					EdgePart->SetRelativeLocation(FVector(0, x + (Width * 10) / 2 - 5 * (Width < 0?-1:1), y * 10 + 5));
				}
				if (Rotate) {
					EdgePart->SetRelativeScale3D(FVector(1,Scale,1));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 0));
				}else {
					EdgePart->SetRelativeScale3D(FVector(1,Scale,1));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 0));
				}
				break;
			case -1: //Center Right
				fx = Width < 0 ? -3 : 1;
				if (Repeat > 1) {
					EdgePart->SetRelativeLocation(FVector(0, x * 10 - 5 * fx, y + cnt * 10 * MeshScale + 5 * (Height < 0?-1:1)));
				}else{
					EdgePart->SetRelativeLocation(FVector(0, x * 10 - 5 * fx, y + (Height * 10) / 2 - 5 * (Height < 0?-1:1)));
				}
				if (Rotate) {
					EdgePart->SetRelativeScale3D(FVector(1,Scale, 1));
					EdgePart->SetRelativeRotation(FRotator(0, 0, -90));	
				}else {
					EdgePart->SetRelativeScale3D(FVector(1,1, Scale));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 0));
				}
				break;
			case 1: //Center Left
				if (Repeat > 1) {
					EdgePart->SetRelativeLocation(FVector(0, x * 10 + 5, y + cnt * 10 * MeshScale + 5 * (Height < 0?-1:1)));
				}else{
					EdgePart->SetRelativeLocation(FVector(0, x * 10 + 5, y + (Height * 10) / 2 - 5 * (Height < 0?-1:1)));
				}
				if (Rotate) {
					EdgePart->SetRelativeScale3D(FVector(1,Scale, 1));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 90));
				}else {
					EdgePart->SetRelativeScale3D(FVector(1,1, Scale));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 0));
				}
				break;
			case 2: //Down Center
				fy = Height < 0 ? -3 : 1;
				if (Repeat > 1) {
					EdgePart->SetRelativeLocation(FVector(0, x + cnt * MeshScale * 10 + 5 * (Height < 0?-1:1), y * 10 - 5 * fy));
				}else{
					EdgePart->SetRelativeLocation(FVector(0, x + (Width * 10) / 2 - 5 * (Width < 0?-1:1), y * 10 - 5 * fy));
				}
				if (Rotate) {
					EdgePart->SetRelativeScale3D(FVector(1,Scale,1));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 180));
				}else {
					EdgePart->SetRelativeScale3D(FVector(1,Scale,1));
					EdgePart->SetRelativeRotation(FRotator(0, 0, 0));
				}
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
}
#pragma optimize( "", on )

#pragma optimize( "", off )
void AFINDefaultDynamicSizedModule::SpawnCornerComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int Width, int Height, bool Rotate, TArray<USceneComponent*>& OutParts) {
	UStaticMeshComponent* CornerPart = NewObject<UStaticMeshComponent>(Parent, Class);
	CornerPart->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	UE_LOG(LogFicsItNetworks_DebugRoze, Warning, L"SpawnCornerComponent %s", *CornerPartMesh->GetName());
	if (Rotate) {
		if (Width < 0) {
			if (Height < 0) {
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
			if (Height < 0) {
				if (r == 0) r = -1;
				else if (r == 2) r = 1;
				else if (r == 1) r = 2;
				else if (r == -1) r = 0;
			}
		}
		int fx = Width < 0 ? -3 : 1;
		int fy = Height < 0 ? -3 : 1;
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
	}else {
		switch(r) {
			case 0:
				CornerPart->SetRelativeLocation(FVector(0, x * 10 - 5, y * 10 - 5));
				break;
			case -1:
				CornerPart->SetRelativeLocation(FVector(0, x * 10 - 5, y * 10 + 5));
				break;
			case 1:
				CornerPart->SetRelativeLocation(FVector(0, x * 10 + 5, y * 10 - 5));
				break;
			case 2:
				CornerPart->SetRelativeLocation(FVector(0, x * 10 + 5, y * 10 + 5));
				break;
			default:
				break;
		}
		CornerPart->SetRelativeRotation(FRotator(0, 0, 0));
	}
	
	CornerPart->RegisterComponent();
	CornerPart->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	CornerPart->SetStaticMesh(CornerPartMesh);
	CornerPart->SetMobility(EComponentMobility::Static);
	OutParts.Add(CornerPart);
}
#pragma optimize( "", on )

void AFINDefaultDynamicSizedModule::GetMaximumDynamicSize_Implementation(int& Width, int& Height) {
	Width = MaxWidth;
	Height = MaxHeight;
}

void AFINDefaultDynamicSizedModule::GetMinimumDynamicSize_Implementation(int& Width, int& Height) {
	Width = MinWidth;
	Height = MinHeight;
}

UStaticMeshComponent* AFINDefaultDynamicSizedModule::CreateMeshComponent(TSubclassOf<UStaticMeshComponent> Class,
	UStaticMesh* Mesh, FVector Location, FRotator Rotation, FVector Scale, AActor* Parent, USceneComponent* Attach) {
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Parent, Class);
	Part->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeRotation(Rotation);
	Part->SetRelativeScale3D(Scale);
	Part->RegisterComponent();
	Part->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	Part->SetStaticMesh(Mesh);
	Part->SetMobility(EComponentMobility::Static);

	return Part;
}

void AFINDefaultDynamicSizedModule::getModuleSize_Implementation(int& Width, int& Height) const {
	Width = std::clamp(static_cast<int>(ModuleSize.X), MinWidth, MaxWidth);
	Height = std::clamp(static_cast<int>(ModuleSize.Y), MinHeight, MaxHeight);
}

void AFINDefaultDynamicSizedModule::SetPanelSize_Implementation(int width, int height) {
	
}
