#pragma once

#include "CoreMinimal.h"
#include "FINFuture.h"
#include "FINPciDeviceInterface.h"
#include "ComputerModules/FINComputerModule.h"
#include "FINInternetCard.generated.h"

class IHttpRequest;

// FIN-1.2-PORT: Snapshot of the HTTP response.
// Execute()/completion delegate run on the game thread (Kernel->HandleFutures resp.
// HttpManager tick), whereas IsDone()/GetOutput() run on the Lua thread (await poll).
// In UE5.6 any Lua-thread access to the still-live FHttpRequest/FHttpResponse corrupts
// the process heap (race with the HttpManager, which fills the response payload TArray
// on the game thread) -> STATUS_HEAP_CORRUPTION (0xc0000374), visible with a delay on the
// render thread. Solution: copy Code/Content/Headers into this shared struct inside the
// completion delegate (game thread); the Lua thread then only reads the snapshot and
// never touches the live object again.
struct FFINInternetCardHttpResult {
	bool bComplete = false;
	int64 Code = 0;
	FString Content;
	TArray<FString> Headers; // alternating Name, Value
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINInternetCardHttpRequestFuture : public FFINFuture {
	GENERATED_BODY()
private:
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request;
	// Shared ptr, because the future struct gets copied: Execute() (game thread) writes the
	// snapshot, IsDone()/GetOutput() (Lua thread) read it. All copies share the same block.
	TSharedPtr<FFINInternetCardHttpResult> Result = MakeShared<FFINInternetCardHttpResult>();

public:
	FFINInternetCardHttpRequestFuture() = default;
	FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InRequest);
	
	// Begin FFINFuture
	virtual void Execute() override;
	virtual bool IsDone() const override;
	virtual TArray<FFIRAnyValue> GetOutput() const override;
	// End FFINFuture
};

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINInternetCard : public AFINComputerModule, public IFINPciDeviceInterface {
	GENERATED_BODY()
public:
	UFUNCTION()
	FFINInternetCardHttpRequestFuture netFunc_request(const FString& InURL, const FString& InMethod, const FString& InData, TArray<FFIRAnyValue> varargs);
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
