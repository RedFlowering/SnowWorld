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
        // ����� ��ü�� Harmonia Registry�� ���� ó��
        if (const UHarmoniaRegistryAsset* Registry = Cast<UHarmoniaRegistryAsset>(Object))
        {
            FHarmoniaMacroGenerator::ValidateAndGenerate(Registry);

            UE_LOG(LogTemp, Log, TEXT("[Harmonia] Macro regenerated from RegistryAsset save! (IsCooking=%d)"),
                Context.IsCooking());
        }
    }
}