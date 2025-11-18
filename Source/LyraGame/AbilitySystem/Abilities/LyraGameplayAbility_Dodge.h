// Copyright 2024 Snow Game Studio.

#pragma once

#include "LyraGameplayAbility.h"
#include "LyraGameplayAbility_Dodge.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * ULyraGameplayAbility_Dodge
 *
 * Gameplay ability for souls-like dodge roll mechanic with invincibility frames.
 * Allows the player to dodge in the movement direction with i-frames.
 */
UCLASS(Abstract)
class LYRAGAME_API ULyraGameplayAbility_Dodge : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	ULyraGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	/** Called to perform the dodge action */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void PerformDodge();

	/** Get the dodge direction based on input */
	FVector GetDodgeDirection() const;

	/** Apply invincibility frames */
	void ApplyInvincibilityFrames();

	/** Remove invincibility frames */
	void RemoveInvincibilityFrames();

protected:
	/** Dodge animation montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/** Dodge distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeDistance = 400.0f;

	/** Dodge duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float DodgeDuration = 0.5f;

	/** Invincibility duration (i-frames) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float InvincibilityDuration = 0.3f;

	/** Gameplay effect for invincibility */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	TSubclassOf<UGameplayEffect> InvincibilityEffect;

	/** Stamina cost for dodging */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge")
	float StaminaCost = 20.0f;

private:
	/** Active invincibility effect handle */
	FActiveGameplayEffectHandle InvincibilityEffectHandle;
};
