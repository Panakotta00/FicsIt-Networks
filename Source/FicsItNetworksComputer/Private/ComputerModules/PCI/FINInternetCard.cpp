#include "ComputerModules/PCI/FINInternetCard.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

FFINInternetCardHttpRequestFuture::FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InRequest) {
	Request = InRequest;
}

void FFINInternetCardHttpRequestFuture::Execute() {
	if (!Request.IsValid()) return;
	// FIN-1.2-PORT: Copy the response into the shared snapshot inside the completion delegate
	// (game thread, HttpManager tick). This way the Lua thread (IsDone/GetOutput) NEVER reads the
	// live FHttpResponse -> no cross-thread race on the response payload -> no
	// heap-corruption crash (0xc0000374) that UE5.6 would otherwise trigger here.
	// The delegate is guaranteed to fire on success AND failure, as long as it is bound (IHttpRequest.h).
	TSharedPtr<FFINInternetCardHttpResult> Res = Result;
	Request->OnProcessRequestComplete().BindLambda(
		[Res](FHttpRequestPtr, FHttpResponsePtr Response, bool bSucceeded) {
			if (!Res.IsValid()) return;
			if (bSucceeded && Response.IsValid()) {
				Res->Code = Response->GetResponseCode();
				Res->Content = Response->GetContentAsString();
				for (const FString& Header : Response->GetAllHeaders()) {
					FString Name;
					FString Value;
					if (Header.Split(TEXT(": "), &Name, &Value)) {
						Res->Headers.Add(Name);
						Res->Headers.Add(Value);
					}
				}
			}
			Res->bComplete = true;
		});
	// If ProcessRequest never starts, the delegate does not fire -> otherwise await()
	// would hang forever. In that case mark it as done immediately (with an empty response).
	if (!Request->ProcessRequest()) {
		Res->bComplete = true;
	}
}

bool FFINInternetCardHttpRequestFuture::IsDone() const {
	// Only read the shared snapshot - no access to the live FHttpRequest from the Lua thread.
	if (!Result.IsValid()) return true;
	return Result->bComplete;
}

TArray<FFIRAnyValue> FFINInternetCardHttpRequestFuture::GetOutput() const {
	TArray<FFIRAnyValue> Response;
	if (!Result.IsValid()) return Response;
	Response.Add((FIRInt)Result->Code);
	Response.Add(Result->Content);
	TArray<FIRAny> Headers;
	for (const FString& Header : Result->Headers) {
		Headers.Add(Header);
	}
	Response.Add(Headers);
	return Response;
}

FFINInternetCardHttpRequestFuture AFINInternetCard::netFunc_request(const FString& InURL, const FString& InMethod, const FString& InData, TArray<FFIRAnyValue> varargs) {
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(InURL);
	Request->SetVerb(InMethod);
	Request->SetContentAsString(InData);
	for (int i = 0; i+1 < varargs.Num(); i += 2) {
		FString Name = varargs[i].GetString();
		FString Value = varargs[i+1].GetString();
		Request->SetHeader(Name, Value);
	}
	if (!InData.IsEmpty() && Request->GetHeader(TEXT("Content-Type")).IsEmpty()) {
		throw FFIRException("Req-Payload given without Content-Type");
	}
	return FFINInternetCardHttpRequestFuture(Request);
}
