// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_CustomAttackWindow.h"
#include "Components/HarmoniaSenseComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UAnimNotifyState_CustomAttackWindow::UAnimNotifyState_CustomAttackWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 150, 100, 255);
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

	// Find attack component
	UHarmoniaSenseComponent* AttackComponent = FindAttackComponent(Owner);
	if (!AttackComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyState_CustomAttackWindow: No HarmoniaSenseComponent found on %s"), *Owner->GetName());
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

void UAnimNotifyState_CustomAttackWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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

		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyState_CustomAttackWindow: Could not find component named '%s' on %s"), *AttackComponentName.ToString(), *Owner->GetName());
		return nullptr;
	}

	// Otherwise, return first found component
	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}

#if WITH_EDITOR
void UAnimNotifyState_CustomAttackWindow::DrawPreviewTrace(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return;
	}

	const FHarmoniaAttackTraceConfig& TraceConfig = bUseCustomAttackData 
		? CustomAttackData.TraceConfig 
		: FHarmoniaAttackTraceConfig();

	// Get trace location (from socket if specified)
	FVector Location = MeshComp->GetComponentLocation() + TraceConfig.TraceOffset;
	if (!TraceConfig.SocketName.IsNone() && MeshComp->DoesSocketExist(TraceConfig.SocketName))
	{
		Location = MeshComp->GetSocketLocation(TraceConfig.SocketName) + TraceConfig.TraceOffset;
	}

	const FRotator Rotation = MeshComp->GetComponentRotation();
	const FVector Extent = TraceConfig.TraceExtent;

	switch (TraceConfig.TraceShape)
	{
	case EHarmoniaAttackTraceShape::Box:
		DrawDebugBox(MeshComp->GetWorld(), Location, Extent, Rotation.Quaternion(), PreviewTraceColor, false, -1.0f, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Sphere:
		DrawDebugSphere(MeshComp->GetWorld(), Location, Extent.X, 12, PreviewTraceColor, false, -1.0f, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Capsule:
		DrawDebugCapsule(MeshComp->GetWorld(), Location, Extent.Z, Extent.X, Rotation.Quaternion(), PreviewTraceColor, false, -1.0f, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Line:
		{
			const FVector EndLocation = Location + Rotation.Vector() * Extent.X;
			DrawDebugLine(MeshComp->GetWorld(), Location, EndLocation, PreviewTraceColor, false, -1.0f, 0, 2.0f);
		}
		break;

	default:
		break;
	}
}
#endif
