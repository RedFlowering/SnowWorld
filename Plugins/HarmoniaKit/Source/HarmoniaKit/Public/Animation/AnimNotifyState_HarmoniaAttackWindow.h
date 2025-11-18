// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "AnimNotifyState_HarmoniaAttackWindow.generated.h"

class UHarmoniaSenseAttackComponent;

/**
 * UAnimNotifyState_HarmoniaAttackWindow
 *
 * Animation Notify State that maintains attack hit detection window
 * The attack component will continuously check for hits during this window
 *
 * Usage:
 * 1. Add this notify state to attack animation montages
 * 2. Set the duration to cover the entire attack swing/motion
 * 3. Configure attack data or use component's default
 *
 * This is ideal for slower, sweeping attacks where the weapon moves through space
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Harmonia Attack Window"))
class HARMONIAKIT_API UAnimNotifyState_HarmoniaAttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_HarmoniaAttackWindow();

	//~UAnimNotifyState interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotifyState interface

protected:
	/**
	 * Name of the HarmoniaSenseAttackComponent to trigger
	 * Leave empty to use the first found component
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName AttackComponentName = NAME_None;

	/**
	 * Whether to use custom attack data or component's default
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bUseCustomAttackData = false;

	/**
	 * Custom attack data (only used if bUseCustomAttackData is true)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (EditCondition = "bUseCustomAttackData"))
	FHarmoniaAttackData CustomAttackData;

	/**
	 * Whether to clear hit targets when window starts
	 * Set to false if you want to track hits across multiple windows
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bClearHitTargetsOnStart = true;

	/**
	 * Find attack component on actor
	 */
	virtual UHarmoniaSenseAttackComponent* FindAttackComponent(AActor* Owner) const;
};
