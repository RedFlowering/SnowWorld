// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSenseInteractionComponent.h"
#include "Components/HarmoniaSenseInteractableComponent.h"
#include "Managers/HarmoniaInteractionManager.h"
#include "SenseReceiverComponent.h"
#include "SenseStimulusComponent.h"
#include "Sensors/SensorBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UHarmoniaSenseInteractionComponent::UHarmoniaSenseInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Default sensor tags to monitor
	MonitoredSensorTags.Add(FName("Proximity"));
	MonitoredSensorTags.Add(FName("Vision"));
	MonitoredSensorTags.Add(FName("Hearing"));
}

void UHarmoniaSenseInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind to sense receiver delegates (this component IS a SenseReceiverComponent)
	OnNewSense.AddDynamic(this, &UHarmoniaSenseInteractionComponent::OnNewSenseDetected);
	OnLostSense.AddDynamic(this, &UHarmoniaSenseInteractionComponent::OnSenseLost);
	OnCurrentSense.AddDynamic(this, &UHarmoniaSenseInteractionComponent::OnSenseUpdated);

	SetupInput();
}

void UHarmoniaSenseInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupInput();

	// Unbind from sense receiver delegates
	OnNewSense.RemoveDynamic(this, &UHarmoniaSenseInteractionComponent::OnNewSenseDetected);
	OnLostSense.RemoveDynamic(this, &UHarmoniaSenseInteractionComponent::OnSenseLost);
	OnCurrentSense.RemoveDynamic(this, &UHarmoniaSenseInteractionComponent::OnSenseUpdated);

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaSenseInteractionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update target priorities
	UpdateTargetPriorities();

	// Process automatic interactions
	if (bEnableAutomaticInteractions)
	{
		ProcessAutomaticInteractions(DeltaTime);
	}

	// Debug visualization
	if (bShowDebugInfo)
	{
		const AActor* Owner = GetOwner();
		if (Owner)
		{
			for (const FInteractableTargetInfo& Target : TrackedTargets)
			{
				if (Target.IsValid())
				{
					FColor LineColor = Target.IsInteractable() ? FColor::Green : FColor::Yellow;
					DrawDebugLine(
						GetWorld(),
						Owner->GetActorLocation(),
						Target.TargetActor->GetActorLocation(),
						LineColor,
						false,
						0.0f,
						0,
						2.0f);
				}
			}
		}
	}
}

// ============================================================================
// Target Management
// ============================================================================

FInteractableTargetInfo UHarmoniaSenseInteractionComponent::GetBestInteractionTarget() const
{
	if (BestTargetIndex != INDEX_NONE && TrackedTargets.IsValidIndex(BestTargetIndex))
	{
		return TrackedTargets[BestTargetIndex];
	}

	return FInteractableTargetInfo();
}

TArray<FInteractableTargetInfo> UHarmoniaSenseInteractionComponent::GetAllInteractableTargets() const
{
	return TrackedTargets;
}

TArray<FInteractableTargetInfo> UHarmoniaSenseInteractionComponent::GetInteractablesBySensorTag(FName SensorTag) const
{
	TArray<FInteractableTargetInfo> FilteredTargets;

	for (const FInteractableTargetInfo& Target : TrackedTargets)
	{
		if (Target.SensorTag == SensorTag)
		{
			FilteredTargets.Add(Target);
		}
	}

	return FilteredTargets;
}

bool UHarmoniaSenseInteractionComponent::HasInteractableTargets() const
{
	return TrackedTargets.Num() > 0;
}

// ============================================================================
// Interaction Functions
// ============================================================================

bool UHarmoniaSenseInteractionComponent::TryInteractWithBestTarget()
{
	const FInteractableTargetInfo BestTarget = GetBestInteractionTarget();

	if (BestTarget.IsValid())
	{
		return TryInteractWithTarget(BestTarget.InteractableComponent);
	}

	return false;
}

bool UHarmoniaSenseInteractionComponent::TryInteractWithTarget(UHarmoniaSenseInteractableComponent* Target)
{
	if (!Target)
	{
		return false;
	}

	// Find the sensor tag for this target
	FName SensorTag = NAME_None;
	for (const FInteractableTargetInfo& TargetInfo : TrackedTargets)
	{
		if (TargetInfo.InteractableComponent == Target)
		{
			SensorTag = TargetInfo.SensorTag;
			break;
		}
	}

	// Perform server interaction
	Server_TryInteract(Target, SensorTag);
	return true;
}

