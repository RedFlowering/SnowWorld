// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Dodge.generated.h"

class UHarmoniaMeleeCombatComponent;

/**
 * Dodge Gameplay Ability
 * Roll/dash to avoid attacks
 *
 * Features:
 * - Direction-based movement
 * - I-frames (invincibility frames)
 * - Stamina consumption
 * - Can attack after dodge
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Dodge : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	/** Animation callbacks */
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageInterrupted();

protected:
	/** Tags to apply while dodging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	FGameplayTagContainer DodgingTags;

	/** Tags to block while dodging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	FGameplayTagContainer BlockedTags;

	/** Dodge/roll animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/** Dodge distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeDistance = 400.0f;

	/** Dodge duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeDuration = 0.6f;

	/** I-frame start time (seconds into dodge) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float IFrameStartTime = 0.1f;

	/** I-frame duration (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float IFrameDuration = 0.3f;

	/** Stamina cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeStaminaCost = 20.0f;

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent;

private:
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** Start i-frames */
	void StartIFrames();

	/** End i-frames */
	void EndIFrames();

	FTimerHandle IFrameStartTimerHandle;
	FTimerHandle IFrameEndTimerHandle;
};
