// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaLockOnComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"

UHarmoniaLockOnComponent::UHarmoniaLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Default targetable tag
	TargetableTag = FGameplayTag::RequestGameplayTag(FName("Character.Type.Enemy"));
}

void UHarmoniaLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UHarmoniaLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

// ============================================================================
// Lock-On Control
// ============================================================================

void UHarmoniaLockOnComponent::ToggleLockOn()
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

void UHarmoniaLockOnComponent::EnableLockOn()
{
	if (bIsLockedOn)
	{
		return;
	}

	AActor* NearestTarget = FindNearestTarget();
	if (NearestTarget)
	{
		TWeakObjectPtr<AActor> OldTarget = CurrentTarget;
		CurrentTarget = NearestTarget;
		bIsLockedOn = true;
		SetComponentTickEnabled(true);

		// Broadcast delegates
		OnLockOnTargetChanged.Broadcast(OldTarget.Get(), CurrentTarget.Get());
		OnLockOnStateChanged.Broadcast(true);
	}
}

void UHarmoniaLockOnComponent::DisableLockOn()
{
	if (!bIsLockedOn)
	{
		return;
	}

	TWeakObjectPtr<AActor> OldTarget = CurrentTarget;
	CurrentTarget.Reset();
	bIsLockedOn = false;
	SetComponentTickEnabled(false);

	// Broadcast delegates
	OnLockOnTargetChanged.Broadcast(OldTarget.Get(), nullptr);
	OnLockOnStateChanged.Broadcast(false);
}

void UHarmoniaLockOnComponent::SwitchTargetRight()
{
	if (!bIsLockedOn)
	{
		return;
	}

	SwitchTarget(true);
}

void UHarmoniaLockOnComponent::SwitchTargetLeft()
{
	if (!bIsLockedOn)
	{
		return;
	}

	SwitchTarget(false);
}

void UHarmoniaLockOnComponent::SetLockOnTarget(AActor* NewTarget)
{
	if (!NewTarget || !IsValidTarget(NewTarget))
	{
		DisableLockOn();
		return;
	}

	TWeakObjectPtr<AActor> OldTarget = CurrentTarget;
	CurrentTarget = NewTarget;

	if (!bIsLockedOn)
	{
		bIsLockedOn = true;
		SetComponentTickEnabled(true);
		OnLockOnStateChanged.Broadcast(true);
	}

	if (OldTarget != CurrentTarget)
	{
		OnLockOnTargetChanged.Broadcast(OldTarget.Get(), CurrentTarget.Get());
	}
}

// ============================================================================
// State Queries
// ============================================================================

float UHarmoniaLockOnComponent::GetDistanceToTarget() const
{
	if (!CurrentTarget.IsValid() || !OwnerCharacter.IsValid())
	{
		return 0.0f;
	}

	return FVector::Distance(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
}

// ============================================================================
// Target Selection
// ============================================================================

AActor* UHarmoniaLockOnComponent::FindNearestTarget() const
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
		FVector ToTarget = (GetTargetLockOnPoint(Target) - CameraLocation).GetSafeNormal();
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

TArray<AActor*> UHarmoniaLockOnComponent::FindValidTargets() const
{
	TArray<AActor*> ValidTargets;

	if (!OwnerCharacter.IsValid())
	{
		return ValidTargets;
	}

	// Get all pawns in range
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (IsValidTarget(Actor))
		{
			ValidTargets.Add(Actor);
		}
	}

	return ValidTargets;
}

bool UHarmoniaLockOnComponent::IsValidTarget(AActor* Target) const
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

	// Check if target is alive
	if (Target->IsPendingKillPending())
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
		GetTargetLockOnPoint(Target),
		ECC_Visibility,
		QueryParams
	);

	// If we hit something, target is not visible
	if (bHit)
	{
		return false;
	}

	return true;
}

FVector UHarmoniaLockOnComponent::GetTargetLockOnPoint(AActor* Target) const
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	FVector LockOnPoint = Target->GetActorLocation();

	// Use socket if specified
	if (TargetSocketName != NAME_None)
	{
		if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
		{
			if (USkeletalMeshComponent* Mesh = TargetCharacter->GetMesh())
			{
				if (Mesh->DoesSocketExist(TargetSocketName))
				{
					return Mesh->GetSocketLocation(TargetSocketName);
				}
			}
		}
	}

	// Apply height offset
	LockOnPoint.Z += TargetHeightOffset;
	return LockOnPoint;
}

