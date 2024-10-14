#include "FINMCPAdvConnector.h"

#include "ConstructorHelpers.h"

UFINMCPAdvConnector::UFINMCPAdvConnector() {
	static ConstructorHelpers::FClassFinder<UFGBuildingDescriptor> ThinNetworkCableClass(TEXT("/FicsItNetworks/Buildings/Network/ThinNetworkCable/BD_ThinNetworkCable.BD_ThinNetworkCable_C"));

	AllowedCableConnections.Empty();
	AllowedCableConnections.Add(ThinNetworkCableClass.Class);
}
