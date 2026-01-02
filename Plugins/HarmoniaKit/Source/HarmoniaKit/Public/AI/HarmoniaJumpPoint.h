// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "HarmoniaJumpPoint.generated.h"

class USenseStimulusComponent;

/**
 * AHarmoniaJumpPoint
 *
 * 점프 목적지를 나타내는 단순 마커 액터입니다.
 * SenseSystem으로 탐지되어 캐릭터가 이 위치로 점프할 수 있습니다.
 *
 * 사용:
 * - 바위, 건물 등 점프할 위치에 배치
 * - SenseReceiverComponent가 탐지 → GA_JumpToPoint로 점프
 */
UCLASS(Blueprintable, ClassGroup=(AI), meta=(DisplayName = "Jump Point"))
class HARMONIAKIT_API AHarmoniaJumpPoint : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaJumpPoint();

	// ==========================================================================
	// Configuration
	// ==========================================================================

	/** 착지 위치 오프셋 (액터 위치 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPoint")
	FVector LandingOffset = FVector::ZeroVector;

	/** 이 지점을 사용할 수 있는 태그 (비어있으면 모두 허용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JumpPoint")
	FGameplayTagContainer AllowedTags;

	// ==========================================================================
	// State
	// ==========================================================================

	/** 현재 점유 중인지 */
	UPROPERTY(BlueprintReadOnly, Category = "JumpPoint|State")
	bool bIsOccupied = false;

	/** 현재 점유자 */
	UPROPERTY(BlueprintReadOnly, Category = "JumpPoint|State")
	TWeakObjectPtr<AActor> OccupyingActor;

	// ==========================================================================
	// Functions
	// ==========================================================================

	/** 착지 위치 (월드 좌표) */
	UFUNCTION(BlueprintPure, Category = "JumpPoint")
	FVector GetLandingLocation() const { return GetActorLocation() + LandingOffset; }

	/** 이 지점이 특정 액터에게 유효한지 */
	UFUNCTION(BlueprintPure, Category = "JumpPoint")
	bool IsValidForActor(AActor* Actor) const;

	/** 지점 점유 */
	UFUNCTION(BlueprintCallable, Category = "JumpPoint")
	bool ClaimPoint(AActor* Claimer);

	/** 지점 해제 */
	UFUNCTION(BlueprintCallable, Category = "JumpPoint")
	void ReleasePoint(AActor* Claimer);

protected:
	/** SenseSystem에서 탐지하기 위한 Stimulus */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JumpPoint")
	TObjectPtr<USenseStimulusComponent> StimulusComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "JumpPoint")
	TObjectPtr<class UBillboardComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, Category = "JumpPoint")
	TObjectPtr<class UArrowComponent> LandingArrow;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