// ============================================================================
// Camera Control
// ============================================================================

void UHarmoniaLockOnComponent::UpdateCameraToTarget(float DeltaTime)
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

	// Get target lock-on point
	FVector TargetLocation = GetTargetLockOnPoint(CurrentTarget.Get());

	// Get camera location
	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// Calculate desired rotation
	FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation);

	// Apply rotation to controller
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		FRotator NewRotation;
		if (bSmoothCameraRotation)
		{
			// Smoothly interpolate to desired rotation
			NewRotation = FMath::RInterpTo(CameraRotation, DesiredRotation, DeltaTime, CameraRotationSpeed);
		}
		else
		{
			// Snap to desired rotation
			NewRotation = DesiredRotation;
		}

		PC->SetControlRotation(NewRotation);
	}
}

// ============================================================================
// Validation
// ============================================================================

void UHarmoniaLockOnComponent::ValidateCurrentTarget()
{
	if (!CurrentTarget.IsValid() || !IsValidTarget(CurrentTarget.Get()))
	{
		// Try to find a new target nearby
		AActor* NewTarget = FindNearestTarget();
		if (NewTarget)
		{
			TWeakObjectPtr<AActor> OldTarget = CurrentTarget;
			CurrentTarget = NewTarget;
			OnLockOnTargetChanged.Broadcast(OldTarget.Get(), CurrentTarget.Get());
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

void UHarmoniaLockOnComponent::SwitchTarget(bool bSwitchRight)
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
	float BestAngle = bSwitchRight ? FLT_MAX : -FLT_MAX;

	FVector CurrentTargetDir = (GetTargetLockOnPoint(CurrentTarget.Get()) - CameraLocation).GetSafeNormal();

	for (AActor* Target : ValidTargets)
	{
		if (Target == CurrentTarget.Get())
		{
			continue; // Skip current target
		}

		FVector TargetDir = (GetTargetLockOnPoint(Target) - CameraLocation).GetSafeNormal();

		// Calculate angle from current target
		float DotRight = FVector::DotProduct(TargetDir, CameraRight);
		float DotCurrent = FVector::DotProduct(TargetDir, CurrentTargetDir);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotCurrent, -1.0f, 1.0f)));

		// Determine if target is on the correct side
		bool bIsOnRightSide = DotRight > 0.0f;

		if (bSwitchRight && bIsOnRightSide)
		{
			// Find target with smallest angle (closest to current target)
			if (Angle > 0.1f && Angle < BestAngle && Angle < 120.0f)
			{
				BestAngle = Angle;
				BestTarget = Target;
			}
		}
		else if (!bSwitchRight && !bIsOnRightSide)
		{
			// Find target with smallest angle (closest to current target)
			if (Angle > 0.1f && Angle < BestAngle && Angle < 120.0f)
			{
				BestAngle = Angle;
				BestTarget = Target;
			}
		}
	}

	if (BestTarget)
	{
		TWeakObjectPtr<AActor> OldTarget = CurrentTarget;
		CurrentTarget = BestTarget;
		OnLockOnTargetChanged.Broadcast(OldTarget.Get(), CurrentTarget.Get());
	}
}

float UHarmoniaLockOnComponent::GetAngleToTarget(AActor* Target) const
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
	FVector ToTarget = (GetTargetLockOnPoint(Target) - CameraLocation).GetSafeNormal();

	return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CameraForward, ToTarget)));
}

FVector2D UHarmoniaLockOnComponent::GetTargetScreenPosition(AActor* Target) const
{
	if (!Target || !OwnerCharacter.IsValid())
	{
		return FVector2D::ZeroVector;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		return FVector2D::ZeroVector;
	}

	FVector2D ScreenPosition;
	bool bSuccess = PC->ProjectWorldLocationToScreen(GetTargetLockOnPoint(Target), ScreenPosition);

	if (!bSuccess)
	{
		return FVector2D::ZeroVector;
	}

	// Convert to -1 to 1 range
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	FVector2D NormalizedPosition;
	NormalizedPosition.X = (ScreenPosition.X / ViewportSizeX) * 2.0f - 1.0f;
	NormalizedPosition.Y = (ScreenPosition.Y / ViewportSizeY) * 2.0f - 1.0f;

	return NormalizedPosition;
}
