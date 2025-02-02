// Copyright 2025 RedFlowering.

#include "AbilitySystem/Abilities/RFAbility_GrapplingHook.h"
#include "Character/RFCharacter.h"
#include "Character/RFCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RFAbility_GrapplingHook)

URFAbilityTask_WaitTick* URFAbilityTask_WaitTick::WaitTick(UGameplayAbility* OwningAbility)
{
	URFAbilityTask_WaitTick* TickTask = NewAbilityTask<URFAbilityTask_WaitTick>(OwningAbility);
	return TickTask;
}

void URFAbilityTask_WaitTick::Activate()
{
	bTickingTask = true; // 매 프레임 Tick을 받도록 설정
}

void URFAbilityTask_WaitTick::TickTask(float DeltaTime)
{
	// Ability가 유효하면 OnTick 델리게이트를 호출합니다.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}

void URFAbilityTask_WaitTick::OnDestroy(bool AbilityEnded)
{
	// AbilityTask가 파괴될 때 Tick을 중지
	bTickingTask = false;
	Super::OnDestroy(AbilityEnded);
}

URFAbility_GrapplingHook::URFAbility_GrapplingHook(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool URFAbility_GrapplingHook::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ARFCharacter* AbilityCharacter = Cast<ARFCharacter>(ActorInfo->AvatarActor.Get());
		if (AbilityCharacter)
		{
			URFCharacterMovementComponent* AbilityMovementComponent =
				Cast<URFCharacterMovementComponent>(AbilityCharacter->GetMovementComponent());

			if (AbilityMovementComponent && (AbilityMovementComponent->IsMovingOnGround() || AbilityMovementComponent->IsFalling()))
			{
				return true;
			}
		}
	}

	return false;
}

void URFAbility_GrapplingHook::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OwnerCharacter = Cast<ARFCharacter>(ActorInfo->AvatarActor.Get());

	if (OwnerCharacter)
	{
		OwnerMovementComponent = Cast<URFCharacterMovementComponent>(OwnerCharacter->GetMovementComponent());
	}

	// 커스텀 AbilityTask 생성 및 Tick 델리게이트 바인딩
	URFAbilityTask_WaitTick* TickTask = URFAbilityTask_WaitTick::WaitTick(this);

	if (TickTask)
	{
		TickTask->OnTick.AddDynamic(this, &URFAbility_GrapplingHook::TickAbility);
		TickTask->ReadyForActivation();
	}
}

void URFAbility_GrapplingHook::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

}

void URFAbility_GrapplingHook::TickAbility(float DeltaTime)
{
	if (HookSetup.CachedHookActor && HookMove.Step >= EGrappleStep::MoveReady)
	{
		const float InitStartTime = -1.0f;

		if (FMath::IsNearlyEqual(HookMove.TeleportStartTime, InitStartTime))
		{
			if (UWorld* World = GetWorld())
			{
				HookMove.TeleportStartTime = World->GetTimeSeconds();
			}
		}

		switch (HookSetup.GrapplingMovementMode)
		{
		case EGrapplingHookMoveMode::Teleport:
		{
			TeleportMovement();
			break;
		}
		case EGrapplingHookMoveMode::Immersive:
		{
			ImmersiveMovement(DeltaTime);
			break;
		}
		case EGrapplingHookMoveMode::Blink:
		{
			BlinkMovement(DeltaTime);
			break;
		}
		default:
			break;
		}
	}
}

void URFAbility_GrapplingHook::ChangeGrapplingHookMode(EGrapplingHookMoveMode Mode)
{
	if (HookMove.Step == EGrappleStep::Idle)
	{
		HookSetup.GrapplingMovementMode = Mode;
	}
}

bool URFAbility_GrapplingHook::IsGrapplingIdle()
{
	return HookMove.Step == EGrappleStep::Idle;
}

void URFAbility_GrapplingHook::SetGrapplingReady()
{
	HookMove.Step = EGrappleStep::MoveReady;
}

bool URFAbility_GrapplingHook::GetGrapplingReady()
{
	return HookMove.Step == EGrappleStep::MoveReady;
}

