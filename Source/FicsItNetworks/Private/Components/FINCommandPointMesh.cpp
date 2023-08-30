#include "Components/FINCommandPointMesh.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Developer/TargetPlatform/Public/Interfaces/IAudioFormat.h"
#include <filesystem>

UTexture2D* UFINCommandPointMesh::LoadTextureFromFile(FString str) {
	FString fsp;
	// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
	if (fsp.IsEmpty()) {
		fsp = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/") TEXT("SaveGames/"));
	}

	auto file = str + TEXT(".png");
	auto path = std::filesystem::path(*file);

	std::filesystem::path root = *fsp;
	root /= "Computers/Icons";
	std::filesystem::create_directories(root);
	auto pathToFile = root / path;
	pathToFile = std::filesystem::absolute(pathToFile);
	auto ps = pathToFile.string();
	if (ps.rfind(std::filesystem::absolute(root).string(), 0) != 0 || !std::filesystem::exists(pathToFile)) {
		return nullptr;
	}

	FString fstrPath = UTF8_TO_TCHAR(ps.c_str());


	FString pngfile = "myimage.png";
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	// Note: PNG format.  Other formats are supported
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	TArray<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *pngfile))
	{
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			TArray<uint8> UncompressedBGRA;
			// bool GetRaw(const ERGBFormat InFormat, int32 InBitDepth, TArray<uint8>& OutRawData)
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
			{
				return UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);
			}
		}
	}

	return nullptr;
}
