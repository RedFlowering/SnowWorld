// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "AnimNotifyState_CustomAttackWindow.generated.h"

class UHarmoniaSenseComponent;

/**
 * UAnimNotifyState_CustomAttackWindow
 *
 * Animation Notify State that maintains attack hit detection window
 * Uses custom attack data configured in the notify itself
 *
 * Usage:
 * 1. Add this notify state to attack animation montages
 * 2. Set bUseCustomAttackData = true
 * 3. Configure CustomAttackData with desired trace settings
 *
 * This is ideal for special attacks that need unique trace configurations
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Custom Attack Window"))
class HARMONIAKIT_API UAnimNotifyState_CustomAttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_CustomAttackWindow();

	//~UAnimNotifyState interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotifyState interface

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
	 * Whether to clear hit targets when window starts
	 * Set to false if you want to track hits across multiple windows
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bClearHitTargetsOnStart = true;

	/**
	 * Preview trace in Persona animation editor
	 * Enable this to visualize trace bounds while adjusting animation timing
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview")
	bool bShowPreviewTrace = false;

#if WITH_EDITORONLY_DATA
	/**
	 * Preview trace color
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview", meta = (EditCondition = "bShowPreviewTrace"))
	FColor PreviewTraceColor = FColor::Cyan;
#endif

	/**
	 * Find attack component on actor
	 */
	virtual UHarmoniaSenseComponent* FindAttackComponent(AActor* Owner) const;

#if WITH_EDITOR
protected:
	/**
	 * Draw preview trace in animation editor (Persona)
	 */
	void DrawPreviewTrace(USkeletalMeshComponent* MeshComp) const;
#endif
};
