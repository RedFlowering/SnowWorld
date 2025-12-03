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
 * @class UHarmoniaGameplayAbility_UseRecoveryItem
 * @brief Gameplay Ability for using recovery items
 * 
 * Can be reused for all recovery item types.
 *
 * Key Features:
 * - Item usage validation (charge count, state, etc.)
 * - Safe usage time implementation (cancel on movement/damage)
 * - Apply recovery effects (instant/over time/time reversal/deployable)
 * - VFX/SFX playback
 * - Animation playback
 * - Network replication
 *
 * Extensibility:
 * - Handles all recovery item types via ItemType parameter
 * - Configuration-based operation (item manager)
 * - Extensible for other consumable items
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

	/** Start using recovery item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void StartUsingRecoveryItem();

	/** Complete using recovery item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CompleteUsingRecoveryItem();

	/** Cancel using recovery item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void CancelUsingRecoveryItem(const FText& Reason);

	/** Apply recovery effects based on item type */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyRecoveryEffects();

	/** Apply instant recovery */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyInstantRecovery(float HealthAmount);

	/** Apply over time recovery (HoT) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyOverTimeRecovery(float HealthPerSecond, float Duration);

	/** Apply time reversal recovery (Frozen Time Snowflake) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void ApplyTimeReversalRecovery();

	/** Deploy recovery area (Life Luminescence) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void DeployRecoveryArea();

	/** Play VFX/SFX effects */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void PlayUsageEffects();

	/** Movement detection (for cancel) */
	UFUNCTION()
	void OnOwnerMoved();

	/** Damage detection (for cancel) */
	UFUNCTION()
	void OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	/** Item type to use (set in Blueprint or passed via EventData) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	/** Resonance shard frequency (for ResonanceShard type) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	/** Item configuration override (optional, uses component config if empty) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	FHarmoniaRecoveryItemConfig ConfigOverride;

	/** Whether to use config override */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Recovery Item")
	bool bUseConfigOverride = false;

	/** Tags applied while using */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer UsingTags;

	/** Tags that block usage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item")
	FGameplayTagContainer BlockedTags;

	/** Usage animation montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	/** Cached rechargeable item component */
	UPROPERTY()
	TObjectPtr<UHarmoniaRechargeableItemComponent> RechargeableItemComponent;

	/** Current active item configuration */
	UPROPERTY()
	FHarmoniaRecoveryItemConfig CurrentConfig;

	/** Usage timer handle */
	FTimerHandle UsageTimerHandle;

	/** Movement check timer handle */
	FTimerHandle MovementCheckTimerHandle;

	/** Usage start location (for movement detection) */
	FVector UsageStartLocation;

	/** Movement detection threshold */
	UPROPERTY(EditDefaultsOnly, Category = "Recovery Item")
	float MovementThreshold = 10.0f;

	/** VFX component */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> VFXComponent;

private:
	UHarmoniaRechargeableItemComponent* GetRechargeableItemComponent() const;
};
