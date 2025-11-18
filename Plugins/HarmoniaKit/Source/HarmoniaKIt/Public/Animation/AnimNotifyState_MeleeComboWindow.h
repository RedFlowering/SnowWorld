// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MeleeComboWindow.generated.h"

class UHarmoniaMeleeCombatComponent;

/**
 * Melee Combo Window Notify State
 * Marks the time window when player can input next combo attack
 *
 * Usage:
 * - Place this at the end of attack animations
 * - During this window, players can queue next combo attack
 * - Automatically integrates with HarmoniaMeleeCombatComponent
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Melee Combo Window"))
class HARMONIAKIT_API UAnimNotifyState_MeleeComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_MeleeComboWindow();

	//~UAnimNotifyState interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotifyState interface

protected:
	/**
	 * Whether to automatically advance combo if next attack was queued
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bAutoAdvanceCombo = true;

	/**
	 * Find melee combat component on actor
	 */
	virtual UHarmoniaMeleeCombatComponent* FindMeleeCombatComponent(AActor* Owner) const;
};
