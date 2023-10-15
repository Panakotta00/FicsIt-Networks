#include "Network/FINMCPAdvConnector.h"

UFINMCPAdvConnector::UFINMCPAdvConnector() {
	AllowedCableConnections.Empty();
	AllowedCableConnections.Add(LoadClass<UFGBuildingDescriptor>(NULL, TEXT("/FicsItNetworks/Network/ThinNetworkCable/BD_ThinNetworkCable.BD_ThinNetworkCable_C")));
}
