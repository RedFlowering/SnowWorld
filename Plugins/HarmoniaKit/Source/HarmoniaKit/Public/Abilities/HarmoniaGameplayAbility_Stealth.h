// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Stealth.generated.h"

/**
 * Stealth Monster Ability
 * - Enter stealth mode (invisible/translucent)
 * - Ambush attack with bonus damage
 * - Break stealth on attack or when damaged
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.6.2 for tag configuration
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Stealth : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Stealth();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	/** Stealth duration (0 = infinite until broken) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float StealthDuration = 0.0f;

	/** Stealth cooldown after breaking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float StealthCooldown = 10.0f;

	/** Opacity when stealthed (0.0 = invisible, 1.0 = visible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StealthOpacity = 0.2f;

	/** Ambush damage multiplier (first attack from stealth) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float AmbushDamageMultiplier = 2.0f;

	/** Movement speed multiplier while stealthed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float StealthMovementSpeedMultiplier = 1.3f;

	/** Detection range multiplier for enemies (how hard to detect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float DetectionRangeMultiplier = 0.3f;

	/** Gameplay effect to apply stealth status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	TSubclassOf<UGameplayEffect> StealthEffect;

	/** Gameplay effect for ambush damage bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	TSubclassOf<UGameplayEffect> AmbushEffect;

	/** Handle for stealth effect */
	FActiveGameplayEffectHandle StealthEffectHandle;

	/** Handle for ambush effect */
	FActiveGameplayEffectHandle AmbushEffectHandle;

	/** Is currently stealthed */
	bool bIsStealthed = false;

	/** Original opacity values for restoration */
	TMap<UMeshComponent*, float> OriginalOpacities;

	/** Enter stealth mode */
	UFUNCTION(BlueprintCallable, Category = "Stealth")
	void EnterStealth();

	/** Break stealth mode */
	UFUNCTION(BlueprintCallable, Category = "Stealth")
	void BreakStealth();

	/** Apply visual stealth effect */
	void ApplyStealthVisuals();

	/** Remove visual stealth effect */
	void RemoveStealthVisuals();

	/** Apply ambush bonus for next attack */
	void ApplyAmbushBonus();

	/** Called when avatar takes damage - breaks stealth */
	UFUNCTION()
	void OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
};
