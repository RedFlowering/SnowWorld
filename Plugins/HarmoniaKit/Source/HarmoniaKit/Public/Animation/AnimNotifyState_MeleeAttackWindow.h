// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "AnimNotifyState_MeleeAttackWindow.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseComponent;

/**
 * Melee Attack Window Notify State
 * Marks the time window when weapon hit detection is active
 *
 * Usage:
 * - Place this at the start of weapon swing to the end
 * - NotifyBegin: Starts hit detection using HarmoniaSenseComponent
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

	/**
	 * Name of attack component to use (leave empty for default MainHand)
	 * Use this for off-hand attacks or specific weapon components
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName AttackComponentName = NAME_None;

	/**
	 * Show debug visualization for hit detection during gameplay
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

	/**
	 * SenseConfig row name for preview drawing
	 * Used to get Sensor BP class for debug visualization
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FName PreviewSenseConfigRowName = NAME_None;

	/**
	 * Preview trace in Persona animation editor
	 * Enable this to visualize trace bounds while adjusting animation timing
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview")
	bool bShowPreviewTrace = false;

#if WITH_EDITORONLY_DATA
	/**
	 * Socket name for preview trace (used when component data unavailable in Persona)
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview", meta = (EditCondition = "bShowPreviewTrace"))
	FName PreviewSocketName = FName("hand_r");

	/**
	 * Preview trace offset from socket
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview", meta = (EditCondition = "bShowPreviewTrace"))
	FVector PreviewTraceOffset = FVector::ZeroVector;

	/**
	 * Preview trace rotation offset
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview", meta = (EditCondition = "bShowPreviewTrace"))
	FRotator PreviewRotationOffset = FRotator::ZeroRotator;

	/**
	 * Preview trace color
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview", meta = (EditCondition = "bShowPreviewTrace"))
	FColor PreviewTraceColor = FColor::Cyan;

	/**
	 * Preview trace radius (fallback when Sensor CDO not available)
	 */
	UPROPERTY(EditAnywhere, Category = "Debug|Editor Preview", meta = (EditCondition = "bShowPreviewTrace"))
	float PreviewTraceRadius = 50.0f;
#endif

private:
	/** Find melee combat component on actor */
	UHarmoniaMeleeCombatComponent* FindMeleeCombatComponent(AActor* Owner) const;

	/** Find attack component on actor */
	UHarmoniaSenseComponent* FindAttackComponent(AActor* Owner) const;

#if WITH_EDITOR
protected:
	/**
	 * Draw preview trace in animation editor (Persona)
	 */
	void DrawPreviewTrace(USkeletalMeshComponent* MeshComp) const;
#endif
};
