#pragma once

#include "FINNetworkTrace.h"
#include "FINNetworkUtils.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINNetworkUtils : public UObject {
	GENERATED_BODY()
public:
	/**
	 * Tries to find a network component based of the given object.
	 * Might return the object it self, or a component if its an actor
	 * f.e. if you refer to an actor with a network connection component
	 * @param[in]	Obj		the object you want to get a network handler for
	 * @return	The network handler it was able to find (nullptr if no handler was found)
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Utils")
	static UObject* FindNetworkComponentFromObject(UObject* Obj);

	UFUNCTION(BlueprintCallable, Category="Network|Utils")
	static FFINNetworkTrace RedirectIfPossible(const FFINNetworkTrace& Trace);
};
