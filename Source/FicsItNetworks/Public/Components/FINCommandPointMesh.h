#pragma once

#include "FGColoredInstanceMeshProxy.h"
#include "FINCommandPointMesh.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class UFINCommandPointMesh : public UFGColoredInstanceMeshProxy {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UTexture2D* LoadTextureFromFile(FString str);
};


USTRUCT(Blueprintable)
struct FFINCommandLabelReferences {
	GENERATED_BODY()

	FFINCommandLabelReferences(UObject* reference, UObject* textReference, int index) :  Index(index), Reference(reference), TextObjectReference(textReference) {}
	FFINCommandLabelReferences() : Index(0), Reference(nullptr), TextObjectReference(nullptr) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int Index;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* Reference;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* TextObjectReference;
	
};

UENUM(BlueprintType)
enum EFINLabelPlacement {
	FIN_LabelPlacement_North, FIN_LabelPlacement_South, FIN_LabelPlacement_East, FIN_LabelPlacement_West
};

UENUM(BlueprintType)
enum EFINButtonFunctionalMode {
	FIN_ButtonMode_Simple, FIN_ButtonMode_Double, FIN_ButtonMode_DoubleLatched, FIN_ButtonMode_MAX
};

USTRUCT()
struct FFINPanelArrowEndData {
	GENERATED_BODY()
	
};

UENUM(BlueprintType)
enum EFINPanelArrowCrossingTypes {
	FIN_PanelArrowCrossing_Lines,
	FIN_PanelArrowCrossing_Dot,
	FIN_PanelArrowCrossing_BridgeH,
	FIN_PanelArrowCrossing_BridgeV,
};

UENUM(BlueprintType)
enum EFINPanelTraceEndTypes {
	FINPanelTraceEnd_None = 0,
	FINPanelTraceEnd_Straight = 1,
	FINPanelTraceEnd_ExtendedStraight = 5,
	FINPanelTraceEnd_RecessedBlockage = 2,
	FINPanelTraceEnd_Blockage = 3,
	FINPanelTraceEnd_ArrowOut = 4,
	FINPanelTraceEnd_ArrowIn = 5,
};

UENUM(BlueprintType)
enum EFINPanelTraceStartTypes {
	FINPanelTraceStart_None,
	FINPanelTraceStart_Half,
	FINPanelTraceStart_CapSquare,
	FINPanelTraceStart_CapRound,
	FINPanelTraceStart_Miter,
};

USTRUCT(Blueprintable, BlueprintType)
struct FFINPanelArrow {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	double rotation = 0.0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	TEnumAsByte<EFINPanelTraceEndTypes> OuterEnd = FINPanelTraceEnd_None;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	TEnumAsByte<EFINPanelTraceStartTypes> InnerEnd = FINPanelTraceStart_CapSquare;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	bool InheritColor = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	FLinearColor ArrowColor = FLinearColor(0,0,0,1);
};

USTRUCT(Blueprintable, BlueprintType)
struct FFINPanelArrowAnchor {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	FVector AnchorPosition = FVector(0,0,0);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	FRotator AnchorRotation = FRotator(0,0,0);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	TEnumAsByte<EFINPanelArrowCrossingTypes> Type = FIN_PanelArrowCrossing_Lines;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	TArray<FFINPanelArrow> Arrows;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
	FLinearColor AnchorColor = FLinearColor(0,0,0,1);

	/*static FString ToString(FFINPanelArrowAnchor Anchor) {
		FString Ret = "";
		Ret.Append(FString::Format(""))
	}*/
};


USTRUCT(Blueprintable)
struct FFINCommandLabelData  {
	GENERATED_BODY()

	FFINCommandLabelData() : ImagePath(""), Vertical(false), Text(""), TextColor(0,0,0,1), Emit(0){}
	FFINCommandLabelData(FString imagePath, FString text, FLinearColor textColor, bool vertical, float emit) : ImagePath(imagePath), Vertical(vertical), Text(text), TextColor(textColor), Emit(emit) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString ImagePath;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool Vertical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FLinearColor TextColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TEnumAsByte<EFINLabelPlacement> placement = FIN_LabelPlacement_North;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float Emit;
};


USTRUCT(Blueprintable)
struct FFINCommandLabelStructure {
	GENERATED_BODY()
	
	FFINCommandLabelStructure(UObject* reference, int index, FString path, bool doUpdate, bool doUpdateText, FString text, FLinearColor color, EFINLabelPlacement placement, float emit) : reference(reference), path(path), index(index), vertical(false), doUpdate(doUpdate), doUpdateText(doUpdateText), text(text), textColor(color), placement(placement), emit(emit) {}
	FFINCommandLabelStructure(UObject* reference, int index, FString path, bool doUpdate, bool doUpdateText, FString text, FLinearColor color, EFINLabelPlacement placement) : reference(reference), path(path), index(index), vertical(false), doUpdate(doUpdate), doUpdateText(doUpdateText), text(text), textColor(color), placement(placement) {}
	FFINCommandLabelStructure(UObject* reference, int index, FString path, bool vertical, bool doUpdate, bool doUpdateText, FString text, FLinearColor color) : reference(reference), path(path), index(index), vertical(vertical), doUpdate(doUpdate), doUpdateText(doUpdateText), text(text), textColor(color) {
		if(vertical) {
			placement = FIN_LabelPlacement_West;
		}
	}
	FFINCommandLabelStructure() : reference(0), path(""), index(0), vertical(false), doUpdate(false), doUpdateText(false), text(""), textColor(0,0,0, 1), emit(0)  {}
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* reference;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* textObjectReference;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString path;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int index;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool vertical;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool doUpdate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool doUpdateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FLinearColor textColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TEnumAsByte<EFINLabelPlacement> placement = FIN_LabelPlacement_North;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float emit;
};
