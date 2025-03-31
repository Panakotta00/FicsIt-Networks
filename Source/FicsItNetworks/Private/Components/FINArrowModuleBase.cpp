#include "Components/FINArrowModuleBase.h"
#include "FicsItNetworksModule.h"
#include "UnrealNetwork.h"


AFINArrowModuleBase::AFINArrowModuleBase() {}

void AFINArrowModuleBase::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
	
	RebuildComponents(this, RootComponent, Anchors, Parts);
}

bool AFINArrowModuleBase::ShouldSave_Implementation() const {
	return true;
}

UStaticMeshComponent* AFINArrowModuleBase::CreateAndAddComponent(UStaticMesh* Mesh, AActor* Parent, USceneComponent* Attach, FVector Location, FRotator Rotation, TArray<UStaticMeshComponent*>& OutParts) {
	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Parent, UStaticMeshComponent::StaticClass());
	Part->AttachToComponent(Attach, FAttachmentTransformRules::KeepRelativeTransform);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeRotation(Rotation);
	Part->RegisterComponent();
	Part->SetStaticMesh(Mesh);
	Part->SetMobility(EComponentMobility::Static);
	Part->SetVisibility(true);
	OutParts.Add(Part);
	return Part;
}

void AFINArrowModuleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINArrowModuleBase, Anchors);
}


/*FString AFINArrowModuleBase::StructToHexString(void* Source, SIZE_T Size) {
	char str[Size];
	static char const hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	memcpy(str, Source, sizeof(Size));
	FString result = "";

	for( int i = 0; i < Size; ++i )
	{
		char const byte = str[i];

		result += hex_chars[ ( byte & 0xF0 ) >> 4 ];
		result += hex_chars[ ( byte & 0x0F ) >> 0 ];
	}

	return result;
}*/

//FString AFINArrowModuleBase::DebugPrintArrow(const FFINPanelArrow& Arrow) {
//	FString res = "{";
//	res+= "Rotation = " + FString::Printf(TEXT("%f, "), Arrow.rotation);
//	res+= "Color = " + FString::Printf(TEXT("{%f,%f,%f}"), Arrow.ArrowColor.R, Arrow.ArrowColor.G, Arrow.ArrowColor.B) + ", ";
//	res+= "InheritColor = " + FString::Printf(TEXT("%d, "), Arrow.InheritColor);
//	res+= "InnerEnd = " + FString::Printf(TEXT("%d, "), Arrow.InnerEnd.GetValue());
//	res+= "OuterEnd = " + FString::Printf(TEXT("%d}"), Arrow.OuterEnd.GetValue());
//	return res;
//}
//
//FString AFINArrowModuleBase::DebugPrintAnchor(FFINPanelArrowAnchor& Anchor) {
//	FString res = "{";
//	res+= "Type = " + FString::FromInt(Anchor.Type.GetValue()) + ", ";
//	res+= "Color = " + FString::Printf(TEXT("{%f,%f,%f}"), Anchor.AnchorColor.R, Anchor.AnchorColor.G, Anchor.AnchorColor.B) + ", ";
//	res+= "Position = " + FString::Printf(TEXT("{%f,%f,%f}"), Anchor.AnchorPosition.X, Anchor.AnchorPosition.Y, Anchor.AnchorPosition.Z) + ", ";
//	res+= "Rotation = " + FString::Printf(TEXT("{%f,%f,%f}"), Anchor.AnchorRotation.Pitch, Anchor.AnchorRotation.Yaw, Anchor.AnchorRotation.Roll) + ", ";
//	res+= "Arrows = {";
//	for (auto Arrow : Anchor.Arrows) {
//		res+= DebugPrintArrow(Arrow);
//	}
//	res+= "}";
//	return res;
//}

void AFINArrowModuleBase::Serialize(FArchive& ar) {
	Super::Serialize(ar);
}

