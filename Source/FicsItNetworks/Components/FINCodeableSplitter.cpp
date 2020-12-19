#include "FINCodeableSplitter.h"
#include "FicsItNetworksModule.h"
#include "Computer/FINComputerSubsystem.h"

AFINCodeableSplitter::AFINCodeableSplitter() {
	RootComponent->SetMobility(EComponentMobility::Movable);
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->SetMobility(EComponentMobility::Movable);

	Input1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Input1");
	Input1->SetDirection(EFactoryConnectionDirection::FCD_INPUT);
	Input1->SetupAttachment(RootComponent);
	Input1->SetMobility(EComponentMobility::Movable);
	Output1 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output1");
	Output1->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output1->SetupAttachment(RootComponent);
	Output1->SetMobility(EComponentMobility::Movable);
	Output2 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output2");
	Output2->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output2->SetupAttachment(RootComponent);
	Output2->SetMobility(EComponentMobility::Movable);
	Output3 = CreateDefaultSubobject<UFGFactoryConnectionComponent>("Output3");
	Output3->SetDirection(EFactoryConnectionDirection::FCD_OUTPUT);
	Output3->SetupAttachment(RootComponent);
	Output3->SetMobility(EComponentMobility::Movable);

	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);
}

AFINCodeableSplitter::~AFINCodeableSplitter() {}

void AFINCodeableSplitter::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
#if !WITH_EDITOR
	if (HasAuthority() && AFINComputerSubsystem::GetComputerSubsystem(this)->Version < EFINCustomVersion::FINCodeableSplitterAttachmentFixes) {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Old Splitter found. Try to apply construction update fixes... '%s'"), *this->GetName());
		Input1->Rename(TEXT("InputConnector"));
	}
#endif
}

void AFINCodeableSplitter::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority() && AFINComputerSubsystem::GetComputerSubsystem(this)->Version < EFINCustomVersion::FINCodeableSplitterAttachmentFixes) {
		UE_LOG(LogFicsItNetworks, Log, TEXT("Old Splitter found. Try to apply beginplay update fixes... '%s'"), *this->GetName());
		Input1->Rename(TEXT("Input1"));
		RootComponent->AddRelativeRotation(FRotator(0,-90.0f,0));
		UFGFactoryConnectionComponent* NewOutput2 = Output1->GetConnection();
		UFGFactoryConnectionComponent* NewOutput1 = Output2->GetConnection();
		Output1->ClearConnection();
		Output2->ClearConnection();
		if (NewOutput1) Output1->SetConnection(NewOutput1);
		if (NewOutput2) Output2->SetConnection(NewOutput2);
	}
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
	TArray<FInventoryItem>& outputQueue = GetOutput(connection);
	if (outputQueue.Num() > 0) {
		out_item = outputQueue[0];
		outputQueue.RemoveAt(0);
		return true;
	}
	return false;
}


void AFINCodeableSplitter::GetDismantleRefund_Implementation(TArray<FInventoryStack>& out_refund) const {
	Super::GetDismantleRefund_Implementation(out_refund);

	out_refund.Append(InputQueue);
	out_refund.Append(OutputQueue1);
	out_refund.Append(OutputQueue2);
	out_refund.Append(OutputQueue3);
}

void AFINCodeableSplitter::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(AFINComputerSubsystem::GetComputerSubsystem(this));
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
		return OutputQueue2;
	case 1:
		return OutputQueue1;
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
