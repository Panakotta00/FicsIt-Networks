#include "Script/Library/FIVSNode_SignalEvent.h"

#include "Script/FIVSScriptContext.h"
#include "FIVSUtils.h"
#include "Editor/FIVSEdObjectSelection.h"
#include "Editor/FIVSEdSignalSelection.h"

/*
UFIVSNode_SignalEvent::UFIVSNode_SignalEvent() {
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Exec"), FText::FromString(TEXT("Exec")));
}

void UFIVSNode_SignalEvent::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
		for (UFINSignal* InSignal : Class.Value->GetSignals(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_SignalEvent::StaticClass();
			Action.Title = FText::FromString(TEXT("Signal Event (") + InSignal->GetDisplayName().ToString() + TEXT(")"));
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(TEXT("General|Events"));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_OUTPUT));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass()))));
			for (UFINProperty* Property : InSignal->GetParameters()) {
				Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(Property)));
			}
			Action.OnExecute.BindLambda([InSignal](UFIVSNode* Node) {
				Cast<UFIVSNode_SignalEvent>(Node)->Signal = InSignal;
			});
			Actions.Add(Action);
		}
	}
}

void UFIVSNode_SignalEvent::SerializeNodeProperties(FFIVSNodeProperties& Properties) const {
	if (Signal) Properties.Properties.Add(TEXT("Signal"), Signal->GetPathName());
	Properties.Properties.Add(TEXT("Sender"), UFIVSUtils::NetworkTraceToString(Sender));
}

void UFIVSNode_SignalEvent::DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {
	if (Properties.Properties.Contains(TEXT("Signal"))) Signal = Cast<UFINSignal>(FSoftObjectPath(Properties.Properties[TEXT("Signal")]).TryLoad());
	if (Properties.Properties.Contains(TEXT("Sender"))) Sender = UFIVSUtils::StringToNetworkTrace(Properties.Properties[TEXT("Sender")]);
}

TSharedPtr<SWidget> UFIVSNode_SignalEvent::CreateDetailsWidget(TScriptInterface<IFIVSScriptContext_Interface> Context) {
	TArray<FFINNetworkTrace> Objs;
	Context->GetRelevantObjects_Implementation(Objs);
	return SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.VAlign(VAlign_Top)[
		SNew(SFIVSEdSignalSelection)
		.InitSelection(Signal)
		.OnSelectionChanged_Lambda([this](UFINSignal* InSignal) {
			SetSignal(Signal);
		})
	]
	+SVerticalBox::Slot()
	.VAlign(VAlign_Top)[
		SNew(SFIVSEdObjectSelection, Objs)
		.InitSelection(Sender)
		.OnSelectionChanged_Lambda([this](FFINNetworkTrace Trace) {
			Sender = Trace;
		})
	];
}

TArray<UFIVSPin*> UFIVSNode_SignalEvent::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return Super::PreExecPin(ExecPin, Context);
}

TArray<UFIVSPin*> UFIVSNode_SignalEvent::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return {ExecOut};
}

void UFIVSNode_SignalEvent::SetSignal(UFINSignal* InSignal) {
	Signal = InSignal;

	if (Signal)	DisplayName = FText::FromString(TEXT("Signal Event (") + Signal->GetDisplayName().ToString() + TEXT(")"));
	else DisplayName = FText::FromString(TEXT("Signal Event"));

	DeletePin(SenderOut);
	DeletePins(Parameters);

	if (Signal) {
		SenderOut = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Sender"), FText::FromString(TEXT("Sender")), FFIVSPinDataType(FIN_TRACE, Cast<UFINClass>(Signal->GetOuter())));
		for (UFINProperty* Property : Signal->GetParameters()) {
			Parameters.Add(CreatePin(FIVS_PIN_DATA_OUTPUT, Property->GetInternalName(), Property->GetDisplayName(), FFIVSPinDataType(Property)));
		}
	}
}
*/