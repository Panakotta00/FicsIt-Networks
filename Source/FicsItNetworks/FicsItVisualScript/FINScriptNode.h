#pragma once

#include "CoreMinimal.h"
#include "Network/FINNetworkValues.h"
#include "SharedPointer.h"
#include "Reflection/FINFunction.h"
#include "Reflection/FINProperty.h"

#include "FINScriptNode.generated.h"

class UFINScriptNode;

UENUM()
enum EFINScriptPinType {
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
ENUM_CLASS_FLAGS(EFINScriptPinType)

UCLASS()
class UFINScriptPin : public UObject {
	GENERATED_BODY()
protected:
	UPROPERTY(SaveGame)
	TArray<UFINScriptPin*> ConnectedPins;
	
public:
	UPROPERTY()
	UFINScriptNode* ParentNode = nullptr;

	/**
	 * Returns the pin type
	 */
	virtual EFINScriptPinType GetPinType();

	/**
	 * Returns the pin data type
	 */
	virtual EFINNetworkValueType GetPinDataType();

	/**
	 * Returns all connected pins
	 */
	virtual const TArray<UFINScriptPin*>& GetConnections();

	/**
	 * Returns the name of the pin
	 */
	virtual FText GetName();

	/**
	 * Checks if the the pin can be connected to the given pin
	 */
	virtual bool CanConnect(UFINScriptPin* Pin);
	
	/**
	 * Creates a connection between this and the given pin
	 */
	void AddConnection(UFINScriptPin* Pin);

	/**
	 * Removes a connection between this and the given pin
	 */
	void RemoveConnection(UFINScriptPin* Pin);

	void GetAllConnected(TArray<UFINScriptPin*>& Searches);

	/**
	 * Removes all connections of this pin
	 */
	void RemoveAllConnections();
};

UCLASS()
class UFINScriptGenericPin : public UFINScriptPin {
	GENERATED_BODY()
public:
	EFINScriptPinType PinType = FIVS_PIN_NONE;
	EFINNetworkValueType PinDataType = FIN_NIL;
	FText Name = FText::FromString("Unnamed");
	
	// Begin UFINScriptPin
	virtual EFINScriptPinType GetPinType() override;
	virtual EFINNetworkValueType GetPinDataType() override;
	virtual FText GetName() override;
	// End UFINScriptPin
	
	static UFINScriptGenericPin* Create(EFINNetworkValueType DataType, EFINScriptPinType PinType, const FString& Name);
};

UCLASS()
class UFINScriptWildcardPin : public UFINScriptPin {
	GENERATED_BODY()
protected:
	EFINNetworkValueType DataType;
	EFINScriptPinType PinType;
	
public:
	// Begin UFINScriptPin
	virtual EFINScriptPinType GetPinType() override;
	virtual EFINNetworkValueType GetPinDataType() override;
	virtual bool CanConnect(UFINScriptPin* Pin) override;
	// End UFINScriptPin
};

UCLASS()
class UFINScriptReflectionPin : public UFINScriptPin {
	GENERATED_BODY()
protected:
	UPROPERTY()
	UFINProperty* Property = nullptr;

public:	
	// Begin UFINScriptPin
	virtual EFINScriptPinType GetPinType() override;
	virtual EFINNetworkValueType GetPinDataType() override;
	virtual FText GetName() override;
	// end UFINScriptPin

	/**
	 * changes the stored property to the given new one
	 */
	void SetProperty(UFINProperty* Prop);
};

/**
 * Notifies if the pin list of the node has changed.
 * Param1: type of change (0 = pin added, 1 = pin removed)
 * Param2: the changed pin
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphPinChanged, int, int);

UCLASS()
class UFINScriptNode : public UObject {
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	FVector2D Pos;
	
	FFINScriptGraphPinChanged OnPinChanged;

	/**
	 * Returns the list of pins of this node
	 */
	virtual TArray<UFINScriptPin*> GetNodePins() const { return TArray<UFINScriptPin*>(); }

	/**
	 * Removes all connections of all pins
	 */
	void RemoveAllConnections();
};

UCLASS()
class UFINScriptRerouteNode : public UFINScriptNode {
	GENERATED_BODY()
	
private:
	UPROPERTY()
	UFINScriptPin* Pin = nullptr;

public:
	UFINScriptRerouteNode();
	
	// Begin UFINScriptNode
	virtual TArray<UFINScriptPin*> GetNodePins() const override;
	// End UFINScriptNode
};

UCLASS()
class UFINScriptFuncNode : public UFINScriptNode {
	GENERATED_BODY()
	
private:
	UPROPERTY(SaveGame)
	TArray<UFINScriptPin*> Pins;

protected:
	/**
	* Adds the given pin to the node
	*
	* @param[in]	Pin		the pin you want to add
	* @return	the index of the pin in the pin array
	*/
	int AddNodePin(UFINScriptPin* Pin);

	/**
	* Removes the pin at the given index from the node
	*
	* @param[in]	index	the index of the pin
	*/
	void RemoveNodePin(int index);
	
public:
	// Begin UFINScriptNode
	virtual TArray<UFINScriptPin*> GetNodePins() const override;
	// End UFINScriptNode

	/**
	 * Returns the header name of that function node
	 */
	virtual FString GetNodeName() const { return "Undefined"; }
};

UCLASS()
class UFINScriptGenericFuncNode : public UFINScriptFuncNode {
	GENERATED_BODY()
public:
	FString Name;

	// Begin UFINScriptFuncNode
	virtual FString GetNodeName() const override { return Name; }
	// End UFINScriptFuncNode

	int AddPin(UFINScriptPin* Pin) { return AddNodePin(Pin); }
	void RemovePin(int index) { RemoveNodePin(index); };
};

UCLASS()
class UFINScriptReflectedFuncNode : public UFINScriptFuncNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFINFunction* Function = nullptr;

public:
	// Begin UFINScriptFuncNode
	virtual FString GetNodeName() const override;
	// End UFINScriptFuncNode

	/**
	 * Sets the function this nodes uses.
	 * Recreates all pins.
	 */
	void SetFunction(UFINFunction* Function);

	/**
	 * Returns the function
	 */
	UFINFunction* GetFunction() const;
};