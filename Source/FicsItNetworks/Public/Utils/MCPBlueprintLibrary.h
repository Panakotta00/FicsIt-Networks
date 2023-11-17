#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MCPBlueprintLibrary.generated.h"

USTRUCT(Blueprintable)
struct FFINUIFileInfo {
	GENERATED_BODY()
	
	FFINUIFileInfo(FString fullPath) : Path(""), Filename(""), FullPath(fullPath)  {}
	FFINUIFileInfo() : Path(), Filename(), FullPath("")  {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Path;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Filename;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FullPath;
};


UCLASS()
class FICSITNETWORKS_API UMCPBlueprintLibrary : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()
	//GENERATED_UCLASS_BODY()
	
    /** Starts an analytics session without any custom attributes specified */
    UFUNCTION(BlueprintCallable, Category="Utilities")
    static FString ExpandPath(FString folder, FString file, FString extension);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static TArray<FString> GetFilesInPath(FString folder, FString extension, bool create);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static FString GetFileName(FString path, bool stripExtension);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static FString ColorToHexString(FLinearColor color);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static FLinearColor HexStringToLinearColor(FString colorString);

	UFUNCTION(BlueprintCallable, Category = "Utilities")
	static FVector2D MeasureStringRenderSize(UFont* Font, FString Text);
};
