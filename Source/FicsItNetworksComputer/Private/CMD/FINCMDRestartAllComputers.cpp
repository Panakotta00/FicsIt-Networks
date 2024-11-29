#include "CoreMinimal.h"
#include "AkGameplayStatics.h"
#include "FicsItNetworksComputer.h"
#include "FINComputerCase.h"
#include "Kismet/GameplayStatics.h"

bool ExecCMDRestartAllComputers(UWorld* World, const TCHAR* Command, FOutputDevice& Ar) {
	if (FParse::Command(&Command, TEXT("FINRestartAllComputers"))) {
		UE_LOG(LogFicsItNetworksComputer, Display, TEXT("Restarting all FicsIt-Networks Computers..."));

		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(World, AFINComputerCase::StaticClass(), actors);
		for (AActor* actor : actors) {
			AFINComputerCase* computer = Cast<AFINComputerCase>(actor);
			if (!computer || !computer->Kernel) continue;
			computer->Kernel->Reset();
		}

		return true;
	}
	return false;
}

[[maybe_unused]] static FStaticSelfRegisteringExec SelfRegisterCMDRestartAllComputers(&ExecCMDRestartAllComputers);