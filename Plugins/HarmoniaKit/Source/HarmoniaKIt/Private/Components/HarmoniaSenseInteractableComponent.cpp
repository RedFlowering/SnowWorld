// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSenseInteractableComponent.h"
#include "Managers/HarmoniaInteractionManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UHarmoniaSenseInteractableComponent::UHarmoniaSenseInteractableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	// Set default sense stimulus settings
	bAutoRegisterSenseStimulus = true;
}

void UHarmoniaSenseInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeDefaultInteractions();
}

void UHarmoniaSenseInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// ============================================================================
// Interaction Interface Implementation
// ============================================================================

void UHarmoniaSenseInteractableComponent::OnInteract_Implementation(
	const FHarmoniaInteractionContext& Context,
	FHarmoniaInteractionResult& OutResult)
{
	if (!bIsActive)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Interactable is not active");
		return;
	}

	AActor* Interactor = Context.Interactor;
	if (!Interactor)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Invalid interactor");
		return;
	}

	// Try to find a matching interaction config
	bool bInteractionProcessed = false;

	for (FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		// Check if this interaction type matches
		if (InteractionData.Config.InteractionType == Context.InteractionType ||
			Context.InteractionType == EHarmoniaInteractionType::Custom)
		{
			// Process the interaction
			if (ProcessInteraction(Interactor, InteractionData, OutResult))
			{
				bInteractionProcessed = true;

				// Broadcast interaction event
				OnSenseInteractionTriggered.Broadcast(Interactor, this, InteractionData.Config.SensorTag);
				break;
			}
		}
	}

	if (!bInteractionProcessed)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("No valid interaction configuration found");
	}
}

// ============================================================================
// Sense Interaction Functions
// ============================================================================

FSenseInteractionData* UHarmoniaSenseInteractableComponent::GetInteractionConfigBySensorTag(FName SensorTag)
{
	for (FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		if (InteractionData.Config.SensorTag == SensorTag)
		{
			return &InteractionData;
		}
	}
	return nullptr;
}

FSenseInteractionData* UHarmoniaSenseInteractableComponent::GetInteractionConfigByTriggerType(
	ESenseInteractionTriggerType TriggerType)
{
	for (FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		if (InteractionData.Config.TriggerType == TriggerType)
		{
			return &InteractionData;
		}
	}
	return nullptr;
}

void UHarmoniaSenseInteractableComponent::AddInteractionConfig(const FSenseInteractionConfig& Config)
{
	FSenseInteractionData NewData;
	NewData.Config = Config;
	NewData.Reset();

	InteractionConfigs.Add(NewData);
}

bool UHarmoniaSenseInteractableComponent::RemoveInteractionConfig(FName SensorTag)
{
	const int32 RemovedCount = InteractionConfigs.RemoveAll([SensorTag](const FSenseInteractionData& Data)
	{
		return Data.Config.SensorTag == SensorTag;
	});

	return RemovedCount > 0;
}

void UHarmoniaSenseInteractableComponent::SetInteractionEnabled(FName SensorTag, bool bEnabled)
{
	if (FSenseInteractionData* InteractionData = GetInteractionConfigBySensorTag(SensorTag))
	{
		InteractionData->Config.bEnabled = bEnabled;
	}
}

void UHarmoniaSenseInteractableComponent::SetAllInteractionsEnabled(bool bEnabled)
{
	for (FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		InteractionData.Config.bEnabled = bEnabled;
	}
}

void UHarmoniaSenseInteractableComponent::ResetInteractionData(FName SensorTag)
{
	if (FSenseInteractionData* InteractionData = GetInteractionConfigBySensorTag(SensorTag))
	{
		InteractionData->Reset();
	}
}

void UHarmoniaSenseInteractableComponent::ResetAllInteractionData()
{
	for (FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		InteractionData.Reset();
	}
}

bool UHarmoniaSenseInteractableComponent::IsInteractionAvailable(FName SensorTag) const
{
	if (!bIsActive)
	{
		return false;
	}

	for (const FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		if (InteractionData.Config.SensorTag == SensorTag)
		{
			const UWorld* World = GetWorld();
			const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
			return InteractionData.IsAvailable(CurrentTime);
		}
	}

	return false;
}

