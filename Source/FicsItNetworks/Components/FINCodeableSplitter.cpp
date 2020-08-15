#include "FINCodeableSplitter.h"

AFINCodeableSplitter::AFINCodeableSplitter() {
	RootComponent->SetMobility(EComponentMobility::Static);
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetMobility(EComponentMobility::Static);

	InputConnector = CreateDefaultSubobject<UFGFactoryConnectionComponent>("InputConnector");
	InputConnector->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
	InputConnector->SetupAttachment(RootComponent);
	InputConnector->SetMobility(EComponentMobility::Static);
	Output1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output1");
	Output1->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output1->SetupAttachment(RootComponent);
	Output1->SetMobility(EComponentMobility::Static);
	Output2 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output2");
	Output2->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output2->SetupAttachment(RootComponent);
	Output2->SetMobility(EComponentMobility::Static);
	Output3 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output3");
	Output3->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output3->SetupAttachment(RootComponent);
	Output3->SetMobility(EComponentMobility::Static);

	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);
}

AFINCodeableSplitter::~AFINCodeableSplitter() {}

void AFINCodeableSplitter::GetDismantleRefund_Implementation(TArray<FInventoryStack>& out_refund) const {
	Super::GetDismantleRefund_Implementation(out_refund);

	//TArray<FInventoryItem> items = InputQueue;
	out_refund.Append(InputQueue);
	out_refund.Append(OutputQueue1);
	out_refund.Append(OutputQueue2);
	out_refund.Append(OutputQueue3);
}

void AFINCodeableSplitter::AddListener_Implementation(FFINNetworkTrace listener) {
	SignalListeners.Add(listener);
}

void AFINCodeableSplitter::RemoveListener_Implementation(FFINNetworkTrace listener) {
	SignalListeners.Remove(listener);
}

TSet<FFINNetworkTrace> AFINCodeableSplitter::GetListeners_Implementation() {
	return SignalListeners;
}

UObject* AFINCodeableSplitter::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
}

void AFINCodeableSplitter::Factory_Tick(float dt) {
	Super::Factory_Tick(dt);
	if (InputQueue.Num() < 2) {
		FInventoryItem item;
		float offset;
		if (InputConnector->Factory_GrabOutput(item, offset)) {
			InputQueue.Add(item);
			netSig_ItemRequest(item);
		}
	}
}

bool AFINCodeableSplitter::Factory_PeekOutput_Implementation(const UFGFactoryConnectionComponent* connection, TArray<FInventoryItem>& out_items, TSubclassOf<UFGItemDescriptor> type) const {
	const TArray<FInventoryItem>& outputQueue = GetOutput(connection);
	if (outputQueue.Num() > 0) {
		out_items.Add(outputQueue[0]);
		return true;
	}
	return false;
}

bool AFINCodeableSplitter::Factory_GrabOutput_Implementation(UFGFactoryConnectionComponent* connection, FInventoryItem& out_item, float& out_OffsetBeyond, TSubclassOf<UFGItemDescriptor> type) {
	TArray<FInventoryItem>& outputQueue = GetOutput(connection);
	if (outputQueue.Num() > 0) {
		out_item = outputQueue[0];
		outputQueue.RemoveAt(0);
		return true;
	}
	return false;
}

bool AFINCodeableSplitter::netFunc_transferItem(int output) {
	TArray<FInventoryItem>& outputQueue = GetOutput(output);

	if (outputQueue.Num() < 2 &&  InputQueue.Num() > 0) {
		FInventoryItem item = InputQueue[0];
		InputQueue.RemoveAt(0);
		outputQueue.Add(item);
		return true;
	}
	return false;
}

FInventoryItem AFINCodeableSplitter::netFunc_getInput() {
	if (InputQueue.Num() > 0) {
		return InputQueue[0];
	}
	return FInventoryItem();
}

bool AFINCodeableSplitter::netFunc_canOutput(int output) {
	TArray<FInventoryItem>& outputQueue = GetOutput(output);
	return outputQueue.Num() < 2;
}

void AFINCodeableSplitter::netSig_ItemRequest_Implementation(const FInventoryItem& item) {}

TArray<FInventoryItem>& AFINCodeableSplitter::GetOutput(int output) {
	output = (output < 0) ? 0 : ((output > 2) ? 2 : output);
	switch (output) {
	case 0:
		return OutputQueue1;
	case 1:
		return OutputQueue2;
	default:
		return OutputQueue3;
	}
}

TArray<FInventoryItem>& AFINCodeableSplitter::GetOutput(UFGFactoryConnectionComponent* connection) {
	return const_cast<TArray<FInventoryItem>&>(GetOutput((const UFGFactoryConnectionComponent*) connection));
}

const TArray<FInventoryItem>& AFINCodeableSplitter::GetOutput(const UFGFactoryConnectionComponent* connection) const {
	if (connection == Output1) {
		return OutputQueue1;
	} else if (connection == Output2) {
		return OutputQueue2;
	} else {
		return OutputQueue3;
	}
}
