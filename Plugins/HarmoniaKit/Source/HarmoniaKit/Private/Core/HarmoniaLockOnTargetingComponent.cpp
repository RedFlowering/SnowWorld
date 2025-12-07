// Copyright 2025 Snow Game Studio.

#include "Core/HarmoniaLockOnTargetingComponent.h"
#include "Core/HarmoniaCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UHarmoniaLockOnTargetingComponent::UHarmoniaLockOnTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHarmoniaLockOnTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AHarmoniaCharacter>(GetOwner());
}

void UHarmoniaLockOnTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLockedOn)
	{
		ValidateCurrentTarget();

		if (bIsLockedOn && CurrentTarget.IsValid())
		{
			UpdateCameraToTarget(DeltaTime);
		}
	}
}

void UHarmoniaLockOnTargetingComponent::ToggleLockOn()
{
	if (bIsLockedOn)
	{
		DisableLockOn();
	}
	else
	{
		EnableLockOn();
	}
}

void UHarmoniaLockOnTargetingComponent::EnableLockOn()
{
	if (bIsLockedOn)
	{
		return;
	}

	AActor* NearestTarget = FindNearestTarget();
	if (NearestTarget)
	{
		CurrentTarget = NearestTarget;
		bIsLockedOn = true;
		SetComponentTickEnabled(true);

		// Notify owner character
		if (OwnerCharacter.IsValid())
		{
			// Could broadcast a delegate or set a gameplay tag here
		}
	}
}

void UHarmoniaLockOnTargetingComponent::DisableLockOn()
{
	if (!bIsLockedOn)
	{
		return;
	}

	CurrentTarget.Reset();
	bIsLockedOn = false;
	SetComponentTickEnabled(false);

	// Notify owner character
	if (OwnerCharacter.IsValid())
	{
		// Could broadcast a delegate or remove a gameplay tag here
	}
}

void UHarmoniaLockOnTargetingComponent::SwitchTargetRight()
{
	if (!bIsLockedOn)
	{
		return;
	}

	SwitchTarget(true);
}

void UHarmoniaLockOnTargetingComponent::SwitchTargetLeft()
{
	if (!bIsLockedOn)
	{
		return;
	}

	SwitchTarget(false);
}

AActor* UHarmoniaLockOnTargetingComponent::FindNearestTarget() const
{
	if (!OwnerCharacter.IsValid())
	{
		return nullptr;
	}

	TArray<AActor*> ValidTargets = FindValidTargets();
	if (ValidTargets.Num() == 0)
	{
		return nullptr;
	}

	// Find the target closest to the camera center
	AActor* BestTarget = nullptr;
	float BestScore = FLT_MAX;

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller)
	{
		return nullptr;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector CameraForward = CameraRotation.Vector();

	for (AActor* Target : ValidTargets)
	{
		FVector ToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();
		float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CameraForward, ToTarget)));

		if (Angle <= MaxLockOnAngle)
		{
			float Distance = FVector::Distance(OwnerCharacter->GetActorLocation(), Target->GetActorLocation());
			// Score based on both angle and distance, favoring targets closer to center
			float Score = Angle * 10.0f + Distance * 0.01f;

			if (Score < BestScore)
			{
				BestScore = Score;
				BestTarget = Target;
			}
		}
	}

	return BestTarget;
}

TArray<AActor*> UHarmoniaLockOnTargetingComponent::FindValidTargets() const
{
	TArray<AActor*> ValidTargets;

	if (!OwnerCharacter.IsValid())
	{
		return ValidTargets;
	}

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetableTag, AllActors);

	for (AActor* Actor : AllActors)
	{
		if (IsValidTarget(Actor))
		{
			ValidTargets.Add(Actor);
		}
	}

	return ValidTargets;
}

