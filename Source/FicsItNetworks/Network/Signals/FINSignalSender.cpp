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
	auto data = FMemory::Malloc(Stack.CurrentNativeFunction->ParmsSize);
	Stack.CurrentNativeFunction->InitializeStruct(data);
	for (auto p = TFieldIterator<UProperty>(Stack.CurrentNativeFunction); p; ++p) {
		auto dp = p->ContainerPtrToValuePtr<void>(data);
		if (Stack.Code) {
			std::invoke(&FFrame::Step, Stack, Context, dp);
		} else {
			Stack.StepExplicitProperty(dp, *p);
		}
	}

	FFINDynamicStructHolder FuncData(FFINFuncParameterList::StaticStruct(), new FFINFuncParameterList(Stack.CurrentNativeFunction, data));
	// create signal instance
	TSharedPtr<FFINSignal> sig = MakeShared<FFINStructSignal>(signalName, FuncData);

	TSet<FFINNetworkTrace> listeners = IFINSignalSender::Execute_GetListeners(Context);

	for (auto listenerTrace : listeners) {
		// TODO: Make sure this cast works and if the underlying object is the reason, remove it
		IFINSignalListener* obj = Cast<IFINSignalListener>(*listenerTrace);
		if (obj) obj->HandleSignal(sig, listenerTrace.getTrace().reverse() / IFINSignalSender::Execute_GetSignalSenderOverride(Context));
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
