// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HarmoniaMapSystemDefinitions.generated.h"

/**
 * Enum for map marker types
 */
UENUM(BlueprintType)
enum class EMapMarkerType : uint8
{
	Player UMETA(DisplayName = "Player"),
	Friend UMETA(DisplayName = "Friend"),
	Ping UMETA(DisplayName = "Ping"),
	Quest UMETA(DisplayName = "Quest"),
	POI UMETA(DisplayName = "Point of Interest"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Struct for map capture settings
 */
USTRUCT(BlueprintType)
struct FMapCaptureSettings
{
	GENERATED_BODY()

	// Bounds for map capture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FBox CaptureBounds;

	// Capture height above the center of the bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	float CaptureHeight = 10000.0f;

	// Resolution of the captured texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FIntPoint TextureResolution = FIntPoint(2048, 2048);

	// FOV for the capture camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	float CameraFOV = 90.0f;

	// Whether to use orthographic projection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	bool bUseOrthographic = true;

	// Orthographic width (if using orthographic projection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture", meta = (EditCondition = "bUseOrthographic"))
	float OrthoWidth = 20000.0f;

	FMapCaptureSettings()
		: CaptureBounds(FBox(ForceInit))
		, CaptureHeight(10000.0f)
		, TextureResolution(2048, 2048)
		, CameraFOV(90.0f)
		, bUseOrthographic(true)
		, OrthoWidth(20000.0f)
	{}
};

/**
 * Struct for captured map data
 */
USTRUCT(BlueprintType)
struct FMapCaptureData
{
	GENERATED_BODY()

	// Name/ID of the captured map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FString MapName;

	// World bounds covered by this map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FBox WorldBounds;

	// Captured texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TObjectPtr<UTexture2D> MapTexture;

	// Minimap texture (optional, smaller version)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TObjectPtr<UTexture2D> MinimapTexture;

	// Map scale (world units per pixel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float WorldUnitsPerPixel = 10.0f;

	FMapCaptureData()
		: MapName(TEXT(""))
		, WorldBounds(FBox(ForceInit))
		, MapTexture(nullptr)
		, MinimapTexture(nullptr)
		, WorldUnitsPerPixel(10.0f)
	{}

	// Convert world position to map UV coordinates
	FVector2D WorldToMapUV(const FVector& WorldPosition) const
	{
		if (!WorldBounds.IsValid)
		{
			return FVector2D::ZeroVector;
		}

		FVector LocalPos = WorldPosition - WorldBounds.Min;
		FVector BoundsSize = WorldBounds.GetSize();

		return FVector2D(
			BoundsSize.X > 0 ? LocalPos.X / BoundsSize.X : 0.0f,
			BoundsSize.Y > 0 ? LocalPos.Y / BoundsSize.Y : 0.0f
		);
	}

	// Convert map UV to world position (Z is preserved from input)
	FVector MapUVToWorld(const FVector2D& UV, float WorldZ = 0.0f) const
	{
		if (!WorldBounds.IsValid)
		{
			return FVector::ZeroVector;
		}

		FVector BoundsSize = WorldBounds.GetSize();
		FVector WorldPos = WorldBounds.Min + FVector(
			UV.X * BoundsSize.X,
			UV.Y * BoundsSize.Y,
			WorldZ
		);

		return WorldPos;
	}
};

/**
 * Struct for explored region data
 */
USTRUCT(BlueprintType)
struct FExploredRegion
{
	GENERATED_BODY()

	// Center of the explored region
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
	FVector Center;

	// Radius of the explored region
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
	float Radius = 1000.0f;

	// Time when this region was explored
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
	float ExploredTime = 0.0f;

	FExploredRegion()
		: Center(FVector::ZeroVector)
		, Radius(1000.0f)
		, ExploredTime(0.0f)
	{}

	FExploredRegion(const FVector& InCenter, float InRadius)
		: Center(InCenter)
		, Radius(InRadius)
		, ExploredTime(0.0f)
	{}

	// Check if a world position is within this explored region
	bool ContainsPoint(const FVector& WorldPosition) const
	{
		return FVector::DistSquared(Center, WorldPosition) <= (Radius * Radius);
	}
};

/**
 * Struct for map location/POI data
 */
USTRUCT(BlueprintType)
struct FMapLocationData
{
	GENERATED_BODY()

	// Location name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FText LocationName;

	// World position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FVector WorldPosition;

	// Icon for this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	TObjectPtr<UTexture2D> Icon;

	// Location type (using gameplay tags for flexibility)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FGameplayTag LocationTag;

	// Whether this location is visible on the map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	bool bVisible = true;

	// Whether this location has been discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	bool bDiscovered = false;

	// Custom data (for quest markers, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
	FString CustomData;

	FMapLocationData()
		: LocationName(FText::GetEmpty())
		, WorldPosition(FVector::ZeroVector)
		, Icon(nullptr)
		, LocationTag(FGameplayTag())
		, bVisible(true)
		, bDiscovered(false)
		, CustomData(TEXT(""))
	{}
};

/**
 * Struct for map ping data
 */
USTRUCT(BlueprintType)
struct FMapPingData
{
	GENERATED_BODY()

	// World position of the ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	FVector WorldPosition;

	// Player who created this ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	FString PlayerName;

	// Player ID who created this ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	int32 PlayerId = -1;

	// Time when this ping was created
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	float CreationTime = 0.0f;

	// Lifetime of the ping (in seconds, 0 = permanent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	float Lifetime = 5.0f;

	// Ping type/color identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	FGameplayTag PingTag;

	FMapPingData()
		: WorldPosition(FVector::ZeroVector)
		, PlayerName(TEXT(""))
		, PlayerId(-1)
		, CreationTime(0.0f)
		, Lifetime(5.0f)
		, PingTag(FGameplayTag())
	{}

	FMapPingData(const FVector& InPosition, const FString& InPlayerName, int32 InPlayerId)
		: WorldPosition(InPosition)
		, PlayerName(InPlayerName)
		, PlayerId(InPlayerId)
		, CreationTime(0.0f)
		, Lifetime(5.0f)
		, PingTag(FGameplayTag())
	{}

	// Check if this ping is expired
	bool IsExpired(float CurrentTime) const
	{
		if (Lifetime <= 0.0f)
		{
			return false; // Permanent ping
		}
		return (CurrentTime - CreationTime) >= Lifetime;
	}
};

/**
 * Data asset for map configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaMapDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Captured map data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FMapCaptureData CapturedMapData;

	// Static POIs for this map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TArray<FMapLocationData> StaticLocations;

	// Exploration radius (how far around the player is revealed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
	float ExplorationRadius = 1000.0f;

	// How often to update exploration (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration")
	float ExplorationUpdateInterval = 1.0f;

	// Default ping lifetime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	float DefaultPingLifetime = 5.0f;

	// Maximum number of active pings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ping")
	int32 MaxActivePings = 10;
};
