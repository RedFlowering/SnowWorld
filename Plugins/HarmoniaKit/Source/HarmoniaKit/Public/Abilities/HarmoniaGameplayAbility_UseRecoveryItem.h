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
 * ?�복 ?�이???�용 Gameplay Ability
 * 모든 ?�복 ?�이???�?�에 ?�사??가??
 *
 * 주요 기능:
 * - ?�이???�용 검�?(충전 ?�수, ?�태 ??
 * - ?�전 ?�간 구현 (?�동/?�격 ??취소)
 * - ?�복 ?�과 ?�용 (즉시/지???�간??��/?�치??
 * - VFX/SFX ?�생
 * - ?�니메이???�생
 * - ?�트?�크 리플리�??�션
 *
 * ?�사??가?�성:
 * - ItemType ?�라미터�?모든 ?�복 ?�이???�??처리
 * - ?�정 기반?�로 ?�작 (?�이???�리�?
 * - ?�른 ?�모???�이?�에???�장 가??
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.5.1 for tag configuration
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
	 * ?�복 ?�이???�용 ?�작
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void StartUsingRecoveryItem();

	/**
	 * ?�복 ?�이???�용 ?�료
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CompleteUsingRecoveryItem();

	/**
	 * ?�복 ?�이???�용 취소
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CancelUsingRecoveryItem(const FText& Reason);

	/**
	 * ?�복 ?�과 ?�용
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyRecoveryEffects();

	/**
	 * 즉시 ?�복 ?�용
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyInstantRecovery(float HealthAmount);

	/**
	 * 지???�복 ?�용 (HoT)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyOverTimeRecovery(float HealthPerSecond, float Duration);

	/**
	 * ?�간 ??�� ?�복 ?�용 (Frozen Time Snowflake)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyTimeReversalRecovery();

	/**
	 * ?�치??범위 ?�복 ?�용 (Life Luminescence)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void DeployRecoveryArea();

	/**
	 * VFX/SFX ?�생
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void PlayUsageEffects();

	/**
	 * ?�동 감�? (?�용 취소??
	 */
	UFUNCTION()
	void OnOwnerMoved();

	/**
	 * ?�격 감�? (?�용 취소??
	 */
	UFUNCTION()
	void OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	/** ?�용???�이???�??(Blueprint?�서 ?�정 ?�는 EventData�??�달) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	/** 공명 ?�편 주파??(ResonanceShard ?�용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	/** ?�이???�정 ?�버?�이??(?�션, 비워?�면 컴포?�트 ?�정 ?�용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	FHarmoniaRecoveryItemConfig ConfigOverride;

	/** ?�정 ?�버?�이???�용 ?��? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	bool bUseConfigOverride = false;

	/** ?�용 �??�용???�그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer UsingTags;

	/** ?�용 �?블록???�그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer BlockedTags;

	/** ?�용 ?�니메이??*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	/** 캐시??Rechargeable Item Component */
	UPROPERTY()
	TObjectPtr<UHarmoniaRechargeableItemComponent> RechargeableItemComponent;

	/** ?�재 ?�용 중인 ?�이???�정 */
	UPROPERTY()
	FHarmoniaRecoveryItemConfig CurrentConfig;

	/** ?�용 ?�?�머 ?�들 */
	FTimerHandle UsageTimerHandle;

	/** ?�동 체크 ?�?�머 ?�들 */
	FTimerHandle MovementCheckTimerHandle;

	/** ?�용 ?�작 ?�치 (?�동 감�??? */
	FVector UsageStartLocation;

	/** ?�동 감�? ?�계�?*/
	UPROPERTY(EditDefaultsOnly, Category = "Recovery Item")
	float MovementThreshold = 10.0f;

	/** VFX 컴포?�트 */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> VFXComponent;

private:
	UHarmoniaRechargeableItemComponent* GetRechargeableItemComponent() const;
};