void AFINArrowModuleBase::RebuildComponents(AActor* Parent, USceneComponent* Attach, TArray<FFINPanelArrowAnchor>& OutAnchors,  TArray<UStaticMeshComponent*>& OutParts) {
	for (UStaticMeshComponent* comp : OutParts) {
		if(IsValid(comp)) {
			comp->UnregisterComponent();
			comp->SetActive(false);
			comp->DestroyComponent();
		}
	}
	
	UE_LOG(LogFicsItNetworks, Verbose, L"RebuildComponents()");
	OutParts.Empty();
	int AnchorIDX = 0;
	for (auto Anchor : OutAnchors) {
		UE_LOG(LogFicsItNetworks, Verbose, L"Calculating Anchor %d", AnchorIDX++);
		FRotator TempRotation = Anchor.AnchorRotation;
		UStaticMeshComponent* MeshComponent = nullptr;
		bool hasArrows = false;
		for (auto Arrow : Anchor.Arrows) {
			if(Arrow.OuterEnd != FINPanelTraceEnd_None) {
				hasArrows = true;
				break;
			}
		}
		if(!hasArrows) {
			Anchor.Type = FIN_PanelArrowCrossing_Dot; // Fallback to prevent invisible modules
		}
		switch (Anchor.Type) {
			case FIN_PanelArrowCrossing_Dot: {
				MeshComponent = CreateAndAddComponent(CenterDotMesh, Parent, Attach, Anchor.AnchorPosition, TempRotation, OutParts);
				break;
			}
			case FIN_PanelArrowCrossing_BridgeH:
				TempRotation+= FRotator(0,90,0);
			case FIN_PanelArrowCrossing_BridgeV: {
				MeshComponent = CreateAndAddComponent(CenterBridgeMesh, Parent, Attach, Anchor.AnchorPosition, TempRotation, OutParts);
				break;
			}
		}
		if(MeshComponent != nullptr) {
			MeshComponent->SetVectorParameterValueOnMaterials("BaseColor", FVector(Anchor.AnchorColor));
		}
		TempRotation = Anchor.AnchorRotation;
		int ArrowIDX = 0;
		for (auto Arrow : Anchor.Arrows) {
			UE_LOG(LogFicsItNetworks, Verbose, L"   Calculating Arrow %d", ArrowIDX++);
			FRotator ArrowRotation = TempRotation;
			ArrowRotation+= FRotator(0,Arrow.rotation, 0);
			FLinearColor ArrowColor = Arrow.ArrowColor;
			if(Arrow.InheritColor) {
				ArrowColor = Anchor.AnchorColor;	
			}
			MeshComponent = nullptr;
			switch(Arrow.OuterEnd) {
				case FINPanelTraceEnd_Straight: {
					UE_LOG(LogFicsItNetworks, Verbose, L"     --- FINPanelTraceEnd_Straight");
					MeshComponent = CreateAndAddComponent(TipStraightMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
					break;
				}
				case FINPanelTraceEnd_Blockage: {
					UE_LOG(LogFicsItNetworks, Verbose, L"     --- FINPanelTraceEnd_Blockage");
					MeshComponent = CreateAndAddComponent(TipBlockedMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
					break;
				}
				case FINPanelTraceEnd_RecessedBlockage: {
					UE_LOG(LogFicsItNetworks, Verbose, L"     --- FINPanelTraceEnd_RecessedBlockage");
					MeshComponent = CreateAndAddComponent(TipRecessedBlockedMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
					break;
				}
				case FINPanelTraceEnd_ArrowOut: {
					UE_LOG(LogFicsItNetworks, Verbose, L"     --- FINPanelTraceEnd_ArrowOut");
					MeshComponent = CreateAndAddComponent(TipArrowMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
					break;
				}
			}
			if(MeshComponent != nullptr) {
				MeshComponent->SetVectorParameterValueOnMaterials("BaseColor", FVector(ArrowColor));
			}
			MeshComponent = nullptr;
			if(Arrow.OuterEnd != FINPanelTraceEnd_None) {
				switch (Arrow.InnerEnd) {
					case FINPanelTraceStart_Half: {
						MeshComponent = CreateAndAddComponent(CenterButtMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
						break;
					}
					case FINPanelTraceStart_CapRound: {
						MeshComponent = CreateAndAddComponent(CenterCapRoundMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
						break;
					}
					case FINPanelTraceStart_CapSquare: {
						MeshComponent = CreateAndAddComponent(CenterCapSquareMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
						break;
					}
					case FINPanelTraceStart_Miter: {
						MeshComponent = CreateAndAddComponent(CenterMiterMesh, Parent, Attach, Anchor.AnchorPosition, ArrowRotation, OutParts);
						break;
					}					
				}
			}
			if(MeshComponent != nullptr) {
				MeshComponent->SetVectorParameterValueOnMaterials("BaseColor", FVector(ArrowColor));
			}
		}
	}
}



void AFINArrowModuleBase::BeginPlay() {
	Super::BeginPlay();
	RebuildComponents(this, RootComponent, Anchors, Parts);
}



