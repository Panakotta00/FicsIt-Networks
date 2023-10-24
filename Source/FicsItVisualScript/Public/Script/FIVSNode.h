#pragma once

#include "CoreMinimal.h"
#include "FIVSNodeSerialization.h"
#include "Network/FINNetworkValues.h"
#include "Reflection/FINProperty.h"
#include "Templates/SubclassOf.h"
#include "FIVSNode.generated.h"

class IFIVSScriptContext_Interface;
class SFIVSEdGraphViewer;
struct FFIVSEdStyle;
class UFIVSGraph;
class SFIVSEdNodeViewer;
class UFIVSNode;

UENUM()
enum EFIVSPinType {
	FIVS_PIN_NONE			= 0b0,
	FIVS_PIN_INPUT			= 0b0001,
	FIVS_PIN_OUTPUT			= 0b0010,
	FIVS_PIN_DATA			= 0b0100,
	FIVS_PIN_EXEC			= 0b1000,
	FIVS_PIN_DATA_INPUT		= FIVS_PIN_DATA | FIVS_PIN_INPUT,
	FIVS_PIN_DATA_OUTPUT	= FIVS_PIN_DATA | FIVS_PIN_OUTPUT,
	FIVS_PIN_EXEC_INPUT		= FIVS_PIN_EXEC | FIVS_PIN_INPUT,
	FIVS_PIN_EXEC_OUTPUT	= FIVS_PIN_EXEC | FIVS_PIN_OUTPUT,
};
ENUM_CLASS_FLAGS(EFIVSPinType)

USTRUCT()
struct FFIVSPinDataType : public FFINExpandedNetworkValueType {
	GENERATED_BODY()
private:
	bool bReference = false;

public:
	FFIVSPinDataType() = default;
	FFIVSPinDataType(const FFIVSPinDataType&) = default;
	FFIVSPinDataType(const FFINExpandedNetworkValueType& Other) : FFINExpandedNetworkValueType(Other) {}
	FFIVSPinDataType(EFINNetworkValueType InType, UFINStruct* InRefType) : FFINExpandedNetworkValueType(InType, InRefType) {}

	bool IsReference() const { return bReference; }
	
	bool Equals(const FFIVSPinDataType& Other) {
		if (Other.bReference && !bReference) return false;
		return Super::Equals(Other);
	}

	bool IsA(const FFIVSPinDataType& Other) const {
		if (Other.bReference && !bReference) return false;
		return Super::IsA(Other);
	}

	FFIVSPinDataType AsRef() const {
		FFIVSPinDataType NewType(*this);
		NewType.bReference = true;
		return NewType;
	}

	FFIVSPinDataType AsVal() const {
		FFIVSPinDataType NewType(*this);
		NewType.bReference = false;
		return NewType;
	}
};

USTRUCT()
struct FFIVSFullPinType {
	GENERATED_BODY()
	
	EFIVSPinType PinType;
	FFIVSPinDataType DataType;

	FFIVSFullPinType() = default;
	FFIVSFullPinType(EFIVSPinType PinType) : PinType(PinType) {}
	FFIVSFullPinType(EFIVSPinType PinType, FFIVSPinDataType DataType) : PinType(PinType), DataType(DataType) {}
	FFIVSFullPinType(UFINProperty* Property) {
		EFINRepPropertyFlags Flags = Property->GetPropertyFlags();
		if (Flags & FIN_Prop_Param) {
			if (Flags & FIN_Prop_OutParam) {
				PinType = FIVS_PIN_DATA_OUTPUT;
			} else {
				PinType = FIVS_PIN_DATA_INPUT;
			}
		} else if (Flags & FIN_Prop_Attrib) {
			PinType = FIVS_PIN_DATA;
		}
		DataType = FFIVSPinDataType(Property);
	}

	bool CanConnect(const FFIVSFullPinType& Other) const;
};

UCLASS(Abstract)
class UFIVSPin : public UObject {
	GENERATED_BODY()
protected:
	UPROPERTY(SaveGame)
	TArray<UFIVSPin*> ConnectedPins;

	UPROPERTY(SaveGame)
	FFINAnyNetworkValue Literal;
	
public:
	UPROPERTY()
	UFIVSNode* ParentNode = nullptr;

	/**
	 * Returns the literal value of the given pin.
	 * The literal value will be used if pin has no connection to any other pins.
	 */
	FFINAnyNetworkValue GetLiteral() {
		if (Literal.GetType() != GetPinDataType().GetType()) {
			SetLiteral(FFINAnyNetworkValue::DefaultValue(GetPinDataType().GetType()));
		}
		return Literal;
	}

	/**
	 * Allows to set/change the literal of the pin.
	 * For more info on literals, see GetLiteral()
	 */
	void SetLiteral(FFINAnyNetworkValue InLiteral) {
		if (InLiteral.GetType() == GetPinDataType().GetType()) Literal = InLiteral;
	}
	