FVector URFAbility_GrapplingHook::GetTargetLocation()
{
	if (Target)
	{
		return Target->GetActorLocation();
	}

	return FVector::ZeroVector;
}

void URFAbility_GrapplingHook::ShootGrapplingHook(AActor* TargetActor)
{
	if (HookMove.Step == EGrappleStep::Idle && !HookSetup.CachedHookActor && TargetActor)
	{
		HookMove.Step = EGrappleStep::Hooked;
		Target = TargetActor;

		const bool bIsSafeTeleport = PerformTrace();
		if (bIsSafeTeleport)
		{
			SpawnGrapplingHookActor();
		}
		else
		{
			HookMove.Step = EGrappleStep::Idle;
		}
	}
}

void URFAbility_GrapplingHook::CancelGrapplingHook()
{
	HookMove.Step = EGrappleStep::Idle;

	switch (HookSetup.GrapplingMovementMode)
	{
	case EGrapplingHookMoveMode::Teleport:
	{
		if (HookMove.Step >= EGrappleStep::MoveStart)
		{
			InitGrapplingHook();
		}
		else if (!GetGrapplingReady())
		{
			ServerReleaseGrapplingHook();
		}

		break;
	}
	case EGrapplingHookMoveMode::Immersive:
	{
		if (OwnerMovementComponent && OwnerCharacter)
		{
			const FVector Deceleration = OwnerMovementComponent->Velocity * -HookSetup.GrapplingHookDecelerationFactor;
			OwnerMovementComponent->AddImpulse(Deceleration, true);
			OwnerMovementComponent->SetMovementMode(static_cast<EMovementMode>(ERFMovementMode::MOVE_Falling));
		}
		break;
	}
	case EGrapplingHookMoveMode::Blink:
	{
		if (OwnerMovementComponent)
		{
			const FVector Deceleration = OwnerMovementComponent->Velocity * -HookSetup.GrapplingHookDecelerationFactor;
			OwnerMovementComponent->AddImpulse(Deceleration, true);
			OwnerMovementComponent->SetMovementMode(static_cast<EMovementMode>(ERFMovementMode::MOVE_Falling));

			InitGrapplingHook();
		}
		break;
	}
	default:
		break;
	}
}

const FGrapplingHookSetup URFAbility_GrapplingHook::GetHookSetup()
{
	return HookSetup;
}

