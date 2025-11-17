// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HarmoniaMapLayerSystem.generated.h"

/**
 * Map layer/floor data for multi-level structures
 */
USTRUCT(BlueprintType)
struct FMapLayerData
{
    GENERATED_BODY()

    // Layer index (0 = ground floor, positive = upper floors, negative = underground)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    int32 LayerIndex = 0;

    // Layer display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    FText LayerName;

    // Layer Z height range (min, max)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    FVector2D HeightRange;

    // Layer map texture (optional separate texture per layer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TObjectPtr<UTexture2D> LayerTexture;

    // Layer color tint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    FLinearColor LayerTint = FLinearColor::White;

    // Whether this layer is visible
    UPROPERTY(BlueprintReadWrite, Category = "Layer")
    bool bVisible = true;

    // Layer connections (stairs, elevators, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TArray<FVector> ConnectionPoints;

    // Connected layer indices
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    TArray<int32> ConnectedLayers;

    FMapLayerData()
    {
        LayerIndex = 0;
        LayerName = FText::GetEmpty();
        HeightRange = FVector2D(0.0f, 300.0f);
        LayerTexture = nullptr;
        LayerTint = FLinearColor::White;
        bVisible = true;
    }

    FMapLayerData(int32 InIndex, const FText& InName, const FVector2D& InHeightRange)
    {
        LayerIndex = InIndex;
        LayerName = InName;
        HeightRange = InHeightRange;
        LayerTexture = nullptr;
        LayerTint = FLinearColor::White;
        bVisible = true;
    }

    // Check if a Z height is within this layer
    bool ContainsHeight(float ZHeight) const
    {
        return ZHeight >= HeightRange.X && ZHeight <= HeightRange.Y;
    }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLayerChanged, int32, NewLayerIndex);

/**
 * System for managing multi-layer/floor maps
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaMapLayerSystem : public UObject
{
    GENERATED_BODY()

public:
    UHarmoniaMapLayerSystem();

    // All layers
    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    TArray<FMapLayerData> Layers;

    // Current active layer
    UPROPERTY(BlueprintReadOnly, Category = "Layer")
    int32 CurrentLayerIndex = 0;

    // Whether to auto-switch layers based on player Z position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    bool bAutoSwitchLayers = true;

    // Layer transition threshold (to prevent rapid switching)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
    float LayerSwitchHysteresis = 50.0f;

    // Delegate called when layer changes
    UPROPERTY(BlueprintAssignable, Category = "Layer|Events")
    FOnLayerChanged OnLayerChanged;

    // Add a layer
    UFUNCTION(BlueprintCallable, Category = "Layer")
    void AddLayer(const FMapLayerData& LayerData);

    // Remove a layer
    UFUNCTION(BlueprintCallable, Category = "Layer")
    bool RemoveLayer(int32 LayerIndex);

    // Get layer by index
    UFUNCTION(BlueprintCallable, Category = "Layer")
    bool GetLayer(int32 LayerIndex, FMapLayerData& OutLayer) const;

    // Get all layers
    UFUNCTION(BlueprintPure, Category = "Layer")
    TArray<FMapLayerData> GetAllLayers() const { return Layers; }

    // Get visible layers
    UFUNCTION(BlueprintCallable, Category = "Layer")
    TArray<FMapLayerData> GetVisibleLayers() const;

    // Set current layer
    UFUNCTION(BlueprintCallable, Category = "Layer")
    void SetCurrentLayer(int32 LayerIndex);

    // Get current layer
    UFUNCTION(BlueprintPure, Category = "Layer")
    int32 GetCurrentLayer() const { return CurrentLayerIndex; }

    // Get current layer data
    UFUNCTION(BlueprintCallable, Category = "Layer")
    bool GetCurrentLayerData(FMapLayerData& OutLayer) const;

    // Set layer visibility
    UFUNCTION(BlueprintCallable, Category = "Layer")
    bool SetLayerVisible(int32 LayerIndex, bool bVisible);

    // Get layer at Z height
    UFUNCTION(BlueprintCallable, Category = "Layer")
    int32 GetLayerAtHeight(float ZHeight) const;

    // Update layer based on player position
    UFUNCTION(BlueprintCallable, Category = "Layer")
    void UpdateLayerFromPosition(const FVector& WorldPosition);

    // Go to next layer (up)
    UFUNCTION(BlueprintCallable, Category = "Layer")
    bool GoToNextLayer();

    // Go to previous layer (down)
    UFUNCTION(BlueprintCallable, Category = "Layer")
    bool GoToPreviousLayer();

    // Get connected layers from current layer
    UFUNCTION(BlueprintCallable, Category = "Layer")
    TArray<int32> GetConnectedLayers() const;

    // Check if can switch to layer
    UFUNCTION(BlueprintPure, Category = "Layer")
    bool CanSwitchToLayer(int32 LayerIndex) const;

    // Sort layers by index
    UFUNCTION(BlueprintCallable, Category = "Layer")
    void SortLayers();

protected:
    // Find layer array index by layer index
    int32 FindLayerArrayIndex(int32 LayerIndex) const;

    // Last player Z position (for hysteresis)
    float LastPlayerZ = 0.0f;

    // Time since last layer switch
    float TimeSinceLayerSwitch = 0.0f;
};