bool UHarmoniaSenseInteractionComponent::TryInteractWithSensor(FName SensorTag)
{
	const TArray<FInteractableTargetInfo> Targets = GetInteractablesBySensorTag(SensorTag);

	if (Targets.Num() > 0)
	{
		// Interact with the first (highest priority) target for this sensor
		return TryInteractWithTarget(Targets[0].InteractableComponent);
	}

	return false;
}

bool UHarmoniaSenseInteractionComponent::Server_TryInteract_Validate(
	UHarmoniaSenseInteractableComponent* Target,
	FName SensorTag)
{
	// Validate target exists
	if (!Target || !Target->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Server_TryInteract: Invalid target"));
		return false;
	}

	// Validate target owner exists
	AActor* TargetOwner = Target->GetOwner();
	if (!TargetOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Server_TryInteract: Target has no owner"));
		return false;
	}

	// Validate distance
	AActor* Interactor = GetOwner();
	if (Interactor)
	{
		float Distance = FVector::Dist(Interactor->GetActorLocation(), TargetOwner->GetActorLocation());
		const float MaxInteractionDistance = 1000.0f; // 10 meters

		if (Distance > MaxInteractionDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Server_TryInteract: Target too far (%.1f > %.1f)"),
				Distance, MaxInteractionDistance);
			return false;
		}
	}

	// Validate target is currently sensed by this component
	bool bTargetIsSensed = false;
	for (const FInteractableTargetInfo& TrackedTarget : TrackedTargets)
	{
		if (TrackedTarget.InteractableComponent == Target)
		{
			bTargetIsSensed = true;
			break;
		}
	}

	if (!bTargetIsSensed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Server_TryInteract: Target not in sensed list"));
		return false;
	}

	return true;
}

void UHarmoniaSenseInteractionComponent::Server_TryInteract_Implementation(
	UHarmoniaSenseInteractableComponent* Target,
	FName SensorTag)
{
	if (!Target)
	{
		return;
	}

	// Create interaction context
	FHarmoniaInteractionContext Context;
	Context.Interactor = GetOwner();
	Context.Interactable = Target->GetOwner();
	Context.InteractionType = EHarmoniaInteractionType::Custom;

	// Try to get specific interaction type from target
	FSenseInteractionData Data;

	if (Target->GetInteractionConfigBySensorTag(SensorTag, Data))
	{
		if (FSenseInteractionData* InteractionData = &Data)
		{
			Context.InteractionType = InteractionData->Config.InteractionType;
		}
	}

	// Perform interaction
	FHarmoniaInteractionResult Result;
	Target->OnInteract(Context, Result);

	// Broadcast result
	OnInteractionCompleted.Broadcast(Target, SensorTag, Result);

	// Send result to client
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			Client_OnInteractionResult(Result);
		}
	}
}

void UHarmoniaSenseInteractionComponent::Client_OnInteractionResult_Implementation(
	const FHarmoniaInteractionResult& Result)
{
	// Handle client-side interaction result
	// Could show UI feedback, play effects, etc.

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction Result: %s - %s"),
			Result.bSuccess ? TEXT("Success") : TEXT("Failed"),
			*Result.Message);
	}
}

// ============================================================================
// Protected Functions
// ============================================================================

void UHarmoniaSenseInteractionComponent::SetupInput()
{
	if (!InteractAction || !InputMapping)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	if (Subsystem)
	{
		Subsystem->AddMappingContext(InputMapping, InputMappingPriority);
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (EnhancedInput)
	{
		EnhancedInput->BindAction(
			InteractAction,
			ETriggerEvent::Started,
			this,
			&UHarmoniaSenseInteractionComponent::HandleInteractAction);
	}
}

void UHarmoniaSenseInteractionComponent::CleanupInput()
{
	if (!InputMapping)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	if (Subsystem)
	{
		Subsystem->RemoveMappingContext(InputMapping);
	}
}

void UHarmoniaSenseInteractionComponent::HandleInteractAction(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		TryInteractWithBestTarget();
	}
}

// ============================================================================
// Sense System Callbacks
// ============================================================================
void UHarmoniaSenseInteractionComponent::OnNewSenseDetected(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli)
{
	if (SensorPtr && MonitoredSensorTags.Contains(SensorPtr->SensorTag))
	{
		const FName SensorTag = SensorPtr->SensorTag;
		ProcessSensedStimuli(SensedStimuli, SensorTag, true);
	}
}

