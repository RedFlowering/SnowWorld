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
 * ?�치???�복 구역 Actor (Life Luminescence??
 * 범위 ???�레?�어�?지?�적?�로 ?�복
 *
 * 주요 기능:
 * - 범위 기반 ?�복 (Sphere Component)
 * - 주기???�복 ??(Tick Interval)
 * - ?�간 ?�한 (Duration)
 * - VFX/SFX ?�생
 * - 멀?�플?�이??지??
 *
 * ?�사??가?�성:
 * - ??구역, 버프 구역 ?�으�??�장 가??
 * - ?�정 기반 ?�작
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
	 * ?�복 구역 초기??
	 * @param Config ?�복 구역 ?�정
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void InitializeRecoveryArea(const FHarmoniaDeployableRecoveryConfig& Config);

	/**
	 * ?�복 구역 ?�성??
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void ActivateRecoveryArea();

	/**
	 * ?�복 구역 비활?�화
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void DeactivateRecoveryArea();

	/**
	 * ?�복 ???�행
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void PerformRecoveryTick();

	/**
	 * ?�복 구역 ???�터??가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Recovery Area")
	TArray<AActor*> GetActorsInRecoveryArea() const;

	/**
	 * ?�복 구역 종료
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void ExpireRecoveryArea();

protected:
	/**
	 * Actor 진입 ?�벤??
	 */
	UFUNCTION()
	void OnActorEnterRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Actor ?�탈 ?�벤??
	 */
	UFUNCTION()
	void OnActorLeaveRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	/** ?�복 구역 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<USphereComponent> RecoveryAreaSphere;

	/** VFX 컴포?�트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<UNiagaraComponent> AreaVFXComponent;

	/** SFX 컴포?�트 (루프) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<UAudioComponent> AreaAudioComponent;

	/** ?�복 구역 ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Area")
	FHarmoniaDeployableRecoveryConfig RecoveryConfig;

	/** ?�복 ???�?�머 */
	FTimerHandle RecoveryTickTimerHandle;

	/** 만료 ?�?�머 */
	FTimerHandle ExpirationTimerHandle;

	/** ?�성???��? */
	UPROPERTY(BlueprintReadOnly, Category = "Recovery Area")
	bool bIsActive = false;

	/** ?�복 구역 ???�터??*/
	UPROPERTY()
	TSet<TObjectPtr<AActor>> ActorsInArea;
};
