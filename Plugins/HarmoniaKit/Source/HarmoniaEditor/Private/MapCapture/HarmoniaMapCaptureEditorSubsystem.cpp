// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaMapCaptureEditorSubsystem.cpp
 * @brief Editor subsystem implementation for map capture operations
 */

#include "MapCapture/HarmoniaMapCaptureEditorSubsystem.h"
#include "MapCapture/HarmoniaMapCaptureVolume.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "LevelEditor.h"
#include "ScopedTransaction.h"
#include "EditorViewportClient.h"

//=============================================================================
// Subsystem Lifecycle
//=============================================================================

void UHarmoniaMapCaptureEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Map Capture Editor Subsystem initialized"));

	// Register editor commands for toolbar/menu access
	RegisterEditorCommands();
}

void UHarmoniaMapCaptureEditorSubsystem::Deinitialize()
{
	// Clean up editor commands before shutdown
	UnregisterEditorCommands();

	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Map Capture Editor Subsystem deinitialized"));

	Super::Deinitialize();
}

//=============================================================================
// Capture Operations
//=============================================================================

void UHarmoniaMapCaptureEditorSubsystem::CaptureAllVolumesInLevel()
{
	// Find all capture volumes in the current level
	TArray<AHarmoniaMapCaptureVolume*> Volumes = GetAllCaptureVolumes();

	if (Volumes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Harmonia] No map capture volumes found in level!"));
		return;
	}

	// Process all found volumes
	BatchCaptureVolumes(Volumes);
}

TArray<AHarmoniaMapCaptureVolume*> UHarmoniaMapCaptureEditorSubsystem::GetAllCaptureVolumes()
{
	TArray<AHarmoniaMapCaptureVolume*> Volumes;

	// Get the current editor world
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return Volumes;
	}

	// Iterate through all HarmoniaMapCaptureVolume actors in the world
	for (TActorIterator<AHarmoniaMapCaptureVolume> It(World); It; ++It)
	{
		Volumes.Add(*It);
	}

	return Volumes;
}

//=============================================================================
// Volume Creation
//=============================================================================

AHarmoniaMapCaptureVolume* UHarmoniaMapCaptureEditorSubsystem::CreateCaptureVolumeAtViewportCenter()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return nullptr;
	}

	// Get viewport camera location for placement
	FVector ViewportLocation = FVector::ZeroVector;
	FRotator ViewportRotation = FRotator::ZeroRotator;

	if (GEditor->GetActiveViewport())
	{
		FViewportClient* ViewportClient = GEditor->GetActiveViewport()->GetClient();
		if (FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(ViewportClient))
		{
			ViewportLocation = EditorViewportClient->GetViewLocation();
			ViewportRotation = EditorViewportClient->GetViewRotation();
		}
	}

	// Create undo transaction for the spawn operation
	FScopedTransaction Transaction(FText::FromString(TEXT("Create Map Capture Volume")));

	// Configure spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the capture volume actor
	AHarmoniaMapCaptureVolume* NewVolume = World->SpawnActor<AHarmoniaMapCaptureVolume>(
		AHarmoniaMapCaptureVolume::StaticClass(),
		ViewportLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewVolume)
	{
		UE_LOG(LogTemp, Log, TEXT("[Harmonia] Created new map capture volume at %s"), *ViewportLocation.ToString());

		// Select the newly created volume in the editor
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(NewVolume, true, true);
	}

	return NewVolume;
}

//=============================================================================
// Batch Processing
//=============================================================================

void UHarmoniaMapCaptureEditorSubsystem::BatchCaptureVolumes(const TArray<AHarmoniaMapCaptureVolume*>& Volumes)
{
	if (Volumes.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Starting batch capture of %d volumes..."), Volumes.Num());

	// Create undo transaction for the batch operation
	FScopedTransaction Transaction(FText::FromString(TEXT("Batch Capture Map Volumes")));

	int32 SuccessCount = 0;
	int32 FailureCount = 0;

	// Process each volume sequentially
	for (int32 i = 0; i < Volumes.Num(); ++i)
	{
		AHarmoniaMapCaptureVolume* Volume = Volumes[i];
		if (!Volume)
		{
			FailureCount++;
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("[Harmonia] [%d/%d] Capturing volume: %s"),
			i + 1, Volumes.Num(), *Volume->GetName());

		try
		{
			// Execute capture for this volume
			Volume->CaptureMap();
			SuccessCount++;
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("[Harmonia] Failed to capture volume: %s"), *Volume->GetName());
			FailureCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Batch capture complete. Success: %d, Failed: %d"),
		SuccessCount, FailureCount);
}

//=============================================================================
// Editor Commands
//=============================================================================

void UHarmoniaMapCaptureEditorSubsystem::RegisterEditorCommands()
{
	// TODO: Register editor commands and menu items for map capture
	// - Add toolbar button for quick capture
	// - Add menu entries in Tools menu
}

void UHarmoniaMapCaptureEditorSubsystem::UnregisterEditorCommands()
{
	// TODO: Unregister all editor commands
	// Clean up registered commands and menu items
}
