#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "FINNetworkUtils.generated.h"

UCLASS()
class FICSITNETWORKSCIRCUIT_API UFINNetworkUtils : public UObject {
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
	static FFIRTrace RedirectIfPossible(const FFIRTrace& Trace);

	/**
	 * Trys to find the nearest network connector to the hit location of the the hit actor
	 *
	 * @param	hit		the hit result you want to use for the search
	 * @return	the neares network connector, nullptr if it was not able to find it
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility")
	static class UFINNetworkConnectionComponent* GetNetworkConnectorFromHit(FHitResult hit);

	template<typename ComponentType>
	static TArray<ComponentType*> GetComponentsFromSubclass(const UClass* InActorClass) {
		TArray< ComponentType* > outComponents;

		if (IsValid(InActorClass)) {
			if(const UBlueprintGeneratedClass* ActorBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(InActorClass)) {
				const TArray<USCS_Node*>& ActorBlueprintNodes = ActorBlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes();

				for (USCS_Node* Node : ActorBlueprintNodes) {
					if (UClass::FindCommonBase(Node->ComponentClass, ComponentType::StaticClass()) && !Node->IsEditorOnly()) {
						if (ComponentType* BlueprintComponent = Cast<ComponentType>(Node->ComponentTemplate)) {
							outComponents.Add(BlueprintComponent);
						}
					}
				}
			}
		}

		return outComponents;
	}
};
