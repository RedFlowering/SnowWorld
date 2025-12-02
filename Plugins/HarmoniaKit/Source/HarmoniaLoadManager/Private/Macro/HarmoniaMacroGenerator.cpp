// Copyright 2025 Snow Game Studio.

#include "Macro/HarmoniaMacroGenerator.h"
#include "HarmoniaRegistryAsset.h"
#include "Interfaces/IPluginManager.h"

void FHarmoniaMacroGenerator::ValidateAndGenerate(const UHarmoniaRegistryAsset* Registry)
{
    if (ValidateConfig(Registry))
    {
        GenerateRowIncludesHeader(Registry);
        GenerateMacroHeaderFromRegistry(Registry);
        GenerateFunctionLibraryFromRegistry(Registry);
    }
}

bool FHarmoniaMacroGenerator::ValidateConfig(const UHarmoniaRegistryAsset* Registry)
{
    bool bResult = true;

    if (!IsValid(Registry))
    {
        UE_LOG(LogTemp, Error, TEXT("[Harmonia] Registry not valid."));
        return false;
    }
    if (Registry->Entries.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Harmonia] No Entries configured in UHarmoniaRegistryAsset."));
        bResult = false;
    }

    // Check for duplicate FunctionNames
    TSet<FString> UsedFunctionNames;
    for (const FHarmoniaDataTableEntry& Entry : Registry->Entries)
    {
        if (Entry.FunctionName.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("[Harmonia] Table entry with empty FunctionName. Skipped."));
            bResult = false;
            continue;
        }
        if (UsedFunctionNames.Contains(Entry.FunctionName))
        {
            UE_LOG(LogTemp, Error, TEXT("[Harmonia] Duplicate FunctionName found: %s. Skipped."), *Entry.FunctionName);
            bResult = false;
            continue;
        }
        UsedFunctionNames.Add(Entry.FunctionName);

		const UDataTable* DataTable = Entry.Table.LoadSynchronous();
		if (!DataTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Harmonia] Failed to load DataTable for: %s. Skipped."), *Entry.FunctionName);
			bResult = false;
			continue;
		}
		const UScriptStruct* RowStruct = DataTable->GetRowStruct();
		if (!RowStruct)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Harmonia] Failed to get RowStruct for: %s. Skipped."), *Entry.FunctionName);
			bResult = false;
			continue;
		}
		
		// 디버그: 실제 RowStruct 이름 출력
		UE_LOG(LogTemp, Log, TEXT("[Harmonia] DataTable '%s' has RowStruct: %s (StructName: %s)"), 
			*Entry.FunctionName, 
			*RowStruct->GetName(),
			*RowStruct->GetStructCPPName());
    }
    return bResult;
}

void FHarmoniaMacroGenerator::GenerateRowIncludesHeader(const UHarmoniaRegistryAsset* Registry)
{
    if (!Registry) return;

    FString Output;
    Output += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    Output += TEXT("#pragma once\n\n");
    Output += TEXT("#include \"Engine/DataTable.h\"\n\n");
    Output += TEXT("// This file is auto-generated. Do not modify manually.\n");
    Output += TEXT("// Any manual changes will be overwritten by the code generator.\n");
    Output += TEXT("// Contains include directives for all DataTable row struct headers.\n\n");

    // 중복 방지를 위한 Set
    TSet<FString> IncludedHeaders;

    for (const FHarmoniaDataTableEntry& Entry : Registry->Entries)
    {
        const UDataTable* DataTable = Entry.Table.LoadSynchronous();
        if (!DataTable) continue;

        const UScriptStruct* RowStruct = DataTable->GetRowStruct();
        if (!RowStruct) continue;

        // RowStruct가 정의된 헤더 파일 경로 추출
        // GetMetaData("ModuleRelativePath")를 통해 상대 경로 획득
        FString ModuleRelativePath = RowStruct->GetMetaData(TEXT("ModuleRelativePath"));
        
        if (!ModuleRelativePath.IsEmpty() && !IncludedHeaders.Contains(ModuleRelativePath))
        {
            IncludedHeaders.Add(ModuleRelativePath);
            Output += FString::Printf(TEXT("#include \"%s\"\n"), *ModuleRelativePath);
            UE_LOG(LogTemp, Log, TEXT("[Harmonia] Added RowStruct header: %s (from %s)"), *ModuleRelativePath, *RowStruct->GetName());
        }
        else if (ModuleRelativePath.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("[Harmonia] Could not find header path for RowStruct: %s"), *RowStruct->GetName());
        }
    }

    Output += TEXT("\n// Auto-generated macro definitions\n");
    Output += TEXT("#include \"Definitions/HarmoniaMacroDefinitions.h\"\n");

    const FString OutputPath = FPaths::Combine(
        IPluginManager::Get().FindPlugin("HarmoniaKit")->GetBaseDir(),
        TEXT("Source/HarmoniaLoadManager/Public/HarmoniaRowIncludes.h")
    );

    FFileHelper::SaveStringToFile(Output, *OutputPath);
    UE_LOG(LogTemp, Log, TEXT("[HarmoniaMacroGenerator] Row includes header generated: %s"), *OutputPath);
}

