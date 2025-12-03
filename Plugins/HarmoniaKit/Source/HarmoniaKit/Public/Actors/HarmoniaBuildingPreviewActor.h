// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingPreviewActor.generated.h"

/**
 * 건축�?배치 ???�리뷰�? ?�시?�는 ?�터
 * - 반투�?메시�?배치 ?�치�?미리 ?�인
 * - 배치 가??불�????�태???�라 ?�상 변�?
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
	// ?�리�??�이???�용
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void ApplyPreviewData(const FHarmoniaBuildingPartData& PartData);

	// 배치 가???��????�른 ?�각 ?�드�?
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void SetIsPlacementValid(bool bIsValid);

	// ?�리�??�전
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void RotatePreview(float DeltaYaw);

protected:
	// 메시 컴포?�트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent = nullptr;

	// 배치 가???�태 머티리얼
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> ValidPlacementMaterial = nullptr;

	// 배치 불�????�태 머티리얼
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> InvalidPlacementMaterial = nullptr;

	// ?�재 배치 가???��?
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCurrentlyValid = false;

	// ?�재 ?�용???�트 ?�이??
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FHarmoniaBuildingPartData CurrentPartData;
};
