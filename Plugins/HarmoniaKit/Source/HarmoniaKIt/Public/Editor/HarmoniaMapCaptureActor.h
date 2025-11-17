// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/BoxComponent.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapCaptureActor.generated.h"

class UTextureRenderTarget2D;

/**
 * Editor-only actor for capturing top-down map views
 */
UCLASS(NotPlaceable, NotBlueprintable, HideCategories = (Rendering, Replication, Collision, HLOD, Physics, Networking, Input, Actor, Cooking))
class HARMONIAKIT_API AHarmoniaMapCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaMapCaptureActor();

protected:
	virtual void BeginPlay() override;

public:
	// Scene capture component for capturing the map
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Capture")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

	// Box component to visualize capture bounds
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Capture")
	TObjectPtr<UBoxComponent> BoundsVisualizer;

	// Map capture settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FMapCaptureSettings CaptureSettings;

	// Output texture target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	// Output path for saved texture (relative to Content folder)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FString OutputTexturePath = TEXT("/Game/Maps/CapturedMaps/");

	// Name for the output texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FString OutputTextureName = TEXT("MapTexture");

	// Whether to capture minimap as well
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	bool bCaptureMinimap = true;

	// Minimap resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture", meta = (EditCondition = "bCaptureMinimap"))
	FIntPoint MinimapResolution = FIntPoint(512, 512);

	// Output data asset path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FString OutputDataAssetPath = TEXT("/Game/Maps/CapturedMaps/");

	// Output data asset name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FString OutputDataAssetName = TEXT("MapData");

#if WITH_EDITOR
	// Setup the capture component based on settings
	UFUNCTION(CallInEditor, Category = "Map Capture")
	void SetupCapture();

	// Capture the map and save to texture
	UFUNCTION(CallInEditor, Category = "Map Capture")
	void CaptureMap();

	// Set capture bounds from selected actors
	UFUNCTION(CallInEditor, Category = "Map Capture")
	void SetBoundsFromSelection();

	// Auto-calculate bounds from all actors in level
	UFUNCTION(CallInEditor, Category = "Map Capture")
	void AutoCalculateBounds();

	// Preview the capture in viewport
	UFUNCTION(CallInEditor, Category = "Map Capture")
	void PreviewCapture();

	// Create and save the map data asset
	UFUNCTION(CallInEditor, Category = "Map Capture")
	void CreateMapDataAsset();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;

private:
	// Update the bounds visualizer
	void UpdateBoundsVisualizer();

	// Create render target with specified resolution
	UTextureRenderTarget2D* CreateRenderTarget(FIntPoint Resolution);

	// Save render target to texture asset
	UTexture2D* SaveRenderTargetToTexture(UTextureRenderTarget2D* InRenderTarget, const FString& PackageName);

	// Setup scene capture component
	void SetupSceneCaptureComponent();
#endif // WITH_EDITOR

	virtual void Tick(float DeltaTime) override;
};
