// Copyright 2025 Snow Game Studio.

#include "MapCapture/HarmoniaMapCaptureEditorSubsystem.h"
#include "MapCapture/HarmoniaMapCaptureVolume.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "LevelEditor.h"
#include "ScopedTransaction.h"
#include "EditorViewportClient.h"

void UHarmoniaMapCaptureEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("HarmoniaMapCaptureEditorSubsystem initialized"));

    RegisterEditorCommands();
}

void UHarmoniaMapCaptureEditorSubsystem::Deinitialize()
{
    UnregisterEditorCommands();

    UE_LOG(LogTemp, Log, TEXT("HarmoniaMapCaptureEditorSubsystem deinitialized"));

    Super::Deinitialize();
}

void UHarmoniaMapCaptureEditorSubsystem::CaptureAllVolumesInLevel()
{
    TArray<AHarmoniaMapCaptureVolume*> Volumes = GetAllCaptureVolumes();

    if (Volumes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No map capture volumes found in level!"));
        return;
    }

    BatchCaptureVolumes(Volumes);
}

TArray<AHarmoniaMapCaptureVolume*> UHarmoniaMapCaptureEditorSubsystem::GetAllCaptureVolumes()
{
    TArray<AHarmoniaMapCaptureVolume*> Volumes;

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return Volumes;
    }

    for (TActorIterator<AHarmoniaMapCaptureVolume> It(World); It; ++It)
    {
        Volumes.Add(*It);
    }

    return Volumes;
}

AHarmoniaMapCaptureVolume* UHarmoniaMapCaptureEditorSubsystem::CreateCaptureVolumeAtViewportCenter()
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return nullptr;
    }

    // Get viewport camera location
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

    // Spawn capture volume
    FScopedTransaction Transaction(FText::FromString(TEXT("Create Map Capture Volume")));

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AHarmoniaMapCaptureVolume* NewVolume = World->SpawnActor<AHarmoniaMapCaptureVolume>(
        AHarmoniaMapCaptureVolume::StaticClass(),
        ViewportLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (NewVolume)
    {
        UE_LOG(LogTemp, Log, TEXT("Created new map capture volume at %s"), *ViewportLocation.ToString());

        // Select the new volume
        GEditor->SelectNone(true, true);
        GEditor->SelectActor(NewVolume, true, true);
    }

    return NewVolume;
}

void UHarmoniaMapCaptureEditorSubsystem::BatchCaptureVolumes(const TArray<AHarmoniaMapCaptureVolume*>& Volumes)
{
    if (Volumes.Num() == 0)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Starting batch capture of %d volumes..."), Volumes.Num());

    FScopedTransaction Transaction(FText::FromString(TEXT("Batch Capture Map Volumes")));

    int32 SuccessCount = 0;
    int32 FailureCount = 0;

    for (int32 i = 0; i < Volumes.Num(); ++i)
    {
        AHarmoniaMapCaptureVolume* Volume = Volumes[i];
        if (!Volume)
        {
            FailureCount++;
            continue;
        }

        UE_LOG(LogTemp, Log, TEXT("[%d/%d] Capturing volume: %s"),
            i + 1, Volumes.Num(), *Volume->GetName());

        try
        {
            Volume->CaptureMap();
            SuccessCount++;
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to capture volume: %s"), *Volume->GetName());
            FailureCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Batch capture complete. Success: %d, Failed: %d"),
        SuccessCount, FailureCount);
}

void UHarmoniaMapCaptureEditorSubsystem::RegisterEditorCommands()
{
    // Register editor commands/menu items here if needed
}

void UHarmoniaMapCaptureEditorSubsystem::UnregisterEditorCommands()
{
    // Unregister commands
}
