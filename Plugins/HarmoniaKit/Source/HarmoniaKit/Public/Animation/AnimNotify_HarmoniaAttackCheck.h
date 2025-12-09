// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "AnimNotify_HarmoniaAttackCheck.generated.h"

class UHarmoniaSenseComponent;

/**
 * UAnimNotify_HarmoniaAttackCheck
 *
 * Animation Notify that triggers attack hit detection via HarmoniaSenseComponent
 * Place this notify at the point in animation where weapon should check for hits
 *
 * Usage:
 * 1. Add this notify to attack animation montages
 * 2. Configure attack data or use component's default
 * 3. Specify which component to use (by name or use default)
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Harmonia Attack Check"))
class HARMONIAKIT_API UAnimNotify_HarmoniaAttackCheck : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_HarmoniaAttackCheck();

	//~UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotify interface

protected:
	/**
	 * Name of the HarmoniaSenseComponent to trigger
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
	 * Whether to stop any ongoing attack before starting new one
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bStopPreviousAttack = true;

	/**
	 * Find attack component on actor
	 */
	virtual UHarmoniaSenseComponent* FindAttackComponent(AActor* Owner) const;
};