void UHarmoniaSenseInteractionComponent::OnSenseLost(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli)
{
	if (SensorPtr)
	{
		for (const FSensedStimulus& Stimulus : SensedStimuli)
		{
			if (Stimulus.StimulusComponent.IsValid())
			{
				if (UHarmoniaSenseInteractableComponent* Interactable = Cast<UHarmoniaSenseInteractableComponent>(Stimulus.StimulusComponent.Get()))
				{
					RemoveInteractableTarget(Interactable);
					OnInteractableLost.Broadcast(Interactable, SensorPtr->SensorTag);
				}
			}
		}
	}
}

void UHarmoniaSenseInteractionComponent::OnSenseUpdated(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli)
{
	if (SensorPtr && MonitoredSensorTags.Contains(SensorPtr->SensorTag))
	{
		const FName SensorTag = SensorPtr->SensorTag;
		ProcessSensedStimuli(SensedStimuli, SensorTag, false);
	}
}

// ============================================================================
// Target Processing
// ============================================================================
void UHarmoniaSenseInteractionComponent::ProcessSensedStimuli(const TArray<FSensedStimulus>& SensedStimuli, FName SensorTag, bool bIsNewSense)
{
	for (const FSensedStimulus& Stimulus : SensedStimuli)
	{
		if (!Stimulus.StimulusComponent.IsValid())
		{
			continue;
		}

		UHarmoniaSenseInteractableComponent* Interactable = nullptr;
		AActor* TargetActor = Stimulus.StimulusComponent->GetOwner();

		if (!TargetActor)
		{
			continue;
		}

		// Check if the stimulus component is a sense-based interactable
		Interactable = Cast<UHarmoniaSenseInteractableComponent>(Stimulus.StimulusComponent.Get());

		// If bInteractableOnly is true, skip non-interactable actors
		if (bInteractableOnly && !Interactable)
		{
			continue;
		}

		// Check if interactable is active (if it's an interactable)
		if (Interactable && !Interactable->bIsSenseActive)
		{
			continue;
		}

		// Add or update the target (interactable or general actor)
		AddInteractableTarget(Interactable, Stimulus, SensorTag);

		// Broadcast new sense event (only for interactables)
		if (bIsNewSense && Interactable)
		{
			OnInteractableSensed.Broadcast(Interactable, SensorTag);
		}
	}
}

void UHarmoniaSenseInteractionComponent::AddInteractableTarget(UHarmoniaSenseInteractableComponent* Interactable, const FSensedStimulus& Stimulus, FName SensorTag)
{
	// Get target actor (from Interactable if available, otherwise from Stimulus)
	AActor* TargetActor = Interactable ? Interactable->GetOwner() :
	                      (Stimulus.StimulusComponent.IsValid() ? Stimulus.StimulusComponent->GetOwner() : nullptr);

	if (!TargetActor)
	{
		return;
	}

	// Check if target already exists
	int32 ExistingIndex = INDEX_NONE;
	for (int32 i = 0; i < TrackedTargets.Num(); ++i)
	{
		if (TrackedTargets[i].TargetActor == TargetActor &&
			TrackedTargets[i].SensorTag == SensorTag)
		{
			ExistingIndex = i;
			break;
		}
	}

	// Calculate distance
	const AActor* Owner = GetOwner();
	const float Distance = Owner ? FVector::Dist(Owner->GetActorLocation(), TargetActor->GetActorLocation()) : 0.0f;

	// Update or add target
	FInteractableTargetInfo TargetInfo;
	TargetInfo.TargetActor = TargetActor;
	TargetInfo.InteractableComponent = Interactable;
	TargetInfo.StimulusData = Stimulus;
	TargetInfo.SensorTag = SensorTag;
	TargetInfo.Distance = Distance;
	TargetInfo.Priority = CalculateTargetPriority(TargetInfo);

	if (ExistingIndex != INDEX_NONE)
	{
		// Update existing target
		TrackedTargets[ExistingIndex] = TargetInfo;
	}
	else
	{
		// Add new target
		if (TrackedTargets.Num() >= MaxTrackedTargets)
		{
			// Remove lowest priority target
			int32 LowestPriorityIndex = 0;
			float LowestPriority = TrackedTargets[0].Priority;

			for (int32 i = 1; i < TrackedTargets.Num(); ++i)
			{
				if (TrackedTargets[i].Priority < LowestPriority)
				{
					LowestPriority = TrackedTargets[i].Priority;
					LowestPriorityIndex = i;
				}
			}

			if (TargetInfo.Priority > LowestPriority)
			{
				TrackedTargets[LowestPriorityIndex] = TargetInfo;
			}
		}
		else
		{
			TrackedTargets.Add(TargetInfo);
		}
	}
}

