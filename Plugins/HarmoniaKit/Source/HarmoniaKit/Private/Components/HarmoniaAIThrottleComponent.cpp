// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaAIThrottleComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

UHarmoniaAIThrottleComponent::UHarmoniaAIThrottleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Low frequency tick
}

void UHarmoniaAIThrottleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache AI Controller
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		CachedAIController = Cast<AAIController>(Pawn->GetController());
	}
	else if (AAIController* AIController = Cast<AAIController>(GetOwner()))
	{
		CachedAIController = AIController;
	}

	// Cache perception component
	if (CachedAIController.IsValid())
	{
		CachedPerceptionComponent = CachedAIController->GetComponentByClass(UAIPerceptionComponent::StaticClass());
		
		if (UAIPerceptionComponent* Perception = Cast<UAIPerceptionComponent>(CachedPerceptionComponent.Get()))
		{
			// Store original values
			// Note: UAIPerceptionComponent doesn't expose interval directly,
			// we use SetSenseEnabled/SetPerceptionUpdatesEnabled instead
		}
	}

	// Register with UpdateRateSubsystem if configured
	if (Config.bAutoRegisterWithUpdateRate)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHarmoniaUpdateRateSubsystem* UpdateRateSubsystem = World->GetSubsystem<UHarmoniaUpdateRateSubsystem>())
			{
				UpdateRateSubsystem->RegisterActor(GetOwner(), Config.UpdateRateConfig);
				UpdateRateSubsystem->OnTierChanged.AddDynamic(this, &UHarmoniaAIThrottleComponent::OnUpdateTierChanged);
			}
		}
	}
}

void UHarmoniaAIThrottleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from UpdateRateSubsystem
	if (Config.bAutoRegisterWithUpdateRate)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHarmoniaUpdateRateSubsystem* UpdateRateSubsystem = World->GetSubsystem<UHarmoniaUpdateRateSubsystem>())
			{
				UpdateRateSubsystem->OnTierChanged.RemoveDynamic(this, &UHarmoniaAIThrottleComponent::OnUpdateTierChanged);
				UpdateRateSubsystem->UnregisterActor(GetOwner());
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaAIThrottleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update boost timer
	if (BoostTimeRemaining > 0.0f)
	{
		BoostTimeRemaining -= DeltaTime;
		if (BoostTimeRemaining <= 0.0f)
		{
			BoostTimeRemaining = 0.0f;
			// Re-evaluate state after boost expires
			if (!bStateForced)
			{
				// State will be updated by UpdateRateSubsystem
			}
		}
	}

	// Apply throttling based on current state
	ApplyPerceptionThrottling();
	ApplyBehaviorTreeThrottling();
	ApplyMovementThrottling();
	ApplyAnimationThrottling();
}

float UHarmoniaAIThrottleComponent::GetCurrentPerceptionInterval() const
{
	switch (CurrentState)
	{
		case EHarmoniaAIThrottleState::FullUpdate:
			return Config.FullPerceptionInterval;
		case EHarmoniaAIThrottleState::ReducedUpdate:
			return Config.ReducedPerceptionInterval;
		case EHarmoniaAIThrottleState::MinimalUpdate:
			return Config.MinimalPerceptionInterval;
		case EHarmoniaAIThrottleState::Suspended:
			return 0.0f; // No perception
		default:
			return Config.FullPerceptionInterval;
	}
}

float UHarmoniaAIThrottleComponent::GetCurrentBTInterval() const
{
	switch (CurrentState)
	{
		case EHarmoniaAIThrottleState::FullUpdate:
			return Config.FullBTInterval;
		case EHarmoniaAIThrottleState::ReducedUpdate:
			return Config.ReducedBTInterval;
		case EHarmoniaAIThrottleState::MinimalUpdate:
			return Config.MinimalBTInterval;
		case EHarmoniaAIThrottleState::Suspended:
			return 0.0f; // Paused
		default:
			return Config.FullBTInterval;
	}
}

void UHarmoniaAIThrottleComponent::ForceThrottleState(EHarmoniaAIThrottleState State)
{
	bStateForced = true;
	ForcedState = State;

	EHarmoniaAIThrottleState OldState = CurrentState;
	CurrentState = State;

	if (OldState != CurrentState)
	{
		OnStateChanged(OldState, CurrentState);
	}
}

void UHarmoniaAIThrottleComponent::ClearForcedState()
{
	bStateForced = false;
	// State will be updated by next UpdateRateSubsystem callback
}

void UHarmoniaAIThrottleComponent::SetInCombat(bool bInCombat)
{
	bIsInCombat = bInCombat;

	if (bInCombat && !bStateForced)
	{
		EHarmoniaAIThrottleState OldState = CurrentState;
		CurrentState = EHarmoniaAIThrottleState::FullUpdate;

		if (OldState != CurrentState)
		{
			OnStateChanged(OldState, CurrentState);
		}
	}
}

void UHarmoniaAIThrottleComponent::BoostUpdateRate(float Duration)
{
	BoostTimeRemaining = Duration;

	if (!bStateForced)
	{
		EHarmoniaAIThrottleState OldState = CurrentState;
		CurrentState = EHarmoniaAIThrottleState::FullUpdate;

		if (OldState != CurrentState)
		{
			OnStateChanged(OldState, CurrentState);
		}
	}
}

void UHarmoniaAIThrottleComponent::OnStateChanged(EHarmoniaAIThrottleState OldState, EHarmoniaAIThrottleState NewState)
{
	OnThrottleStateChanged.Broadcast(OldState, NewState);

	// Log state change in development
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Verbose, TEXT("[HarmoniaAIThrottle] %s: %s -> %s"),
		*GetOwner()->GetName(),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));
