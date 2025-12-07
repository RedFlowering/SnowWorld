// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MeleeAttackHit.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseAttackComponent;

/**
 * Melee Attack Hit Notify
 * Triggers melee attack detection at the exact animation frame
 * 
 * Responsibilities:
 * - Trigger attack detection at correct timing
 * - Find and call HarmoniaSenseAttackComponent
 * 
 * NOT responsible for:
 * - Damage calculation (handled by AttackComponent using AttributeSet)
 * - Damage multipliers (handled by MeleeCombatComponent / DataTable)
 * - Critical hit (handled by AttributeSet)
 *
 * Usage:
 * - Place at the exact frame where weapon should hit
 * - Component will use current combo step data for damage config
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

