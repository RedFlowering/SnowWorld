// Copyright 2025 Snow Game Studio.

#include "Macro/HarmoniaMacroGenerator.h"
#include "Settings/HarmoniaProjectSettings.h"
#include "Interfaces/IPluginManager.h"

void FHarmoniaMacroGenerator::GenerateMacroHeaderFromConfig()
{
    if (!UObjectInitialized())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HarmoniaMacroGenerator] Skipped generation — UObject system is uninitialized."));
        return;
    }

    const UHarmoniaProjectSettings* Settings = GetDefault<UHarmoniaProjectSettings>();
    if (!IsValid(Settings) || Settings->Tables.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HarmoniaMacroGenerator] No tables configured in HarmoniaProjectSettings."));
        return;
    }

    FString Output;
    Output += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    Output += TEXT("#pragma once\n\n");
    Output += TEXT("#include \"Managers/HarmoniaLoadManager.h\"\n\n");
    Output += TEXT("#define HARMONIALOADMANAGER() UHarmoniaLoadManager::Get()\n\n");

    for (const FHarmoniaDataTableSetting& Table : Settings->Tables)
    {
        if (!Table.FunctionName.IsEmpty())
        {
            const FString FunctionName = Table.FunctionName;
            const FString MacroName = FString::Printf(TEXT("GET%sDATATABLE"), *FunctionName.ToUpper());
            const FString Line = FString::Printf(TEXT("#define %s() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT(\"%s\"))\n"), *MacroName, *FunctionName);
            Output += Line;
        }
    }

    const FString OutputPath = FPaths::Combine(
        IPluginManager::Get().FindPlugin("HarmoniaKit")->GetBaseDir(),
        TEXT("Source/HarmoniaLoadManager/Public/Definitions/HarmoniaDataTableDefinitions.h")
    );

    FFileHelper::SaveStringToFile(Output, *OutputPath);
    UE_LOG(LogTemp, Log, TEXT("[HarmoniaMacroGenerator] Header generated: %s"), *OutputPath);
}

void FHarmoniaMacroGenerator::GenerateFunctionLibraryFromConfig()
{
    if (!UObjectInitialized())
        return;

    const UHarmoniaProjectSettings* Settings = GetDefault<UHarmoniaProjectSettings>();
    if (!IsValid(Settings) || Settings->Tables.Num() == 0)
        return;

    FString Header;
    Header += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    Header += TEXT("#pragma once\n\n");
    Header += TEXT("#include \"Kismet/BlueprintFunctionLibrary.h\"\n");
    Header += TEXT("#include \"HarmoniaLoadManager.h\"\n");
    Header += TEXT("#include \"HarmoniaDataTableBFL.generated.h\"\n\n");
    Header += TEXT("UCLASS()\nclass HARMONIALOADMANAGER_API UHarmoniaDataTableBFL : public UBlueprintFunctionLibrary\n{\n    GENERATED_BODY()\npublic:\n");

    FString Source;
    Source += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    Source += TEXT("#include \"Managers/HarmoniaDataTableBFL.h\"\n\n");

    for (const FHarmoniaDataTableSetting& Table : Settings->Tables)
    {
        if (!Table.FunctionName.IsEmpty())
        {
            const FString FuncName = FString::Printf(TEXT("Get%sDataTable"), *Table.FunctionName);
            Header += FString::Printf(TEXT("    UFUNCTION(BlueprintPure, Category = \"Harmonia|LoadManager\")\n    static UDataTable* %s();\n\n"), *FuncName);
            Source += FString::Printf(TEXT("UDataTable* UHarmoniaDataTableBFL::%s()\n{\n    return UHarmoniaLoadManager::Get()->GetDataTableByKey(TEXT(\"%s\"));\n}\n\n"), *FuncName, *Table.FunctionName);
        }
    }

    Header += TEXT("};\n");

    const FString PluginBaseDir = IPluginManager::Get().FindPlugin("HarmoniaKit")->GetBaseDir();
    const FString HeaderPath = FPaths::Combine(PluginBaseDir, TEXT("Source/HarmoniaLoadManager/Public/Managers/HarmoniaDataTableBFL.h"));
    const FString SourcePath = FPaths::Combine(PluginBaseDir, TEXT("Source/HarmoniaLoadManager/Private/Managers/HarmoniaDataTableBFL.cpp"));

    FFileHelper::SaveStringToFile(Header, *HeaderPath);
    FFileHelper::SaveStringToFile(Source, *SourcePath);
}