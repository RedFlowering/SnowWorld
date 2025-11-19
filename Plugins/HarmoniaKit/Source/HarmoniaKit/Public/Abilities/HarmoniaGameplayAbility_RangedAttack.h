// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaGameplayAbility_RangedAttack.generated.h"

class UHarmoniaRangedCombatComponent;
class AHarmoniaProjectile;

/**
 * UHarmoniaGameplayAbility_RangedAttack
 *
 * Base gameplay ability for ranged attacks
 * Handles:
 * - Bow/Crossbow attacks
 * - Throwing weapons
 * - Firearms
 * - Resource consumption (ammo, stamina, mana)
 * - Animation playback
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_RangedAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_RangedAttack();

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UGameplayAbility interface

protected:
	/**
	 * Start charging/drawing (for bows)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	void StartCharging();

	/**
	 * Release and fire
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	void ReleaseAndFire();

	/**
	 * Fire projectile
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	virtual void FireProjectile();

	/**
	 * Get ranged combat component
	 */
	UHarmoniaRangedCombatComponent* GetRangedCombatComponent(const FGameplayAbilityActorInfo* ActorInfo) const;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Weapon type this ability is for (None = use equipped weapon) */
	UPROPERTY(EditDefaultsOnly, Category = "Ranged Attack")
	EHarmoniaRangedWeaponType WeaponType = EHarmoniaRangedWeaponType::None;

	/** Projectile type to spawn (overrides weapon default) */
	UPROPERTY(EditDefaultsOnly, Category = "Ranged Attack")
	EHarmoniaProjectileType ProjectileType = EHarmoniaProjectileType::Arrow;

	/** Use custom projectile type? */
	UPROPERTY(EditDefaultsOnly, Category = "Ranged Attack")
	bool bUseCustomProjectileType = false;

	/** Fire animation montage */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	/** Reload animation montage */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	/** Draw/charge animation montage (for bows) */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DrawMontage;

	/** Gameplay tags to apply while attacking */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer AttackingTags;

	/** Gameplay tags that block this ability */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;

	/** Stamina cost */
	UPROPERTY(EditDefaultsOnly, Category = "Costs")
	float StaminaCost = 0.0f;

	/** Mana cost */
	UPROPERTY(EditDefaultsOnly, Category = "Costs")
	float ManaCost = 0.0f;

	/** Damage multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageMultiplier = 1.0f;

	/** Camera shake on fire */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UCameraShakeBase> FireCameraShakeClass;

	/** Camera shake scale */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float CameraShakeScale = 1.0f;

	// ============================================================================
	// State
	// ============================================================================

	/** Is currently charging? */
	bool bIsCharging = false;

	/** Charge start time */
	float ChargeStartTime = 0.0f;
};
