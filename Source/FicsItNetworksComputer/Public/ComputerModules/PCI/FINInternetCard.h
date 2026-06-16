#pragma once

#include "CoreMinimal.h"
#include "FINFuture.h"
#include "FINPciDeviceInterface.h"
#include "ComputerModules/FINComputerModule.h"
#include "FINInternetCard.generated.h"

class IHttpRequest;

// FIN-1.2-PORT: Snapshot der HTTP-Antwort.
// Execute()/Completion-Delegate laufen auf dem Game-Thread (Kernel->HandleFutures bzw.
// HttpManager-Tick), IsDone()/GetOutput() dagegen auf dem Lua-Thread (await-Poll).
// In UE5.6 zerschiesst jeder Lua-Thread-Zugriff auf das noch lebende FHttpRequest/
// FHttpResponse den Prozess-Heap (Race mit dem HttpManager, der den Response-Payload-
// TArray auf dem Game-Thread befuellt) -> STATUS_HEAP_CORRUPTION (0xc0000374), verzoegert
// auf dem Render-Thread sichtbar. Loesung: Code/Content/Headers im Completion-Delegate
// (Game-Thread) in diese geteilte Struct kopieren; der Lua-Thread liest nur noch den
// Snapshot und fasst das Live-Objekt nie mehr an.
struct FFINInternetCardHttpResult {
	bool bComplete = false;
	int64 Code = 0;
	FString Content;
	TArray<FString> Headers; // alternierend Name, Value
};

USTRUCT()
struct FICSITNETWORKSCOMPUTER_API FFINInternetCardHttpRequestFuture : public FFINFuture {
	GENERATED_BODY()
private:
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request;
	// Geteilter Ptr, da das Future-Struct kopiert wird: Execute() (Game-Thread) schreibt den
	// Snapshot, IsDone()/GetOutput() (Lua-Thread) lesen ihn. Alle Kopien teilen denselben Block.
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