bool URFAbility_GrapplingHook::PerformTrace()
{
	// Obstacle Simple Check
	const UWorld* World = GetWorld();

	bool IsSafeTeleport = false;

	if (World && OwnerCharacter && Target)
	{
		UCapsuleComponent* CapsuleComponent = OwnerCharacter->GetCapsuleComponent();

		if (CapsuleComponent)
		{
			// Trace Param
			TArray<FHitResult> HitResults;

			// Point Trace
			const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleComponent->GetUnscaledCapsuleRadius(), CapsuleComponent->GetUnscaledCapsuleHalfHeight());

			FVector HookStartLocation = OwnerCharacter->GetActorLocation();
			FVector HookPointLocation = Target->GetActorLocation();

			HookPointLocation.Z += Target->GetActorUpVector().GetSafeNormal().Z * CapsuleShape.GetCapsuleHalfHeight();

			FQuat CapsuleRotation = FQuat::Identity;
			FVector HookCenterLocation = HookStartLocation + ((HookPointLocation - HookStartLocation) / 2.0f);
			float BoxSize = (HookPointLocation - HookStartLocation).Size() / 2.0f;
			FVector BoxExtent(BoxSize, 5.0f, 5.0f);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(HookStartLocation, Target->GetActorLocation());
			const FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);

			float CapsuleDistance = 2 * CapsuleShape.GetCapsuleRadius();
			int32 NumCapsules = FMath::FloorToInt((HookPointLocation - HookStartLocation).Size() / CapsuleDistance);

			FVector Start = HookStartLocation;
			FVector End;
			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(OwnerCharacter);

			FCollisionQueryParams TraceParams(FName(TEXT("SweepMultiTrace")), true, OwnerCharacter);
			TraceParams.bTraceComplex = true;
			TraceParams.bFindInitialOverlaps = true;
			TraceParams.bReturnPhysicalMaterial = false;

			// Direction Calculation
			FVector PointDir = HookPointLocation - HookStartLocation;
			FVector PointDirNormal = PointDir.GetSafeNormal();

			switch (HookTrace.CollisionShape)
			{
			case EGrapplingTraceShape::None:
				IsSafeTeleport = !World->OverlapBlockingTestByChannel(HookPointLocation, CapsuleRotation, HookTrace.CollisionChannel, CapsuleShape, TraceParams);
#if WITH_EDITOR
				if (HookTrace.DebugMode)
				{
					DrawDebugCapsule(World, HookPointLocation, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), CapsuleRotation, IsSafeTeleport ? FColor::Green : FColor::Red, false, HookTrace.DebugTraceLifeTime);
				}
#endif
				break;
			case EGrapplingTraceShape::LineTrace:
				IsSafeTeleport = !World->OverlapBlockingTestByChannel(HookPointLocation, CapsuleRotation, HookTrace.CollisionChannel, CapsuleShape, TraceParams) &&
					!World->LineTraceMultiByChannel(HitResults, HookStartLocation, HookPointLocation, HookTrace.CollisionChannel, TraceParams);
#if WITH_EDITOR
				if (HookTrace.DebugMode)
				{
					DrawDebugCapsule(World, HookPointLocation, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), CapsuleRotation, IsSafeTeleport ? FColor::Green : FColor::Red, false, HookTrace.DebugTraceLifeTime);
					DrawDebugLine(World, HookStartLocation, HookPointLocation, IsSafeTeleport ? FColor::Green : FColor::Red, false, HookTrace.DebugTraceLifeTime);
				}
#endif
				break;
			case EGrapplingTraceShape::CapsuleTrace:
				for (int32 i = 0; i <= NumCapsules; ++i)
				{
					End = Start + (PointDirNormal * CapsuleDistance);
					World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, HookTrace.CollisionChannel, CapsuleShape, CollisionParams);
					Start = End;

#if WITH_EDITOR
					if (HookTrace.DebugMode)
					{
						DrawDebugCapsule(World, Start, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), CapsuleRotation, HitResult.bBlockingHit ? FColor::Red : FColor::Green, false, HookTrace.DebugTraceLifeTime);
					}
#endif

					if (HitResult.bBlockingHit)
						break;
				}
				IsSafeTeleport = !World->OverlapBlockingTestByChannel(HookPointLocation, CapsuleRotation, HookTrace.CollisionChannel, CapsuleShape, TraceParams) && !HitResult.bBlockingHit;

				break;
			case EGrapplingTraceShape::BoxTrace:
				IsSafeTeleport = !World->OverlapBlockingTestByChannel(HookPointLocation, CapsuleRotation, HookTrace.CollisionChannel, CapsuleShape, TraceParams) &&
					!World->OverlapBlockingTestByChannel(HookCenterLocation, FQuat(LookAtRotation), HookTrace.CollisionChannel, BoxShape, TraceParams);
#if WITH_EDITOR
				if (HookTrace.DebugMode)
				{
					DrawDebugCapsule(World, HookPointLocation, CapsuleShape.GetCapsuleHalfHeight(), CapsuleShape.GetCapsuleRadius(), CapsuleRotation, IsSafeTeleport ? FColor::Green : FColor::Red, false, HookTrace.DebugTraceLifeTime);
					DrawDebugBox(World, HookCenterLocation, BoxExtent, FQuat(LookAtRotation), IsSafeTeleport ? FColor::Green : FColor::Red, false, HookTrace.DebugTraceLifeTime);
				}
#endif
				break;
			}
		}
	}

	return IsSafeTeleport;
}

void URFAbility_GrapplingHook::SpawnGrapplingHookActor()
{
	ServerSpawnGrapplingHookActor(Target->GetActorLocation());
}