void FHarmoniaMacroGenerator::GenerateMacroHeaderFromRegistry(const UHarmoniaRegistryAsset* Registry)
{
    if (!Registry) return;

    FString Output;
    Output += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    Output += TEXT("#pragma once\n\n");
    Output += TEXT("#include \"HarmoniaLoadManager.h\"\n");
    Output += TEXT("#include \"HarmoniaRowIncludes.h\"\n\n");
    Output += TEXT("// This file is auto-generated. Do not modify manually.\n");
    Output += TEXT("// Any manual changes will be overwritten by the code generator.\n\n");
    Output += TEXT("#define HARMONIALOADMANAGER() UHarmoniaLoadManager::Get()\n\n");

    for (const FHarmoniaDataTableEntry& Entry : Registry->Entries)
    {
        const UDataTable* DataTable = Entry.Table.LoadSynchronous();

        if (!Entry.FunctionName.IsEmpty() && DataTable)
        {
            FString FunctionName = Entry.FunctionName;
            FString MacroGet = FString::Printf(TEXT("GET%sDATATABLE"), *FunctionName.ToUpper());
            FString MacroFind = FString::Printf(TEXT("FIND%sROW"), *FunctionName.ToUpper());
            FString MacroAll = FString::Printf(TEXT("GETALL%sROWS"), *FunctionName.ToUpper());

            // Extracting RowStruct names - GetStructCPPName()으로 정확한 C++ 이름 획득
            FString StructName = TEXT("UNKNOWN");

			if (const UScriptStruct* RowStruct = DataTable->GetRowStruct())
			{
                // GetStructCPPName()은 이미 F 접두사가 포함된 정확한 C++ 구조체 이름을 반환
                StructName = RowStruct->GetStructCPPName();
                
                UE_LOG(LogTemp, Verbose, TEXT("[Harmonia] Macro Gen - %s uses RowStruct: %s"), *FunctionName, *StructName);

                // Get<DataTable>()
                Output += FString::Printf(TEXT("#define %s() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT(\"%s\"))\n"), *MacroGet, *FunctionName);

                //// Find<Key>Row(RowName)
                //Output += FString::Printf(
                //    TEXT("#define %s(RowName) static_cast<const %s*>(%s() ? %s()->FindRow<%s>(RowName, TEXT(\"%s\")) : nullptr)\n"),
                //    *MacroFind, *StructName, *MacroGet, *MacroGet, *StructName, *MacroFind
                //);

                //// GetAll<Key>Rows(OutRows)
                //Output += FString::Printf(
                //    TEXT("#define %s(OutRows) \\\n\tdo { \\\n\t\tOutRows.Empty(); \\\n\t\tif (UDataTable* Table = %s()) { \\\n\t\t\tfor (const auto& Elem : Table->GetRowMap()) { \\\n\t\t\t\tif (const %s* Row = reinterpret_cast<const %s*>(Elem.Value)) { \\\n\t\t\t\t\tOutRows.Add(Row); \\\n\t\t\t\t} \\\n\t\t\t} \\\n\t\t} \\\n\t} while(0)\n\n"),
                //    *MacroAll, *MacroGet, *StructName, *StructName
                //);
			}
        }
    }

    const FString OutputPath = FPaths::Combine(
        IPluginManager::Get().FindPlugin("HarmoniaKit")->GetBaseDir(),
        TEXT("Source/HarmoniaLoadManager/Public/Definitions/HarmoniaMacroDefinitions.h")
    );

    FFileHelper::SaveStringToFile(Output, *OutputPath);
    UE_LOG(LogTemp, Log, TEXT("[HarmoniaMacroGenerator] Macro header generated: %s"), *OutputPath);
}

