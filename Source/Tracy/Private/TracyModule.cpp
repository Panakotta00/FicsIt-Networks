#include "TracyModule.h"

#include "Misc/CoreDelegates.h"
#include "tracy/Tracy.hpp"

DEFINE_LOG_CATEGORY(LogTracy);
IMPLEMENT_GAME_MODULE(FTracyModule, Tracy);

void FTracyModule::StartupModule() {
	OnBeginFrameHandle = FCoreDelegates::OnBeginFrame.AddLambda([]() {
		FrameMark;
	});
}

void FTracyModule::ShutdownModule() {
	FCoreDelegates::OnBeginFrame.Remove(OnBeginFrameHandle);
}