void URFAbility_GrapplingHook::ServerSpawnGrapplingHookActor_Implementation(FVector TargetLocation)
{
	MulticastSpawnGrapplingHookActor(TargetLocation);
}

void URFAbility_GrapplingHook::MulticastSpawnGrapplingHookActor_Implementation(FVector TargetLocation)
{
	UWorld* World = GetWorld();

	if (World && OwnerCharacter && HookSetup.RopeActorClass && HookSetup.HookActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter->GetInstigator();

		HookSetup.CachedRopeActor = World->SpawnActor<AActor>(HookSetup.RopeActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		FRotator TargetRot = FRotator::ZeroRotator;

		if (HookSetup.CachedRopeActor)
		{
			HookSetup.CachedRopeActor->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true), FName("hand_r"));

			SpawnParams.Owner = HookSetup.CachedRopeActor;
			SpawnParams.Instigator = HookSetup.CachedRopeActor->GetInstigator();
			TargetRot = (TargetLocation - HookSetup.CachedRopeActor->GetActorLocation()).Rotation();
		}

		HookSetup.CachedHookActor = World->SpawnActor<AActor>(HookSetup.HookActorClass, HookSetup.CachedRopeActor->GetActorLocation(), TargetRot, SpawnParams);

		if (HookSetup.CachedRopeActor && HookSetup.CachedHookActor)
		{
			OnSpawnedHook.Broadcast(TargetLocation);
		}
		else
		{
			CancelGrapplingHook();
		}
	}
}

void URFAbility_GrapplingHook::ReleaseGrapplingHook()
{
	UWorld* World = GetWorld();

	if (World)
	{
		if (HookSetup.CachedRopeActor)
		{
			World->DestroyActor(HookSetup.CachedRopeActor);
			HookSetup.CachedRopeActor = nullptr;
		}

		if (HookSetup.CachedRopeActor)
		{
			World->DestroyActor(HookSetup.CachedRopeActor);
			HookSetup.CachedRopeActor = nullptr;
		}
	}
}

void URFAbility_GrapplingHook::ServerReleaseGrapplingHook_Implementation()
{
	MulticastReleaseGrapplingHook();
}

void URFAbility_GrapplingHook::MulticastReleaseGrapplingHook_Implementation()
{
	ReleaseGrapplingHook();
}

void URFAbility_GrapplingHook::InitGrapplingHook()
{
	ServerReleaseGrapplingHook();

	if (EGrappleStep::MoveStart <= HookMove.Step && OwnerMovementComponent)
	{
		OwnerMovementComponent->StopMovementImmediately();
		OwnerMovementComponent->SetMovementMode(static_cast<EMovementMode>(ERFMovementMode::MOVE_Falling));
	}

	Target = nullptr;
	HookMove.Step = EGrappleStep::Idle;
	HookMove.LastDistance = FVector::ZeroVector;
	HookMove.TeleportStartTime = -1.0;
	OnGrappleInit.Broadcast();
}

void URFAbility_GrapplingHook::TeleportMovement()
{
	UWorld* World = GetWorld();
	const float InitStartTime = -1.0f;

	if (World && Target && !FMath::IsNearlyEqual(HookMove.TeleportStartTime, InitStartTime))
	{
		const float CurrentTime = World->GetTimeSeconds();
		const float ElapsedTime = static_cast<float>(CurrentTime - HookMove.TeleportStartTime);
		const float PreTeleportTime = HookSetup.BlinkPreTeleportTime;
		const float PostTeleportTime = HookSetup.BlinkPreTeleportTime + HookSetup.BlinkTeleportHoldTime;

		switch (HookMove.Step)
		{
		case EGrappleStep::MoveReady:
		{
			HookMove.Step = EGrappleStep::MoveStart;
			OnMoveReady.Broadcast();
		}
		break;
		case EGrappleStep::MoveStart:
		{
			if (ElapsedTime >= PreTeleportTime)
			{
				HookMove.Step = EGrappleStep::Moving;
				OnMoveStart.Broadcast();
			}
		}
		break;
		case EGrappleStep::Moving:
		{
			HookMove.Step = EGrappleStep::Moved;
			TeleportToTarget();
			OnMoving.Broadcast();
		}
		break;
		case EGrappleStep::Moved:
		{
			if (ElapsedTime >= PostTeleportTime)
			{
				HookMove.Step = EGrappleStep::MoveEnd;
				OnMoved.Broadcast();
			}
		}
		break;
		case EGrappleStep::MoveEnd:
		{
			HookMove.Step = EGrappleStep::GrappleEnd;
			OnMoveEnd.Broadcast();
		}
		break;
		case EGrappleStep::GrappleEnd:
		{
			InitGrapplingHook();
		}
		break;
		default:
			break;
		}
	}
	else
	{
		InitGrapplingHook();
		OnMoveEnd.Broadcast();
	}
}

