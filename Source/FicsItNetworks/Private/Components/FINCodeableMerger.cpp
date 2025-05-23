﻿#include "Components/FINCodeableMerger.h"

#include "FINAdvancedNetworkConnectionComponent.h"

AFINCodeableMerger::AFINCodeableMerger() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetMobility(EComponentMobility::Static);
	NetworkConnector->SetIsReplicated(true);

	Input2 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Input2");
	Input2->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
	Input2->SetupAttachment(RootComponent);
	Input2->SetMobility(EComponentMobility::Static);
	Input1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Input1");
	Input1->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
	Input1->SetupAttachment(RootComponent);
	Input1->SetMobility(EComponentMobility::Static);
	Input3 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Input3");
	Input3->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
	Input3->SetupAttachment(RootComponent);
	Input3->SetMobility(EComponentMobility::Static);
	Output1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output1");
	Output1->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output1->SetupAttachment(RootComponent);
	Output1->SetMobility(EComponentMobility::Static);

	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);
}

AFINCodeableMerger::~AFINCodeableMerger() {}

void AFINCodeableMerger::GetDismantleRefund_Implementation(TArray<FInventoryStack>& out_refund, bool noBuildCostEnabled) const {
	Super::GetDismantleRefund_Implementation(out_refund, noBuildCostEnabled);

	FScopeLock lock(const_cast<FCriticalSection*>(&Mutex));
	//TArray<FInventoryItem> items = InputQueue;
	out_refund.Append(OutputQueue);
	out_refund.Append(InputQueue1);
	out_refund.Append(InputQueue2);
	out_refund.Append(InputQueue3);

	for (uint64 i = 0; i < out_refund.Num(); ++i) {
		if (!out_refund[i].HasItems() || out_refund[i].Item.IsValid()) {
			out_refund.RemoveAt(i);
			--i;
		}
	}
}

UObject* AFINCodeableMerger::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
}

void AFINCodeableMerger::BeginPlay() {
	Super::BeginPlay();

	FScopeLock lock(&Mutex);
	for (TArray<FInventoryItem>* list : { &InputQueue1, &InputQueue2, &InputQueue3, &OutputQueue }) {
		for (size_t i = 0; i < list->Num(); ++i) {
			if (!(*list)[i].IsValid()) {
				list->RemoveAt(i);
				--i;
			}
		}
	}
}

void AFINCodeableMerger::TickInput(UFGFactoryConnectionComponent* Connector, int InputId) {
	TArray<FInventoryItem>& InputQueue = GetInput(Connector);
	if (InputQueue.Num() < 2) {
		FInventoryItem item;
		float offset;
		if (Connector->Factory_GrabOutput(item, offset)) {
			FScopeLock lock(&Mutex);
			InputQueue.Add(item);
			netSig_ItemRequest(InputId, item);
		}
	}
}

void AFINCodeableMerger::Factory_Tick(float dt) {
	Super::Factory_Tick(dt);

	if (HasAuthority()) {
		TickInput(Input1, 1);
		TickInput(Input2, 0);
		TickInput(Input3, 2);
	}
}

bool AFINCodeableMerger::Factory_PeekOutput_Implementation(const UFGFactoryConnectionComponent* connection, TArray<FInventoryItem>& out_items, TSubclassOf<UFGItemDescriptor> type) const {
	FScopeLock lock(const_cast<FCriticalSection*>(&Mutex));
	if (OutputQueue.Num() > 0 || OutputQueue[0].IsValid()) {
		out_items.Add(OutputQueue[0]);
		return true;
	}
	return false;
}

bool AFINCodeableMerger::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type) {
	FScopeLock lock(&Mutex);
	if (OutputQueue.Num() > 0) {
		out_item = OutputQueue[0];
		if (out_item.IsValid()) {
			OutputQueue.RemoveAt(0);
			netSig_ItemOutputted(out_item);
			return true;
		}
	}
	return false;
}


bool AFINCodeableMerger::netFunc_transferItem(int input) {
	TArray<FInventoryItem>& InputQueue = GetInput(input);

	FScopeLock lock(&Mutex);
	if (OutputQueue.Num() < 2 &&  InputQueue.Num() > 0) {
		FInventoryItem item = InputQueue[0];
		InputQueue.RemoveAt(0);
		OutputQueue.Add(item);
		return true;
	}
	return false;
}

FInventoryItem AFINCodeableMerger::netFunc_getInput(int input) {
	TArray<FInventoryItem>& InputQueue = GetInput(input);

	FScopeLock lock(&Mutex);
	if (InputQueue.Num() > 0) {
		return InputQueue[0];
	}
	return FInventoryItem();
}

bool AFINCodeableMerger::netPropGet_canOutput() {
	FScopeLock lock(&Mutex);
	return OutputQueue.Num() < 2;
}

void AFINCodeableMerger::netSig_ItemRequest_Implementation(int input, FInventoryItem item) {}
void AFINCodeableMerger::netSig_ItemOutputted_Implementation(FInventoryItem item) {}

TArray<FInventoryItem>& AFINCodeableMerger::GetInput(int output) {
	output = (output < 0) ? 0 : ((output > 2) ? 2 : output);
	switch (output) {
	case 0:
		return InputQueue2;
	case 1:
		return InputQueue1;
	default:
		return InputQueue3;
	}
}

TArray<FInventoryItem>& AFINCodeableMerger::GetInput(UFGFactoryConnectionComponent* connection) {
	return const_cast<TArray<FInventoryItem>&>(GetInput((const UFGFactoryConnectionComponent*) connection));
}

const TArray<FInventoryItem>& AFINCodeableMerger::GetInput(const UFGFactoryConnectionComponent* connection) const {
	if (connection == Input1) {
		return InputQueue1;
	} else if (connection == Input2) {
		return InputQueue2;
	} else {
		return InputQueue3;
	}
}
