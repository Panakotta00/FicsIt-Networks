#include "ComputerModules/PCI/FINInternetCard.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

FFINInternetCardHttpRequestFuture::FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InRequest) {
	Request = InRequest;
}

void FFINInternetCardHttpRequestFuture::Execute() {
	if (!Request.IsValid()) return;
	// FIN-1.2-PORT: Antwort im Completion-Delegate (Game-Thread, HttpManager-Tick) in den
	// geteilten Snapshot kopieren. Damit liest der Lua-Thread (IsDone/GetOutput) NIE das
	// lebende FHttpResponse -> kein Cross-Thread-Race auf dem Response-Payload -> kein
	// Heap-Corruption-Crash (0xc0000374), den UE5.6 hier sonst ausloest.
	// Der Delegate feuert garantiert bei Erfolg UND Fehlschlag, sofern gebunden (IHttpRequest.h).
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
	// Wenn ProcessRequest gar nicht erst startet, feuert der Delegate nicht -> sonst haengt
	// await() ewig. Dann sofort als fertig (mit leerer Antwort) markieren.
	if (!Request->ProcessRequest()) {
		Res->bComplete = true;
	}
}

bool FFINInternetCardHttpRequestFuture::IsDone() const {
	// Nur den geteilten Snapshot lesen - kein Zugriff auf das Live-FHttpRequest vom Lua-Thread.
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
