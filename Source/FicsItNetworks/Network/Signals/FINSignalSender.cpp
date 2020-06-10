#include "FINSignalSender.h"

#include "Stack.h"
#include "FicsItKernel/Network/Signal.h"

void execRecieveSignal(UObject* Context, FFrame& Stack, RESULT_DECL) {
	// get signal name
	FString signalName = Stack.CurrentNativeFunction->GetName();
	signalName.RemoveFromStart("netSig_");

	// allocate signal data storage and copy data
	auto data = malloc(Stack.CurrentNativeFunction->ParmsSize);
	memset(data, 0, Stack.CurrentNativeFunction->ParmsSize);
	for (auto p = TFieldIterator<UProperty>(Stack.CurrentNativeFunction); p; ++p) {
		auto dp = p->ContainerPtrToValuePtr<void>(data);
		if (Stack.Code) {
			Stack.Step(Context, dp);
		} else {
			Stack.StepExplicitProperty(dp, *p);
		}
	}

	// create signal instance
	auto sig = std::shared_ptr<FicsItKernel::Network::Signal>((FicsItKernel::Network::Signal*) new UFINSignalUtility::BPSignal{TCHAR_TO_UTF8(*signalName), Stack.CurrentNativeFunction, data});

	auto listeners = IFINSignalSender::Execute_GetListeners(Context);

	for (auto listenerTrace : listeners) {
		// TODO: Make sure this cast works and if the underlying object is the reason, remove it
		UObject* obj = *listenerTrace;
		if (IsValid(obj) && obj->Implements<UFINSignalListener>()) IFINSignalListener::Execute_HandleSignal(obj, FFINSignal(sig), listenerTrace / IFINSignalSender::Execute_GetSignalSenderOverride(Context));
	}

	P_FINISH;
}

void UFINSignalUtility::SetupSender(UClass* signalSender) {
	for (auto func = TFieldIterator<UFunction>(signalSender); func; ++func) {
		auto funcName = func->GetName();
		if (!funcName.RemoveFromStart("netSig_")) continue;
		func->SetNativeFunc(&execRecieveSignal);
		func->FunctionFlags |= FUNC_Native;
	}
}

FicsItKernel::Network::SignalTypeRegisterer regBPSignal("BPSignal", [](FArchive& Ar) {
	return std::shared_ptr<FicsItKernel::Network::Signal>(new UFINSignalUtility::BPSignal(Ar));
});

UFINSignalUtility::BPSignal::BPSignal(std::string name, UFunction* func, void* data) : Signal(name), func(func), data(data) {}

UFINSignalUtility::BPSignal::BPSignal(FArchive& Ar) : Signal("") {
	Serialize(Ar);
}

int UFINSignalUtility::BPSignal::operator>>(FicsItKernel::Network::SignalReader& reader) const {
	int count = 0;
	for (auto p = TFieldIterator<UProperty>(func); p; ++p) {
		++count;
		auto dp = p->ContainerPtrToValuePtr<void>(data);
		if (auto vp = Cast<UStrProperty>(*p)) reader << TCHAR_TO_UTF8(*vp->GetPropertyValue_InContainer(data));
		else if (auto vp = Cast<UIntProperty>(*p)) reader << vp->GetPropertyValue_InContainer(data);
		else if (auto vp = Cast<UInt64Property>(*p)) reader << (int)vp->GetPropertyValue_InContainer(data);
		else if (auto vp = Cast<UFloatProperty>(*p)) reader << vp->GetPropertyValue_InContainer(data);
		else if (auto vp = Cast<UBoolProperty>(*p)) reader << vp->GetPropertyValue_InContainer(data);
		else if (auto vp = Cast<UObjectProperty>(*p)) reader << vp->GetPropertyValue_InContainer(data);
		//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
		// TODO: Add Array support
		else --count;
	}
	return count;
}

std::string UFINSignalUtility::BPSignal::getTypeName() {
	return "BPSignal";
}

void UFINSignalUtility::BPSignal::Serialize(FArchive& Ar) {
	FString n = name.c_str();
	Ar << n;
	name = std::string(TCHAR_TO_UTF8(*n), n.Len());
	
	Ar << func;
	
	if (Ar.IsLoading()) {
		data = malloc(func->ParmsSize);
		memset(data, 0, func->ParmsSize);
	}
	
	for (auto p = TFieldIterator<UProperty>(func); p; ++p) {
		if (Ar.IsLoading()) p->InitializeValue_InContainer(data);
		auto dp = p->ContainerPtrToValuePtr<void>(data);
		if (auto vp = Cast<UStrProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(data);
		else if (auto vp = Cast<UIntProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(data);
		else if (auto vp = Cast<UInt64Property>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(data);
		else if (auto vp = Cast<UFloatProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(data);
		else if (auto vp = Cast<UBoolProperty>(*p)) {
			bool b = vp->GetPropertyValue_InContainer(data);
			Ar << b;
			vp->SetPropertyValue_InContainer(data, b);
		} else if (auto vp = Cast<UObjectProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(data);
		//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
		// TODO: Add Array support
	}
}

