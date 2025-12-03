// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HarmoniaWorldMarkerComponent.generated.h"

class UWidgetComponent;
class UUserWidget;

/**
 * World marker display mode
 */
UENUM(BlueprintType)
enum class EHarmoniaMarkerDisplayMode : uint8
{
	Always,           // Always show marker
	WhenInRange,      // Show when player is within range
	WhenLooking,      // Show when player is looking at marker
	WhenTracked,      // Show only when actively tracked
	WhenDiscovered    // Show after discovery
};

/**
 * World marker data
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaWorldMarkerData
{
	GENERATED_BODY()

	/** Unique marker ID */
	UPROPERTY(BlueprintReadWrite, Category = "Marker")
	FGuid MarkerID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FText Description;

	/** World location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FVector WorldLocation = FVector::ZeroVector;

	/** Tracked actor (optional) */
	UPROPERTY(BlueprintReadWrite, Category = "Marker")
	TWeakObjectPtr<AActor> TrackedActor;

	/** Icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** Icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FLinearColor IconColor = FLinearColor::White;

	/** Category tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FGameplayTag CategoryTag;

	/** Display mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	EHarmoniaMarkerDisplayMode DisplayMode = EHarmoniaMarkerDisplayMode::Always;

	/** Maximum distance to show marker (0 = infinite) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker", meta = (ClampMin = "0"))
	float MaxDisplayDistance = 0.0f;

	/** Minimum distance to show marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker", meta = (ClampMin = "0"))
	float MinDisplayDistance = 0.0f;

	/** Whether to show distance text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bShowDistance = true;

	/** Whether to clamp to screen edge when off-screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bClampToScreenEdge = true;

	/** Screen edge padding (when clamped) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker", meta = (ClampMin = "0", ClampMax = "100"))
	float ScreenEdgePadding = 50.0f;

	/** Priority for occlusion (higher = shown on top) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	int32 Priority = 0;

	/** Whether marker scales with distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bScaleWithDistance = true;

	/** Minimum scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MinScale = 0.5f;

	/** Maximum scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float MaxScale = 1.5f;

	/** Whether marker fades with distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bFadeWithDistance = true;

	/** Custom data for game-specific use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FString CustomData;

	/** Internal: Current visibility state */
	UPROPERTY(BlueprintReadOnly, Category = "Internal")
	bool bIsCurrentlyVisible = true;

	/** Internal: Current screen position */
	UPROPERTY(BlueprintReadOnly, Category = "Internal")
	FVector2D CurrentScreenPosition = FVector2D::ZeroVector;

	/** Internal: Is off-screen */
	UPROPERTY(BlueprintReadOnly, Category = "Internal")
	bool bIsOffScreen = false;

	/** Internal: Current distance */
	UPROPERTY(BlueprintReadOnly, Category = "Internal")
	float CurrentDistance = 0.0f;

	FHarmoniaWorldMarkerData()
	{
		MarkerID = FGuid::NewGuid();
	}
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldMarkerAdded, const FHarmoniaWorldMarkerData&, Marker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldMarkerRemoved, FGuid, MarkerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldMarkerClicked, FGuid, MarkerID, const FHarmoniaWorldMarkerData&, Marker);

