#include "Components/FINCodeableSplitter.h"
#include "Computer/FINComputerSubsystem.h"

AFINCodeableSplitter::AFINCodeableSplitter() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetMobility(EComponentMobility::Movable);
	NetworkConnector->SetIsReplicated(true);

	Output2 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output2");
	Output2->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output2->SetupAttachment(RootComponent);
	Output2->SetMobility(EComponentMobility::Movable);
	Output1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output1");
	Output1->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output1->SetupAttachment(RootComponent);
	Output1->SetMobility(EComponentMobility::Movable);
	Output3 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output3");
	Output3->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output3->SetupAttachment(RootComponent);
	Output3->SetMobility(EComponentMobility::Movable);
	Input1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Input1");
	Input1->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
	Input1->SetupAttachment(RootComponent);
	Input1->SetMobility(EComponentMobility::Movable);
	
	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);
}

AFINCodeableSplitter::~AFINCodeableSplitter() {}

void AFINCodeableSplitter::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
}

void AFINCodeableSplitter::BeginPlay() {
	Super::BeginPlay();
}

void AFINCodeableSplitter::Factory_Tick(float dt) {
	Super::Factory_Tick(dt);
	if (HasAuthority() && InputQueue.Num() < 2) {
		FInventoryItem item;
		float offset;
		if (Input1->Factory_GrabOutput(item, offset)) {
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
	int32 Index = 0;
	TArray<FInventoryItem>& outputQueue = GetOutput(connection, &Index);
	if (outputQueue.Num() > 0) {
		out_item = outputQueue[0];
		outputQueue.RemoveAt(0);
		netSig_ItemOutputted(Index, out_item);
		return true;
	}
	return false;
}


void AFINCodeableSplitter::GetDismantleRefund_Implementation(TArray<FInventoryStack>& out_refund, bool noBuildCostEnabled) const {
	Super::GetDismantleRefund_Implementation(out_refund, noBuildCostEnabled);

	out_refund.Append(InputQueue);
	out_refund.Append(OutputQueue1);
	out_refund.Append(OutputQueue2);
	out_refund.Append(OutputQueue3);
}

void AFINCodeableSplitter::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(AFINComputerSubsystem::GetComputerSubsystem(this));
}

UObject* AFINCodeableSplitter::GetSignalSenderOverride_Implementation() {
	return NetworkConnector;
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

void AFINCodeableSplitter::netSig_ItemRequest_Implementation(FInventoryItem item) {}
void AFINCodeableSplitter::netSig_ItemOutputted_Implementation(int output, FInventoryItem item) {}

TArray<FInventoryItem>& AFINCodeableSplitter::GetOutput(int output) {
	output = (output < 0) ? 0 : ((output > 2) ? 2 : output);
	switch (output) {
	case 0:
		return OutputQueue2;
	case 1:
		return OutputQueue1;
	default:
		return OutputQueue3;
	}
}

TArray<FInventoryItem>& AFINCodeableSplitter::GetOutput(UFGFactoryConnectionComponent* connection, int32* Index) {
	return const_cast<TArray<FInventoryItem>&>(GetOutput((const UFGFactoryConnectionComponent*) connection, Index));
}

const TArray<FInventoryItem>& AFINCodeableSplitter::GetOutput(const UFGFactoryConnectionComponent* connection, int32* Index) const {
	if (connection == Output1) {
		if (Index) *Index = 1;
		return OutputQueue1;
	}
	if (connection == Output2) {
		if (Index) *Index = 0;
		return OutputQueue2;
	}
	if (Index) *Index = 2;
	return OutputQueue3;
}
