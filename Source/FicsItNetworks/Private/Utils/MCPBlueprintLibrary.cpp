#include "Utils/MCPBlueprintLibrary.h"
#include "FGSaveSystem.h"
#include <filesystem>

#include "Engine/Font.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

FString UMCPBlueprintLibrary::ExpandPath(FString folder, FString fileName, FString extension){
	FString fsp = UFGSaveSystem::GetSaveDirectoryPath();
	extension = extension.ToLower();
	auto file = fileName + extension;
	auto proot = fsp / folder.Replace(TEXT("\\"), TEXT("/"));
	proot = FPaths::ConvertRelativePathToFull(proot);
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(ToCStr(proot));

	auto pathToFile = proot / file;
	auto ps = FString(pathToFile);
	if ( ps.Find(proot, ESearchCase::IgnoreCase, ESearchDir::FromEnd) != 0 || !FPlatformFileManager::Get().GetPlatformFile().FileExists(ToCStr(pathToFile))) {
		return TCHAR_TO_UTF8("");
	}
	return ps;

}

class MyVisitor : public IPlatformFile::FDirectoryVisitor{
public:

	MyVisitor(TArray<FString>* list, FString extensionMask, int maxDepth = 0, int depth = 0) : List(list), ExtensionMask(extensionMask), Depth(depth), MaxDepth(maxDepth) {	}

	FString ExtensionMask;
	TArray<FString>* List;
	int MaxDepth;
	int Depth;

	bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) {
		UE_LOG(LogTemp, Verbose, TEXT("%s"), FilenameOrDirectory);
		if(!bIsDirectory) {
			FString str = FString(FilenameOrDirectory);
			if(str.EndsWith(ExtensionMask)) {
				List->Add(FString(FilenameOrDirectory));
			}
		}else if(MaxDepth > 0 && Depth < MaxDepth - 1) {
			MyVisitor Visitor = MyVisitor(List, ExtensionMask);
			FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(FilenameOrDirectory, Visitor);
		}
		return true;
	}
};

TArray<FString> UMCPBlueprintLibrary::GetFilesInPath(FString folder, FString extension, bool create) {
	FString fsp = UFGSaveSystem::GetSaveDirectoryPath();
	FString root = fsp;
	if(!root.EndsWith("/")) {
		root+= "/";
	}
	root+= folder;
	FString absPath = FPaths::ConvertRelativePathToFull(root);
	if(create == true) {
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(ToCStr(absPath));
	}
	TArray<FString> list;

	if (FPaths::DirectoryExists(absPath)){
		MyVisitor Visitor = MyVisitor(&list, extension);
		FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(ToCStr(absPath), Visitor);
	}

	return list;
}


FString UMCPBlueprintLibrary::GetFileName(FString path, bool stripExtension) {
	FString toMutate = path;
	int32 pos = 0;
	if(toMutate.FindLastChar('/', pos)) {
		toMutate = toMutate.Mid(pos + 1);
	}
	if(stripExtension) {
		pos = 0;
		if(toMutate.FindLastChar('.', pos)) {
			toMutate = toMutate.Mid(0, pos);
		}
	}
	return toMutate;
}

FString UMCPBlueprintLibrary::ColorToHexString(FLinearColor color) {
	const FColor c = color.ToFColor(true);
	return c.ToHex();
}

FLinearColor UMCPBlueprintLibrary::HexStringToLinearColor(FString colorString) {
	const FColor c = FColor::FromHex(colorString);
	return FLinearColor::FromSRGBColor(c);
}

FVector2D UMCPBlueprintLibrary::MeasureStringRenderSize(UFont* Font, FString Text) {
	//TSharedRef<FSlateFontMeasure> fontMeasureService = (
	//	FSlateApplication::Get().GetRenderer()->GetFontMeasureService()	
	//);
	//return fontMeasureService->Measure(Text, Font->GetLegacySlateFontInfo());
	const TCHAR* Delims[] = {TEXT("\n"), TEXT("\r\n")};
	FVector2D vector;
	TArray<FString> Array;
	Text.ParseIntoArray(Array, Delims, 2);
	int MWidth = 0;
	int MHeight = 0;
	for (auto Line : Array) {
		int W = Font->GetStringSize(*Line);
		int H = Font->GetStringHeightSize(*Line);
		if(W > MWidth) {
			MWidth = W;
		}
		if(H > MHeight) {
			MHeight = H;
		}
	}
	vector.X = MWidth;
	vector.Y = MHeight * Array.Num();
	return vector;
}