/**
 * Harmonia World Marker Component
 * 
 * Manages 3D world markers that appear on the HUD.
 * Converts world positions to screen coordinates and handles
 * off-screen clamping, distance display, and scaling.
 *
 * Features:
 * - 3D to 2D marker projection
 * - Off-screen edge clamping with direction indicators
 * - Distance-based scaling and fading
 * - Category filtering
 * - Priority-based occlusion handling
 * - Multiple display modes
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaWorldMarkerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaWorldMarkerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Marker Management
	// ============================================================================

	/** Add a world marker */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	FGuid AddMarker(const FHarmoniaWorldMarkerData& Marker);

	/** Add a simple marker at a location */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	FGuid AddLocationMarker(FVector WorldLocation, FText Name, UTexture2D* Icon = nullptr, FLinearColor Color = FLinearColor::White);

	/** Add a marker tracking an actor */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	FGuid AddActorMarker(AActor* Actor, FText Name, UTexture2D* Icon = nullptr, FLinearColor Color = FLinearColor::White);

	/** Add a quest objective marker */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	FGuid AddQuestMarker(FVector WorldLocation, FText ObjectiveName, bool bIsPrimary = false);

	/** Remove a marker */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	bool RemoveMarker(FGuid MarkerID);

	/** Remove markers by category */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	void RemoveMarkersByCategory(FGameplayTag Category);

	/** Remove all markers */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	void RemoveAllMarkers();

	/** Update marker data */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	bool UpdateMarker(FGuid MarkerID, const FHarmoniaWorldMarkerData& NewData);

	/** Update marker location */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker")
	bool UpdateMarkerLocation(FGuid MarkerID, FVector NewLocation);

	// ============================================================================
	// Marker Queries
	// ============================================================================

	/** Get all markers */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	TArray<FHarmoniaWorldMarkerData> GetAllMarkers() const { return Markers; }

	/** Get visible markers (for UI rendering) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	TArray<FHarmoniaWorldMarkerData> GetVisibleMarkers() const;

	/** Get marker by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	bool GetMarkerByID(FGuid MarkerID, FHarmoniaWorldMarkerData& OutMarker) const;

	/** Get markers by category */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	TArray<FHarmoniaWorldMarkerData> GetMarkersByCategory(FGameplayTag Category) const;

	/** Get closest marker to screen center */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	bool GetClosestMarkerToCenter(FHarmoniaWorldMarkerData& OutMarker) const;

	// ============================================================================
	// Category Control
	// ============================================================================

	/** Set category visibility */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|WorldMarker|Filter")
	void SetCategoryVisible(FGameplayTag Category, bool bVisible);

	/** Check if category is visible */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker|Filter")
	bool IsCategoryVisible(FGameplayTag Category) const;

	// ============================================================================
	// Screen Conversion
	// ============================================================================

	/** Convert world position to screen position for a specific marker */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	bool WorldToScreen(FVector WorldPosition, FVector2D& OutScreenPosition, bool& bOutIsOnScreen) const;

	/** Get clamped screen position for off-screen markers */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	FVector2D GetClampedScreenPosition(FVector WorldPosition, float EdgePadding) const;

	/** Get rotation angle for off-screen indicator */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	float GetOffScreenIndicatorAngle(FVector WorldPosition) const;

	// ============================================================================
	// Distance Formatting
	// ============================================================================

	/** Format distance for display */
	UFUNCTION(BlueprintPure, Category = "Harmonia|WorldMarker")
	FText FormatDistance(float DistanceInCM) const;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|WorldMarker")
	FOnWorldMarkerAdded OnMarkerAdded;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|WorldMarker")
	FOnWorldMarkerRemoved OnMarkerRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|WorldMarker")
	FOnWorldMarkerClicked OnMarkerClicked;

protected:
	/** Update all marker positions and visibility */
	void UpdateMarkers();

	/** Calculate marker visibility based on display mode */
	bool ShouldMarkerBeVisible(const FHarmoniaWorldMarkerData& Marker, float Distance) const;

	/** Calculate marker scale based on distance */
	float CalculateMarkerScale(const FHarmoniaWorldMarkerData& Marker, float Distance) const;

	/** Calculate marker opacity based on distance */
	float CalculateMarkerOpacity(const FHarmoniaWorldMarkerData& Marker, float Distance) const;

	/** Clean up markers tracking destroyed actors */
	void CleanupInvalidMarkers();

	/** Get player camera location */
	FVector GetCameraLocation() const;

	/** Get player camera rotation */
	FRotator GetCameraRotation() const;

private:
	/** All registered markers */
	UPROPERTY()
	TArray<FHarmoniaWorldMarkerData> Markers;

	/** Hidden categories */
	UPROPERTY()
	TArray<FGameplayTag> HiddenCategories;

	/** Cached viewport size */
	FVector2D CachedViewportSize;

	/** Quest marker icon (loaded once) */
	UPROPERTY()
	TObjectPtr<UTexture2D> DefaultQuestIcon;

	/** Maximum number of visible markers (performance) */
	UPROPERTY(EditAnywhere, Category = "Performance", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxVisibleMarkers = 50;

	/** Distance format mode */
	UPROPERTY(EditAnywhere, Category = "Display")
	bool bUseMetricUnits = true;
};
