// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaBaseAIComponent.h"
#include "HarmoniaLogCategories.h"
#include "AIController.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UHarmoniaBaseAIComponent::UHarmoniaBaseAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaBaseAIComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bComponentEnabled)
	{
		OnComponentEnabled();
		InitializeAIComponent();
		bInitialized = true;
	}
}

void UHarmoniaBaseAIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bInitialized)
	{
		CleanupAIComponent();
		bInitialized = false;
	}

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaBaseAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bComponentEnabled)
	{
		return;
	}

	// Handle update rate
	if (UpdateRate > 0.0f)
	{
		UpdateTimeAccumulator += DeltaTime;
		if (UpdateTimeAccumulator >= UpdateRate)
		{
			UpdateAIComponent(UpdateTimeAccumulator);
			UpdateTimeAccumulator = 0.0f;
		}
	}
	else
	{
		// Update every frame
		UpdateAIComponent(DeltaTime);
	}

	// Debug visualization
	if (bShowDebugInfo)
	{
		DrawDebugInfo();
	}
}

// ====================================
// Enable/Disable
// ====================================

void UHarmoniaBaseAIComponent::SetAIComponentEnabled(bool bEnabled)
{
	if (bComponentEnabled == bEnabled)
	{
		return;
	}

	bComponentEnabled = bEnabled;

	if (bEnabled)
	{
		OnComponentEnabled();
		SetComponentTickEnabled(true);

		if (!bInitialized)
		{
			InitializeAIComponent();
			bInitialized = true;
		}
	}
	else
	{
		OnComponentDisabled();
		SetComponentTickEnabled(false);
	}
}

// ====================================
// AI Controller Access
// ====================================

AAIController* UHarmoniaBaseAIComponent::GetAIController()
{
	if (CachedAIController)
	{
		return CachedAIController;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Try to get AI controller directly if owner is a pawn
	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		CachedAIController = Cast<AAIController>(Pawn->GetController());
	}

	return CachedAIController;
}

AHarmoniaMonsterBase* UHarmoniaBaseAIComponent::GetMonster()
{
	if (CachedMonster)
	{
		return CachedMonster;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	CachedMonster = Cast<AHarmoniaMonsterBase>(Owner);
	return CachedMonster;
}

APawn* UHarmoniaBaseAIComponent::GetControlledPawn()
{
	AAIController* Controller = GetAIController();
	if (!Controller)
	{
		// If owner is a pawn, return it
		return Cast<APawn>(GetOwner());
	}

	return Controller->GetPawn();
}

// ====================================
// Component State Queries
// ====================================

bool UHarmoniaBaseAIComponent::HasValidTarget() const
{
	AActor* Target = GetCurrentTarget();
	return Target && !Target->IsPendingKillPending();
}

bool UHarmoniaBaseAIComponent::IsInCombat() const
{
	// Base implementation - subclasses should override
	return HasValidTarget();
}

AActor* UHarmoniaBaseAIComponent::GetCurrentTarget() const
{
	if (CachedAIController)
	{
		if (UBlackboardComponent* Blackboard = CachedAIController->GetBlackboardComponent())
		{
			return Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetActor")));
		}
	}

	return nullptr;
}

// ====================================
// Debug Visualization
// ====================================

void UHarmoniaBaseAIComponent::DrawDebugInfo()
{
	// Base implementation - subclasses can override
	if (!GetWorld())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector OwnerLocation = Owner->GetActorLocation();
	FString DebugString = FString::Printf(TEXT("AI Component: %s\nEnabled: %s"),
		*GetClass()->GetName(),
		bComponentEnabled ? TEXT("Yes") : TEXT("No"));

	DrawDebugString(GetWorld(), OwnerLocation + FVector(0, 0, 100), DebugString, nullptr, FColor::Yellow, 0.0f, true);
}

// ====================================
// Protected Helpers
// ====================================

void UHarmoniaBaseAIComponent::OnComponentEnabled()
{
	// Override in subclasses for custom logic
}

void UHarmoniaBaseAIComponent::OnComponentDisabled()
{
	// Override in subclasses for custom logic
}

void UHarmoniaBaseAIComponent::UpdateAIComponent(float DeltaTime)
{
	// Override in subclasses for update logic
}

void UHarmoniaBaseAIComponent::InitializeAIComponent()
{
	// Override in subclasses for initialization
}

void UHarmoniaBaseAIComponent::CleanupAIComponent()
{
	// Override in subclasses for cleanup
}

void UHarmoniaBaseAIComponent::InvalidateCachedReferences()
{
	CachedAIController = nullptr;
	CachedMonster = nullptr;
}

// ====================================
// Blackboard Integration
// ====================================

UBlackboardComponent* UHarmoniaBaseAIComponent::GetBlackboardComponent() const
{
	if (CachedAIController)
	{
		return CachedAIController->GetBlackboardComponent();
	}
	return nullptr;
}

void UHarmoniaBaseAIComponent::SetBlackboardValueAsObject(FName KeyName, UObject* ObjectValue)
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		Blackboard->SetValueAsObject(KeyName, ObjectValue);
	}
}

UObject* UHarmoniaBaseAIComponent::GetBlackboardValueAsObject(FName KeyName) const
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		return Blackboard->GetValueAsObject(KeyName);
	}
	return nullptr;
}

void UHarmoniaBaseAIComponent::SetBlackboardValueAsVector(FName KeyName, FVector VectorValue)
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		Blackboard->SetValueAsVector(KeyName, VectorValue);
	}
}

FVector UHarmoniaBaseAIComponent::GetBlackboardValueAsVector(FName KeyName) const
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		return Blackboard->GetValueAsVector(KeyName);
	}
	return FVector::ZeroVector;
}

void UHarmoniaBaseAIComponent::SetBlackboardValueAsBool(FName KeyName, bool BoolValue)
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		Blackboard->SetValueAsBool(KeyName, BoolValue);
	}
}

bool UHarmoniaBaseAIComponent::GetBlackboardValueAsBool(FName KeyName) const
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		return Blackboard->GetValueAsBool(KeyName);
	}
	return false;
}

void UHarmoniaBaseAIComponent::SetBlackboardValueAsFloat(FName KeyName, float FloatValue)
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		Blackboard->SetValueAsFloat(KeyName, FloatValue);
	}
}

float UHarmoniaBaseAIComponent::GetBlackboardValueAsFloat(FName KeyName) const
{
	if (UBlackboardComponent* Blackboard = GetBlackboardComponent())
	{
		return Blackboard->GetValueAsFloat(KeyName);
	}
	return 0.0f;
}

// ====================================
// Behavior Tree Integration
// ====================================

UBehaviorTreeComponent* UHarmoniaBaseAIComponent::GetBehaviorTreeComponent() const
{
	if (CachedAIController)
	{
		return Cast<UBehaviorTreeComponent>(CachedAIController->BrainComponent);
	}
	return nullptr;
}

bool UHarmoniaBaseAIComponent::IsBehaviorTreeRunning() const
{
	if (UBehaviorTreeComponent* BTComponent = GetBehaviorTreeComponent())
	{
		return BTComponent->IsRunning();
	}
	return false;
}
