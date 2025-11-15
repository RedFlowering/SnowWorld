// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingPreviewActor.generated.h"

/**
 * 건축물 배치 전 프리뷰를 표시하는 액터
 * - 반투명 메시로 배치 위치를 미리 확인
 * - 배치 가능/불가능 상태에 따라 색상 변경
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
	// 프리뷰 데이터 적용
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void ApplyPreviewData(const FBuildingPartData& PartData);

	// 배치 가능 여부에 따른 시각 피드백
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void SetIsPlacementValid(bool bIsValid);

	// 프리뷰 회전
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void RotatePreview(float DeltaYaw);

protected:
	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

	// 배치 가능 상태 머티리얼
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> ValidPlacementMaterial;

	// 배치 불가능 상태 머티리얼
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> InvalidPlacementMaterial;

	// 현재 배치 가능 여부
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCurrentlyValid = false;

	// 현재 적용된 파트 데이터
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FBuildingPartData CurrentPartData;
};
