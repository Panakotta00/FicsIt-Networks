#include "Script/Library/FIVSNode_SignalEvent.h"

#include "FGHUD.h"
#include "FGPlayerController.h"
#include "FINUMGWidget.h"
#include "Script/FIVSScriptContext.h"
#include "FIVSUtils.h"
#include "Editor/FIVSEdObjectSelection.h"
#include "Editor/FIVSEdSignalSelection.h"

UFIVSNode_SignalEvent::UFIVSNode_SignalEvent() {
	ExecOut = CreateDefaultPin(FIVS_PIN_EXEC_OUTPUT, TEXT("Exec"), FText::FromString(TEXT("Exec")));
}

void UFIVSNode_SignalEvent::GetNodeActions(TArray<FFIVSNodeAction>& Actions) const {
	for (TPair<UClass*, UFIRClass*> Class : FFicsItReflectionModule::Get().GetClasses()) {
		for (UFIRSignal* InSignal : Class.Value->GetSignals(false)) {
			FFIVSNodeAction Action;
			Action.NodeType = UFIVSNode_SignalEvent::StaticClass();
			Action.Title = FText::FromString(TEXT("Signal Event (") + InSignal->GetDisplayName().ToString() + TEXT(")"));
			Action.SearchableText = Action.Title;
			Action.Category = FText::FromString(TEXT("General|Events"));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_EXEC_OUTPUT));
			Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(FIR_TRACE, FFicsItReflectionModule::Get().FindClass(UObject::StaticClass()))));
			for (UFIRProperty* Property : InSignal->GetParameters()) {
				Action.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFIVSPinDataType(Property)));
			}
			Action.OnExecute.BindLambda([InSignal](UFIVSNode* Node) {
				Cast<UFIVSNode_SignalEvent>(Node)->Signal = InSignal;
			});
			Actions.Add(Action);
		}
	}
}

void UFIVSNode_SignalEvent::SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const {
	if (Signal) Properties->SetStringField(TEXT("Signal"), Signal->GetPathName());
	Properties->SetStringField(TEXT("Sender"), UFIVSUtils::NetworkTraceToString(Sender));
}

void UFIVSNode_SignalEvent::DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) {
	if (Properties->HasField(TEXT("Sender"))) Sender = UFIVSUtils::StringToNetworkTrace(Properties->GetStringField(TEXT("Sender")));
	if (Properties->HasField(TEXT("Signal"))) SetSignal(Cast<UFIRSignal>(FSoftObjectPath(Properties->GetStringField(TEXT("Signal"))).TryLoad()));
}

TSharedPtr<SWidget> UFIVSNode_SignalEvent::CreateDetailsWidget(TScriptInterface<IFIVSScriptContext_Interface> Context) {
	TArray<FFIRTrace> Objs;
	Context->GetRelevantObjects_Implementation(Objs);
	return SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.VAlign(VAlign_Top)[
		SNew(SFIVSEdSignalSelection)
		.InitSelection(Signal)
		.OnSelectionChanged_Lambda([this](UFIRSignal* InSignal) {
			SetSignal(Signal);
		})
	]
	+SVerticalBox::Slot()
	.VAlign(VAlign_Top)[
		SNew(SFIVSEdObjectSelection, Objs)
		.InitSelection(Sender)
		.OnSelectionChanged_Lambda([this](FFIRTrace Trace) {
			Sender = Trace;
		})
	];
}

void UFIVSNode_SignalEvent::CompileNodeToLua(FFIVSLuaCompilerContext& Context) {
	if (!IsValid(Signal)) return;
	FString valEvent = FString::Printf(TEXT("\"%s\""), *Signal->GetInternalName());
	FString valSender = FIRValueToLuaLiteral(Sender);
	TArray<FString> params;
	params.Add(TEXT("_"));
	FString senderName = Context.FindAndClaimLocalName(TEXT("sender"));
	params.Add(senderName);
	Context.AddLValue(SenderOut, senderName);
	for (UFIVSPin* param : Parameters) {
		FString name = Context.FindAndClaimLocalName(param->Name);
		params.Add(name);
		Context.AddLValue(param, name);
	}

	Context.AddPlain(FString::Printf(TEXT("event.registerListener({event=%s, sender=%s}, function(%s)\n"), *valEvent, *valSender, *FString::Join(params, TEXT(","))));
	Context.EnterNewSection();
	Context.ContinueCurrentSection(ExecOut);
	Context.LeaveSection();
	Context.AddPlain(TEXT("end)\n"));
}

void UFIVSNode_SignalEvent::SetSignal(UFIRSignal* InSignal) {
	Signal = InSignal;

	if (Signal)	DisplayName = FText::FromString(TEXT("Signal Event (") + Signal->GetDisplayName().ToString() + TEXT(")"));
	else DisplayName = FText::FromString(TEXT("Signal Event"));

	DeletePin(SenderOut);
	DeletePins(Parameters);

	if (Signal) {
		SenderOut = CreatePin(FIVS_PIN_DATA_OUTPUT, TEXT("Event Sender"), FText::FromString(TEXT("Event Sender")), FFIVSPinDataType(FIR_TRACE, Cast<UFIRClass>(Signal->GetOuter())));
		for (UFIRProperty* Property : Signal->GetParameters()) {
			Parameters.Add(CreatePin(FIVS_PIN_DATA_OUTPUT, Property->GetInternalName(), Property->GetDisplayName(), FFIVSPinDataType(Property)));
		}
	}
}
