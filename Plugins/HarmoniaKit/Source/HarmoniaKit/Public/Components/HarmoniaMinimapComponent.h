// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "HarmoniaMinimapComponent.generated.h"

class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

/**
 * Minimap icon data
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMinimapIcon
{
	GENERATED_BODY()

	/** Unique identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Icon")
	FGuid IconID;

	/** World location */
	UPROPERTY(BlueprintReadWrite, Category = "Icon")
	FVector WorldLocation = FVector::ZeroVector;

	/** Actor to track (if tracking an actor) */
	UPROPERTY(BlueprintReadWrite, Category = "Icon")
	TWeakObjectPtr<AActor> TrackedActor;

	/** Icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** Icon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	FLinearColor IconColor = FLinearColor::White;

	/** Icon size on minimap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon", meta = (ClampMin = "4", ClampMax = "64"))
	float IconSize = 16.0f;

	/** Icon category tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	FGameplayTag CategoryTag;

	/** Priority (higher = drawn on top) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	int32 Priority = 0;

	/** Whether to rotate with tracked actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	bool bRotateWithActor = false;

	/** Whether to show direction indicator when off-screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	bool bShowOffScreenIndicator = false;

	/** Whether this icon pulses/animates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	bool bPulse = false;

	/** Visibility flag */
	UPROPERTY(BlueprintReadWrite, Category = "Icon")
	bool bVisible = true;

	/** Whether to clamp to edge when off-screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon")
	bool bClampToEdge = false;

	/** Current icon rotation */
	UPROPERTY(BlueprintReadWrite, Category = "Icon")
	float IconRotation = 0.0f;

	/** Current position on minimap (UV) */
	UPROPERTY(BlueprintReadWrite, Category = "Icon")
	FVector2D MinimapPosition = FVector2D::ZeroVector;

	FHarmoniaMinimapIcon()
	{
		IconID = FGuid::NewGuid();
	}
};

