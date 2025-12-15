// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSweepTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "CosmeticBFL.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"

UHarmoniaSweepTraceComponent::UHarmoniaSweepTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	// Default object types to Pawn
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3); // Pawn
}

void UHarmoniaSweepTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR
	// Editor preview - always show when selected and preview enabled
	if (bShowEditorPreview && !GetWorld()->IsGameWorld())
	{
		FTransform SocketTransformWorld = GetSweepSocketTransform();
		if (!SocketTransformWorld.Equals(FTransform::Identity))
		{
			switch (SweepShapeType)
			{
			case ESweepShapeType::Box:
				DrawDebugBox(
					GetWorld(),
					SocketTransformWorld.GetLocation(),
					SweepBoxHalfExtent,
					SocketTransformWorld.GetRotation(),
					EditorPreviewColor,
					false,
					-1.0f,
					0,
					EditorPreviewThickness
				);
				break;
			case ESweepShapeType::Sphere:
				DrawDebugSphere(
					GetWorld(),
					SocketTransformWorld.GetLocation(),
					SweepRadius,
					EditorPreviewSegments,
					EditorPreviewColor,
					false,
					-1.0f,
					0,
					EditorPreviewThickness
				);
				break;
			case ESweepShapeType::Capsule:
				DrawDebugCapsule(
					GetWorld(),
					SocketTransformWorld.GetLocation(),
					SweepCapsuleHalfHeight,
					SweepRadius,
					SocketTransformWorld.GetRotation(),
					EditorPreviewColor,
					false,
					-1.0f,
					0,
					EditorPreviewThickness
				);
				break;
			}
		}
	}
#endif
}

#if WITH_EDITOR
void UHarmoniaSweepTraceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// Auto-find mesh when socket name changes
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UHarmoniaSweepTraceComponent, SocketName))
	{
		if (bAutoFindOwnerMesh && !TargetMeshComponent.IsValid())
		{
			TargetMeshComponent = FindOwnerSkeletalMesh();
		}
	}
}
#endif

// ============================================================================
// API Implementation
// ============================================================================

void UHarmoniaSweepTraceComponent::StartSweep()
{
	// Auto-find mesh if needed
	if (bAutoFindOwnerMesh && !TargetMeshComponent.IsValid())
	{
		TargetMeshComponent = FindOwnerSkeletalMesh();
	}

	bIsSweeping = true;
	bHasPreviousTransform = false;
	HitActorsThisSweep.Empty();
	
	// Record initial position
	PreviousTransform = GetSweepSocketTransform();
	bHasPreviousTransform = true;

	UE_LOG(LogTemp, Verbose, TEXT("[SWEEP_TRACE] StartSweep: Socket='%s'"), *SocketName.ToString());
}

void UHarmoniaSweepTraceComponent::TickSweep(float DeltaTime)
{
	if (!bIsSweeping)
	{
		return;
	}

	FTransform CurrentTransform = GetSweepSocketTransform();
	
	if (bHasPreviousTransform)
	{
		const float DistanceSquared = FVector::DistSquared(PreviousTransform.GetLocation(), CurrentTransform.GetLocation());
		
		// Always draw debug trace (even without movement)
		if (bDebugDraw)
		{
			DrawDebugSweep(
				PreviousTransform.GetLocation(),
				CurrentTransform.GetLocation(),
				CurrentTransform.GetRotation(),
				false
			);
		}
		
		// Only perform actual sweep if there's meaningful movement
		if (DistanceSquared > KINDA_SMALL_NUMBER)
		{
			const int32 Steps = FMath::Clamp(InterpolationSteps, 1, 10);
			
			if (Steps == 1)
			{
				// No interpolation
				PerformSweepWithoutDebug(PreviousTransform, CurrentTransform);
			}
			else
			{
				// Interpolated sweeps
				for (int32 i = 0; i < Steps; ++i)
				{
					const float Alpha0 = static_cast<float>(i) / static_cast<float>(Steps);
					const float Alpha1 = static_cast<float>(i + 1) / static_cast<float>(Steps);
					
					FTransform InterpFrom;
					InterpFrom.Blend(PreviousTransform, CurrentTransform, Alpha0);
					
					FTransform InterpTo;
					InterpTo.Blend(PreviousTransform, CurrentTransform, Alpha1);
					
					PerformSweepWithoutDebug(InterpFrom, InterpTo);
				}
			}
		}
	}
	
	// Store for next frame
	PreviousTransform = CurrentTransform;
	bHasPreviousTransform = true;
}

