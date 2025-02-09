// Copyright 2025 RedFlowering.

#include "AbilitySystem/Abilities/RFAbility_GrapplingHook.h"
#include "Character/RFCharacter.h"
#include "Character/RFCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Actors/HookActor.h"
#include "Actors/RopeActor.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RFAbility_GrapplingHook)

URFAbilityTask_WaitTick* URFAbilityTask_WaitTick::WaitTick(UGameplayAbility* OwningAbility)
{
	URFAbilityTask_WaitTick* TickTask = NewAbilityTask<URFAbilityTask_WaitTick>(OwningAbility);
	return TickTask;
}

void URFAbilityTask_WaitTick::Activate()
{
	// Set to receive a tick every frame
	bTickingTask = true; 
}

void URFAbilityTask_WaitTick::TickTask(float DeltaTime)
{
	// If the Ability is valid, it calls the OnTick delegate.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}

void URFAbilityTask_WaitTick::OnDestroy(bool AbilityEnded)
{
	// Stop Tick when AbilityTask is destroyed
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
				Cast<URFCharacterMovementComponent>(AbilityCharacter->GetCharacterMovement());

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
		OwnerMovementComponent = Cast<URFCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());
	}

	ALyraPlayerController* OwnerPlayerController = Cast<ALyraPlayerController>(ActorInfo->PlayerController);

	if (OwnerPlayerController)
	{
		FHitResult HitResult;

		if (OwnerPlayerController->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
		{
			AActor* TargetActor = HitResult.GetActor();

			ShootGrapplingHook(TargetActor);
		}
	}

	// Creating a custom AbilityTask and binding a Tick delegate
	URFAbilityTask_WaitTick* TickTask = URFAbilityTask_WaitTick::WaitTick(this);

	if (TickTask)
	{
		TickTask->OnTick.AddDynamic(this, &URFAbility_GrapplingHook::TickAbility);
		TickTask->ReadyForActivation();
	}
}

void URFAbility_GrapplingHook::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
	if (HookMove.Step == EGrappleStep::Idle && !HookSetup.CachedHookActor && TargetActor && OwnerCharacter)
	{
		HookMove.Step = EGrappleStep::Hooked;
		Target = TargetActor;
		
		OwnerCharacter->SetRightHandIK(Target->GetActorTransform());
		OwnerCharacter->SetUseRightHandIK(true);

		const bool bIsSafeTeleport = PerformTrace();
		if (bIsSafeTeleport)
		{
			SpawnGrapplingHookActor();
			// SetGrapplingReady();
		}
		else
		{
			HookMove.Step = EGrappleStep::Idle;
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/ true, /*bWasCancelled=*/ false);
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
			ReleaseGrapplingHook();
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
	if (Target)
	{
		ServerSpawnGrapplingHookActor(Target->GetActorLocation());
	}
}

void URFAbility_GrapplingHook::ServerSpawnGrapplingHookActor_Implementation(FVector TargetLocation)
{
	UWorld* World = GetWorld();

	if (World && OwnerCharacter && HookSetup.RopeActorClass && HookSetup.HookActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter->GetInstigator();
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Using the player's right hand socket position as the Hook spawn location
		FVector SpawnLocation = OwnerCharacter->GetMesh()->GetSocketLocation(FName("hand_r"));

		// Compute rotation from SpawnLocation towards TargetLocation
		FRotator HookRotation = (TargetLocation - SpawnLocation).Rotation();

		// Hook Actor
		HookSetup.CachedHookActor = World->SpawnActor<AHookActor>(HookSetup.HookActorClass, SpawnLocation, HookRotation, SpawnParams);

		if (HookSetup.CachedHookActor)
		{
			OnSpawnedHook.Broadcast(TargetLocation);
			HookSetup.CachedHookActor->SetReplicates(true);

			HookSetup.CachedHookActor->MoveToTarget(TargetLocation);
			HookSetup.CachedHookActor->OnHookArrived.AddDynamic(this, &URFAbility_GrapplingHook::OnHookArrivedHandler);
		}
		else
		{
			CancelGrapplingHook();
			return;
		}

		// Rope Actor
		HookSetup.CachedRopeActor = World->SpawnActor<ARopeActor>(HookSetup.RopeActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (HookSetup.CachedRopeActor && HookSetup.CachedHookActor)
		{
			HookSetup.CachedRopeActor->SetReplicates(true);
			HookSetup.CachedRopeActor->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("hand_r"));
			HookSetup.CachedRopeActor->UpdateCableEndpoint(HookSetup.CachedHookActor);
		}
		else
		{
			CancelGrapplingHook();
			return;
		}
	}
}

void URFAbility_GrapplingHook::OnHookArrivedHandler()
{
	SetGrapplingReady();

	if (HookSetup.CachedHookActor)
	{
		HookSetup.CachedHookActor->OnHookArrived.RemoveDynamic(this, &URFAbility_GrapplingHook::OnHookArrivedHandler);
	}
}

void URFAbility_GrapplingHook::ReleaseGrapplingHook()
{
	ServerReleaseGrapplingHook();
}

void URFAbility_GrapplingHook::ServerReleaseGrapplingHook_Implementation()
{
	UWorld* World = GetWorld();

	if (World)
	{
		if (HookSetup.CachedHookActor)
		{
			World->DestroyActor(HookSetup.CachedHookActor);
			HookSetup.CachedHookActor = nullptr;
		}

		if (HookSetup.CachedRopeActor)
		{
			World->DestroyActor(HookSetup.CachedRopeActor);
			HookSetup.CachedRopeActor = nullptr;
		}
	}
}

void URFAbility_GrapplingHook::InitGrapplingHook()
{
	ReleaseGrapplingHook();

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

	OwnerCharacter->SetRightHandIK(FTransform::Identity);
	OwnerCharacter->SetUseRightHandIK(false);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/ true, /*bWasCancelled=*/ false);
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
	float LandingExtent = 100.0f;

	if (OwnerCharacter && !TargetLocation.Equals(OwnerCharacter->GetActorLocation(), LandingExtent))
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
			OwnerMovementComponent->SetMovementMode(MOVE_Custom, 1);
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

	const float LandingExtent = 100.0f;

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

	const float LandingExtent = 100.0f;

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

		const float LandingExtent = 100.0f;

		if (PassedByPoint < 0.0f || TargetDistance.IsNearlyZero(LandingExtent))
		{
			HookMove.Step = EGrappleStep::MoveEnd;
			return;
		}

		OwnerMovementComponent->SetGrapplingHookMovementVector(TargetDirection * HookSetup.GrapplingHookSpeed * DeltaTime);
	}
}