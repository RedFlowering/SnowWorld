// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_MeleeAttackWindow.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

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

	UHarmoniaSenseComponent* AttackComp = FindAttackComponent(Owner);
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

#if WITH_EDITOR
	// Editor preview trace visualization
	if (bShowPreviewTrace && MeshComp)
	{
		if (UWorld* World = MeshComp->GetWorld())
		{
			// Only draw in editor preview (not in PIE/game)
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

	UHarmoniaSenseComponent* AttackComp = FindAttackComponent(Owner);
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

UHarmoniaSenseComponent* UAnimNotifyState_MeleeAttackWindow::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// If specific component name is provided, find by name
	if (!AttackComponentName.IsNone())
	{
		TArray<UHarmoniaSenseComponent*> AttackComponents;
		Owner->GetComponents<UHarmoniaSenseComponent>(AttackComponents);
		
		for (UHarmoniaSenseComponent* Comp : AttackComponents)
		{
			if (Comp && Comp->GetFName() == AttackComponentName)
			{
				return Comp;
			}
		}
	}

	// Default: find first attack component
	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}

#if WITH_EDITOR
void UAnimNotifyState_MeleeAttackWindow::DrawPreviewTrace(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	// Get trace transform from preview socket with local offset
	FVector Location = MeshComp->GetComponentLocation() + PreviewTraceOffset;
	FRotator Rotation = MeshComp->GetComponentRotation() + PreviewRotationOffset;
	
	if (!PreviewSocketName.IsNone() && MeshComp->DoesSocketExist(PreviewSocketName))
	{
		// Compose transforms: Socket * LocalOffset (rotation first, then position)
		FTransform SocketTransform = MeshComp->GetSocketTransform(PreviewSocketName);
		FTransform LocalOffset(FQuat(PreviewRotationOffset), PreviewTraceOffset);
		FTransform FinalTransform = LocalOffset * SocketTransform;
		
		Location = FinalTransform.GetLocation();
		Rotation = FinalTransform.Rotator();
	}

	constexpr float TraceDuration = 0.1f; // Persist until next tick

	switch (PreviewTraceShape)
	{
	case EHarmoniaAttackTraceShape::Box:
		DrawDebugBox(MeshComp->GetWorld(), Location, PreviewTraceExtent, Rotation.Quaternion(), PreviewTraceColor, false, TraceDuration, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Sphere:
		DrawDebugSphere(MeshComp->GetWorld(), Location, PreviewTraceExtent.X, 12, PreviewTraceColor, false, TraceDuration, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Capsule:
		DrawDebugCapsule(MeshComp->GetWorld(), Location, PreviewTraceExtent.Z, PreviewTraceExtent.X, Rotation.Quaternion(), PreviewTraceColor, false, TraceDuration, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Line:
		{
			const FVector EndLocation = Location + Rotation.Vector() * PreviewTraceExtent.X;
			DrawDebugLine(MeshComp->GetWorld(), Location, EndLocation, PreviewTraceColor, false, TraceDuration, 0, 2.0f);
		}
		break;

	default:
		break;
	}
}
#endif