void FHarmoniaMacroGenerator::GenerateFunctionLibraryFromRegistry(const UHarmoniaRegistryAsset* Registry)
{
    if (!Registry) return;

    FString HeaderOutput;
    HeaderOutput += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    HeaderOutput += TEXT("#pragma once\n\n");
    HeaderOutput += TEXT("#include \"HarmoniaRowIncludes.h\"\n");
    HeaderOutput += TEXT("#include \"Kismet/BlueprintFunctionLibrary.h\"\n");
    HeaderOutput += TEXT("#include \"HarmoniaLoadManager.h\"\n\n");
    HeaderOutput += TEXT("#include \"HarmoniaDataTableBFL.generated.h\"\n\n");
    HeaderOutput += TEXT("// This file is auto-generated. Do not modify manually.\n");
    HeaderOutput += TEXT("// Any manual changes will be overwritten by the code generator.\n\n");
    HeaderOutput += TEXT("UCLASS()\nclass HARMONIALOADMANAGER_API UHarmoniaDataTableBFL : public UBlueprintFunctionLibrary\n{\n    GENERATED_BODY()\n\npublic:\n");

    FString SourceOutput;
    SourceOutput += TEXT("// Copyright 2025 Snow Game Studio.\n\n");
    SourceOutput += TEXT("#include \"HarmoniaDataTableBFL.h\"\n\n");
    SourceOutput += TEXT("// This file is auto-generated. Do not modify manually.\n");
    SourceOutput += TEXT("// Any manual changes will be overwritten by the code generator.\n\n");

    for (const FHarmoniaDataTableEntry& Entry : Registry->Entries)
    {
        const UDataTable* DataTable = Entry.Table.LoadSynchronous();

        if (!Entry.FunctionName.IsEmpty() && DataTable)
        {
            const FString Key = Entry.FunctionName;

            // Extracting RowStruct names - GetStructCPPName()으로 정확한 C++ 이름 획득
            FString StructName = TEXT("UNKNOWN");

			if (const UScriptStruct* RowStruct = DataTable->GetRowStruct())
			{
                // GetStructCPPName()은 이미 F 접두사가 포함된 정확한 C++ 구조체 이름을 반환
                StructName = RowStruct->GetStructCPPName();
                
                UE_LOG(LogTemp, Verbose, TEXT("[Harmonia] BFL Gen - %s uses RowStruct: %s"), *Key, *StructName);

				// BFL function name
				const FString FuncGet = FString::Printf(TEXT("Get%sDataTable"), *Key);
				const FString FuncFind = FString::Printf(TEXT("Find%sRow"), *Key);
				const FString FuncAll = FString::Printf(TEXT("GetAll%sRows"), *Key);

				// Automatically generate BFL function signatures in headers
				HeaderOutput += FString::Printf(TEXT("    UFUNCTION(BlueprintPure, Category = \"Harmonia|LoadManager\")\n    static UDataTable* %s();\n\n"), *FuncGet);
				//HeaderOutput += FString::Printf(TEXT("    UFUNCTION(BlueprintPure, Category = \"Harmonia|LoadManager\")\n    static %s %s(FName RowName);\n\n"), *StructName, *FuncFind);
				//HeaderOutput += FString::Printf(TEXT("    UFUNCTION(BlueprintPure, Category = \"Harmonia|LoadManager\")\n    static void %s(TArray<%s>& OutRows);\n\n"), *FuncAll, *StructName);

				// Implementing CPP functions
				SourceOutput += FString::Printf(TEXT("UDataTable* UHarmoniaDataTableBFL::%s()\n{\n    return HARMONIALOADMANAGER()->GetDataTableByKey(TEXT(\"%s\"));\n}\n\n"), *FuncGet, *Key);

				//// Find<Key>Row
				//SourceOutput += FString::Printf(
				//	TEXT("%s UHarmoniaDataTableBFL::%s(FName RowName)\n{\n    if (UDataTable* Table = %s())\n    {\n        return *Table->FindRow<%s>(RowName, TEXT(\"%s\"));\n    }\n    return %s();\n}\n\n"),
				//	*StructName, *FuncFind, *FuncGet, *StructName, *FuncFind, *StructName
				//);

				//// GetAll<Key>Rows
				//SourceOutput += FString::Printf(
				//	TEXT("void UHarmoniaDataTableBFL::%s(TArray<%s>& OutRows)\n{\n    OutRows.Empty();\n    if (UDataTable* Table = %s())\n    {\n        for (const auto& Elem : Table->GetRowMap())\n        {\n            if (%s* Row = reinterpret_cast<%s*>(Elem.Value))\n            {\n                OutRows.Add(*Row);\n            }\n        }\n    }\n}\n\n"),
				//	*FuncAll, *StructName, *FuncGet, *StructName, *StructName
				//);
			}
		}
    }

    HeaderOutput += "};\n";

    const FString PluginDir = IPluginManager::Get().FindPlugin("HarmoniaKit")->GetBaseDir();
    const FString HeaderPath = FPaths::Combine(PluginDir, TEXT("Source/HarmoniaLoadManager/Public/HarmoniaDataTableBFL.h"));
    const FString SourcePath = FPaths::Combine(PluginDir, TEXT("Source/HarmoniaLoadManager/Private/HarmoniaDataTableBFL.cpp"));

    FFileHelper::SaveStringToFile(HeaderOutput, *HeaderPath);
    FFileHelper::SaveStringToFile(SourceOutput, *SourcePath);

    UE_LOG(LogTemp, Log, TEXT("[HarmoniaMacroGenerator] Function library generated: %s and %s"), *HeaderPath, *SourcePath);
}