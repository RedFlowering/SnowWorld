// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_SweepTrace.h"
#include "Components/HarmoniaSweepTraceComponent.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UAnimNotifyState_SweepTrace::UAnimNotifyState_SweepTrace()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(200, 100, 50, 255);  // Orange-ish color
#endif
}

FString UAnimNotifyState_SweepTrace::GetNotifyName_Implementation() const
{
	if (!ComponentTag.IsNone())
	{
		return FString::Printf(TEXT("Sweep Trace [%s]"), *ComponentTag.ToString());
	}
	return TEXT("Sweep Trace");
}

void UAnimNotifyState_SweepTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// Store notify duration for debug draw persistence
	NotifyDuration = TotalDuration;

	// Find all matching sweep components
	TArray<UHarmoniaSweepTraceComponent*> SweepComps = FindAllSweepComponents(MeshComp);
	if (SweepComps.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANIM_NOTIFY_SWEEP] NotifyBegin: No SweepTraceComponent found on '%s'"),
			MeshComp->GetOwner() ? *MeshComp->GetOwner()->GetName() : TEXT("NULL"));
		return;
	}

	// Clear previous cached data
	CachedSweepComponents.Empty();
	OriginalValues.Empty();

	// Cache and configure all components
	for (UHarmoniaSweepTraceComponent* SweepComp : SweepComps)
	{
		if (!SweepComp)
		{
			continue;
		}

		// Cache for later use
		CachedSweepComponents.Add(SweepComp);

		// Store original values
		FOriginalComponentValues Original;
		Original.SweepExtent = SweepComp->SweepBoxHalfExtent;
		Original.bDebugDraw = SweepComp->bDebugDraw;
		OriginalValues.Add(Original);

		// Apply overrides
		if (!OverrideSweepExtent.IsNearlyZero())
		{
			SweepComp->SweepBoxHalfExtent = OverrideSweepExtent;
		}
		
		if (bDebugDrawDuringNotify)
		{
			SweepComp->bDebugDraw = true;
			// Set duration to notify length so traces persist
			SweepComp->DebugDrawDuration = TotalDuration;
		}

		// Start the sweep
		SweepComp->StartSweep();

		UE_LOG(LogTemp, Verbose, TEXT("[ANIM_NOTIFY_SWEEP] NotifyBegin: Started sweep on '%s'"),
			*SweepComp->GetName());
	}

	// Clear hit actors for new attack window on MeleeCombatComponent
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UHarmoniaMeleeCombatComponent* MeleeCombat = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
		{
			MeleeCombat->SetInAttackWindow(true);
		}
	}
}

void UAnimNotifyState_SweepTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// Editor preview drawing
	if (bShowPreviewInEditor && MeshComp && MeshComp->GetWorld())
	{
		if (MeshComp->GetWorld()->WorldType == EWorldType::EditorPreview)
		{
			DrawEditorPreview(MeshComp);
			return; // Don't run actual sweep in editor
		}
	}

	// Tick all cached sweep components
	for (TWeakObjectPtr<UHarmoniaSweepTraceComponent>& WeakComp : CachedSweepComponents)
	{
		if (UHarmoniaSweepTraceComponent* SweepComp = WeakComp.Get())
		{
			SweepComp->TickSweep(FrameDeltaTime);
		}
	}
}

