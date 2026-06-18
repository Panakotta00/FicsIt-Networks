#pragma once

#include "CoreMinimal.h"
#include "FINRepoModel.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "FINRepoEndpoint.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFINRepoSearchPackagesCallback, FGuid, Request, bool, Success, TArray<FFINRepoPackageCard>, Packages);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFINRepoGetPackageCallback, FGuid, Request, bool, Success, FFINRepoPackage, Package, FFINRepoVersion, Version);

USTRUCT(BlueprintType)
struct FICSITNETWORKSREPOSITORY_API FFINRepoSearchQuery {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Query;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Tags;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Authors;
};

UCLASS()
class FICSITNETWORKSREPOSITORY_API UFINRepoEndpoint : public UEngineSubsystem {
	GENERATED_BODY()
public:
	const FString BaseURL = TEXT("https://ficsit-networks-repository.panakotta00.dev");
	//const FString BaseURL = TEXT("http://localhost:3000");
	const FString GitHubRawURL = TEXT("https://raw.githubusercontent.com/Panakotta00/FicsIt-Networks-Repository/main");

	UPROPERTY(BlueprintAssignable)
	FFINRepoSearchPackagesCallback OnSearchPackagesCallback;
	UPROPERTY(BlueprintAssignable)
	FFINRepoGetPackageCallback OnGetPackageCallback;

	TSet<TSharedRef<IHttpRequest>> Requests;

	UFUNCTION(BlueprintCallable)
	FGuid SearchPackages(const FFINRepoSearchQuery& Query);
	TSharedRef<IHttpRequest> CreateSearchPackagesRequest(const FFINRepoSearchQuery& Query, uint64 Page = 0, uint64 PageSize = 10);
	static TOptional<TArray<FFINRepoPackageCard>> PackagesFromResponse(const TSharedPtr<IHttpResponse>& Response);

	UFUNCTION(BlueprintCallable)
	FGuid GetPackage(const FString& ID, const FString& VersionString);
	TSharedRef<IHttpRequest> CreateGetPackageRequest(const FString& ID, const FString& VersionString);
	static TOptional<TTuple<FFINRepoPackage, FFINRepoVersion>> PackageFromResponse(const TSharedPtr<IHttpResponse>& Response);

	TSharedRef<TMulticastDelegate<void(TOptional<FString>)>> GetEEPROMContent(const FString& PackageID, const FString& Version, const FString& EEPROMName);
};