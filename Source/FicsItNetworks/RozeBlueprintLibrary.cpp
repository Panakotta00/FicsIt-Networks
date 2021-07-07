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
	root /= *folder;
	std::filesystem::create_directories(root);
	auto pathToFile = root / path;
	pathToFile = std::filesystem::absolute(pathToFile);
	auto ps = pathToFile.string();
	if (ps.rfind(std::filesystem::absolute(root).string(), 0) != 0 || !std::filesystem::exists(pathToFile)) {
		return TCHAR_TO_UTF8("");
	}
	FString ret(ps.c_str());
	return ret;
}
