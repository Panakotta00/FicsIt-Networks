#pragma once

#include "FINComputerModule.h"
#include "FINPciDeviceInterface.h"
#include "FicsItNetworks/Network/FINFuture.h"
#include "Interfaces/IHttpRequest.h"

#include "FINInternetCard.generated.h"

USTRUCT()
struct FFINInternetCardHttpRequestFuture : public FFINFuture {
	GENERATED_BODY()
private:
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request;

public:
	FFINInternetCardHttpRequestFuture() = default;
	FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InRequest);
	
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
	FFINInternetCardHttpRequestFuture netFunc_request(const FString& InURL, const FString& InMethod, const FString& InData, TArray<FFINAnyNetworkValue> varargs);
	UFUNCTION()
	void netFuncMeta_request(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "request";
		DisplayName = FText::FromString("Request");
		Description = FText::FromString("Does an HTTP-Request. If a payload is given, the Content-Type header has to be set. All additional parameters have to be strings and in pairs of two for defining the http headers and values.");
		ParameterInternalNames.Add("url");
		ParameterDisplayNames.Add(FText::FromString("URL"));
		ParameterDescriptions.Add(FText::FromString("The URL for which you want to make an HTTP Request."));
		ParameterInternalNames.Add("method");
		ParameterDisplayNames.Add(FText::FromString("Method"));
		ParameterDescriptions.Add(FText::FromString("The http request method/verb you want to make the request. f.e. 'GET', 'POST'"));
		ParameterInternalNames.Add("data");
		ParameterDisplayNames.Add(FText::FromString("Data"));
		ParameterDescriptions.Add(FText::FromString("The http request payload you want to sent."));
		Runtime = 1;
	}
};
