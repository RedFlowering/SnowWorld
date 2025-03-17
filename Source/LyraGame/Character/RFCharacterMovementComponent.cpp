// Copyright 2024 RedFlowering.

#include "Character/RFCharacterMovementComponent.h"
#include "RFCharacter.h"
#include "GameFramework/PhysicsVolume.h"
#include "Net/UnrealNetwork.h"

URFCharacterMovementComponent::URFCharacterMovementComponent()
	: Super()
{

}

void URFCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ARFCharacter>(CharacterOwner);

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Found Owner RF Character!"));
	}
}

void URFCharacterMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void URFCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void URFCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}
}

void URFCharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode /*= 0*/)
{
	Super::SetMovementMode(NewMovementMode, NewCustomMode);

	RFMovementMode = static_cast<ERFMovementMode>(NewMovementMode);
	RFCustomMovementMode = static_cast<ERFCustomMovementMode>(NewCustomMode);
}

ERFMovementMode URFCharacterMovementComponent::GetMovementMode()
{
	return RFMovementMode;
}

ERFCustomMovementMode URFCharacterMovementComponent::GetCustomMovementMode()
{
	return RFCustomMovementMode;
}

void URFCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	UE_LOG(LogTemp, Log, TEXT("Movement Mode Changed: NewMode = %d, CustomMode = %d"), MovementMode, CustomMovementMode);
}

void URFCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult /*= nullptr*/)
{
	Super::MoveSmooth(InVelocity, DeltaTime, StepDownResult);
}

void URFCharacterMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	// Ignore the parent implementation of this function and provide our own, because the parent
	// implementation has no effect when we ignore rotation changes in AAlsCharacter::FaceRotation().

	const auto& BasedMovement{ CharacterOwner->GetBasedMovement() };

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
		MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const auto DeltaRotation{ (MovementBaseRotation * OldBaseQuat.Inverse()).Rotator() };
		auto NewControlRotation{ CharacterOwner->Controller->GetControlRotation() };

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->Controller->SetControlRotation(NewControlRotation);
	}
}

void URFCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);
}

void URFCharacterMovementComponent::PhysWalking(const float DeltaTime, int32 IterationsCount)
{
	RefreshGroundedMovementSettings();

	auto Iterations{ IterationsCount };

	// TODO Copied with modifications from UCharacterMovementComponent::PhysWalking(). After the
	// TODO release of a new engine version, this code should be updated to match the source code.

	// ReSharper disable All

	// SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);

	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration = FVector::VectorPlaneProject(Acceleration, -GetGravityDirection());

		static const auto* EnsureAlwaysEnabledConsoleVariable{
			IConsoleManager::Get().FindConsoleVariable(TEXT("p.LedgeMovement.ApplyDirectMove"))
		};
		check(EnsureAlwaysEnabledConsoleVariable != nullptr)

			// Apply acceleration
			const bool bSkipForLedgeMove = bTriedLedgeMove && EnsureAlwaysEnabledConsoleVariable->GetBool();
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bSkipForLedgeMove)
		{
			CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
			// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}

		ApplyRootMotionToVelocity(timeTick);
		// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
		{
			// Root motion could have taken us out of our current mode
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
			else if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
			{
				// pawn ended up in a different mode, probably due to the step-up-and-over flow
				// let's refund the estimated unused time (if any) and keep moving in the new mode
				const float DesiredDist = UE_REAL_TO_FLOAT(Delta.Size());
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = UE_REAL_TO_FLOAT(ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - OldLocation).Size());
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if (!NewDelta.IsZero())
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				Iterations--;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// TODO Start of custom ALS code block.

						ApplyPendingPenetrationAdjustment();

						// TODO End of custom ALS code block.

						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				// TODO Start of custom ALS code block.

				ApplyPendingPenetrationAdjustment();

				// TODO End of custom ALS code block.

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + MAX_FLOOR_DIST * -GetGravityDirection();
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO Start of custom ALS code block.

				PrePenetrationAdjustmentVelocity = MoveVelocity;
				bPrePenetrationAdjustmentVelocityValid = true;

				// TODO End of custom ALS code block.

				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}

	// ReSharper restore All
}

void URFCharacterMovementComponent::PhysNavWalking(const float DeltaTime, int32 IterationsCount)
{
	Super::PhysNavWalking(DeltaTime, IterationsCount);
}

void URFCharacterMovementComponent::PhysCustom(const float DeltaTime, int32 IterationsCount)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		Super::PhysCustom(DeltaTime, IterationsCount);
		return;
	}

	switch (static_cast<ERFCustomMovementMode>(CustomMovementMode))
	{
	case ERFCustomMovementMode::MOVE_None:
		Super::PhysCustom(DeltaTime, IterationsCount);
		break;
	case ERFCustomMovementMode::MOVE_GrapplingHook:
		PhysGrpplingHook(DeltaTime, IterationsCount);
		break;
	case ERFCustomMovementMode::MOVE_Climbing:
		PhysClimbing(DeltaTime, IterationsCount);
		break;
	default:
		Super::PhysCustom(DeltaTime, IterationsCount);
		break;
	}
}

void URFCharacterMovementComponent::PhysGrpplingHook(float DeltaTime, int32 IterationCount)
{
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	FVector Adjusted = GrapplingHookVector;

	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.0f)
	{
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = Velocity + Adjusted;
	}

	Super::PhysFalling(DeltaTime, IterationCount);
}

void URFCharacterMovementComponent::PhysClimbing(float DeltaTime, int32 IterationCount)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		if (bCheatFlying && Acceleration.IsZero())
		{
			Velocity = FVector::ZeroVector;
		}
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
		CalcVelocity(DeltaTime, Friction, true, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(DeltaTime);

	IterationCount++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = ClimbingVector; // Velocity * DeltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = VelDir | GetGravityDirection();

		bool bSteppedUp = false;
		if ((FMath::Abs(GetGravitySpaceZ(Hit.ImpactNormal)) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			const FVector::FReal StepZ = GetGravitySpaceZ(UpdatedComponent->GetComponentLocation());
			bSteppedUp = StepUp(GetGravityDirection(), Adjusted * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{
				const FVector::FReal LocationZ = GetGravitySpaceZ(UpdatedComponent->GetComponentLocation()) + (GetGravitySpaceZ(OldLocation) - StepZ);
				SetGravitySpaceZ(OldLocation, LocationZ);
			}
		}

		if (!bSteppedUp)
		{
			//adjust and try again
			HandleImpact(Hit, DeltaTime, Adjusted);
			SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	}

	if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation - Adjusted) / DeltaTime;
	}
}

FVector URFCharacterMovementComponent::GetGrapplingHookMovmentVector()
{
	return GrapplingHookVector;
}

void URFCharacterMovementComponent::SetGrapplingHookMovementVector(FVector GrapplingVector)
{
	GrapplingHookVector = GrapplingVector;
}

FVector URFCharacterMovementComponent::GetClimbingMovementVector()
{
	return ClimbingVector;
}

void URFCharacterMovementComponent::SetClimbingMovementVector(FVector GrapplingVector)
{
	ClimbingVector = GrapplingVector;
}
