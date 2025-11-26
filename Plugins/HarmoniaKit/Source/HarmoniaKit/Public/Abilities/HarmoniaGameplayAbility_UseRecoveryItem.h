// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaRecoveryItemDefinitions.h"
#include "HarmoniaGameplayAbility_UseRecoveryItem.generated.h"

class UHarmoniaRechargeableItemComponent;
class UNiagaraComponent;
class UGameplayEffect;

/**
 * UHarmoniaGameplayAbility_UseRecoveryItem
 *
 * 회복 아이템 사용 Gameplay Ability
 * 모든 회복 아이템 타입에 재사용 가능
 *
 * 주요 기능:
 * - 아이템 사용 검증 (충전 횟수, 상태 등)
 * - 시전 시간 구현 (이동/피격 시 취소)
 * - 회복 효과 적용 (즉시/지속/시간역행/설치형)
 * - VFX/SFX 재생
 * - 애니메이션 재생
 * - 네트워크 리플리케이션
 *
 * 재사용 가능성:
 * - ItemType 파라미터로 모든 회복 아이템 타입 처리
 * - 설정 기반으로 동작 (데이터 드리븐)
 * - 다른 소모성 아이템에도 확장 가능
 *
 * ============================================================================
 * Required Tag Configuration (set in Blueprint or derived class):
 * ============================================================================
 *
 * AbilityTags:
 *   - Ability.Item.UseRecovery (identifies this ability)
 *
 * ActivationOwnedTags (tags applied while using item):
 *   - State.UsingItem
 *   - State.Casting (during cast time)
 *
 * ActivationBlockedTags (tags that prevent item use):
 *   - State.Combat.Attacking
 *   - State.Dodging
 *   - State.HitReaction
 *   - State.UsingItem (already using item)
 *
 * BlockAbilitiesWithTag (abilities to block while using item):
 *   - State.Combat.Attacking
 *   - State.Dodging
 *
 * CancelAbilitiesWithTag (abilities to cancel when using item):
 *   - (none by default)
 *
 * Note: Item use is cancelled by:
 *   - Movement during cast time
 *   - Receiving damage during cast time
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_UseRecoveryItem : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_UseRecoveryItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	/**
	 * 회복 아이템 사용 시작
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void StartUsingRecoveryItem();

	/**
	 * 회복 아이템 사용 완료
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CompleteUsingRecoveryItem();

	/**
	 * 회복 아이템 사용 취소
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CancelUsingRecoveryItem(const FText& Reason);

	/**
	 * 회복 효과 적용
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyRecoveryEffects();

	/**
	 * 즉시 회복 적용
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyInstantRecovery(float HealthAmount);

	/**
	 * 지속 회복 적용 (HoT)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyOverTimeRecovery(float HealthPerSecond, float Duration);

	/**
	 * 시간 역행 회복 적용 (Frozen Time Snowflake)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyTimeReversalRecovery();

	/**
	 * 설치형 범위 회복 적용 (Life Luminescence)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void DeployRecoveryArea();

	/**
	 * VFX/SFX 재생
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void PlayUsageEffects();

	/**
	 * 이동 감지 (사용 취소용)
	 */
	UFUNCTION()
	void OnOwnerMoved();

	/**
	 * 피격 감지 (사용 취소용)
	 */
	UFUNCTION()
	void OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	/** 사용할 아이템 타입 (Blueprint에서 설정 또는 EventData로 전달) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	/** 공명 파편 주파수 (ResonanceShard 전용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	/** 아이템 설정 오버라이드 (옵션, 비워두면 컴포넌트 설정 사용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	FHarmoniaRecoveryItemConfig ConfigOverride;

	/** 설정 오버라이드 사용 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	bool bUseConfigOverride = false;

	/** 사용 중 적용할 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer UsingTags;

	/** 사용 중 블록할 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer BlockedTags;

	/** 사용 애니메이션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	/** 캐시된 Rechargeable Item Component */
	UPROPERTY()
	TObjectPtr<UHarmoniaRechargeableItemComponent> RechargeableItemComponent;

	/** 현재 사용 중인 아이템 설정 */
	UPROPERTY()
	FHarmoniaRecoveryItemConfig CurrentConfig;

	/** 사용 타이머 핸들 */
	FTimerHandle UsageTimerHandle;

	/** 이동 체크 타이머 핸들 */
	FTimerHandle MovementCheckTimerHandle;

	/** 사용 시작 위치 (이동 감지용) */
	FVector UsageStartLocation;

	/** 이동 감지 임계값 */
	UPROPERTY(EditDefaultsOnly, Category = "Recovery Item")
	float MovementThreshold = 10.0f;

	/** VFX 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> VFXComponent;

private:
	UHarmoniaRechargeableItemComponent* GetRechargeableItemComponent() const;
};