void UHarmoniaSenseInteractionComponent::RemoveInteractableTarget(UHarmoniaSenseInteractableComponent* Interactable)
{
	TrackedTargets.RemoveAll([Interactable](const FInteractableTargetInfo& Target)
	{
		return Target.InteractableComponent == Interactable;
	});
}

void UHarmoniaSenseInteractionComponent::UpdateTargetPriorities()
{
	// Update all target priorities and distances
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	for (FInteractableTargetInfo& Target : TrackedTargets)
	{
		if (Target.IsValid())
		{
			Target.Distance = FVector::Dist(Owner->GetActorLocation(), Target.TargetActor->GetActorLocation());
			Target.Priority = CalculateTargetPriority(Target);
		}
	}

	// Sort targets by priority
	TrackedTargets.Sort([](const FInteractableTargetInfo& A, const FInteractableTargetInfo& B)
	{
		return A.Priority > B.Priority;
	});

	// Update best target
	const int32 OldBestIndex = BestTargetIndex;
	BestTargetIndex = TrackedTargets.Num() > 0 ? 0 : INDEX_NONE;

	// Check if best target changed
	UHarmoniaSenseInteractableComponent* NewBestTarget = nullptr;
	if (BestTargetIndex != INDEX_NONE && TrackedTargets.IsValidIndex(BestTargetIndex))
	{
		NewBestTarget = TrackedTargets[BestTargetIndex].InteractableComponent;
	}

	if (NewBestTarget != LastBestTarget.Get())
	{
		OnBestTargetChanged.Broadcast(NewBestTarget, LastBestTarget.Get());
		LastBestTarget = NewBestTarget;
	}
}

float UHarmoniaSenseInteractionComponent::CalculateTargetPriority(const FInteractableTargetInfo& TargetInfo) const
{
	float Priority = 0.0f;

	// Base priority from sense score
	Priority += TargetInfo.StimulusData.Score * 100.0f;

	// Distance priority (closer = higher priority)
	if (bPrioritizeByDistance && TargetInfo.Distance > 0.0f)
	{
		Priority += (1000.0f / FMath::Max(TargetInfo.Distance, 1.0f));
	}

	// Interaction availability priority (only for interactables)
	if (TargetInfo.IsInteractable())
	{
		if (TargetInfo.InteractableComponent->IsInteractionAvailable(TargetInfo.SensorTag))
		{
			Priority += 50.0f;
		}
	}

	return Priority;
}

void UHarmoniaSenseInteractionComponent::ProcessAutomaticInteractions(float DeltaTime)
{
	for (const FInteractableTargetInfo& TargetInfo : TrackedTargets)
	{
		if (!TargetInfo.IsValid() || !TargetInfo.IsInteractable())
		{
			continue;
		}

		// Get interaction config for this target
		FSenseInteractionData Data;
		FSenseInteractionData* InteractionData;

		if (TargetInfo.InteractableComponent->GetInteractionConfigBySensorTag(TargetInfo.SensorTag, Data))
		{
			InteractionData = &Data;

			if (InteractionData)
			{
				// Check if automatic interaction should trigger
				if (ShouldTriggerAutomaticInteraction(TargetInfo, InteractionData))
				{
					TryInteractWithTarget(TargetInfo.InteractableComponent);
				}
			}
		}
	}
}

bool UHarmoniaSenseInteractionComponent::ShouldTriggerAutomaticInteraction(const FInteractableTargetInfo& TargetInfo, const FSenseInteractionData* InteractionData) const
{
	if (!InteractionData || !InteractionData->Config.bEnabled)
	{
		return false;
	}

	// Check if trigger type is automatic
	if (InteractionData->Config.TriggerType != ESenseInteractionTriggerType::Automatic &&
		InteractionData->Config.TriggerType != ESenseInteractionTriggerType::OnProximityEnter)
	{
		return false;
	}

	// Check if interaction is available
	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;

	if (!InteractionData->IsAvailable(CurrentTime))
	{
		return false;
	}

	// Check sense score threshold
	if (TargetInfo.StimulusData.Score < InteractionData->Config.MinimumSenseScore)
	{
		return false;
	}

	return true;
}
