#pragma once

#include "FINComputerModule.h"
#include "FINPciDeviceInterface.h"
#include "FicsItNetworks/Network/FINFuture.h"
#include "Interfaces/IHttpRequest.h"

#include "FINInternetCard.generated.h"

USTRUCT()
struct FFINInternetCardHttpResponse {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FString Content;

	UPROPERTY(SaveGame)
	int StatusCode;

	UPROPERTY(SaveGame)
	TArray<FString> Headers;
};

USTRUCT()
struct FFINInternetCardHttpRequestFuture : public FFINFuture {
	GENERATED_BODY()
private:
	TSharedPtr<IHttpRequest> Request;

public:
	FFINInternetCardHttpRequestFuture() = default;
	FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest> InRequest);
	
	// Begin FFINFuture
	virtual void Execute() override;
	virtual bool IsDone() const override;
	virtual TArray<FFINAnyNetworkValue> GetOutput() const override;
	// End FFINFuture
};

UCLASS()
class AFINInternetCard : public AFINComputerModule, public IFINPciDeviceInterface {
	GENERATED_BODY()
public:
	UFUNCTION()
	FFINInternetCardHttpRequestFuture netFunc_request(const FString& InURL, const FString& InMethod, const FString& InData, const TArray<FFINAnyNetworkValue>& varargs);
};