void UHarmoniaSweepTraceComponent::StopSweep()
{
	bIsSweeping = false;
	bHasPreviousTransform = false;
	// Note: We don't clear HitActorsThisSweep here - it persists until next StartSweep
	// This allows querying GetHitActorsThisSweep() after the sweep ends
	
	UE_LOG(LogTemp, Verbose, TEXT("[SWEEP_TRACE] StopSweep: HitCount=%d"), HitActorsThisSweep.Num());
}

void UHarmoniaSweepTraceComponent::SetTargetMesh(USkeletalMeshComponent* InMesh)
{
	TargetMeshComponent = InMesh;
}

FTransform UHarmoniaSweepTraceComponent::GetSweepSocketTransform() const
{
	if (!TargetMeshComponent.IsValid())
	{
		// Try auto-find
		USkeletalMeshComponent* FoundMesh = FindOwnerSkeletalMesh();
		if (FoundMesh)
		{
			const_cast<UHarmoniaSweepTraceComponent*>(this)->TargetMeshComponent = FoundMesh;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SWEEP_TRACE] GetSweepSocketTransform: No TargetMeshComponent found"));
			return FTransform::Identity;
		}
	}

	if (!TargetMeshComponent->DoesSocketExist(SocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SWEEP_TRACE] GetSweepSocketTransform: Socket '%s' not found on mesh '%s'"),
			*SocketName.ToString(),
			*TargetMeshComponent->GetName());
		return FTransform::Identity;
	}

	// Get socket transform in world space
	FTransform SocketTransformWorld = TargetMeshComponent->GetSocketTransform(SocketName, ERelativeTransformSpace::RTS_World);

	// Apply local offset and rotation
	if (!SocketOffset.IsNearlyZero() || !SocketRotation.IsNearlyZero())
	{
		FTransform LocalTransform(SocketRotation, SocketOffset);
		SocketTransformWorld = LocalTransform * SocketTransformWorld;
	}

	return SocketTransformWorld;
}

TArray<AActor*> UHarmoniaSweepTraceComponent::GetHitActorsThisSweep() const
{
	TArray<AActor*> Result;
	for (const TWeakObjectPtr<AActor>& WeakActor : HitActorsThisSweep)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Result.Add(Actor);
		}
	}
	return Result;
}

void UHarmoniaSweepTraceComponent::ClearHitActors()
{
	HitActorsThisSweep.Empty();
}

// ============================================================================
// Internal Implementation
// ============================================================================

USkeletalMeshComponent* UHarmoniaSweepTraceComponent::FindOwnerSkeletalMesh() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Priority 1: CosmeticActor's visual mesh (if option enabled)
	if (bUseCosmeticActorMesh)
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			if (USkeletalMeshComponent* CosmeticMesh = UCosmeticBFL::GetVisualMesh(Character))
			{
				return CosmeticMesh;
			}
		}
	}

	// Priority 2: Character's mesh
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		return Character->GetMesh();
	}

	// Priority 3: Find any skeletal mesh component
	return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

