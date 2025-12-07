// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_MeleeAttackWindow.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotifyState_MeleeAttackWindow::UAnimNotifyState_MeleeAttackWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 100, 100, 255); // Red-ish color for attack window
#endif
}

void UAnimNotifyState_MeleeAttackWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
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

	UHarmoniaSenseAttackComponent* AttackComp = FindAttackComponent(Owner);
	if (!AttackComp)
	{
		return;
	}

	// Try to get attack data from combo table first
	UHarmoniaMeleeCombatComponent* MeleeComp = FindMeleeCombatComponent(Owner);
	FHarmoniaAttackData AttackData;
	bool bGotAttackData = false;

	if (MeleeComp)
	{
		// Get attack data from current combo step in table
		bGotAttackData = MeleeComp->GetCurrentComboAttackData(AttackData);
	}

	// Fallback to component's default if no combo data
	if (!bGotAttackData)
	{
		AttackData = AttackComp->AttackData;
	}

	// Apply debug setting
	if (bShowDebug)
	{
		AttackData.TraceConfig.bShowDebugTrace = true;
	}

	// Start attack detection with the resolved attack data
	AttackComp->RequestStartAttack(AttackData);
}

void UAnimNotifyState_MeleeAttackWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	// Tick is handled by SenseAttackComponent internally
	// No additional logic needed here
}

void UAnimNotifyState_MeleeAttackWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
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

	UHarmoniaSenseAttackComponent* AttackComp = FindAttackComponent(Owner);
	if (AttackComp && AttackComp->IsAttacking())
	{
		AttackComp->RequestStopAttack();
	}
}

FString UAnimNotifyState_MeleeAttackWindow::GetNotifyName_Implementation() const
{
	return TEXT("Melee Attack Window");
}

UHarmoniaMeleeCombatComponent* UAnimNotifyState_MeleeAttackWindow::FindMeleeCombatComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
}

UHarmoniaSenseAttackComponent* UAnimNotifyState_MeleeAttackWindow::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// If specific component name is provided, find by name
	if (!AttackComponentName.IsNone())
	{
		TArray<UHarmoniaSenseAttackComponent*> AttackComponents;
		Owner->GetComponents<UHarmoniaSenseAttackComponent>(AttackComponents);
		
		for (UHarmoniaSenseAttackComponent* Comp : AttackComponents)
		{
			if (Comp && Comp->GetFName() == AttackComponentName)
			{
				return Comp;
			}
		}
	}

	// Default: find first attack component
	return Owner->FindComponentByClass<UHarmoniaSenseAttackComponent>();
}
