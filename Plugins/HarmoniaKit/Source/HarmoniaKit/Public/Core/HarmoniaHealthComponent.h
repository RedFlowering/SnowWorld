// Copyright 2025 Snow Game Studio.

#pragma once

#include "Character/LyraHealthComponent.h"
#include "HarmoniaHealthComponent.generated.h"

class UHarmoniaHealthComponent;
class UHarmoniaAttributeSet;
class UAbilitySystemComponent;
class AActor;
struct FGameplayEffectSpec;

/**
 * UHarmoniaHealthComponent
 *
 * Health component that extends LyraHealthComponent to use HarmoniaAttributeSet.
 * Can be used with SetDefaultSubobjectClass to replace LyraHealthComponent in child classes.
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaHealthComponent : public ULyraHealthComponent
{
	GENERATED_BODY()

public:

	UHarmoniaHealthComponent(const FObjectInitializer& ObjectInitializer);

	// Initialize the component using an ability system component.
	// Overrides parent to use HarmoniaAttributeSet instead of LyraHealthSet.
	void InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC);

	// Returns the current health value from HarmoniaAttributeSet.
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	float GetHarmoniaHealth() const;

	// Returns the current maximum health value from HarmoniaAttributeSet.
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	float GetHarmoniaMaxHealth() const;

	// Returns the current health in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	float GetHarmoniaHealthNormalized() const;

	// Returns the current stamina value from HarmoniaAttributeSet.
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	float GetStamina() const;

	// Returns the current maximum stamina value from HarmoniaAttributeSet.
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	float GetMaxStamina() const;

	// Returns the current stamina in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	float GetStaminaNormalized() const;

protected:

	// Cached pointer to HarmoniaAttributeSet for quick access.
	UPROPERTY()
	TObjectPtr<const UHarmoniaAttributeSet> HarmoniaAttributeSet;

	// Handle attribute changes from HarmoniaAttributeSet.
	virtual void HandleHarmoniaHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleHarmoniaMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleHarmoniaOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
};
