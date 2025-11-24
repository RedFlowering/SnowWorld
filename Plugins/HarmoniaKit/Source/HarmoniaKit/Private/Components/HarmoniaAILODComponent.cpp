// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaAILODComponent.h"
#include "HarmoniaLogCategories.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"

UHarmoniaAILODComponent::UHarmoniaAILODComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// LOD evaluation doesn't need to happen every frame
	PrimaryComponentTick.TickInterval = 0.2f;
}

void UHarmoniaAILODComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initial LOD calculation
	UpdateLODLevel();
}

void UHarmoniaAILODComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableLOD)
	{
		return;
	}

	TimeSinceLastUpdate += DeltaTime;

	// Update LOD level periodically
	UpdateLODLevel();
}

// ============================================================================
// Public Functions
// ============================================================================

float UHarmoniaAILODComponent::GetCurrentUpdateInterval() const
{
	switch (CurrentLODLevel)
	{
	case EHarmoniaAILODLevel::VeryHigh:
		return VeryHighUpdateInterval;
	case EHarmoniaAILODLevel::High:
		return HighUpdateInterval;
	case EHarmoniaAILODLevel::Medium:
		return MediumUpdateInterval;
	case EHarmoniaAILODLevel::Low:
		return LowUpdateInterval;
	case EHarmoniaAILODLevel::VeryLow:
		return VeryLowUpdateInterval;
	case EHarmoniaAILODLevel::Disabled:
		return 999.0f; // Effectively never update
	default:
		return HighUpdateInterval;
	}
}

bool UHarmoniaAILODComponent::ShouldUpdateThisFrame() const
{
	if (!bEnableLOD)
	{
		return true; // Always update if LOD disabled
	}

	if (CurrentLODLevel == EHarmoniaAILODLevel::Disabled)
	{
		return false;
	}

	// Check if enough time has passed
	float UpdateInterval = GetCurrentUpdateInterval();
	return TimeSinceLastUpdate >= UpdateInterval;
}

void UHarmoniaAILODComponent::ForceLODLevel(EHarmoniaAILODLevel Level)
{
	bLODLevelForced = true;
	ForcedLODLevel = Level;

	EHarmoniaAILODLevel OldLevel = CurrentLODLevel;
	CurrentLODLevel = Level;

	if (OldLevel != CurrentLODLevel)
	{
		OnLODLevelChanged.Broadcast(OldLevel, CurrentLODLevel);
		TimeSinceLastUpdate = 0.0f;
	}
}

void UHarmoniaAILODComponent::ClearForcedLODLevel()
{
	bLODLevelForced = false;
	UpdateLODLevel();
}

APlayerController* UHarmoniaAILODComponent::GetNearestPlayer() const
{
	return NearestPlayer;
}

// ============================================================================
// Protected Functions
// ============================================================================