void URFAbility_GrapplingHook::TeleportToTarget()
{
	if (OwnerMovementComponent && Target)
	{
		OwnerMovementComponent->SetMovementMode(static_cast<EMovementMode>(ERFMovementMode::MOVE_None));

		ServerTeleportToTarget(Target->GetActorLocation());

		HookMove.Step = EGrappleStep::Moved;
	}
}

void URFAbility_GrapplingHook::ServerTeleportToTarget_Implementation(FVector TargetLocation)
{
	float LandingRange = 100.0f;

	if (OwnerCharacter && !TargetLocation.Equals(OwnerCharacter->GetActorLocation(), LandingRange))
	{
		OwnerCharacter->TeleportTo(TargetLocation, FRotator::ZeroRotator);
	}
}

void URFAbility_GrapplingHook::ImmersiveMovement(float DeltaTime)
{
	if (Target && OwnerMovementComponent && OwnerCharacter)
	{
		switch (HookMove.Step)
		{
		case EGrappleStep::MoveReady:
		{
			// Init
			HookMove.Step = EGrappleStep::MoveStart;
			OwnerMovementComponent->SetMovementMode(static_cast<EMovementMode>(ERFMovementMode::MOVE_Custom), static_cast<uint8>(ERFCustomMovementMode::MOVE_GrapplingHook));
			OnMoveReady.Broadcast();
		}
		break;
		case EGrappleStep::MoveStart:
		{
			HookMove.Step = EGrappleStep::Moving;
			OnMoveStart.Broadcast();
		}
		case EGrappleStep::Moving:
		{
			ImmersiveMoveToTarget(DeltaTime);
			OnMoving.Broadcast();
		}
		break;
		case EGrappleStep::Moved:
		{
			HookMove.Step = EGrappleStep::MoveEnd;
			OnMoved.Broadcast();
		}
		break;
		case EGrappleStep::MoveEnd:
		{
			HookMove.Step = EGrappleStep::GrappleEnd;
			OnMoveEnd.Broadcast();
		}
		break;
		case EGrappleStep::GrappleEnd:
		{
			InitGrapplingHook();
		}
		break;
		default:
			break;
		}
	}
}

void URFAbility_GrapplingHook::ImmersiveMoveToTarget(float DeltaTime)
{
	FVector StartLocation = OwnerCharacter->GetActorLocation();
	FVector EndLocation = Target->GetActorLocation();

	FVector TargetDistance = EndLocation - StartLocation;
	FVector TargetDirection = TargetDistance.GetSafeNormal();

	if (HookMove.LastDistance.IsNearlyZero())
	{
		HookMove.LastDistance = TargetDistance;
	}

	float PassedByPoint = FVector::DotProduct(TargetDirection, HookMove.LastDistance.GetSafeNormal());
	HookMove.LastDistance = TargetDistance;

	const float LandingExtent = 30.0f;

	if (PassedByPoint < 0.0f || TargetDistance.IsNearlyZero(LandingExtent))
	{
		HookMove.Step = EGrappleStep::Moved;
		return;
	}

	OwnerMovementComponent->SetGrapplingHookMovementVector(TargetDirection * HookSetup.GrapplingHookSpeed * DeltaTime);
}

