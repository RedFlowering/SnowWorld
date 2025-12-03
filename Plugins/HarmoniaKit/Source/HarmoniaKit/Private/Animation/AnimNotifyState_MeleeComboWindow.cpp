// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_MeleeComboWindow.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "GameFramework/Actor.h"

UAnimNotifyState_MeleeComboWindow::UAnimNotifyState_MeleeComboWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(100, 255, 100, 255); // Green for combo window
#endif
}

void UAnimNotifyState_MeleeComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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

	UHarmoniaMeleeCombatComponent* MeleeComponent = FindMeleeCombatComponent(Owner);
	if (!MeleeComponent)
	{
		return;
	}

	// Combo window is now active - managed by MeleeCombatComponent's timer
	// This notify state is just for visual feedback in the animation editor
}

void UAnimNotifyState_MeleeComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	UHarmoniaMeleeCombatComponent* MeleeComponent = FindMeleeCombatComponent(Owner);
	if (!MeleeComponent)
	{
		return;
	}

	// If auto advance is enabled and combo was queued, advance it
	if (bAutoAdvanceCombo && MeleeComponent->IsNextComboQueued())
	{
		MeleeComponent->AdvanceCombo();
	}
}

FString UAnimNotifyState_MeleeComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("Melee Combo Window");
}

UHarmoniaMeleeCombatComponent* UAnimNotifyState_MeleeComboWindow::FindMeleeCombatComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
}
