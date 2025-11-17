// Copyright 2025 Snow Game Studio.

#include "AI/BTService_OptimizedMonsterUpdate.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/HarmoniaAILODComponent.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "GameFramework/Pawn.h"

// Memory structure for this service
struct FBTOptimizedMonsterUpdateMemory
{
	FBTCachedQueryResult CachedResult;
	float TimeSinceLastUpdate = 0.0f;
};

UBTService_OptimizedMonsterUpdate::UBTService_OptimizedMonsterUpdate()
{
	NodeName = "Optimized Monster Update";

	// Default update interval (will be overridden by LOD if enabled)
	Interval = 0.25f;
	RandomDeviation = 0.05f;

	bNotifyTick = true;
	bNotifyBecomeRelevant = true;

	bOnlyUpdateInCombat = true;
	bUseLODSystem = true;
	bEnableCaching = true;
	MaxCacheAge = 0.5f;
	AttackRangeThreshold = 300.0f;
}

void UBTService_OptimizedMonsterUpdate::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// Initialize blackboard keys
	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		DistanceKey.ResolveSelectedKey(*BBAsset);
		InAttackRangeKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_OptimizedMonsterUpdate::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Get memory
	FBTOptimizedMonsterUpdateMemory* Memory = CastInstanceNodeMemory<FBTOptimizedMonsterUpdateMemory>(NodeMemory);
	if (!Memory)
	{
		return;
	}

	Memory->TimeSinceLastUpdate += DeltaSeconds;

	// Check if should update this tick
	if (!ShouldUpdateThisTick(OwnerComp, DeltaSeconds))
	{
		return;
	}

	// Reset timer
	Memory->TimeSinceLastUpdate = 0.0f;

	// Get cached results or update
	FBTCachedQueryResult* Cache = GetCache(NodeMemory);
	if (!Cache)
	{
		return;
	}

	float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();

	// Use cache if valid
	if (bEnableCaching && Cache->IsCacheValid(CurrentTime, MaxCacheAge))
	{
		// Use cached values
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsObject(TargetKey.SelectedKeyName, Cache->CachedTarget);
			BlackboardComp->SetValueAsFloat(DistanceKey.SelectedKeyName, Cache->CachedDistance);
			BlackboardComp->SetValueAsBool(InAttackRangeKey.SelectedKeyName, Cache->bInAttackRange);
		}
		return;
	}

	// Cache invalid or disabled, update queries
	UpdateCachedQueries(OwnerComp, NodeMemory);
}

FString UBTService_OptimizedMonsterUpdate::GetStaticDescription() const
{
	return FString::Printf(TEXT("Optimized Monster Update\nLOD: %s, Cache: %s, Combat Only: %s"),
		bUseLODSystem ? TEXT("On") : TEXT("Off"),
		bEnableCaching ? TEXT("On") : TEXT("Off"),
		bOnlyUpdateInCombat ? TEXT("Yes") : TEXT("No"));
}

// ============================================================================
// Protected Functions
// ============================================================================

bool UBTService_OptimizedMonsterUpdate::ShouldUpdateThisTick(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	// Check combat state if configured
	if (bOnlyUpdateInCombat && !IsInCombat(ControlledPawn))
	{
		return false;
	}

	// Check LOD system
	if (bUseLODSystem)
	{
		UHarmoniaAILODComponent* LODComponent = ControlledPawn->FindComponentByClass<UHarmoniaAILODComponent>();
		if (LODComponent && LODComponent->bEnableLOD)
		{
			// Use LOD component to determine if should update
			return LODComponent->ShouldUpdateThisFrame();
		}
	}

	// Fallback to default interval check
	FBTOptimizedMonsterUpdateMemory* Memory = CastInstanceNodeMemory<FBTOptimizedMonsterUpdateMemory>(OwnerComp.GetNodeMemory(this, OwnerComp.GetActiveInstanceIdx()));
	if (Memory)
	{
		return Memory->TimeSinceLastUpdate >= Interval;
	}

	return true;
}

void UBTService_OptimizedMonsterUpdate::UpdateCachedQueries(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	// Get current target
	AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

	// Find best target if no current target
	if (!CurrentTarget && ControlledPawn->Implements<UHarmoniaMonsterInterface>())
	{
		// This is a potentially expensive query - cache the result
		CurrentTarget = IHarmoniaMonsterInterface::Execute_GetCurrentTarget(ControlledPawn);
	}

	// Calculate distance and attack range
	float Distance = 0.0f;
	bool bInRange = false;

	if (CurrentTarget)
	{
		Distance = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		bInRange = Distance <= AttackRangeThreshold;
	}

	// Update cache
	FBTCachedQueryResult* Cache = GetCache(NodeMemory);
	if (Cache)
	{
		Cache->CachedTarget = CurrentTarget;
		Cache->CachedDistance = Distance;
		Cache->bInAttackRange = bInRange;
		Cache->CacheTime = OwnerComp.GetWorld()->GetTimeSeconds();
		Cache->bIsValid = true;
	}

	// Update blackboard
	BlackboardComp->SetValueAsObject(TargetKey.SelectedKeyName, CurrentTarget);
	BlackboardComp->SetValueAsFloat(DistanceKey.SelectedKeyName, Distance);
	BlackboardComp->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInRange);
}

FBTCachedQueryResult* UBTService_OptimizedMonsterUpdate::GetCache(uint8* NodeMemory)
{
	FBTOptimizedMonsterUpdateMemory* Memory = CastInstanceNodeMemory<FBTOptimizedMonsterUpdateMemory>(NodeMemory);
	return Memory ? &Memory->CachedResult : nullptr;
}

bool UBTService_OptimizedMonsterUpdate::IsInCombat(AActor* Owner) const
{
	if (!Owner)
	{
		return false;
	}

	// Check if owner implements monster interface
	if (Owner->Implements<UHarmoniaMonsterInterface>())
	{
		return IHarmoniaMonsterInterface::Execute_IsInCombat(Owner);
	}

	return false;
}

// Register memory size
uint16 UBTService_OptimizedMonsterUpdate::GetInstanceMemorySize() const
{
	return sizeof(FBTOptimizedMonsterUpdateMemory);
}
