// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaRecoveryItemDefinitions.h"
#include "HarmoniaRecoveryAreaActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UAudioComponent;
class UGameplayEffect;

/**
 * AHarmoniaRecoveryAreaActor
 *
 * 설치형 회복 구역 Actor (Life Luminescence용)
 * 범위 내 플레이어를 지속적으로 회복
 *
 * 주요 기능:
 * - 범위 기반 회복 (Sphere Component)
 * - 주기적 회복 틱 (Tick Interval)
 * - 시간 제한 (Duration)
 * - VFX/SFX 재생
 * - 멀티플레이어 지원
 *
 * 재사용 가능성:
 * - 독 구역, 버프 구역 등으로 확장 가능
 * - 설정 기반 동작
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaRecoveryAreaActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaRecoveryAreaActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/**
	 * 회복 구역 초기화
	 * @param Config 회복 구역 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void InitializeRecoveryArea(const FHarmoniaDeployableRecoveryConfig& Config);

	/**
	 * 회복 구역 활성화
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void ActivateRecoveryArea();

	/**
	 * 회복 구역 비활성화
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void DeactivateRecoveryArea();

	/**
	 * 회복 틱 실행
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void PerformRecoveryTick();

	/**
	 * 회복 구역 내 액터들 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Recovery Area")
	TArray<AActor*> GetActorsInRecoveryArea() const;

	/**
	 * 회복 구역 종료
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void ExpireRecoveryArea();

protected:
	/**
	 * Actor 진입 이벤트
	 */
	UFUNCTION()
	void OnActorEnterRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Actor 이탈 이벤트
	 */
	UFUNCTION()
	void OnActorLeaveRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	/** 회복 구역 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<USphereComponent> RecoveryAreaSphere;

	/** VFX 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<UNiagaraComponent> AreaVFXComponent;

	/** SFX 컴포넌트 (루프) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<UAudioComponent> AreaAudioComponent;

	/** 회복 구역 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Area")
	FHarmoniaDeployableRecoveryConfig RecoveryConfig;

	/** 회복 틱 타이머 */
	FTimerHandle RecoveryTickTimerHandle;

	/** 만료 타이머 */
	FTimerHandle ExpirationTimerHandle;

	/** 활성화 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Recovery Area")
	bool bIsActive = false;

	/** 회복 구역 내 액터들 */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> ActorsInArea;
};
