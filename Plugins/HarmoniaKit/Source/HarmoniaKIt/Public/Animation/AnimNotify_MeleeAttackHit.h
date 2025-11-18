// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MeleeAttackHit.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseAttackComponent;

/**
 * Melee Attack Hit Notify
 * Integrated melee combat attack check notify
 * Combines HarmoniaSenseAttackComponent with HarmoniaMeleeCombatComponent
 *
 * Usage:
 * - Place at the exact frame where weapon should hit
 * - Automatically uses current weapon's attack data
 * - Works with combo system
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Melee Attack Hit"))
class HARMONIAKIT_API UAnimNotify_MeleeAttackHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_MeleeAttackHit();

	//~UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotify interface

protected:
	/**
	 * Damage multiplier for this specific hit
	 * Multiplied with weapon's base damage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float DamageMultiplier = 1.0f;

	/**
	 * Whether this is a critical hit point (e.g., sweet spot)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bIsCriticalHitPoint = false;

	/**
	 * Critical damage multiplier if this is a critical hit point
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (EditCondition = "bIsCriticalHitPoint"))
	float CriticalDamageMultiplier = 2.0f;

	/**
	 * Name of attack component to use (leave empty for default)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName AttackComponentName = NAME_None;

	/**
	 * Find components
	 */
	virtual UHarmoniaMeleeCombatComponent* FindMeleeCombatComponent(AActor* Owner) const;
	virtual UHarmoniaSenseAttackComponent* FindAttackComponent(AActor* Owner) const;
};
