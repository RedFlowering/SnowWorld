// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_MeleeAttackWindow.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UAnimNotifyState_MeleeAttackWindow::UAnimNotifyState_MeleeAttackWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 150, 100, 255);
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

	// Find combat component
	UHarmoniaMeleeCombatComponent* MeleeComp = FindMeleeCombatComponent(Owner);
	if (!MeleeComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyState_MeleeAttackWindow: No HarmoniaMeleeCombatComponent found on %s"), *Owner->GetName());
		return;
	}

	// Start attack via combat component (also opens attack window)
	MeleeComp->StartAttack(MeleeComp->GetCurrentAttackType());
	MeleeComp->SetInAttackWindow(true);
}

void UAnimNotifyState_MeleeAttackWindow::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// Trigger Manual sensors during attack window
	if (UHarmoniaSenseComponent* SenseComp = Owner->FindComponentByClass<UHarmoniaSenseComponent>())
	{
		SenseComp->TriggerManualSensors();
	}

#if WITH_EDITOR
	// Editor preview trace visualization
	if (bShowPreviewTrace && MeshComp)
	{
		if (UWorld* World = MeshComp->GetWorld())
		{
			if (!World->IsGameWorld())
			{
				DrawPreviewTrace(MeshComp);
			}
		}
	}
#endif
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

	// Find combat component
	UHarmoniaMeleeCombatComponent* MeleeComp = FindMeleeCombatComponent(Owner);
	if (MeleeComp)
	{
		// Close attack window and end attack
		MeleeComp->SetInAttackWindow(false);
		if (MeleeComp->IsAttacking())
		{
			MeleeComp->EndAttack();
		}
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

UHarmoniaSenseComponent* UAnimNotifyState_MeleeAttackWindow::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}

#if WITH_EDITOR
void UAnimNotifyState_MeleeAttackWindow::DrawPreviewTrace(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	// Draw simple sphere preview at component location
	FVector Location = MeshComp->GetComponentLocation();
	constexpr float PreviewRadius = 50.0f;
	DrawDebugSphere(MeshComp->GetWorld(), Location, PreviewRadius, 12, PreviewTraceColor, false, -1.0f, 0, 2.0f);
}
#endif
