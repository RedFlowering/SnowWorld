// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MeleeAttackWindow.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseAttackComponent;

/**
 * Melee Attack Window Notify State
 * Marks the time window when weapon hit detection is active
 *
 * Usage:
 * - Place this at the start of weapon swing to the end
 * - NotifyBegin: Starts hit detection using HarmoniaSenseAttackComponent
 * - NotifyEnd: Stops hit detection
 * - Works with HarmoniaMeleeCombatComponent for damage calculation
 *
 * Example Timeline:
 * [Start]═══[30% Begin]════════════════[70% End]═══[100%]
 *                 ↓      Hit Detection      ↓
 *            StartAttack              StopAttack
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Melee Attack Window"))
class HARMONIAKIT_API UAnimNotifyState_MeleeAttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_MeleeAttackWindow();

	//~UAnimNotifyState interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotifyState interface

protected:
	/**
	 * Name of attack component to use (leave empty for default MainHand)
	 * Use this for off-hand attacks or specific weapon components
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName AttackComponentName = NAME_None;

	/**
	 * Show debug visualization for hit detection
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

private:
	/** Find melee combat component on actor */
	UHarmoniaMeleeCombatComponent* FindMeleeCombatComponent(AActor* Owner) const;

	/** Find attack component on actor */
	UHarmoniaSenseAttackComponent* FindAttackComponent(AActor* Owner) const;
};
