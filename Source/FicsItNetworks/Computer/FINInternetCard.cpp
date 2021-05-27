#include "FINInternetCard.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

FFINInternetCardHttpRequestFuture::FFINInternetCardHttpRequestFuture(TSharedRef<IHttpRequest> InRequest) {
	Request = InRequest;
}

void FFINInternetCardHttpRequestFuture::Execute() {
	if (Request.IsValid()) Request->ProcessRequest();
}

bool FFINInternetCardHttpRequestFuture::IsDone() const {
	return Request.IsValid() && !(Request->GetStatus() == EHttpRequestStatus::Processing || Request->GetStatus() == EHttpRequestStatus::NotStarted);
}

TArray<FFINAnyNetworkValue> FFINInternetCardHttpRequestFuture::GetOutput() const {
	TArray<FFINAnyNetworkValue> Response;
	if (!Request.IsValid() || !Request->GetResponse().IsValid()) return Response;
	Response.Add(Request->GetResponse()->GetContentAsString());
	return Response;
}


FFINInternetCardHttpRequestFuture AFINInternetCard::netFunc_request(const FString& InURL, const FString& InMethod, const FString& InData, const TArray<FFINAnyNetworkValue>& varargs) {
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(InURL);
	Request->SetVerb(InMethod);
	Request->SetContentAsString(InData);
	for (int i = 0; i+1 < varargs.Num(); i += 2) {
		FString Name = varargs[i].GetString();
		FString Value = varargs[i+1].GetString();
		Request->SetHeader(Name, Value);
	}
	return FFINInternetCardHttpRequestFuture(Request);
}
