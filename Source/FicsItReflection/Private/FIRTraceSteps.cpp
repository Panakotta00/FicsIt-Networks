#include "FIRTrace.h"

#include "FGPowerCircuit.h"
#include "FGPowerConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "FGRailroadTimeTable.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FGRailroadVehicle.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "FGTrain.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "Buildables/FGBuildableTrainPlatform.h"

#define StepFuncName(A, B) Step ## _ ## A ## _ ## B
#define StepRegName(A, B) StepReg ## _ ## A ## _ ## B
#define StepRegSigName(A, B) StepRegSig ## _ ## A ## _ ## B
#define StepFuncSig(A, B) bool StepFuncName(A, B)(UObject* oA, UObject* oB)
#define StepFuncSigReg(A, B) \
	TPair<TPair<UClass*, UClass*>, TPair<FString, FFINTraceStep*>> StepRegSigName(A, B)() { \
		return TPair<TPair<UClass*, UClass*>, TPair<FString, FFINTraceStep*>>{ \
			TPair<UClass*, UClass*>{ \
				A::StaticClass(), \
				B::StaticClass() \
			}, \
			TPair<FString, FFINTraceStep*>{ \
				#A "_" #B, \
				new FFINTraceStep(&StepFuncName(A, B)) \
			} \
		}; \
	}
#define Step(CA, CB, Code) \
	StepFuncSig(CA, CB); \
	StepFuncSigReg(CA, CB) \
	FFINTraceStepRegisterer StepRegName(CA, CB)(&StepRegSigName(CA, CB)); \
	StepFuncSig(CA, CB) { \
		CA* A = Cast<CA>(oA); \
		CB* B = Cast<CB>(oB); \
		Code \
	}


class FFINTraceStepRegisterer {
public:
	FFINTraceStepRegisterer(TPair<TPair<UClass*, UClass*>, TPair<FString, FFINTraceStep*>>(*regSig)()) {
		FFIRTrace::toRegister.Add(regSig);
	}
};

void traceRegisterSteps() {
	for (auto& stepSig : FFIRTrace::toRegister) {
		auto step = stepSig();
		FFINTraceStep* tStep = step.Value.Value;
		UClass* A = step.Key.Key;
		UClass* B = step.Key.Value;
		TMap<UClass*, TPair<TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>, TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>>>* AMap;
		if (A->GetSuperClass() == UInterface::StaticClass()) AMap = &FFIRTrace::interfaceTraceStepMap;
		else AMap = &FFIRTrace::traceStepMap;
		TMap<UClass*, TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>>* BMap;
		if (B->GetSuperClass() == UInterface::StaticClass()) BMap = &(*AMap).FindOrAdd(A).Value;
		else BMap = &(*AMap).FindOrAdd(A).Key;
		TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe> stepPtr = TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>(tStep);
		(*BMap).FindOrAdd(B) = stepPtr;
		FFIRTrace::traceStepRegistry.FindOrAdd(step.Value.Key) = stepPtr;
		FFIRTrace::inverseTraceStepRegistry.FindOrAdd(stepPtr) = step.Value.Key;
	}
	FFIRTrace::toRegister.Empty();
}


/* ############### */
/* # Trace Steps # */
/* ############### */

// TODO: Add these Trace Steps back
/*
Step(UFINNetworkComponent, UFINNetworkComponent, {
	AFINNetworkCircuit* Circuit = IFINNetworkCircuitNode::Execute_GetCircuit(oA);
	return Circuit && oB && Circuit->HasNode(oB);
})

Step(UFINNetworkComponent, AFINNetworkCircuit, {
	return B->HasNode(oA);
})
Step(AFINNetworkCircuit, UFINNetworkComponent, {
	return A->HasNode(oB);
})

Step(AFGVehicle, AFINVehicleScanner, {
	TArray<AActor*> Actors;
	B->GetOverlappingActors(Actors);
	return Actors.Contains(A);
})
Step(AFINVehicleScanner, AFGVehicle, {
	TArray<AActor*> Actors;
	A->GetOverlappingActors(Actors);
	return Actors.Contains(B);
})
*/

