#pragma once

#include "CoreMinimal.h"
#include "FicsItNetworks/Network/FINNetworkValues.h"
#include "FicsItNetworks/Reflection/FINFunction.h"
#include "FicsItNetworks/Reflection/FINProperty.h"
#include "FIVSNode.generated.h"

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

UCLASS()
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
		if (Literal.GetType() != GetPinDataType()) {
			SetLiteral(FFINAnyNetworkValue(GetPinDataType()));
		}
		return Literal;
	}

	/**
	 * Allows to set/change the literal of the pin.
	 * For more info on literals, see GetLiteral()
	 */
	void SetLiteral(FFINAnyNetworkValue InLiteral) {
		if (InLiteral.GetType() == GetPinDataType()) Literal = InLiteral;
	}
	
	/**
	 * Returns the pin type
	 */
	virtual EFIVSPinType GetPinType();

	/**
	 * Returns the pin data type
	 */
	virtual EFINNetworkValueType GetPinDataType();

	/**
	 * Returns all connected pins
	 */
	virtual const TArray<UFIVSPin*>& GetConnections();

	/**
	 * Returns the name of the pin
	 */
	virtual FText GetName();

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
	EFINNetworkValueType PinDataType = FIN_NIL;
	FText Name = FText::FromString("Unnamed");
	
	// Begin UFINScriptPin
	virtual EFIVSPinType GetPinType() override;
	virtual EFINNetworkValueType GetPinDataType() override;
	virtual FText GetName() override;
	// End UFINScriptPin
	
	static UFIVSGenericPin* Create(EFINNetworkValueType DataType, EFIVSPinType PinType, const FString& Name);
};

UCLASS()
class UFIVSWildcardPin : public UFIVSPin {
	GENERATED_BODY()
protected:
	EFINNetworkValueType DataType;
	EFIVSPinType PinType;
	
public:
	// Begin UFINScriptPin
	virtual EFIVSPinType GetPinType() override;
	virtual EFINNetworkValueType GetPinDataType() override;
	virtual bool CanConnect(UFIVSPin* Pin) override;
	// End UFINScriptPin
};

/**
 * Notifies if the pin list of the node has changed.
 * Param1: type of change (0 = pin added, 1 = pin removed)
 * Param2: the changed pin
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphPinChanged, int, int);

UCLASS()
class UFIVSNode : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	FVector2D Pos;
	
	FFINScriptGraphPinChanged OnPinChanged;

	/**
	 * Returns the list of pins of this node
	 */
	virtual TArray<UFIVSPin*> GetNodePins() const { return TArray<UFIVSPin*>(); }

	/**
	 * Removes all connections of all pins
	 */
	void RemoveAllConnections();
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
	// End UFINScriptNode
};