void UHarmoniaAILODComponent::UpdateLODLevel()
{
	if (!GetOwner() || !GetWorld())
	{
		return;
	}

	// If LOD level is forced, don't auto-update
	if (bLODLevelForced)
	{
		return;
	}

	// Find nearest player
	FindNearestPlayer();

	if (!NearestPlayer)
	{
		// No players, use lowest LOD
		EHarmoniaAILODLevel OldLevel = CurrentLODLevel;
		CurrentLODLevel = EHarmoniaAILODLevel::VeryLow;

		if (OldLevel != CurrentLODLevel)
		{
			OnLODLevelChanged.Broadcast(OldLevel, CurrentLODLevel);
			TimeSinceLastUpdate = 0.0f;
		}
		return;
	}

	// Combat override - always high LOD when in combat and close
	if (bForceHighLODInCombat && IsInCombat() && DistanceToNearestPlayer <= CombatHighLODDistance)
	{
		EHarmoniaAILODLevel OldLevel = CurrentLODLevel;
		CurrentLODLevel = EHarmoniaAILODLevel::VeryHigh;

		if (OldLevel != CurrentLODLevel)
		{
			OnLODLevelChanged.Broadcast(OldLevel, CurrentLODLevel);
			TimeSinceLastUpdate = 0.0f;
		}
		return;
	}

	// Calculate LOD from distance
	EHarmoniaAILODLevel DistanceLOD = CalculateLODFromDistance(DistanceToNearestPlayer);

	// Check screen visibility
	bool bVisible = true;
	if (bCheckScreenVisibility)
	{
		bVisible = IsVisibleOnScreen(NearestPlayer);
		bIsVisibleOnScreen = bVisible;
	}

	// Determine final LOD level
	EHarmoniaAILODLevel NewLOD = DistanceLOD;

	// Downgrade if not visible on screen
	if (!bVisible && DistanceLOD != EHarmoniaAILODLevel::VeryLow)
	{
		// Off-screen entities get one level lower LOD
		switch (DistanceLOD)
		{
		case EHarmoniaAILODLevel::VeryHigh:
			NewLOD = EHarmoniaAILODLevel::High;
			break;
		case EHarmoniaAILODLevel::High:
			NewLOD = EHarmoniaAILODLevel::Medium;
			break;
		case EHarmoniaAILODLevel::Medium:
			NewLOD = EHarmoniaAILODLevel::Low;
			break;
		case EHarmoniaAILODLevel::Low:
			NewLOD = EHarmoniaAILODLevel::VeryLow;
			break;
		default:
			break;
		}
	}

	// Update LOD level
	if (CurrentLODLevel != NewLOD)
	{
		EHarmoniaAILODLevel OldLevel = CurrentLODLevel;
		CurrentLODLevel = NewLOD;
		OnLODLevelChanged.Broadcast(OldLevel, CurrentLODLevel);
		TimeSinceLastUpdate = 0.0f;

		UE_LOG(LogHarmoniaAI, Verbose, TEXT("%s LOD changed: %s -> %s (Distance: %.0f, Visible: %s)"),
			*GetOwner()->GetName(),
			*UEnum::GetValueAsString(OldLevel),
			*UEnum::GetValueAsString(NewLOD),
			DistanceToNearestPlayer,
			bVisible ? TEXT("Yes") : TEXT("No"));
	}
}

EHarmoniaAILODLevel UHarmoniaAILODComponent::CalculateLODFromDistance(float Distance) const
{
	if (Distance <= VeryHighDistance)
	{
		return EHarmoniaAILODLevel::VeryHigh;
	}
	else if (Distance <= HighDistance)
	{
		return EHarmoniaAILODLevel::High;
	}
	else if (Distance <= MediumDistance)
	{
		return EHarmoniaAILODLevel::Medium;
	}
	else if (Distance <= LowDistance)
	{
		return EHarmoniaAILODLevel::Low;
	}
	else
	{
		return EHarmoniaAILODLevel::VeryLow;
	}
}

bool UHarmoniaAILODComponent::IsVisibleOnScreen(APlayerController* PlayerController) const
{
	if (!PlayerController || !GetOwner())
	{
		return false;
	}

	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (!CameraManager)
	{
		return false;
	}

	FVector ActorLocation = GetOwner()->GetActorLocation();
	FVector CameraLocation = CameraManager->GetCameraLocation();
	FRotator CameraRotation = CameraManager->GetCameraRotation();

	// Get direction to actor
	FVector DirectionToActor = (ActorLocation - CameraLocation).GetSafeNormal();
	FVector CameraForward = CameraRotation.Vector();

	// Check if in front of camera
	float DotProduct = FVector::DotProduct(DirectionToActor, CameraForward);
	if (DotProduct <= 0.0f)
	{
		return false; // Behind camera
	}

	// Check if within FOV
	float FOV = CameraManager->GetFOVAngle();
	float HalfFOVRadians = FMath::DegreesToRadians(FOV * 0.5f);
	float AngleToActor = FMath::Acos(DotProduct);

	// Add some margin to FOV check
	const float FOVMargin = 1.1f;
	return AngleToActor <= (HalfFOVRadians * FOVMargin);
}

void UHarmoniaAILODComponent::FindNearestPlayer()
{
	if (!GetWorld() || !GetOwner())
	{
		return;
	}

	float MinDistance = MAX_FLT;
	APlayerController* Nearest = nullptr;

	// Get all player controllers
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->GetPawn())
		{
			continue;
		}

		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PC->GetPawn()->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			Nearest = PC;
		}
	}

	NearestPlayer = Nearest;
	DistanceToNearestPlayer = MinDistance;
}

bool UHarmoniaAILODComponent::IsInCombat() const
{
	if (!GetOwner())
	{
		return false;
	}

	// Check if owner implements monster interface
	if (GetOwner()->Implements<UHarmoniaMonsterInterface>())
	{
		return IHarmoniaMonsterInterface::Execute_IsInCombat(GetOwner());
	}

	return false;
}
