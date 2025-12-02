// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingPreviewActor.generated.h"

/**
 * ê±´ì¶•ë¬?ë°°ì¹˜ ???„ë¦¬ë·°ë? ?œì‹œ?˜ëŠ” ?¡í„°
 * - ë°˜íˆ¬ëª?ë©”ì‹œë¡?ë°°ì¹˜ ?„ì¹˜ë¥?ë¯¸ë¦¬ ?•ì¸
 * - ë°°ì¹˜ ê°€??ë¶ˆê????íƒœ???°ë¼ ?‰ìƒ ë³€ê²?
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaBuildingPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaBuildingPreviewActor();

protected:
	virtual void BeginPlay() override;

public:
	// ?„ë¦¬ë·??°ì´???ìš©
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void ApplyPreviewData(const FHarmoniaBuildingPartData& PartData);

	// ë°°ì¹˜ ê°€???¬ë????°ë¥¸ ?œê° ?¼ë“œë°?
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void SetIsPlacementValid(bool bIsValid);

	// ?„ë¦¬ë·??Œì „
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void RotatePreview(float DeltaYaw);

protected:
	// ë©”ì‹œ ì»´í¬?ŒíŠ¸
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent = nullptr;

	// ë°°ì¹˜ ê°€???íƒœ ë¨¸í‹°ë¦¬ì–¼
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> ValidPlacementMaterial = nullptr;

	// ë°°ì¹˜ ë¶ˆê????íƒœ ë¨¸í‹°ë¦¬ì–¼
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> InvalidPlacementMaterial = nullptr;

	// ?„ì¬ ë°°ì¹˜ ê°€???¬ë?
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCurrentlyValid = false;

	// ?„ì¬ ?ìš©???ŒíŠ¸ ?°ì´??
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FHarmoniaBuildingPartData CurrentPartData;
};
