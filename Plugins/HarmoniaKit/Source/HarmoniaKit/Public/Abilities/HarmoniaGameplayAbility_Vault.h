// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Vault.generated.h"

/**
 * Vault/Parkour Gameplay Ability
 * Allows character to vault over obstacles
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.4.3 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Vault : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Vault(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted);

protected:
	/** Vault animation montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	TObjectPtr<UAnimMontage> VaultMontage;

	/** Vault distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	float VaultDistance = 300.0f;

	/** Maximum vault height */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	float MaxVaultHeight = 150.0f;

	/** Detection range for vaultable objects */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	float DetectionRange = 200.0f;

	/** Stamina cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	float StaminaCost = 20.0f;

private:
	/** Find vaultable obstacle */
	bool FindVaultableObstacle(const AActor* Avatar, FVector& OutVaultLocation) const;
};
