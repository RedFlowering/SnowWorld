// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaMapLayerSystem.h"

UHarmoniaMapLayerSystem::UHarmoniaMapLayerSystem()
{
    CurrentLayerIndex = 0;
    bAutoSwitchLayers = true;
    LayerSwitchHysteresis = 50.0f;
    LastPlayerZ = 0.0f;
    TimeSinceLayerSwitch = 0.0f;
}

void UHarmoniaMapLayerSystem::AddLayer(const FMapLayerData& LayerData)
{
    // Check if layer with this index already exists
    int32 ArrayIndex = FindLayerArrayIndex(LayerData.LayerIndex);
    if (ArrayIndex != INDEX_NONE)
    {
        // Update existing
        Layers[ArrayIndex] = LayerData;
    }
    else
    {
        // Add new
        Layers.Add(LayerData);
        SortLayers();
    }
}

bool UHarmoniaMapLayerSystem::RemoveLayer(int32 LayerIndex)
{
    int32 ArrayIndex = FindLayerArrayIndex(LayerIndex);
    if (ArrayIndex != INDEX_NONE)
    {
        Layers.RemoveAt(ArrayIndex);

        // Switch to a valid layer if current was removed
        if (CurrentLayerIndex == LayerIndex && Layers.Num() > 0)
        {
            SetCurrentLayer(Layers[0].LayerIndex);
        }

        return true;
    }
    return false;
}

bool UHarmoniaMapLayerSystem::GetLayer(int32 LayerIndex, FMapLayerData& OutLayer) const
{
    int32 ArrayIndex = FindLayerArrayIndex(LayerIndex);
    if (ArrayIndex != INDEX_NONE)
    {
        OutLayer = Layers[ArrayIndex];
        return true;
    }
    return false;
}

TArray<FMapLayerData> UHarmoniaMapLayerSystem::GetVisibleLayers() const
{
    TArray<FMapLayerData> Result;

    for (const FMapLayerData& Layer : Layers)
    {
        if (Layer.bVisible)
        {
            Result.Add(Layer);
        }
    }

    return Result;
}

void UHarmoniaMapLayerSystem::SetCurrentLayer(int32 LayerIndex)
{
    if (CurrentLayerIndex != LayerIndex)
    {
        int32 ArrayIndex = FindLayerArrayIndex(LayerIndex);
        if (ArrayIndex != INDEX_NONE)
        {
            CurrentLayerIndex = LayerIndex;
            TimeSinceLayerSwitch = 0.0f;
            OnLayerChanged.Broadcast(LayerIndex);

            UE_LOG(LogTemp, Log, TEXT("Switched to layer %d: %s"),
                LayerIndex, *Layers[ArrayIndex].LayerName.ToString());
        }
    }
}

bool UHarmoniaMapLayerSystem::GetCurrentLayerData(FMapLayerData& OutLayer) const
{
    return GetLayer(CurrentLayerIndex, OutLayer);
}

bool UHarmoniaMapLayerSystem::SetLayerVisible(int32 LayerIndex, bool bVisible)
{
    int32 ArrayIndex = FindLayerArrayIndex(LayerIndex);
    if (ArrayIndex != INDEX_NONE)
    {
        Layers[ArrayIndex].bVisible = bVisible;
        return true;
    }
    return false;
}

int32 UHarmoniaMapLayerSystem::GetLayerAtHeight(float ZHeight) const
{
    for (const FMapLayerData& Layer : Layers)
    {
        if (Layer.ContainsHeight(ZHeight))
        {
            return Layer.LayerIndex;
        }
    }

    // Return current layer if no match
    return CurrentLayerIndex;
}

void UHarmoniaMapLayerSystem::UpdateLayerFromPosition(const FVector& WorldPosition)
{
    if (!bAutoSwitchLayers)
    {
        return;
    }

    float ZHeight = WorldPosition.Z;

    // Apply hysteresis to prevent rapid switching
    if (FMath::Abs(ZHeight - LastPlayerZ) < LayerSwitchHysteresis)
    {
        return;
    }

    int32 LayerAtHeight = GetLayerAtHeight(ZHeight);
    if (LayerAtHeight != CurrentLayerIndex)
    {
        SetCurrentLayer(LayerAtHeight);
        LastPlayerZ = ZHeight;
    }
}

bool UHarmoniaMapLayerSystem::GoToNextLayer()
{
    if (Layers.Num() == 0)
    {
        return false;
    }

    // Find current layer in sorted array
    int32 CurrentArrayIndex = FindLayerArrayIndex(CurrentLayerIndex);
    if (CurrentArrayIndex != INDEX_NONE && CurrentArrayIndex < Layers.Num() - 1)
    {
        SetCurrentLayer(Layers[CurrentArrayIndex + 1].LayerIndex);
        return true;
    }

    return false;
}

bool UHarmoniaMapLayerSystem::GoToPreviousLayer()
{
    if (Layers.Num() == 0)
    {
        return false;
    }

    // Find current layer in sorted array
    int32 CurrentArrayIndex = FindLayerArrayIndex(CurrentLayerIndex);
    if (CurrentArrayIndex > 0)
    {
        SetCurrentLayer(Layers[CurrentArrayIndex - 1].LayerIndex);
        return true;
    }

    return false;
}

TArray<int32> UHarmoniaMapLayerSystem::GetConnectedLayers() const
{
    FMapLayerData CurrentLayer;
    if (GetCurrentLayerData(CurrentLayer))
    {
        return CurrentLayer.ConnectedLayers;
    }

    return TArray<int32>();
}

bool UHarmoniaMapLayerSystem::CanSwitchToLayer(int32 LayerIndex) const
{
    // Check if layer exists
    return FindLayerArrayIndex(LayerIndex) != INDEX_NONE;
}

void UHarmoniaMapLayerSystem::SortLayers()
{
    Layers.Sort([](const FMapLayerData& A, const FMapLayerData& B)
    {
        return A.LayerIndex < B.LayerIndex;
    });
}

int32 UHarmoniaMapLayerSystem::FindLayerArrayIndex(int32 LayerIndex) const
{
    for (int32 i = 0; i < Layers.Num(); ++i)
    {
        if (Layers[i].LayerIndex == LayerIndex)
        {
            return i;
        }
    }
    return INDEX_NONE;
}
