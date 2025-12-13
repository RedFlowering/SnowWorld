// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_CustomAttackWindow.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UAnimNotifyState_CustomAttackWindow::UAnimNotifyState_CustomAttackWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 100, 150, 255);
#endif
}

void UAnimNotifyState_CustomAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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
	UHarmoniaMeleeCombatComponent* CombatComponent = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyState_CustomAttackWindow: No HarmoniaMeleeCombatComponent found on %s"), *Owner->GetName());
		return;
	}

	// Start attack via combat component
	// AttackType determined by component's current state
	EHarmoniaAttackType AttackType = bUseCustomAttackData ? EHarmoniaAttackType::Light : CombatComponent->GetCurrentAttackType();
	CombatComponent->StartAttack(AttackType);
}

void UAnimNotifyState_CustomAttackWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

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

void UAnimNotifyState_CustomAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
	UHarmoniaMeleeCombatComponent* CombatComponent = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
	if (!CombatComponent)
	{
		return;
	}

	// End attack via combat component
	if (CombatComponent->IsAttacking())
	{
		CombatComponent->EndAttack();
	}
}

FString UAnimNotifyState_CustomAttackWindow::GetNotifyName_Implementation() const
{
	if (AttackComponentName.IsNone())
	{
		return TEXT("Custom Attack Window");
	}

	return FString::Printf(TEXT("Custom Attack Window (%s)"), *AttackComponentName.ToString());
}

UHarmoniaSenseComponent* UAnimNotifyState_CustomAttackWindow::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// Legacy - returns HarmoniaSenseComponent for compatibility
	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}

#if WITH_EDITOR
void UAnimNotifyState_CustomAttackWindow::DrawPreviewTrace(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	// Use component location for preview (Sensor BP handles actual detection position)
	FVector Location = MeshComp->GetComponentLocation();

	// Draw simple sphere preview (SenseSystem handles actual detection shape)
	constexpr float PreviewRadius = 50.0f;
	DrawDebugSphere(MeshComp->GetWorld(), Location, PreviewRadius, 12, PreviewTraceColor, false, -1.0f, 0, 2.0f);
}
#endif
