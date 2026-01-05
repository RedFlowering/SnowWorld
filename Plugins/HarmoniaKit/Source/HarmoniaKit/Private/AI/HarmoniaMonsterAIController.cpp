// Copyright 2025 Snow Game Studio.

#include "AI/HarmoniaMonsterAIController.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Navigation/CrowdFollowingComponent.h"

AHarmoniaMonsterAIController::AHarmoniaMonsterAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create AI Perception Component
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));

	// Create sight config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 2000.0f;
		SightConfig->LoseSightRadius = 2500.0f;
		SightConfig->PeripheralVisionAngleDegrees = 360.0f;  // 360 degree vision
		// Detect ALL actors regardless of team affiliation
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		// Auto succeed for visibility checks
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;

		GetPerceptionComponent()->ConfigureSense(*SightConfig);
		GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
	}

	// Create hearing config
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 1500.0f;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

		GetPerceptionComponent()->ConfigureSense(*HearingConfig);
	}

	// Create damage config
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	if (DamageConfig)
	{
		GetPerceptionComponent()->ConfigureSense(*DamageConfig);
	}

	// Bind perception events
	if (GetPerceptionComponent())
	{
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AHarmoniaMonsterAIController::OnPerceptionUpdated);
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AHarmoniaMonsterAIController::OnTargetPerceptionUpdated);
	}

	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
}

void AHarmoniaMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Cache monster reference
	ControlledMonster = Cast<AHarmoniaMonsterBase>(InPawn);

	if (ControlledMonster)
	{
		// Store home location
		HomeLocation = ControlledMonster->GetActorLocation();

		// Bind perception events (re-bind in case constructor binding failed)
		if (GetPerceptionComponent())
		{
			GetPerceptionComponent()->OnPerceptionUpdated.RemoveDynamic(this, &AHarmoniaMonsterAIController::OnPerceptionUpdated);
			GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AHarmoniaMonsterAIController::OnPerceptionUpdated);
			
			GetPerceptionComponent()->OnTargetPerceptionUpdated.RemoveDynamic(this, &AHarmoniaMonsterAIController::OnTargetPerceptionUpdated);
			GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AHarmoniaMonsterAIController::OnTargetPerceptionUpdated);
			
		}
		else
		{
		}

		// Start behavior tree
		StartBehaviorTree();
	}
}

void AHarmoniaMonsterAIController::OnUnPossess()
{
	// Stop behavior tree
	StopBehaviorTree();

	ControlledMonster = nullptr;

	Super::OnUnPossess();
}

void AHarmoniaMonsterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Validate current target (only if we have one)
	if (GetCurrentTarget() && !IsTargetValid())
	{
		ClearTarget();
	}
}

// ============================================================================
// Target Management
// ============================================================================

AActor* AHarmoniaMonsterAIController::GetCurrentTarget() const
{
	if (ControlledMonster)
	{
		return IHarmoniaMonsterInterface::Execute_GetCurrentTarget(ControlledMonster);
	}
	return nullptr;
}

void AHarmoniaMonsterAIController::SetCurrentTarget(AActor* NewTarget)
{
	if (ControlledMonster)
	{
		IHarmoniaMonsterInterface::Execute_SetCurrentTarget(ControlledMonster, NewTarget);

		// Update blackboard
		if (GetBlackboardComponent())
		{
			GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), NewTarget);
		}
	}
}

void AHarmoniaMonsterAIController::ClearTarget()
{
	SetCurrentTarget(nullptr);
}

bool AHarmoniaMonsterAIController::IsTargetValid() const
{
	AActor* Target = GetCurrentTarget();
	if (!Target || Target->IsPendingKillPending())
	{
		return false;
	}

	// Check if target implements monster interface and is dead
	if (Target->Implements<UHarmoniaMonsterInterface>())
	{
		if (IHarmoniaMonsterInterface::Execute_IsDead(Target))
		{
			return false;
		}
	}

	// Check if target is a pawn and is dead
	if (APawn* TargetPawn = Cast<APawn>(Target))
	{
		if (TargetPawn->GetController() == nullptr)
		{
			return false;
		}
	}

	return true;
}

float AHarmoniaMonsterAIController::GetDistanceToTarget() const
{
	AActor* Target = GetCurrentTarget();
	if (!Target || !ControlledMonster)
	{
		return -1.0f;
	}

	return FVector::Dist(ControlledMonster->GetActorLocation(), Target->GetActorLocation());
}

// ============================================================================
// Combat
// ============================================================================