void UAnimNotifyState_SweepTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Restore and stop all cached sweep components
	for (int32 i = 0; i < CachedSweepComponents.Num(); ++i)
	{
		if (UHarmoniaSweepTraceComponent* SweepComp = CachedSweepComponents[i].Get())
		{
			// Stop the sweep
			SweepComp->StopSweep();

			// Restore original values
			if (OriginalValues.IsValidIndex(i))
			{
				if (!OverrideSweepExtent.IsNearlyZero())
				{
					SweepComp->SweepBoxHalfExtent = OriginalValues[i].SweepExtent;
				}
				
				if (bDebugDrawDuringNotify)
				{
					SweepComp->bDebugDraw = OriginalValues[i].bDebugDraw;
					SweepComp->DebugDrawDuration = 0.0f;
				}
			}

			UE_LOG(LogTemp, Verbose, TEXT("[ANIM_NOTIFY_SWEEP] NotifyEnd: Stopped sweep on '%s', HitCount=%d"),
				*SweepComp->GetName(), SweepComp->GetHitActorsThisSweep().Num());
		}
	}

	// Clear attack window on MeleeCombatComponent
	if (MeshComp)
	{
		if (AActor* Owner = MeshComp->GetOwner())
		{
			if (UHarmoniaMeleeCombatComponent* MeleeCombat = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
			{
				MeleeCombat->SetInAttackWindow(false);
			}
		}
	}

	CachedSweepComponents.Empty();
	OriginalValues.Empty();
}

TArray<UHarmoniaSweepTraceComponent*> UAnimNotifyState_SweepTrace::FindAllSweepComponents(USkeletalMeshComponent* MeshComp) const
{
	TArray<UHarmoniaSweepTraceComponent*> Result;

	if (!MeshComp)
	{
		return Result;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return Result;
	}

	TArray<UHarmoniaSweepTraceComponent*> AllSweepComps;
	Owner->GetComponents<UHarmoniaSweepTraceComponent>(AllSweepComps);

	// If ComponentTag is specified, filter by tag
	if (!ComponentTag.IsNone())
	{
		for (UHarmoniaSweepTraceComponent* Comp : AllSweepComps)
		{
			if (Comp && Comp->ComponentHasTag(ComponentTag))
			{
				Result.Add(Comp);
			}
		}
		
		if (Result.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANIM_NOTIFY_SWEEP] FindAllSweepComponents: No components with tag '%s' found on '%s'"),
				*ComponentTag.ToString(), *Owner->GetName());
		}
	}
	else
	{
		// No tag specified, return all sweep components
		Result = AllSweepComps;
	}

	return Result;
}

void UAnimNotifyState_SweepTrace::DrawEditorPreview(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp || !MeshComp->GetWorld())
	{
		return;
	}

	// Check if socket exists on preview mesh
	if (!MeshComp->DoesSocketExist(PreviewSocketName))
	{
		return;
	}

	// Get socket transform
	FTransform SocketTransform = MeshComp->GetSocketTransform(PreviewSocketName, ERelativeTransformSpace::RTS_World);

	// Apply local offset and rotation
	if (!PreviewSocketOffset.IsNearlyZero() || !PreviewSocketRotation.IsNearlyZero())
	{
		FTransform LocalTransform(PreviewSocketRotation, PreviewSocketOffset);
		SocketTransform = LocalTransform * SocketTransform;
	}

	// Use override extent if set, otherwise use preview extent
	FVector SweepExtent = !OverrideSweepExtent.IsNearlyZero() ? OverrideSweepExtent : PreviewSweepExtent;

	// Draw based on shape type
	switch (PreviewShapeType)
	{
	case EPreviewShapeType::Box:
		DrawDebugBox(
			MeshComp->GetWorld(),
			SocketTransform.GetLocation(),
			SweepExtent,
			SocketTransform.GetRotation(),
			PreviewColor,
			false,
			0.0f,
			0,
			PreviewThickness
		);
		break;

	case EPreviewShapeType::Sphere:
		DrawDebugSphere(
			MeshComp->GetWorld(),
			SocketTransform.GetLocation(),
			PreviewRadius,
			PreviewSegments,
			PreviewColor,
			false,
			0.0f,
			0,
			PreviewThickness
		);
		break;

	case EPreviewShapeType::Capsule:
		DrawDebugCapsule(
			MeshComp->GetWorld(),
			SocketTransform.GetLocation(),
			PreviewCapsuleHalfHeight,
			PreviewRadius,
			SocketTransform.GetRotation(),
			PreviewColor,
			false,
			0.0f,
			0,
			PreviewThickness
		);
		break;
	}
}
