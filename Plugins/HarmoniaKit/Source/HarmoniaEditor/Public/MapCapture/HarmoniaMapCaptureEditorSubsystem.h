// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaMapCaptureEditorSubsystem.h
 * @brief Editor subsystem for map capture operations
 * 
 * Provides editor-time tools for capturing orthographic map images
 * from designated volumes in the level.
 */

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "HarmoniaMapCaptureEditorSubsystem.generated.h"

class AHarmoniaMapCaptureVolume;

/**
 * @class UHarmoniaMapCaptureEditorSubsystem
 * @brief Editor subsystem for managing map capture operations
 * 
 * This subsystem provides tools for:
 * - Capturing orthographic map images from designated volumes
 * - Batch processing multiple capture volumes
 * - Creating and managing capture volumes in the editor
 */
UCLASS()
class HARMONIAEDITOR_API UHarmoniaMapCaptureEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Initialize the subsystem.
	 * Registers editor commands and sets up capture infrastructure.
	 * @param Collection - The subsystem collection this belongs to
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Deinitialize the subsystem.
	 * Cleans up resources and unregisters commands.
	 */
	virtual void Deinitialize() override;

	/**
	 * Capture all map capture volumes in the current level.
	 * Iterates through all HarmoniaMapCaptureVolume actors and triggers capture.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
	void CaptureAllVolumesInLevel();

	/**
	 * Get all map capture volumes in the current level.
	 * @return Array of all HarmoniaMapCaptureVolume actors in the level
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
	TArray<AHarmoniaMapCaptureVolume*> GetAllCaptureVolumes();

	/**
	 * Create a new capture volume at the current viewport center.
	 * Spawns a HarmoniaMapCaptureVolume at the editor camera location.
	 * @return The newly created capture volume actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
	AHarmoniaMapCaptureVolume* CreateCaptureVolumeAtViewportCenter();

	/**
	 * Batch capture multiple volumes with progress reporting.
	 * Captures all specified volumes sequentially with logging.
	 * @param Volumes - Array of volumes to capture
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
	void BatchCaptureVolumes(const TArray<AHarmoniaMapCaptureVolume*>& Volumes);

private:
	/**
	 * Register editor commands for map capture operations.
	 */
	void RegisterEditorCommands();

	/**
	 * Unregister all editor commands.
	 */
	void UnregisterEditorCommands();
};