/**
 * Minimap configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMinimapConfig
{
	GENERATED_BODY()

	/** Minimap capture radius in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "500", ClampMax = "50000"))
	float CaptureRadius = 5000.0f;

	/** Orthographic capture width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "1000", ClampMax = "100000"))
	float CaptureOrthoWidth = 5000.0f;

	/** Capture height above player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float CaptureHeight = 5000.0f;

	/** Minimap render resolution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "64", ClampMax = "1024"))
	int32 RenderResolution = 256;

	/** Texture resolution for render target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "64", ClampMax = "2048"))
	int32 TextureResolution = 512;

	/** Whether to rotate minimap with player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRotateWithPlayer = true;

	/** Whether to capture every frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCaptureEveryFrame = true;

	/** Capture height offset (from player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float CaptureHeightOffset = 5000.0f;

	/** Minimum zoom level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MinZoomLevel = 0.5f;

	/** Maximum zoom level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxZoomLevel = 3.0f;

	/** Update frequency (frames) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "1", ClampMax = "60"))
	int32 UpdateFrameInterval = 2;

	/** Minimap zoom levels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<float> ZoomLevels = { 2000.0f, 5000.0f, 10000.0f };

	/** Current zoom level index */
	UPROPERTY(BlueprintReadWrite, Category = "Config")
	int32 CurrentZoomIndex = 1;

	/** North indicator rotation offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float NorthRotationOffset = 0.0f;
};

/**
 * Delegate for minimap events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapIconAdded, const FHarmoniaMinimapIcon&, Icon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapIconRemoved, FGuid, IconID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapZoomChanged, float, NewZoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinimapInitialized);

/**
 * Harmonia Minimap Component
 * 
 * Provides real-time minimap functionality with icon management.
 * Attaches to the player controller or HUD.
 *
 * Features:
 * - Scene capture based minimap rendering
 * - Dynamic icon management
 * - Multiple zoom levels
 * - Icon categories and filtering
 * - Off-screen direction indicators
 * - Fog of war integration (optional)
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMinimapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaMinimapComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Minimap Control
	// ============================================================================

	/** Initialize the minimap rendering system */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap")
	void InitializeMinimap();

	/** Set minimap configuration */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap")
	void SetConfig(const FHarmoniaMinimapConfig& NewConfig);

	/** Get current configuration */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap")
	FHarmoniaMinimapConfig GetConfig() const { return Config; }

	/** Set the actor to center the minimap on */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap")
	void SetCenterActor(AActor* Actor);

	/** Get the render target for UI binding */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	// ============================================================================
	// Zoom Control
	// ============================================================================

	/** Zoom in */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap")
	void ZoomIn(float Amount = 0.1f);

	/** Zoom out */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap")
	void ZoomOut(float Amount = 0.1f);

	/** Set zoom level directly (float) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap")
	void SetZoomLevel(float NewZoomLevel);

	/** Get current zoom radius */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap")
	float GetCurrentZoomRadius() const;

	// ============================================================================
	// Icon Management
	// ============================================================================

	/** Add an icon to the minimap */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	FGuid AddIcon(const FHarmoniaMinimapIcon& Icon);

	/** Add an icon tracking an actor */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	FGuid AddActorIcon(AActor* Actor, UTexture2D* IconTexture, FLinearColor Color = FLinearColor::White, bool bRotateWithActor = false);

	/** Add a static location icon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	FGuid AddLocationIcon(FVector WorldLocation, UTexture2D* IconTexture, FLinearColor Color = FLinearColor::White);

	/** Remove an icon by ID */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	bool RemoveIcon(FGuid IconID);

	/** Remove all icons */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	void RemoveAllIcons();

	/** Remove all icons tracking a specific actor */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	void RemoveActorIcons(AActor* Actor);

	/** Remove all icons with a specific category */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	void RemoveIconsByCategory(FGameplayTag Category);

	/** Update icon location */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	bool UpdateIconLocation(FGuid IconID, FVector NewWorldLocation);

	/** Update an icon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	bool UpdateIcon(FGuid IconID, const FHarmoniaMinimapIcon& NewData);

	/** Set icon visibility */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Icons")
	void SetIconVisible(FGuid IconID, bool bVisible);

	/** Get all icons */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Icons")
	TArray<FHarmoniaMinimapIcon> GetAllIcons() const { return Icons; }

	/** Get visible icons */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Icons")
	TArray<FHarmoniaMinimapIcon> GetVisibleIcons() const;

	/** Get icon by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Icons")
	bool GetIconByID(FGuid IconID, FHarmoniaMinimapIcon& OutIcon) const;

	/** Get icons by category */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Icons")
	TArray<FHarmoniaMinimapIcon> GetIconsByCategory(FGameplayTag Category) const;

	// ============================================================================
	// Category Filtering
	// ============================================================================

	/** Set category visibility */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Filter")
	void SetCategoryVisible(FGameplayTag Category, bool bVisible);

	/** Toggle category visibility */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Minimap|Filter")
	void ToggleCategoryVisibility(FGameplayTag Category);

	/** Check if category is visible */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Filter")
	bool IsCategoryVisible(FGameplayTag Category) const;

	// ============================================================================
	// Coordinate Conversion
	// ============================================================================

	/** Convert world position to minimap UV coordinates */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Conversion")
	FVector2D WorldToMinimapUV(FVector WorldPosition) const;

	/** Convert minimap UV to world position */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Conversion")
	FVector MinimapUVToWorld(FVector2D UV) const;

	/** Check if world position is within minimap bounds */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Conversion")
	bool IsPositionInMinimapBounds(FVector WorldPosition) const;

	/** Get direction to off-screen position (for edge indicators) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Minimap|Conversion")
	float GetDirectionToPosition(FVector WorldPosition) const;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Minimap")
	FOnMinimapIconAdded OnIconAdded;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Minimap")
	FOnMinimapIconRemoved OnIconRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Minimap")
	FOnMinimapZoomChanged OnZoomChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Minimap")
	FOnMinimapInitialized OnMinimapInitialized;

protected:
	/** Update scene capture position and rotation */
	void UpdateSceneCapture();

	/** Update scene capture transform */
	void UpdateSceneCaptureTransform();

	/** Update tracked actor positions */
	void UpdateTrackedIcons();

	/** Update icon positions on minimap */
	void UpdateIconPositions();

	/** Clean up invalid actor references */
	void CleanupInvalidIcons();

	/** Clean up minimap resources */
	void CleanupMinimap();

	/** Create scene capture component */
	void CreateSceneCaptureComponent();

	/** Create render target */
	void CreateRenderTarget();

	/** Configure show flags for scene capture */
	void ConfigureShowFlags();

	/** Capture scene manually */
	void CaptureScene();

	/** Clamp UV to minimap edge */
	FVector2D ClampToMinimapEdge(FVector2D UV) const;

	/** Check if world location is visible on minimap */
	bool IsWorldLocationVisible(FVector WorldLocation) const;

private:
	/** Minimap configuration */
	UPROPERTY(EditAnywhere, Category = "Config")
	FHarmoniaMinimapConfig Config;

	/** All registered icons */
	UPROPERTY()
	TArray<FHarmoniaMinimapIcon> Icons;

	/** Hidden categories */
	UPROPERTY()
	TArray<FGameplayTag> HiddenCategories;

	/** Actor to center minimap on */
	UPROPERTY()
	TWeakObjectPtr<AActor> CenterActor;

	/** Render target for minimap */
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** Scene capture component actor */
	UPROPERTY()
	TObjectPtr<AActor> SceneCaptureActor;

	/** Scene capture component */
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	/** Current zoom level */
	float CurrentZoomLevel = 1.0f;

	/** Frame counter for update throttling */
	int32 FrameCounter = 0;

	/** Whether minimap is initialized */
	bool bIsInitialized = false;
};
