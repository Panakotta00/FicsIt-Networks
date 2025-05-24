#pragma once

#include "CoreMinimal.h"
#include "FIVSNodeSerialization.h"
#include "FIVSPin.h"
#include "Templates/SubclassOf.h"
#include "FIVSNode.generated.h"

class IFIVSScriptContext_Interface;
class UFIVSGraph;
class UFIVSNode;
class SFIVSEdNodeViewer;
class SFIVSEdGraphViewer;
struct FFIVSEdNodeStyle;

DECLARE_DELEGATE_RetVal(TSharedRef<SWidget>, FFIVSNodeActionCreateTooltip)
DECLARE_DELEGATE_OneParam(FFIVSNodeActionExecute, UFIVSNode*)

struct FFIVSNodeAction {
	/**
	 * This is class is used to create the node when the action gets executed
	 */
	TSubclassOf<UFIVSNode> NodeType;
	
	/**
	 * The text that is shown in the tree-view of the action selection menu
	 */
	FText Title;

	/**
	 * The category in which this node action should be shown in
	 */
	FText Category;

	/**
	 * The searchable text used by the text filter to determine if a given user query matches this node action
	 */
	FText SearchableText;
	
	/**
	 * This array contains descriptions of all pins of the node action and is used to filter by context.
	 */
	TArray<FFIVSFullPinType> Pins;
	
	/**
	 * This delegate gets called if the user wants to execute this action.
	 */
	FFIVSNodeActionExecute OnExecute;

	/**
	 * This delegate gets called if the user hovers long enough over a action in the action selection menu
	 * and a more detailed description of the action should appear, this delegate should create this
	 * slate widget. If unbound, no tooltip will be shown.
	 */
	FFIVSNodeActionCreateTooltip OnCreateTooltip;
};

UENUM()
enum EFIVSNodePinChange {
	FIVS_PinChange_Added,
	FIVS_PinChange_Removed,
};

/**
 * Notifies if the pin list of the node has changed.
 * Param1: type of change
 * Param2: the changed pin
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphPinChanged, EFIVSNodePinChange, UFIVSPin*);

UCLASS(Abstract)
class UFIVSNode : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FGuid NodeId = FGuid::NewGuid();

	UPROPERTY()
	FVector2D Pos;

	UPROPERTY()
	FText DisplayName = FText::FromString(TEXT("Unnamed"));

	FFINScriptGraphPinChanged OnPinChanged;

	/**
	 * Returns the list of pins of this node
	 */
	virtual TArray<UFIVSPin*> GetNodePins() const { return TArray<UFIVSPin*>(); }

	/**
	 * This function will be called on the CDO of this class and should return a list of Actions that
	 * will get added to the action selection menu of the graph editor.
	 */
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const { }

	/**
	 * Returns a SFIVSEdNodeViewer that is used to display this node in a graph editor.
	 */
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style, class UFIVSEdEditor* Context);

	/**
	 * Creates and returns a new widget that can be used to change detailed information of this node.
	 */
	virtual TSharedPtr<SWidget> CreateDetailsWidget(TScriptInterface<IFIVSScriptContext_Interface> Context) { return nullptr; }

	/**
	 * Return a FUICommandList used for the Context Menu of a given Pin
	 */
	virtual TSharedPtr<FUICommandList> GetPinCommands(UFIVSPin* InPin) { return nullptr; }

	/**
	 * Extend the context menu of a given Pin
	 */
	virtual void ExtendPinContextMenu(UFIVSPin* InPin, FMenuBuilder& MenuBuilder) {}

	/**
	 * Called if this nodes gets serialized.
	 * Is supposed to store additional node properties to the serialization data that will be used on deserialization
	 * for initializing the node so it can successfully recreate the Node name, Pins, functionality, etc.
	 */
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Value) const {}

	/**
	 * Called when the node gets deserialized.
	 * When a (partial) graph, gets deserialized, a new object of this node-class my get created.
	 * It then has to be initialized with data stored additionally in the serialization data (see SerializeNodeProperties(...)),
	 * this should happen here, so that directly after this function got called, the InitPins() function can be called normally,
	 * to create all Pins like it was before serialization.
	 */
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Value) {}

	/**
	 * Removes all connections of all pins
	 */
	void RemoveAllConnections();

	/**
	 * Returns true if the node is a pure node.
	 * A pure node is a node that does not have any exec pins.
	 */
	bool IsPure() {
		for (UFIVSPin* Pin : GetNodePins()) {
			if (Pin->GetPinType() & FIVS_PIN_EXEC) return false;
		}
		return true;
	}

	/**
	 * Tries to find a Pin with the given internal name.
	 * If no pin was found, returns nullptr.
	 */
	UFIVSPin* FindPinByName(const FString& Name) {
		for (UFIVSPin* Pin : GetNodePins()) {
			if (Pin->GetName() == Name) return Pin;
		}
		return nullptr;
	}

	/**
	 * Retruns the outer/parent graph of this node
	 */
	UFIVSGraph* GetOuterGraph() const;

	bool IsStartNode() {
		bool bHasExecOut = false;
		for (UFIVSPin* Pin : GetNodePins()) {
			EFIVSPinType type = Pin->GetPinType();
			if (!(type & FIVS_PIN_EXEC)) {
				continue;
			}

			if (type & FIVS_PIN_INPUT) {
				return false;
			}
			if (type & FIVS_PIN_OUTPUT) {
				if (bHasExecOut) {
					return false;
				}
				bHasExecOut = true;
			}
		}
		return bHasExecOut;
	}
};

UCLASS()
class UFIVSRerouteNode : public UFIVSNode {
	GENERATED_BODY()
	
private:
	UPROPERTY(Instanced)
	UFIVSPin* Pin = nullptr;

public:
	UFIVSRerouteNode();
	
	// Begin UFINScriptNode
	virtual TArray<UFIVSPin*> GetNodePins() const override;
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, const FFIVSEdNodeStyle* Style, UFIVSEdEditor* Context) override;
	// End UFINScriptNode
};