#endif
}

void UHarmoniaAIThrottleComponent::ApplyPerceptionThrottling()
{
	if (!CachedAIController.IsValid())
	{
		return;
	}

	UAIPerceptionComponent* Perception = Cast<UAIPerceptionComponent>(CachedPerceptionComponent.Get());
	if (!Perception)
	{
		return;
	}

	switch (CurrentState)
	{
		case EHarmoniaAIThrottleState::FullUpdate:
			Perception->SetComponentTickEnabled(true);
			Perception->Activate(true);
			break;

		case EHarmoniaAIThrottleState::ReducedUpdate:
		case EHarmoniaAIThrottleState::MinimalUpdate:
			// Keep perception enabled but at reduced rate
			// Note: Direct interval control requires modifying AI config
			Perception->SetComponentTickEnabled(true);
			break;

		case EHarmoniaAIThrottleState::Suspended:
			if (Config.bDisableSightWhenSuspended || Config.bDisableHearingWhenSuspended)
			{
				Perception->SetComponentTickEnabled(false);
			}
			break;
	}
}

void UHarmoniaAIThrottleComponent::ApplyBehaviorTreeThrottling()
{
	if (!CachedAIController.IsValid())
	{
		return;
	}

	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(CachedAIController->GetBrainComponent());
	if (!BTComp)
	{
		return;
	}

	switch (CurrentState)
	{
		case EHarmoniaAIThrottleState::FullUpdate:
			if (BTComp->IsPaused())
			{
				BTComp->ResumeLogic(TEXT("AIThrottle_FullUpdate"));
			}
			break;

		case EHarmoniaAIThrottleState::ReducedUpdate:
		case EHarmoniaAIThrottleState::MinimalUpdate:
			if (BTComp->IsPaused())
			{
				BTComp->ResumeLogic(TEXT("AIThrottle_ThrottledUpdate"));
			}
			// Note: BT tick interval adjustment requires custom BT tick management
			break;

		case EHarmoniaAIThrottleState::Suspended:
			if (Config.bPauseBTWhenSuspended && !BTComp->IsPaused())
			{
				BTComp->PauseLogic(TEXT("AIThrottle_Suspended"));
			}
			break;
	}
}

void UHarmoniaAIThrottleComponent::ApplyMovementThrottling()
{
	if (!CachedAIController.IsValid())
	{
		return;
	}

	UPathFollowingComponent* PathComp = CachedAIController->GetPathFollowingComponent();
	if (!PathComp)
	{
		return;
	}

	switch (CurrentState)
	{
		case EHarmoniaAIThrottleState::FullUpdate:
			// Normal movement
			break;

		case EHarmoniaAIThrottleState::ReducedUpdate:
		case EHarmoniaAIThrottleState::MinimalUpdate:
			// Throttled path updates handled by path update interval
			break;

		case EHarmoniaAIThrottleState::Suspended:
			if (Config.bStopMovementWhenSuspended)
			{
				CachedAIController->StopMovement();
			}
			break;
	}
}

void UHarmoniaAIThrottleComponent::ApplyAnimationThrottling()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	USkeletalMeshComponent* SkelMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!SkelMesh)
	{
		return;
	}

	switch (CurrentState)
	{
		case EHarmoniaAIThrottleState::FullUpdate:
			if (Config.bReduceAnimationLOD)
			{
				SkelMesh->SetComponentTickEnabled(true);
				SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			}
			break;

		case EHarmoniaAIThrottleState::ReducedUpdate:
			if (Config.bReduceAnimationLOD)
			{
				SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			}
			break;

		case EHarmoniaAIThrottleState::MinimalUpdate:
			if (Config.bReduceAnimationLOD)
			{
				SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
			}
			break;

		case EHarmoniaAIThrottleState::Suspended:
			if (Config.bSkipAnimationWhenSuspended)
			{
				SkelMesh->SetComponentTickEnabled(false);
			}
			break;
	}
}

EHarmoniaAIThrottleState UHarmoniaAIThrottleComponent::TierToThrottleState(EHarmoniaUpdateTier Tier) const
{
	switch (Tier)
	{
		case EHarmoniaUpdateTier::Critical:
		case EHarmoniaUpdateTier::High:
			return EHarmoniaAIThrottleState::FullUpdate;

		case EHarmoniaUpdateTier::Medium:
			return EHarmoniaAIThrottleState::ReducedUpdate;

		case EHarmoniaUpdateTier::Low:
		case EHarmoniaUpdateTier::Minimal:
			return EHarmoniaAIThrottleState::MinimalUpdate;

		case EHarmoniaUpdateTier::Dormant:
			return EHarmoniaAIThrottleState::Suspended;

		default:
			return EHarmoniaAIThrottleState::FullUpdate;
	}
}

void UHarmoniaAIThrottleComponent::OnUpdateTierChanged(AActor* Actor, EHarmoniaUpdateTier OldTier, EHarmoniaUpdateTier NewTier)
{
	// Only handle our owner
	if (Actor != GetOwner())
	{
		return;
	}

	// Skip if state is forced, in combat, or boosted
	if (bStateForced || bIsInCombat || BoostTimeRemaining > 0.0f)
	{
		return;
	}

	EHarmoniaAIThrottleState NewState = TierToThrottleState(NewTier);
	
	if (NewState != CurrentState)
	{
		EHarmoniaAIThrottleState OldState = CurrentState;
		CurrentState = NewState;
		OnStateChanged(OldState, NewState);
	}
}
