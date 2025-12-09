// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Jump.generated.h"

class ACharacter;

/**
 * UHarmoniaGameplayAbility_Jump
 *
 * Jump ability with stamina integration and state tag management.
 * Uses ALS/Character's built-in jump system.
 *
 * Behavior:
 * - Triggers jump on activate
 * - Persists until landing (maintains State.InAir tag)
 * - Optional stamina cost via CostGameplayEffectClass
 * - Auto-mantling handled by ALS and HarmoniaCharacter (not this ability)
 *
 * Flow:
 * 1. Input pressed: ActivateAbility() -> Character->Jump() -> ActivationOwnedTags applied
 * 2. Character lands: OnLanded() -> EndAbility() -> Tags removed
 * 3. If ALS detects mantle: HarmoniaCharacter->OnMantlingStarted() -> Mantle GA activated -> Jump GA cancelled
 *
 * Usage:
 * - Add to AbilitySet with InputTag.Jump
 * - Bind to Space key with Pressed trigger in IMC
 * - Set ActivationOwnedTags: State.InAir (blocks other abilities while in air)
 * - Set CostGameplayEffectClass for stamina consumption (optional)
 * - Configure MantleAbilityClass in HarmoniaCharacter for auto-mantling
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Jump : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** Sprint ability class to cancel when jumping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	TSubclassOf<UGameplayAbility> SprintAbilityClass;

	/** Called when character lands - ends the ability */
	UFUNCTION()
	void OnLanded(const FHitResult& Hit);
};

