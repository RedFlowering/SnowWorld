// Copyright 2025 Snow Game Studio.

#include "HarmoniaLoadManagerEditorModule.h"
#include "Modules/ModuleManager.h"
#include "HarmoniaRegistryAsset.h"
#include "Macro/HarmoniaMacroGenerator.h"

IMPLEMENT_MODULE(FHarmoniaLoadManagerEditor, HarmoniaLoadManagerEditor);

void FHarmoniaLoadManagerEditor::StartupModule()
{
	FCoreUObjectDelegates::OnObjectSaved.AddRaw(this, &FHarmoniaLoadManagerEditor::OnAssetSaved);
}

void FHarmoniaLoadManagerEditor::ShutdownModule()
{
	FCoreUObjectDelegates::OnObjectSaved.RemoveAll(this);
}

void FHarmoniaLoadManagerEditor::OnAssetSaved(UObject* Object)
{
    if (const UHarmoniaRegistryAsset* Registry = Cast<UHarmoniaRegistryAsset>(Object))
    {
        FHarmoniaMacroGenerator::ValidateAndGenerate(Registry);

        UE_LOG(LogTemp, Log, TEXT("[Harmonia] Macro regenerated from RegistryAsset save!"));
    }
}
