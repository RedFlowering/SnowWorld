// Copyright 2025 Snow Game Studio.

#pragma once

class UHarmoniaRegistryAsset;

class HARMONIALOADMANAGER_API FHarmoniaMacroGenerator
{
public:
    static void ValidateAndGenerate(const UHarmoniaRegistryAsset* Registry);

private:
    static bool ValidateConfig(const UHarmoniaRegistryAsset* Registry);
    static void GenerateRowIncludesHeader(const UHarmoniaRegistryAsset* Registry);
    static void GenerateMacroHeaderFromRegistry(const UHarmoniaRegistryAsset* Registry);
    static void GenerateFunctionLibraryFromRegistry(const UHarmoniaRegistryAsset* Registry);
};