bool AHarmoniaMonsterAIController::RequestAttack(FName AttackID)
{
	// This function is deprecated - use Behavior Tree with BTTask_MonsterAttack instead
	UE_LOG(LogTemp, Warning, TEXT("RequestAttack is deprecated. Use BTTask_MonsterAttack with AbilityClass."));
	return false;
}

FName AHarmoniaMonsterAIController::SelectBestAttack() const
{
	// This function is deprecated - attack selection is now handled by Behavior Tree
	return NAME_None;
}

// ============================================================================
// Patrol & Movement
// ============================================================================

FVector AHarmoniaMonsterAIController::GetRandomPatrolLocation() const
{
	if (!ControlledMonster)
	{
		return FVector::ZeroVector;
	}

	// Default patrol radius (now configured in BT, not in MonsterData)
	float PatrolRadius = 500.0f;

	// Generate random point around home location
	FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, PatrolRadius);
	RandomOffset.Z = 0.0f; // Keep on same plane

	return HomeLocation + RandomOffset;
}

bool AHarmoniaMonsterAIController::MoveToPatrolLocation(const FVector& PatrolLocation)
{
	if (!ControlledMonster)
	{
		return false;
	}

	EPathFollowingRequestResult::Type Result = MoveToLocation(PatrolLocation, 50.0f);
	return Result == EPathFollowingRequestResult::RequestSuccessful;
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaMonsterAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!ControlledMonster)
	{
		return;
	}

	// Get aggro type
	EHarmoniaMonsterAggroType AggroType = IHarmoniaMonsterInterface::Execute_GetAggroType(ControlledMonster);

	// If passive, ignore perception
	if (AggroType == EHarmoniaMonsterAggroType::Passive)
	{
		return;
	}

	// If we already have a target, don't switch unless current is invalid
	if (IsTargetValid())
	{
		return;
	}

	// Find valid targets
	TArray<AActor*> ValidTargets;
	for (AActor* Actor : UpdatedActors)
	{
		if (ShouldTargetActor(Actor))
		{
			ValidTargets.Add(Actor);
		}
	}

	// Select best target
	if (ValidTargets.Num() > 0)
	{
		AActor* BestTarget = SelectBestTarget(ValidTargets);
		if (BestTarget)
		{
			SetCurrentTarget(BestTarget);
		}
	}
}

void AHarmoniaMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!ControlledMonster || !Actor)
	{
		return;
	}

	// If we lost sight of current target, clear it
	if (Actor == GetCurrentTarget() && !Stimulus.WasSuccessfullySensed())
	{
		ClearTarget();
	}
}

bool AHarmoniaMonsterAIController::ShouldTargetActor(AActor* Actor) const
{
	if (!Actor || !ControlledMonster)
	{
		return false;
	}

	// Don't target self
	if (Actor == ControlledMonster)
	{
		return false;
	}

	// Don't target other monsters (for now - could add faction system later)
	if (Actor->Implements<UHarmoniaMonsterInterface>())
	{
		return false;
	}

	// Check if it's a pawn
	APawn* TargetPawn = Cast<APawn>(Actor);
	if (!TargetPawn)
	{
		return false;
	}

	// Check if pawn is alive
	if (!TargetPawn->GetController())
	{
		return false;
	}

	return true;
}

AActor* AHarmoniaMonsterAIController::SelectBestTarget(const TArray<AActor*>& PotentialTargets) const
{
	if (!ControlledMonster || PotentialTargets.Num() == 0)
	{
		return nullptr;
	}

	// Simple selection: closest target
	AActor* BestTarget = nullptr;
	float ClosestDistance = MAX_FLT;

	for (AActor* Target : PotentialTargets)
	{
		float Distance = FVector::Dist(ControlledMonster->GetActorLocation(), Target->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

void AHarmoniaMonsterAIController::StartBehaviorTree()
{
	if (!ControlledMonster)
	{
		return;
	}

	// Get behavior tree from monster data
	UHarmoniaMonsterData* MonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(ControlledMonster);
	if (!MonsterData || !MonsterData->BehaviorTree)
	{
		return;
	}

	CurrentBehaviorTree = MonsterData->BehaviorTree;

	// Run behavior tree
	if (CurrentBehaviorTree)
	{
		RunBehaviorTree(CurrentBehaviorTree);

		// Initialize blackboard values
		if (GetBlackboardComponent())
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
		}
	}
}

void AHarmoniaMonsterAIController::StopBehaviorTree()
{
	if (GetBrainComponent())
	{
		GetBrainComponent()->StopLogic(TEXT("Unpossessed"));
	}

	CurrentBehaviorTree = nullptr;
}
