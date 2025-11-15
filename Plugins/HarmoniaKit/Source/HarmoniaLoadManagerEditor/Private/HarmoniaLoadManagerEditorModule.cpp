// Copyright 2025 Snow Game Studio.

#include "HarmoniaLoadManagerEditorModule.h"
#include "Modules/ModuleManager.h"
#include "HarmoniaRegistryAsset.h"
#include "Macro/HarmoniaMacroGenerator.h"
#include "UObject/ObjectSaveContext.h"

IMPLEMENT_MODULE(FHarmoniaLoadManagerEditor, HarmoniaLoadManagerEditor);

void FHarmoniaLoadManagerEditor::StartupModule()
{
	FCoreUObjectDelegates::OnObjectPreSave.AddRaw(this, &FHarmoniaLoadManagerEditor::HandleObjectPreSave);
}

void FHarmoniaLoadManagerEditor::ShutdownModule()
{
	FCoreUObjectDelegates::OnObjectPreSave.RemoveAll(this);
}

void FHarmoniaLoadManagerEditor::HandleObjectPreSave(UObject* Object, FObjectPreSaveContext Context)
{
    if (Object != nullptr)
    {
        // 历厘等 按眉啊 Harmonia Registry老 锭父 贸府
        if (const UHarmoniaRegistryAsset* Registry = Cast<UHarmoniaRegistryAsset>(Object))
        {
            FHarmoniaMacroGenerator::ValidateAndGenerate(Registry);

            UE_LOG(LogTemp, Log, TEXT("[Harmonia] Macro regenerated from RegistryAsset save! (IsCooking=%d)"),
                Context.IsCooking());
        }
    }
}