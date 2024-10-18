#include "ComputerModules/PCI/FINInternetCard.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

FFINInternetCardHttpRequestFuture::FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InRequest) {
	Request = InRequest;
}

void FFINInternetCardHttpRequestFuture::Execute() {
	if (Request.IsValid()) Request->ProcessRequest();
}

bool FFINInternetCardHttpRequestFuture::IsDone() const {
	return !Request.IsValid() || !(Request->GetStatus() == EHttpRequestStatus::Processing || Request->GetStatus() == EHttpRequestStatus::NotStarted);
}

TArray<FFIRAnyValue> FFINInternetCardHttpRequestFuture::GetOutput() const {
	TArray<FFIRAnyValue> Response;
	if (!Request.IsValid() || !Request->GetResponse().IsValid()) return Response;
	Response.Add((FIRInt)Request->GetResponse()->GetResponseCode());
	Response.Add(Request->GetResponse()->GetContentAsString());
	TArray<FIRAny> Headers;
	for (FString Header : Request->GetResponse()->GetAllHeaders()) {
		FString Name;
		FString Value;
		Header.Split(": ", &Name, &Value);
		Headers.Add(Name);
		Headers.Add(Value);
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
	if (!InData.IsEmpty() && Request->GetHeader(L"Content-Type").IsEmpty()) {
		throw FFIRException("Req-Payload given without Content-Type");
	}
	return FFINInternetCardHttpRequestFuture(Request);
}
