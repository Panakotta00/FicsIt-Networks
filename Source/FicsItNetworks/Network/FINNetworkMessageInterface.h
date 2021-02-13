#pragma once

#include "CoreMinimal.h"
#include "FINNetworkValues.h"
#include "Interface.h"

#include "FINNetworkMessageInterface.generated.h"

class AFINNetworkCircuit;
UINTERFACE(Blueprintable)
class FICSITNETWORKS_API UFINNetworkMessageInterface : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKS_API IFINNetworkMessageInterface {
	GENERATED_BODY()
public:
	/**
	 * Allows to check if the message interface is listening on the given port.
	 *
	 * @param[in]	Port	the port you want to check
	 * @return	true if the message interface listens to the port
	 */
	virtual bool IsPortOpen(int Port) { return false; };

	/**
	 * Lets the network message implemnter handle internally a new message
	 * on the given port.
	 * Doesn't need to apply port filtering.
	 * Routers should make sure to not cause a message loop.
	 *
	 * @param[in]	ID				A GUID generated on message send which allows routers to check if message got already sent, to prevent message loops
	 * @param[in]	Sender			Guid containing the address of the sender
	 * @param[in]	Receiver		Guid containing the address of the receiver
	 * @param[in]	Port			The port on which the message got sent
	 * @param[in]	Data			The data frame of the message
	 */
	virtual void HandleMessage(FGuid ID, FGuid Sender, FGuid Receiver, int Port, const FINArray& Data) {};

	/**
	 * Allows to check if this network message handler is capable
	 * of rerouting the network message to a different system.
	 * When a network message gets send, the sender should first check
	 * if it can find the correct receiver in the network, afterwards
	 * it should call the HandleMessage on all network message routers
	 * which have the correct port open.
	 *
	 * @return	True if this network message handler is a router
	 */
	virtual bool IsNetworkMessageRouter() const { return false; }
};