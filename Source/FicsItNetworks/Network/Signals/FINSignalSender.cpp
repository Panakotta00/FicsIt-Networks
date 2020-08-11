#include "FINSignalSender.h"


#include "FINSignalListener.h"
#include "FINStructSignal.h"
#include "FINStructSignal.h"
#include "Network/FINDynamicStructHolder.h"
#include "Network/FINFuncParameterList.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

#pragma optimize("", off)
void execRecieveSignal(UObject* Context, FFrame& Stack, RESULT_DECL) {
	// get signal name
	FString signalName = Stack.CurrentNativeFunction->GetName();
	signalName.RemoveFromStart("netSig_");

	// allocate signal data storage and copy data
	void* data = FMemory::Malloc(Stack.CurrentNativeFunction->PropertiesSize);
	FMemory::Memzero(((uint8*)data) + Stack.CurrentNativeFunction->ParmsSize, Stack.CurrentNativeFunction->PropertiesSize - Stack.CurrentNativeFunction->ParmsSize);
	Stack.CurrentNativeFunction->InitializeStruct(data);
	for (UProperty* LocalProp = Stack.CurrentNativeFunction->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
		LocalProp->InitializeValue_InContainer(data);
	}

	for (auto p = TFieldIterator<UProperty>(Stack.CurrentNativeFunction); p; ++p) {
		auto dp = p->ContainerPtrToValuePtr<void>(data);
		if (Stack.Code) {
			std::invoke(&FFrame::Step, Stack, Context, dp);
		} else {
			Stack.StepExplicitProperty(dp, *p);
		}
	}

	// create signal instance
	TFINDynamicStruct<FFINSignal> sig =  FFINStructSignal(signalName, FFINFuncParameterList(Stack.CurrentNativeFunction, data));

	TSet<FFINNetworkTrace> listeners = IFINSignalSender::Execute_GetListeners(Context);

	for (auto listenerTrace : listeners) {
		// TODO: Make sure this cast works and if the underlying object is the reason, remove it
		IFINSignalListener* obj = Cast<IFINSignalListener>(*listenerTrace);
		if (obj) obj->HandleSignal(sig, listenerTrace.Reverse() / IFINSignalSender::Execute_GetSignalSenderOverride(Context));
	}

	P_FINISH;
}
#pragma optimize("", on)

void UFINSignalUtility::SetupSender(UClass* signalSender) {
	for (auto func = TFieldIterator<UFunction>(signalSender); func; ++func) {
		auto funcName = func->GetName();
		if (!funcName.RemoveFromStart("netSig_")) continue;
		func->SetNativeFunc(&execRecieveSignal);
		func->FunctionFlags |= FUNC_Native;
	}
}