void UHarmoniaSweepTraceComponent::PerformSweepWithoutDebug(const FTransform& FromTransform, const FTransform& ToTransform)
{
	if (!GetWorld())
	{
		return;
	}

	// Setup collision query parameters
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	QueryParams.AddIgnoredActor(GetOwner());
	
	// Ignore all actors attached to owner (CosmeticActor, equipment, etc.)
	if (AActor* Owner = GetOwner())
	{
		TArray<AActor*> AttachedActors;
		Owner->GetAttachedActors(AttachedActors, true, true);  // Recursive, include all descendants
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AttachedActor)
			{
				QueryParams.AddIgnoredActor(AttachedActor);
			}
		}
	}
	
	// Add additional ignored actors
	for (AActor* IgnoredActor : IgnoredActors)
	{
		if (IgnoredActor)
		{
			QueryParams.AddIgnoredActor(IgnoredActor);
		}
	}

	// Setup object type query
	FCollisionObjectQueryParams ObjectQueryParams;
	for (EObjectTypeQuery ObjectType : ObjectTypes)
	{
		ObjectQueryParams.AddObjectTypesToQuery(UEngineTypes::ConvertToCollisionChannel(ObjectType));
	}

	// Create collision shape based on type
	FCollisionShape CollisionShape;
	switch (SweepShapeType)
	{
	case ESweepShapeType::Box:
		CollisionShape = FCollisionShape::MakeBox(SweepBoxHalfExtent);
		break;
	case ESweepShapeType::Sphere:
		CollisionShape = FCollisionShape::MakeSphere(SweepRadius);
		break;
	case ESweepShapeType::Capsule:
		CollisionShape = FCollisionShape::MakeCapsule(SweepRadius, SweepCapsuleHalfHeight);
		break;
	}

	// Perform sweep
	TArray<FHitResult> HitResults;
	GetWorld()->SweepMultiByObjectType(
		HitResults,
		FromTransform.GetLocation(),
		ToTransform.GetLocation(),
		FromTransform.GetRotation(),
		ObjectQueryParams,
		CollisionShape,
		QueryParams
	);

	// Process hits (no debug draw here - handled separately in TickSweep)
	ProcessHits(HitResults);
}

void UHarmoniaSweepTraceComponent::PerformSweep(const FTransform& FromTransform, const FTransform& ToTransform)
{
	PerformSweepWithoutDebug(FromTransform, ToTransform);
	
	if (bDebugDraw)
	{
		DrawDebugSweep(
			FromTransform.GetLocation(),
			ToTransform.GetLocation(),
			FromTransform.GetRotation(),
			false
		);
	}
}

void UHarmoniaSweepTraceComponent::ProcessHits(const TArray<FHitResult>& HitResults)
{
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		// Skip owner
		if (HitActor == GetOwner())
		{
			continue;
		}

		// Record hit for tracking (but always broadcast - deduplication is handled by listener)
		bool bIsNewHit = !HitActorsThisSweep.Contains(HitActor);
		if (bIsNewHit)
		{
			HitActorsThisSweep.Add(HitActor);
		}

		// Always broadcast delegate (let listener handle deduplication)
		OnSweepTraceHit.Broadcast(HitActor, Hit, this);

		if (bIsNewHit)
		{
			UE_LOG(LogTemp, Log, TEXT("[SWEEP_TRACE] New Hit: Actor='%s', Location='%s'"),
				*HitActor->GetName(),
				*Hit.ImpactPoint.ToString());
		}
	}
}

void UHarmoniaSweepTraceComponent::DrawDebugSweep(const FVector& From, const FVector& To, const FQuat& Rotation, bool bHadHits) const
{
	if (!GetWorld())
	{
		return;
	}

	const float Duration = DebugDrawDuration;
	const float Thickness = EditorPreviewThickness;
	FColor CurrentColor = bHadHits ? FColor::Red : FColor::Green;

	// Draw based on shape type
	switch (SweepShapeType)
	{
	case ESweepShapeType::Box:
		DrawDebugBox(GetWorld(), From, SweepBoxHalfExtent, Rotation, FColor::Yellow, false, Duration, 0, Thickness);
		DrawDebugBox(GetWorld(), To, SweepBoxHalfExtent, Rotation, CurrentColor, false, Duration, 0, Thickness);
		break;
	case ESweepShapeType::Sphere:
		DrawDebugSphere(GetWorld(), From, SweepRadius, EditorPreviewSegments, FColor::Yellow, false, Duration, 0, Thickness);
		DrawDebugSphere(GetWorld(), To, SweepRadius, EditorPreviewSegments, CurrentColor, false, Duration, 0, Thickness);
		break;
	case ESweepShapeType::Capsule:
		DrawDebugCapsule(GetWorld(), From, SweepCapsuleHalfHeight, SweepRadius, Rotation, FColor::Yellow, false, Duration, 0, Thickness);
		DrawDebugCapsule(GetWorld(), To, SweepCapsuleHalfHeight, SweepRadius, Rotation, CurrentColor, false, Duration, 0, Thickness);
		break;
	}

	// Sweep path (cyan line)
	DrawDebugLine(GetWorld(), From, To, FColor::Cyan, false, Duration, 0, Thickness);
}