Step(UFGPowerConnectionComponent, UFGPowerCircuit, {
	return A->GetCircuitID() == B->GetCircuitID();
})
Step(UFGPowerCircuit, UFGPowerConnectionComponent, {
	return A->GetCircuitID() == B->GetCircuitID();
})

Step(UFGPowerInfoComponent, UFGPowerCircuit, {
	return A->GetPowerCircuit()->GetCircuitID() == B->GetCircuitID();
})
Step(UFGPowerCircuit, UFGPowerInfoComponent, {
	return A->GetCircuitID() == B->GetPowerCircuit()->GetCircuitID();
})

Step(UFGPowerInfoComponent, UFGPowerConnectionComponent, {
	return A == B->GetPowerInfo();
})
Step(UFGPowerConnectionComponent, UFGPowerInfoComponent, {
	return A->GetPowerInfo() == B;
})

Step(AFGBuildableTrainPlatform, AFGBuildableTrainPlatform, {
	return A->GetTrackGraphID() == B->GetTrackGraphID();
})

Step(AFGBuildableTrainPlatform, AFGRailroadVehicle, {
	return A->GetTrackGraphID() == B->GetTrackGraphID();
})
Step(AFGRailroadVehicle, AFGBuildableTrainPlatform, {
	return A->GetTrackGraphID() == B->GetTrackGraphID();
})

Step(AFGRailroadVehicle, AFGTrain, {
	return A->GetTrackGraphID() == B->GetTrackGraphID();
})
Step(AFGTrain, AFGRailroadVehicle, {
	return A->GetTrackGraphID() == B->GetTrackGraphID();
})

Step(AFGTrain, AFGRailroadTimeTable, {
	return A->GetTimeTable() == B;
})
Step(AFGRailroadTimeTable, AFGTrain, {
    return A == B->GetTimeTable();
})

Step(AFGRailroadVehicle, AFGRailroadVehicle, {
	return A->GetTrackGraphID() == B->GetTrackGraphID();
})

Step(AFGRailroadVehicle, UFGRailroadVehicleMovementComponent, {
	return A->GetRailroadVehicleMovementComponent() == B;
});
Step(UFGRailroadVehicleMovementComponent, AFGRailroadVehicle, {
    return A == B->GetRailroadVehicleMovementComponent();
});

Step(AFGBuildableRailroadTrack, UFGRailroadTrackConnectionComponent, {
	return A->GetTrackGraphID() == B->GetTrack()->GetTrackGraphID();
})
Step(UFGRailroadTrackConnectionComponent, AFGBuildableRailroadTrack, {
    return  B->GetConnection(0) && A->GetTrack()->GetTrackGraphID() == B->GetConnection(0)->GetTrack()->GetTrackGraphID();
})

Step(UFGRailroadTrackConnectionComponent, UFGRailroadTrackConnectionComponent, {
	return A->GetTrack()->GetTrackGraphID() == B->GetTrack()->GetTrackGraphID();
})

Step(UFGRailroadTrackConnectionComponent, AFGBuildableRailroadSignal, {
    return B->GetGuardedConnections().Num() > 0 && A->GetTrack()->GetTrackGraphID() == B->GetGuardedConnections()[0]->GetTrack()->GetTrackGraphID();
})
Step(AFGBuildableRailroadSignal, UFGRailroadTrackConnectionComponent, {
    return A->GetGuardedConnections().Num() > 0 && A->GetGuardedConnections()[0]->GetTrack()->GetTrackGraphID() == B->GetTrack()->GetTrackGraphID();
})

Step(UFGRailroadTrackConnectionComponent, AFGBuildableRailroadStation, {
    return A->GetTrack()->GetTrackGraphID() == B->GetTrackGraphID();
})
Step(AFGBuildableRailroadStation, UFGRailroadTrackConnectionComponent, {
    return A->GetTrackGraphID() == B->GetTrack()->GetTrackGraphID();
})

Step(AFGVehicle, AFGBuildableDockingStation, {
	return B->GetDockedActor() == A;
})
Step(AFGBuildableDockingStation, AFGVehicle, {
	return A->GetDockedActor() == B;
})
