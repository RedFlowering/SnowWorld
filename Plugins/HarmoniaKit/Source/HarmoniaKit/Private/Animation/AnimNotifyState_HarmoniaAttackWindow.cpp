// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_HarmoniaAttackWindow.h"
#include "Components/HarmoniaSenseComponent.h"
#include "GameFramework/Actor.h"

UAnimNotifyState_HarmoniaAttackWindow::UAnimNotifyState_HarmoniaAttackWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 150, 100, 255);
#endif
}

void UAnimNotifyState_HarmoniaAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// Find attack component
	UHarmoniaSenseComponent* AttackComponent = FindAttackComponent(Owner);
	if (!AttackComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyState_HarmoniaAttackWindow: No HarmoniaSenseComponent found on %s"), *Owner->GetName());
		return;
	}

	// Clear hit targets if requested
	if (bClearHitTargetsOnStart)
	{
		AttackComponent->ClearHitTargets();
	}

	// Prepare attack data
	FHarmoniaAttackData AttackData = bUseCustomAttackData ? CustomAttackData : AttackComponent->AttackData;

	// Override detection duration to match notify state duration
	AttackData.TraceConfig.bContinuousDetection = true;
	AttackData.TraceConfig.DetectionDuration = TotalDuration;

	// Start attack
	AttackComponent->RequestStartAttack(AttackData);
}

void UAnimNotifyState_HarmoniaAttackWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// Attack component handles detection in its own tick
	// This tick is here if we need to do any per-frame updates in the future
}

void UAnimNotifyState_HarmoniaAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// Find attack component
	UHarmoniaSenseComponent* AttackComponent = FindAttackComponent(Owner);
	if (!AttackComponent)
	{
		return;
	}

	// Stop attack
	if (AttackComponent->IsAttacking())
	{
		AttackComponent->RequestStopAttack();
	}
}

FString UAnimNotifyState_HarmoniaAttackWindow::GetNotifyName_Implementation() const
{
	if (AttackComponentName.IsNone())
	{
		return TEXT("Harmonia Attack Window");
	}

	return FString::Printf(TEXT("Harmonia Attack Window (%s)"), *AttackComponentName.ToString());
}

UHarmoniaSenseComponent* UAnimNotifyState_HarmoniaAttackWindow::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// If component name is specified, find by name
	if (!AttackComponentName.IsNone())
	{
		TArray<UHarmoniaSenseComponent*> AttackComponents;
		Owner->GetComponents<UHarmoniaSenseComponent>(AttackComponents);

		for (UHarmoniaSenseComponent* Component : AttackComponents)
		{
			if (Component && Component->GetFName() == AttackComponentName)
			{
				return Component;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyState_HarmoniaAttackWindow: Could not find component named '%s' on %s"), *AttackComponentName.ToString(), *Owner->GetName());
		return nullptr;
	}

	// Otherwise, return first found component
	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}
