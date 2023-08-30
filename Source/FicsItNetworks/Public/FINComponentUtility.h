#pragma once

#include "Network/FINNetworkConnectionComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FINComponentUtility.generated.h"

/**
 * Provides Utility functions for BP implemented network components
 */
UCLASS()
class FICSITNETWORKS_API UFINComponentUtility : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	/**
	 * Trys to find the nearest network connector to the hit location of the the hit actor
	 *
	 * @param	hit		the hit result you want to use for the search
	 * @return	the neares network connector, nullptr if it was not able to find it
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility")
	static UFINNetworkConnectionComponent* GetNetworkConnectorFromHit(FHitResult hit);

	/**
	 * Sets the clipboard to the given string
	 *
	 * @param	str		the string you want to put into the clipboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	static void ClipboardCopy(FString str);

	template<typename ComponentType>
	static TArray<ComponentType*> GetComponentsFromSubclass(const UClass* InActorClass) {
		TArray< ComponentType* > outComponents;

		if (IsValid(InActorClass)) {
			if(const UBlueprintGeneratedClass* ActorBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(InActorClass)) {
				const TArray< USCS_Node* >& ActorBlueprintNodes = ActorBlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes();

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