	/**
	 * Returns the pin type use to determine if this pin is a input, output, data or exec pin.
	 */
	virtual EFIVSPinType GetPinType();

	/**
	 * Returns the pin data type use to check if two pins can be connected.
	 */
	virtual FFIVSPinDataType GetPinDataType();

	/**
	 * Returns all connected pins
	 */
	virtual const TArray<UFIVSPin*>& GetConnections();

	/**
	 * Returns the internal name of the pin. Mainly used for referencing the pin.
	 */
	virtual FString GetName();

	/**
	 * Returns the display name of the pin.
	 */
	virtual FText GetDisplayName() { return FText::FromString(GetName()); }

	/**
	 * Checks if the the pin can be connected to the given pin
	 */
	virtual bool CanConnect(UFIVSPin* Pin);
	
	/**
	 * Creates a connection between this and the given pin
	 */
	void AddConnection(UFIVSPin* Pin);

	/**
	 * Removes a connection between this and the given pin
	 */
	void RemoveConnection(UFIVSPin* Pin);

	void GetAllConnected(TArray<UFIVSPin*>& Searches);
	TArray<UFIVSPin*> GetAllConnected() {
		TArray<UFIVSPin*> Connected;
		GetAllConnected(Connected);
		return Connected;
	}

	/**
	 * Trys to find the data-source for the network of pins this pin is connected to,
	 * or in the case of exec pins, trys to find the next Exec-Pin.
	 * */
	UFIVSPin* FindConnected();

	/**
	 * Removes all connections of this pin
	 */
	void RemoveAllConnections();
};

UCLASS()
class UFIVSGenericPin : public UFIVSPin {
	GENERATED_BODY()
public:
	EFIVSPinType PinType = FIVS_PIN_NONE;
	FFIVSPinDataType PinDataType = FFINExpandedNetworkValueType(FIN_NIL);
	FString Name = TEXT("Unnamed");
	FText DisplayName = FText::FromString("Unnamed");
	
	// Begin UFINScriptPin
	virtual EFIVSPinType GetPinType() override;
	virtual FFIVSPinDataType GetPinDataType() override;
	virtual FString GetName() override;
	// End UFINScriptPin
	
	static UFIVSGenericPin* Create(FFIVSPinDataType DataType, EFIVSPinType PinType, const FString& Name, const FText& DisplayName);
};

UCLASS()
class UFIVSWildcardPin : public UFIVSPin {
	GENERATED_BODY()
protected:
	FFIVSPinDataType DataType;
	EFIVSPinType PinType;
	
public:
	// Begin UFINScriptPin
	virtual EFIVSPinType GetPinType() override;
	virtual FFIVSPinDataType GetPinDataType() override;
	virtual bool CanConnect(UFIVSPin* Pin) override;
	// End UFINScriptPin
};

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
	UPROPERTY(SaveGame)
	FVector2D Pos;
	
	FFINScriptGraphPinChanged OnPinChanged;

	/**
	 * Should create all pins of this node
	 */
	virtual void InitPins() {}
	
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
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(SFIVSEdGraphViewer* GraphViewer, const FFIVSEdStyle* Style);

	/**
	 * Creates and returns a new widget that can be used to change detailed information of this node.
	 */
	virtual TSharedPtr<SWidget> CreateDetailsWidget(TScriptInterface<IFIVSScriptContext_Interface> Context) { return nullptr; }
	
	/**
	 * Removed all Pins and calls InitPins again, may cause the UI to update its rendering.
	 */
	virtual void ReconstructPins();

	/**
	 * Called if this nodes gets serialized.
	 * Is supposed to store additional node properties to the serialization data that will be used on deserialization
	 * for initializing the node so it can successfully recreate the Node name, Pins, functionality, etc.
	 */
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const {}

	/**
	 * Called when the node gets deserialized.
	 * When a (partial) graph, gets deserialized, a new object of this node-class my get created.
	 * It then has to be initialized with data stored additionally in the serialization data (see SerializeNodeProperties(...)),
	 * this should happen here, so that directly after this function got called, the InitPins() function can be called normally,
	 * to create all Pins like it was before serialization.
	 */
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) {};
	
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
};

UCLASS()
class UFIVSRerouteNode : public UFIVSNode {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame)
	UFIVSPin* Pin = nullptr;

public:
	UFIVSRerouteNode();
	
	// Begin UFINScriptNode
	virtual TArray<UFIVSPin*> GetNodePins() const override;
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual TSharedRef<SFIVSEdNodeViewer> CreateNodeViewer(SFIVSEdGraphViewer* GraphViewer, const FFIVSEdStyle* Style) override;
	// End UFINScriptNode
};
