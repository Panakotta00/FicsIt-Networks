#include "RozeBlueprintLibrary.h"

#include "FGSaveSystem.h"
#include <filesystem>
#include <fstream>
#include <sstream>




FString URozeBlueprintLibrary::ExpandPath(FString folder, FString fileName, FString extension){
	FString fsp = UFGSaveSystem::GetSaveDirectoryPath();

	auto file = fileName + extension;
	auto path = std::filesystem::path(*file);

	std::filesystem::path root = *fsp;
	root /= *folder.Replace(TEXT("/"), TEXT("\\"));
	std::filesystem::create_directories(root);
	auto pathToFile = root / path;
	pathToFile = std::filesystem::absolute(pathToFile);
	auto ps = pathToFile.string();
	extension = extension.ToLower();
	if (ps.rfind(std::filesystem::absolute(root).string(), 0) != 0 || !std::filesystem::exists(pathToFile)) {
		return TCHAR_TO_UTF8("");
	}
	FString ret(ps.c_str());
	return ret;
}

void EnumFiles(std::filesystem::path folder, TArray<FString>* ret, FString extensionMask, int maxDepth = 0, int depth = 0) {
	for(auto& p: std::filesystem::directory_iterator(folder)) {
		std::filesystem::path _p = p;
		if (p.is_directory()) {
			if(maxDepth > 0 && depth < maxDepth - 1) {
				// EnumFiles(p, ret, maxDepth, depth + 1)
			}
		} else {
			FString toAdd = p.path().string().c_str();
			if (toAdd.ToLower().EndsWith(extensionMask)) {
				ret->Add(toAdd);
			}
		}
	}
}

TArray<FString> URozeBlueprintLibrary::GetFilesInPath(FString folder, FString extension) {
	FString fsp = UFGSaveSystem::GetSaveDirectoryPath();

	std::filesystem::path root = *fsp;
	root /= *folder;
	std::filesystem::create_directories(root);

	TArray<FString> list;

	if (std::filesystem::exists(root) && std::filesystem::is_directory(root)) {
		EnumFiles(root, &list, extension);
	}

	return list;
}


FString URozeBlueprintLibrary::GetFileName(FString path, bool stripExtension) {
	FString toMutate = path;
	int32 pos = 0;
	if(toMutate.FindLastChar('\\', pos)) {
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

FString URozeBlueprintLibrary::ColorToHexString(FLinearColor color) {
	const FColor c = color.ToRGBE();
	return c.ToHex();
}

FLinearColor URozeBlueprintLibrary::HexStringToLinearColor(FString colorString) {
	const FColor c = FColor::FromHex(colorString);
	return FLinearColor::FromSRGBColor(c);
}