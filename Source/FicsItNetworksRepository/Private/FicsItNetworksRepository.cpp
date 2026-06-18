#include "FicsItNetworksRepository.h"

#include "FINRepoEndpoint.h"
#include "GameDelegates.h"
#include "Interfaces/IHttpRequest.h"

#define LOCTEXT_NAMESPACE "FFicsItNetworksRepositoryModule"

DEFINE_LOG_CATEGORY(LogFicsItNetworksRepo)

void FFicsItNetworksRepositoryModule::StartupModule() {}

void FFicsItNetworksRepositoryModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItNetworksRepositoryModule, FicsItNetworksRepository)