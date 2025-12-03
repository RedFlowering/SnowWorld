// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "HarmoniaMapRegionDefinitions.generated.h"

/**
 * Map region/zone data
 */
USTRUCT(BlueprintType)
struct FMapRegionData
{
    GENERATED_BODY()

    // Region ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FName RegionID;

    // Region display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FText RegionName;

    // Region description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FText RegionDescription;

    // Region bounds (world space)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FBox RegionBounds;

    // Region color (for map overlay)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FLinearColor RegionColor = FLinearColor::Blue;

    // Region icon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    TObjectPtr<UTexture2D> RegionIcon;

    // Region type tag
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FGameplayTag RegionTag;

    // Whether this region has been discovered
    UPROPERTY(BlueprintReadWrite, Category = "Region")
    bool bDiscovered = false;

    // Discovery time
    UPROPERTY(BlueprintReadOnly, Category = "Region")
    float DiscoveryTime = 0.0f;

    // Custom map texture for this region (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    TObjectPtr<UTexture2D> DetailMapTexture;

    // Region level (for hierarchical regions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    int32 RegionLevel = 0;

    // Parent region (for sub-regions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Region")
    FName ParentRegionID;

    FMapRegionData()
    {
        RegionID = NAME_None;
        RegionName = FText::GetEmpty();
        RegionDescription = FText::GetEmpty();
        RegionBounds = FBox(ForceInit);
        RegionColor = FLinearColor::Blue;
        RegionIcon = nullptr;
        RegionTag = FGameplayTag();
        bDiscovered = false;
        DiscoveryTime = 0.0f;
        DetailMapTexture = nullptr;
        RegionLevel = 0;
        ParentRegionID = NAME_None;
    }

    // Check if a point is within this region
    bool ContainsPoint(const FVector& WorldPosition) const
    {
        return RegionBounds.IsValid && RegionBounds.IsInside(WorldPosition);
    }
};

/**
 * Custom marker data (player-created)
 */
USTRUCT(BlueprintType)
struct FCustomMarkerData
{
    GENERATED_BODY()

    // Marker ID
    UPROPERTY(BlueprintReadWrite, Category = "Marker")
    FGuid MarkerID;

    // World position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    FVector WorldPosition;

    // Marker name/label
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    FText MarkerLabel;

    // Marker note/description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    FText MarkerNote;

    // Marker icon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    TObjectPtr<UTexture2D> Icon;

    // Marker color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    FLinearColor Color = FLinearColor::White;

    // Marker type tag
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    FGameplayTag MarkerTag;

    // Creation time
    UPROPERTY(BlueprintReadOnly, Category = "Marker")
    float CreationTime = 0.0f;

    // Creator player ID (for multiplayer)
    UPROPERTY(BlueprintReadWrite, Category = "Marker")
    int32 CreatorPlayerID = -1;

    // Creator player name
    UPROPERTY(BlueprintReadWrite, Category = "Marker")
    FString CreatorPlayerName;

    // Whether this marker is shared with party/friends
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
    bool bShared = false;

    FCustomMarkerData()
    {
        MarkerID = FGuid::NewGuid();
        WorldPosition = FVector::ZeroVector;
        MarkerLabel = FText::GetEmpty();
        MarkerNote = FText::GetEmpty();
        Icon = nullptr;
        Color = FLinearColor::White;
        MarkerTag = FGameplayTag();
        CreationTime = 0.0f;
        CreatorPlayerID = -1;
        CreatorPlayerName = TEXT("");
        bShared = false;
    }
};

/**
 * Data asset for region definitions
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaMapRegionDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Regions defined for this map
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Regions")
    TArray<FMapRegionData> Regions;

    // Get region by ID
    UFUNCTION(BlueprintCallable, Category = "Regions")
    bool GetRegion(FName RegionID, FMapRegionData& OutRegion) const;

    // Get region at world position
    UFUNCTION(BlueprintCallable, Category = "Regions")
    bool GetRegionAtPosition(const FVector& WorldPosition, FMapRegionData& OutRegion) const;

    // Get all regions containing a point
    UFUNCTION(BlueprintCallable, Category = "Regions")
    TArray<FMapRegionData> GetRegionsAtPosition(const FVector& WorldPosition) const;
};
