// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "HarmoniaMapCaptureEditorSubsystem.generated.h"

class AHarmoniaMapCaptureVolume;

/**
 * Editor subsystem for managing map capture operations
 */
UCLASS()
class HARMONIAEDITOR_API UHarmoniaMapCaptureEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Capture all volumes in the current level
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
    void CaptureAllVolumesInLevel();

    // Get all capture volumes in current level
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
    TArray<AHarmoniaMapCaptureVolume*> GetAllCaptureVolumes();

    // Create a new capture volume at viewport center
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
    AHarmoniaMapCaptureVolume* CreateCaptureVolumeAtViewportCenter();

    // Batch process: capture multiple volumes with progress reporting
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
    void BatchCaptureVolumes(const TArray<AHarmoniaMapCaptureVolume*>& Volumes);

private:
    void RegisterEditorCommands();
    void UnregisterEditorCommands();
};
