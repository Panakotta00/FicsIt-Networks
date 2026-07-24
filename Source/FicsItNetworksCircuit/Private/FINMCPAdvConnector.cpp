#include "FINMCPAdvConnector.h"

#include "UObject/ConstructorHelpers.h"

UFINMCPAdvConnector::UFINMCPAdvConnector() {
	static ConstructorHelpers::FClassFinder<UFGBuildingDescriptor> ThinNetworkCableClass(TEXT("/FicsItNetworks/Buildings/Network/ThinNetworkCable/Desc_ThinNetworkCable.Desc_ThinNetworkCable_C"));

	AllowedCableConnections.Empty();
	AllowedCableConnections.Add(ThinNetworkCableClass.Class);
}