void URFAbility_GrapplingHook::BlinkMovement(float DeltaTime)
{
	FVector StartLocation = OwnerCharacter->GetActorLocation();
	FVector EndLocation = Target->GetActorLocation();

	FVector TargetDistance = EndLocation - StartLocation;
	FVector TargetDirection = TargetDistance.GetSafeNormal();

	if (HookMove.LastDistance.IsNearlyZero())
	{
		HookMove.LastDistance = TargetDistance;
	}

	float PassedByPoint = FVector::DotProduct(TargetDirection, HookMove.LastDistance.GetSafeNormal());
	HookMove.LastDistance = TargetDistance;

	const float LandingExtent = 30.0f;

	if (PassedByPoint < 0.0f || TargetDistance.IsNearlyZero(LandingExtent))
	{
		HookMove.Step = EGrappleStep::Moved;
		return;
	}

	OwnerMovementComponent->SetGrapplingHookMovementVector(TargetDirection * HookSetup.GrapplingHookSpeed * DeltaTime);
}

void URFAbility_GrapplingHook::PreMovement(float DeltaTime)
{
	if (OwnerCharacter && OwnerMovementComponent && Target)
	{
		const FVector StartLocation = OwnerCharacter->GetActorLocation();
		const FVector EndLocation = GetTargetLocation();

		const FVector TargetDistance = EndLocation - StartLocation;
		const FVector TargetDirection = TargetDistance.GetSafeNormal();

		OwnerMovementComponent->SetGrapplingHookMovementVector(TargetDirection * HookSetup.GrapplingHookSpeed * DeltaTime);
	}
}

void URFAbility_GrapplingHook::BlinkTeleportMovement()
{
	if (OwnerCharacter && Target)
	{
		if (FMath::IsNearlyZero(HookSetup.BlinkPostTeleportTime))
		{
			const FVector LandingPoint = GetTargetLocation();
			ServerTeleportToTarget(LandingPoint);

			OnMoved.Broadcast();

			HookMove.Step = EGrappleStep::MoveEnd;
			OnMoveEnd.Broadcast();
		}
		else
		{
			const FVector StartLocation = OwnerCharacter->GetActorLocation();
			const FVector EndLocation = GetTargetLocation();

			const FVector TargetDistance = EndLocation - StartLocation;
			const FVector TargetDirection = TargetDistance.GetSafeNormal();

			// Teleport to BlinkPostTeleportTime away from end location, but never further away than the current position
			const float CurrentDistance = TargetDistance.Size();
			float DesiredTeleportDistance = HookSetup.GrapplingHookSpeed * HookSetup.BlinkPostTeleportTime;
			DesiredTeleportDistance = FMath::Min(DesiredTeleportDistance, CurrentDistance);

			FVector TeleportLocation = EndLocation - TargetDirection * DesiredTeleportDistance;

			ServerTeleportToTarget(TeleportLocation);

			HookMove.Step = EGrappleStep::Moved;
			OnMoved.Broadcast();
		}
	}
}

void URFAbility_GrapplingHook::PostMovement(float DeltaTime)
{
	if (OwnerCharacter && OwnerMovementComponent && Target)
	{
		const FVector StartLocation = OwnerCharacter->GetActorLocation();
		const FVector EndLocation = GetTargetLocation();

		const FVector TargetDistance = EndLocation - StartLocation;
		const FVector TargetDirection = TargetDistance.GetSafeNormal();

		if (HookMove.LastDistance.IsNearlyZero())
		{
			HookMove.LastDistance = TargetDistance;
		}

		float PassedByPoint = FVector::DotProduct(TargetDirection, HookMove.LastDistance.GetSafeNormal());
		HookMove.LastDistance = TargetDistance;

		const float LandingExtent = 30.0f;

		if (PassedByPoint < 0.0f || TargetDistance.IsNearlyZero(LandingExtent))
		{
			HookMove.Step = EGrappleStep::MoveEnd;
			return;
		}

		OwnerMovementComponent->SetGrapplingHookMovementVector(TargetDirection * HookSetup.GrapplingHookSpeed * DeltaTime);
	}
}