bool UHarmoniaLockOnTargetingComponent::IsValidTarget(AActor* Target) const
{
	if (!Target || !OwnerCharacter.IsValid())
	{
		return false;
	}

	// Don't target self
	if (Target == OwnerCharacter.Get())
	{
		return false;
	}

	// Check if target is a pawn
	APawn* TargetPawn = Cast<APawn>(Target);
	if (!TargetPawn)
	{
		return false;
	}

	// Check if target is alive (you might want to add a health check here)
	// For now, just check if it's not being destroyed
	// IsPendingKillPending() is deprecated in newer engine versions, but assuming 5.x it might still be there or use IsValid()
	if (!IsValid(Target))
	{
		return false;
	}

	// Check distance
	float Distance = FVector::Distance(OwnerCharacter->GetActorLocation(), Target->GetActorLocation());
	if (Distance > MaxLockOnDistance)
	{
		return false;
	}

	// Check line of sight
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter.Get());
	QueryParams.AddIgnoredActor(Target);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		OwnerCharacter->GetActorLocation(),
		Target->GetActorLocation(),
		ECC_Visibility,
		QueryParams
	);

	// If we hit something, target is not visible
	return !bHit;
}

void UHarmoniaLockOnTargetingComponent::UpdateCameraToTarget(float DeltaTime)
{
	if (!CurrentTarget.IsValid() || !OwnerCharacter.IsValid())
	{
		return;
	}

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller)
	{
		return;
	}

	// Calculate target location with height offset
	FVector TargetLocation = CurrentTarget->GetActorLocation();
	TargetLocation.Z += TargetHeightOffset;

	// Get camera location
	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// Calculate desired rotation
	FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation);

	// Smoothly interpolate to desired rotation
	FRotator NewRotation = FMath::RInterpTo(CameraRotation, DesiredRotation, DeltaTime, CameraRotationSpeed);

	// Apply rotation to controller
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		PC->SetControlRotation(NewRotation);
	}
}

void UHarmoniaLockOnTargetingComponent::ValidateCurrentTarget()
{
	if (!CurrentTarget.IsValid() || !IsValidTarget(CurrentTarget.Get()))
	{
		// Try to find a new target nearby
		AActor* NewTarget = FindNearestTarget();
		if (NewTarget)
		{
			CurrentTarget = NewTarget;
		}
		else
		{
			DisableLockOn();
		}
		return;
	}

	// Check if target is too far
	float Distance = FVector::Distance(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Distance > LockOnBreakDistance)
	{
		DisableLockOn();
	}
}

void UHarmoniaLockOnTargetingComponent::SwitchTarget(bool bSwitchRight)
{
	if (!CurrentTarget.IsValid() || !OwnerCharacter.IsValid())
	{
		return;
	}

	TArray<AActor*> ValidTargets = FindValidTargets();
	if (ValidTargets.Num() <= 1)
	{
		return; // No other targets to switch to
	}

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller)
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector CameraRight = CameraRotation.Quaternion().GetRightVector();

	AActor* BestTarget = nullptr;
	float BestAngle = bSwitchRight ? -180.0f : 180.0f;

	FVector CurrentTargetDir = (CurrentTarget->GetActorLocation() - CameraLocation).GetSafeNormal();

	for (AActor* Target : ValidTargets)
	{
		if (Target == CurrentTarget.Get())
		{
			continue; // Skip current target
		}

		FVector TargetDir = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();

		// Calculate angle from current target
		float DotRight = FVector::DotProduct(TargetDir, CameraRight);
		float DotCurrent = FVector::DotProduct(TargetDir, CurrentTargetDir);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(DotCurrent));

		// Determine if target is on the correct side
		bool bIsOnRightSide = DotRight > 0.0f;

		if (bSwitchRight && bIsOnRightSide)
		{
			if (Angle > BestAngle && Angle < 120.0f)
			{
				BestAngle = Angle;
				BestTarget = Target;
			}
		}
		else if (!bSwitchRight && !bIsOnRightSide)
		{
			if (Angle < BestAngle && Angle < 120.0f)
			{
				BestAngle = Angle;
				BestTarget = Target;
			}
		}
	}

	if (BestTarget)
	{
		CurrentTarget = BestTarget;
	}
}

float UHarmoniaLockOnTargetingComponent::GetAngleToTarget(AActor* Target) const
{
	if (!Target || !OwnerCharacter.IsValid())
	{
		return 0.0f;
	}

	AController* Controller = OwnerCharacter->GetController();
	if (!Controller)
	{
		return 0.0f;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector CameraForward = CameraRotation.Vector();
	FVector ToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();

	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CameraForward, ToTarget)));
}
