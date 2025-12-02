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
 * ?Œë³µ ?„ì´???¬ìš© Gameplay Ability
 * ëª¨ë“  ?Œë³µ ?„ì´???€?…ì— ?¬ì‚¬??ê°€??
 *
 * ì£¼ìš” ê¸°ëŠ¥:
 * - ?„ì´???¬ìš© ê²€ì¦?(ì¶©ì „ ?Ÿìˆ˜, ?íƒœ ??
 * - ?œì „ ?œê°„ êµ¬í˜„ (?´ë™/?¼ê²© ??ì·¨ì†Œ)
 * - ?Œë³µ ?¨ê³¼ ?ìš© (ì¦‰ì‹œ/ì§€???œê°„??–‰/?¤ì¹˜??
 * - VFX/SFX ?¬ìƒ
 * - ? ë‹ˆë©”ì´???¬ìƒ
 * - ?¤íŠ¸?Œí¬ ë¦¬í”Œë¦¬ì??´ì…˜
 *
 * ?¬ì‚¬??ê°€?¥ì„±:
 * - ItemType ?Œë¼ë¯¸í„°ë¡?ëª¨ë“  ?Œë³µ ?„ì´???€??ì²˜ë¦¬
 * - ?¤ì • ê¸°ë°˜?¼ë¡œ ?™ì‘ (?°ì´???œë¦¬ë¸?
 * - ?¤ë¥¸ ?Œëª¨???„ì´?œì—???•ì¥ ê°€??
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
	 * ?Œë³µ ?„ì´???¬ìš© ?œì‘
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void StartUsingRecoveryItem();

	/**
	 * ?Œë³µ ?„ì´???¬ìš© ?„ë£Œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CompleteUsingRecoveryItem();

	/**
	 * ?Œë³µ ?„ì´???¬ìš© ì·¨ì†Œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CancelUsingRecoveryItem(const FText& Reason);

	/**
	 * ?Œë³µ ?¨ê³¼ ?ìš©
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyRecoveryEffects();

	/**
	 * ì¦‰ì‹œ ?Œë³µ ?ìš©
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyInstantRecovery(float HealthAmount);

	/**
	 * ì§€???Œë³µ ?ìš© (HoT)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyOverTimeRecovery(float HealthPerSecond, float Duration);

	/**
	 * ?œê°„ ??–‰ ?Œë³µ ?ìš© (Frozen Time Snowflake)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyTimeReversalRecovery();

	/**
	 * ?¤ì¹˜??ë²”ìœ„ ?Œë³µ ?ìš© (Life Luminescence)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void DeployRecoveryArea();

	/**
	 * VFX/SFX ?¬ìƒ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void PlayUsageEffects();

	/**
	 * ?´ë™ ê°ì? (?¬ìš© ì·¨ì†Œ??
	 */
	UFUNCTION()
	void OnOwnerMoved();

	/**
	 * ?¼ê²© ê°ì? (?¬ìš© ì·¨ì†Œ??
	 */
	UFUNCTION()
	void OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	/** ?¬ìš©???„ì´???€??(Blueprint?ì„œ ?¤ì • ?ëŠ” EventDataë¡??„ë‹¬) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	/** ê³µëª… ?Œí¸ ì£¼íŒŒ??(ResonanceShard ?„ìš©) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	/** ?„ì´???¤ì • ?¤ë²„?¼ì´??(?µì…˜, ë¹„ì›Œ?ë©´ ì»´í¬?ŒíŠ¸ ?¤ì • ?¬ìš©) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	FHarmoniaRecoveryItemConfig ConfigOverride;

	/** ?¤ì • ?¤ë²„?¼ì´???¬ìš© ?¬ë? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	bool bUseConfigOverride = false;

	/** ?¬ìš© ì¤??ìš©???œê·¸ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer UsingTags;

	/** ?¬ìš© ì¤?ë¸”ë¡???œê·¸ */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer BlockedTags;

	/** ?¬ìš© ? ë‹ˆë©”ì´??*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	/** ìºì‹œ??Rechargeable Item Component */
	UPROPERTY()
	TObjectPtr<UHarmoniaRechargeableItemComponent> RechargeableItemComponent;

	/** ?„ì¬ ?¬ìš© ì¤‘ì¸ ?„ì´???¤ì • */
	UPROPERTY()
	FHarmoniaRecoveryItemConfig CurrentConfig;

	/** ?¬ìš© ?€?´ë¨¸ ?¸ë“¤ */
	FTimerHandle UsageTimerHandle;

	/** ?´ë™ ì²´í¬ ?€?´ë¨¸ ?¸ë“¤ */
	FTimerHandle MovementCheckTimerHandle;

	/** ?¬ìš© ?œì‘ ?„ì¹˜ (?´ë™ ê°ì??? */
	FVector UsageStartLocation;

	/** ?´ë™ ê°ì? ?„ê³„ê°?*/
	UPROPERTY(EditDefaultsOnly, Category = "Recovery Item")
	float MovementThreshold = 10.0f;

	/** VFX ì»´í¬?ŒíŠ¸ */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> VFXComponent;

private:
	UHarmoniaRechargeableItemComponent* GetRechargeableItemComponent() const;
};
