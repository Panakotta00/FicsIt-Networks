#include "FINRepoEndpoint.h"

#include "HttpModule.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IHttpResponse.h"
#include "ModLoading/ModLoadingLibrary.h"

FGuid UFINRepoEndpoint::SearchPackages(const FFINRepoSearchQuery& Query) {
	FGuid guid = FGuid::NewGuid();
	TSharedRef<IHttpRequest> request = CreateSearchPackagesRequest(Query);
	Requests.Add(request);
	request->OnProcessRequestComplete().BindLambda([this, guid, request](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully) {
		TOptional<TArray<FFINRepoPackageCard>> packages = PackagesFromResponse(Response);
		if (TArray<FFINRepoPackageCard>* ptr = packages.GetPtrOrNull()) {
			OnSearchPackagesCallback.Broadcast(guid, true, *ptr);
		} else {
			OnSearchPackagesCallback.Broadcast(guid, false, {});
		}
		Requests.Remove(request);
	});
	request->ProcessRequest();

	return guid;
}

TSharedRef<IHttpRequest> UFINRepoEndpoint::CreateSearchPackagesRequest(const FFINRepoSearchQuery& Query, uint64 Page, uint64 PageSize) {
	FString searchQuery = Query.Query;
	for (const FString& tag : Query.Tags) {
		searchQuery += FString::Printf(TEXT(" +tag:\"%s\""), *tag);
	}
	for (const FString& author : Query.Authors) {
		searchQuery += FString::Printf(TEXT(" +author:\"%s\""), *author);
	}
	FString search = FGenericPlatformHttp::UrlEncode(searchQuery);
	FString url = FString::Printf(TEXT("%s?search=%s&page=%llu"), *BaseURL, *search, Page);

	if (IsValid(GEngine->GameViewport)) if (UGameInstance* gameInstance = GEngine->GameViewport->GetGameInstance()) {
		UModLoadingLibrary* modLib = gameInstance->GetSubsystem<UModLoadingLibrary>();
		for (const FModInfo& mod : modLib->GetLoadedMods()) {
			FString version = FGenericPlatformHttp::UrlEncode(*mod.Version.ToString());
			if (mod.Name == TEXT("FicsItNetworks")) {
				url += FString::Printf(TEXT("&fin_version=%s"), *version);
			} else {
				url += FString::Printf(TEXT("&mod_%s=%s"), *mod.Name, *version);
			}
		}
	}
	// TODO: Add game version to url

	TSharedRef<IHttpRequest> request = FHttpModule::Get().CreateRequest();
	request->SetURL(url);
	request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	return request;
}

TOptional<TArray<FFINRepoPackageCard>> UFINRepoEndpoint::PackagesFromResponse(const TSharedPtr<IHttpResponse>& Response) {
	if (Response->GetResponseCode() != 200) return {};

	TSharedRef<TJsonReader<TCHAR>> reader = FJsonStringReader::Create(Response->GetContentAsString());
	TArray<TSharedPtr<FJsonValue>> values;
	if (!FJsonSerializer::Deserialize(reader, values)) return {};

	TArray<FFINRepoPackageCard> packages;
	for (auto value : values) {
		TSharedPtr<FJsonObject>* JsonObject;
		if (value->TryGetObject(JsonObject)) {
			TOptional<FFINRepoPackageCard> package = FFINRepoPackageCard::FromJsonObject(*JsonObject);
			if (package.IsSet()) {
				packages.Add(*package);
			}
		}
	}

	return packages;
}

FGuid UFINRepoEndpoint::GetPackage(const FString& ID, const FString& VersionString) {
	FGuid guid = FGuid::NewGuid();
	TSharedRef<IHttpRequest> request = CreateGetPackageRequest(ID, VersionString);
	Requests.Add(request);
	request->OnProcessRequestComplete().BindLambda([this, guid, request](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully) {
		TOptional<TTuple<FFINRepoPackage, FFINRepoVersion>> packages = PackageFromResponse(Response);
		if (TTuple<FFINRepoPackage, FFINRepoVersion>* ptr = packages.GetPtrOrNull()) {
			OnGetPackageCallback.Broadcast(guid, true, ptr->Get<0>(), ptr->Get<1>());
		} else {
			OnGetPackageCallback.Broadcast(guid, false, {}, {});
		}
		Requests.Remove(request);
	});

	request->ProcessRequest();

	return guid;
}

TSharedRef<IHttpRequest> UFINRepoEndpoint::CreateGetPackageRequest(const FString& ID, const FString& VersionString) {
	FString url = FString::Printf(TEXT("%s/package/%s"), *BaseURL, *ID);

	if (!VersionString.IsEmpty()) {
		url += FString::Printf(TEXT("?version=%s"), *VersionString);
	}

	TSharedRef<IHttpRequest> request = FHttpModule::Get().CreateRequest();
	request->SetURL(url);
	request->SetHeader(TEXT("Accept"), TEXT("application/json"));

	return request;
}

TOptional<TTuple<FFINRepoPackage, FFINRepoVersion>> UFINRepoEndpoint::PackageFromResponse(const TSharedPtr<IHttpResponse>& Response) {
	if (Response->GetResponseCode() != 200) return {};

	TSharedRef<TJsonReader<TCHAR>> reader = FJsonStringReader::Create(Response->GetContentAsString());
	TSharedPtr<FJsonObject> object;
	if (!FJsonSerializer::Deserialize(reader, object)) return {};

	const TSharedPtr<FJsonObject>* package;
	if (!object->TryGetObjectField(TEXT("package"), package)) return {};
	const TSharedPtr<FJsonObject>* version;
	if (!object->TryGetObjectField(TEXT("version"), version)) return {};
	TOptional<FFINRepoPackage> pkg = FFINRepoPackage::FromJsonObject(*package);
	if (!pkg.IsSet()) return {};
	TOptional<FFINRepoVersion> v = FFINRepoVersion::FromJsonObject(*version);
	if (!v.IsSet()) return {};
	return TTuple<FFINRepoPackage, FFINRepoVersion>(*pkg, *v);
}

TSharedRef<TMulticastDelegate<void(TOptional<FString>)>> UFINRepoEndpoint::GetEEPROMContent(const FString& PackageID, const FString& Version, const FString& EEPROMName) {
	FString url = FString::Printf(TEXT("%s/Packages/%s/v%s/%s"), *GitHubRawURL, *PackageID, *Version, *EEPROMName);

	TSharedRef<IHttpRequest> request = FHttpModule::Get().CreateRequest();
	request->SetURL(url);

	Requests.Add(request);

	TSharedRef<TMulticastDelegate<void(TOptional<FString>)>> Callback = MakeShared<TMulticastDelegate<void(TOptional<FString>)>>();

	request->OnProcessRequestComplete().BindLambda([this, request, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully) {
		Requests.Remove(request);
		if (Response->GetResponseCode() != 200) {
			Callback->Broadcast({});
		} else {
			Callback->Broadcast({Response->GetContentAsString()});
		}
	});

	request->ProcessRequest();

	return Callback;
}
