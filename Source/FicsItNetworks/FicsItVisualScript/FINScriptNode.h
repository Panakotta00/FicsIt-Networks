#pragma once

#include "CoreMinimal.h"
#include "Network/FINNetworkValues.h"
#include "SharedPointer.h"
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

struct FFINScriptPin;

struct FFINScriptPin : public TSharedFromThis<FFINScriptPin> {
protected:
	TArray<TSharedPtr<FFINScriptPin>> ConnectedPins;

	void GetAllConnected(TArray<TSharedPtr<FFINScriptPin>>& Searches);
	
public:
	UFINScriptNode* ParentNode = nullptr;
	EFINNetworkValueType DataType;
	EFINScriptPinType PinType;
	FString Name;
	
public:
	FFINScriptPin() = default;
	FFINScriptPin(EFINNetworkValueType DataType, EFINScriptPinType PinType, FString Name) : DataType(DataType), PinType(PinType), Name(Name) {}
	virtual ~FFINScriptPin();
	
	/**
	 * Creates a connection between this and the given pin
	 */
	void AddConnection(const TSharedPtr<FFINScriptPin>& Pin);

	/**
	 * Removes a connection between this and the given pin
	 */
	void RemoveConnection(const TSharedPtr<FFINScriptPin>& Pin);

	/**
	 * Returns all connected pins
	 */
	virtual const TArray<TSharedPtr<FFINScriptPin>>& GetConnections();

	/**
	 * Returns the pin type
	 */
	virtual EFINScriptPinType GetPinType();

	/**
	 * Checks if the the pin can be connected to the given pin
	 */
	virtual bool CanConnect(const TSharedPtr<FFINScriptPin>& Pin);

	/**
	 * Returns the pin data type
	 */
	virtual EFINNetworkValueType GetPinDataType();
};

struct FFINScriptWildcardPin : public FFINScriptPin {
	FFINScriptWildcardPin();

	TSharedPtr<FFINScriptPin> IgnoreNext;

	virtual EFINScriptPinType GetPinType() override;
	virtual EFINNetworkValueType GetPinDataType() override;
	virtual bool CanConnect(const TSharedPtr<FFINScriptPin>& Pin) override;
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
	UPROPERTY()
	FVector2D Pos;
	
	FFINScriptGraphPinChanged OnPinChanged;

	/**
	 * Returns the list of pins of this node
	 */
	virtual TArray<TSharedRef<FFINScriptPin>> GetNodePins() const { return TArray<TSharedRef<FFINScriptPin>>(); }
};

UCLASS()
class UFINScriptRerouteNode : public UFINScriptNode {
	GENERATED_BODY()
	
private:
	TSharedRef<FFINScriptPin> Pin;

public:
	UFINScriptRerouteNode();
	
	// Begin UFINScriptNode
	virtual TArray<TSharedRef<FFINScriptPin>> GetNodePins() const override;
	// End UFINScriptNode
};

UCLASS()
class UFINScriptFuncNode : public UFINScriptNode {
	GENERATED_BODY()
	
private:
	TArray<TSharedRef<FFINScriptPin>> Pins;

protected:
	/**
	* Adds the given pin to the node
	*
	* @param[in]	Pin		the pin you want to add
	* @return	the index of the pin in the pin array
	*/
	int AddNodePin(const TSharedRef<FFINScriptPin>& Pin);

	/**
	* Removes the pin at the given index from the node
	*
	* @param[in]	index	the index of the pin
	*/
	void RemoveNodePin(int index);
	
public:
	// Begin UFINScriptNode
	virtual TArray<TSharedRef<FFINScriptPin>> GetNodePins() const override;
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
	virtual FString GetNodeName() const { return Name; }
	// End UFINScriptFuncNode

	int AddPin(const TSharedRef<FFINScriptPin>& Pin) { return AddNodePin(Pin); }
	void RemovePin(int index) { RemoveNodePin(index); };
};