FText UHarmoniaSenseInteractableComponent::GetInteractionPrompt(FName SensorTag) const
{
	for (const FSenseInteractionData& InteractionData : InteractionConfigs)
	{
		if (InteractionData.Config.SensorTag == SensorTag)
		{
			if (InteractionData.Config.bShowInteractionPrompt)
			{
				return InteractionData.Config.InteractionPrompt;
			}
		}
	}

	return FText::GetEmpty();
}

// ============================================================================
// Protected Functions
// ============================================================================

void UHarmoniaSenseInteractableComponent::InitializeDefaultInteractions()
{
	// If no interactions are configured, set up a default manual interaction
	if (InteractionConfigs.Num() == 0)
	{
		FSenseInteractionConfig DefaultConfig;
		DefaultConfig.InteractionType = EHarmoniaInteractionType::Custom;
		DefaultConfig.TriggerType = ESenseInteractionTriggerType::Manual;
		DefaultConfig.InteractionMode = ESenseInteractionMode::Repeatable;
		DefaultConfig.SensorTag = FName("Default");
		DefaultConfig.InteractionRange = DefaultInteractionRange;
		DefaultConfig.MinimumSenseScore = 0.5f;
		DefaultConfig.CooldownTime = 1.0f;
		DefaultConfig.bEnabled = true;

		AddInteractionConfig(DefaultConfig);
	}
}

bool UHarmoniaSenseInteractableComponent::ProcessInteraction(
	AActor* Interactor,
	FSenseInteractionData& InteractionData,
	FHarmoniaInteractionResult& OutResult)
{
	if (!Interactor)
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Invalid interactor");
		return false;
	}

	// Validate interaction conditions
	if (!ValidateInteractionConditions(Interactor, InteractionData))
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Interaction conditions not met");
		return false;
	}

	// Check if interaction is available (cooldown, one-time, etc.)
	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	if (!InteractionData.IsAvailable(CurrentTime))
	{
		OutResult.bSuccess = false;
		OutResult.Message = TEXT("Interaction not available (cooldown or already completed)");
		return false;
	}

	// Process the interaction
	UpdateInteractionState(InteractionData);

	// Success!
	OutResult.bSuccess = true;
	OutResult.Message = FString::Printf(
		TEXT("Interaction successful: %s"),
		*InteractionData.Config.InteractionPrompt.ToString());

	return true;
}

bool UHarmoniaSenseInteractableComponent::ValidateInteractionConditions(
	AActor* Interactor,
	const FSenseInteractionData& InteractionData) const
{
	if (!InteractionData.Config.bEnabled)
	{
		return false;
	}

	// Check line of sight if required
	if (InteractionData.Config.bRequiresLineOfSight)
	{
		const UWorld* World = GetWorld();
		if (World)
		{
			const FVector Start = Interactor->GetActorLocation();
			const FVector End = GetOwner()->GetActorLocation();

			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Interactor);
			QueryParams.AddIgnoredActor(GetOwner());

			if (World->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECC_Visibility,
				QueryParams))
			{
				// Something is blocking line of sight
				return false;
			}
		}
	}

	// Check interaction range for proximity-based interactions
	if (InteractionData.Config.TriggerType == ESenseInteractionTriggerType::OnProximityEnter ||
		InteractionData.Config.TriggerType == ESenseInteractionTriggerType::OnProximityStay)
	{
		const float Distance = FVector::Dist(
			Interactor->GetActorLocation(),
			GetOwner()->GetActorLocation());

		if (Distance > InteractionData.Config.InteractionRange)
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaSenseInteractableComponent::UpdateInteractionState(FSenseInteractionData& InteractionData)
{
	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	InteractionData.LastInteractionTime = CurrentTime;
	InteractionData.InteractionCount++;

	// Mark as completed if it's a one-time interaction
	if (InteractionData.Config.InteractionMode == ESenseInteractionMode::OneTime)
	{
		InteractionData.bCompleted = true;
	}
}
