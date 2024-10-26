#include "FicsItNetworksDocumentation.h"

#include "CommandLine.h"
#include "CoreDelegates.h"
#include "FicsItReflection.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "FFicsItNetworksDocumentationModule"

DEFINE_LOG_CATEGORY(LogFicsItNetworksDocumentation)

void FFicsItNetworksDocumentationModule::StartupModule() {
    FFicsItReflectionModule::OnReflectionInitialized.AddLambda([]() {
        if (FParse::Param(FCommandLine::Get(), TEXT("FINGenDocAndQuit"))) {
            UWorld* World = GEngine->GetCurrentPlayWorld();
            GEngine->Exec(World, TEXT("FINGenLuaDoc"));
            GEngine->Exec(World, TEXT("FINGenJsonDoc"));
            GEngine->Exec(World, TEXT("quit"));
        }
    });
}

void FFicsItNetworksDocumentationModule::ShutdownModule() {
    
}


#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItNetworksDocumentationModule, FicsItNetworksDocumentation)