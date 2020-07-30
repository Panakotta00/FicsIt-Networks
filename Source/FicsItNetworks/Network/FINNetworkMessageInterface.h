#pragma once

#include "CoreMinimal.h"
#include "FINDynamicStructHolder.h"
#include "FINNetworkTrace.h"
#include "FINParameterList.h"
#include "Interface.h"

#include "FINNetworkMessageInterface.generated.h"

UINTERFACE(Blueprintable)
class UFINNetworkMessageInterface : public UInterface {
	GENERATED_BODY()
};

class IFINNetworkMessageInterface {
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
	 *
	 * @param[in]	Sender	Network Trace pointing from this to the sender
	 * @param[in]	Port	The port on which the message got sent
	 * @param[in]	Data	The data frame of the message
	 */
	virtual void HandleMessage(FFINNetworkTrace Sender, int Port, const TFINDynamicStruct<FFINParameterList>& Data) {};
};