#pragma once

#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "FINConfigurationStruct.generated.h"

struct FFINConfigurationStruct_LogViewer;

USTRUCT(BlueprintType)
struct FFINConfigurationStruct_LogViewer {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool TextLog;

    UPROPERTY(BlueprintReadWrite)
    bool TextLogTimestamp;

    UPROPERTY(BlueprintReadWrite)
    bool TextLogVerbosity;

    UPROPERTY(BlueprintReadWrite)
    bool TextLogMultilineAlign;
};

/* Struct generated from Mod Configuration Asset '/FicsItNetworks/FIN_Configuration' */
USTRUCT(BlueprintType)
struct FFINConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FFINConfigurationStruct_LogViewer LogViewer;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FFINConfigurationStruct GetActiveConfig() {
        FFINConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"FicsItNetworks", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FFINConfigurationStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

