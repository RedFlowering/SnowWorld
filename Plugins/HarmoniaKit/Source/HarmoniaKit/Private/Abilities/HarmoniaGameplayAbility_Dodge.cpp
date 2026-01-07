// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Dodge.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_Dodge::UHarmoniaGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UHarmoniaGameplayAbility_Dodge::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
		{
			FGameplayTagContainer OwnedTags;
			ActorInfo->AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
			
			UE_LOG(LogTemp, Warning, TEXT("[Dodge] CanActivateAbility FAILED: Super::CanActivateAbility returned false"));
			UE_LOG(LogTemp, Warning, TEXT("[Dodge] ASC Owned Tags: %s"), *OwnedTags.ToStringSimple());
			UE_LOG(LogTemp, Warning, TEXT("[Dodge] Ability ActivationRequiredTags: %s"), *ActivationRequiredTags.ToStringSimple());
			UE_LOG(LogTemp, Warning, TEXT("[Dodge] Ability ActivationBlockedTags: %s"), *ActivationBlockedTags.ToStringSimple());
			
			if (OptionalRelevantTags)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Dodge] FailureTags: %s"), *OptionalRelevantTags->ToStringSimple());
			}
		}
		return false;
	}

	UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent();
	if (!MeleeComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dodge] CanActivateAbility FAILED: MeleeCombatComponent not found"));
		return false;
	}

	if (!MeleeComp->CanDodge())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dodge] CanActivateAbility FAILED: MeleeCombatComponent::CanDodge() returned false"));
		return false;
	}

	float Distance, Duration, IFrameStart, IFrameDuration, StaminaCost;
	GetDodgeParameters(Distance, Duration, IFrameStart, IFrameDuration, StaminaCost);

	if (!MeleeComp->HasEnoughStamina(StaminaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dodge] CanActivateAbility FAILED: Not enough stamina (Required=%.1f)"), StaminaCost);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Dodge] CanActivateAbility SUCCESS"));
	return true;
}

void UHarmoniaGameplayAbility_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("[Dodge] ActivateAbility: Starting"));

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dodge] ActivateAbility FAILED: CommitAbility returned false"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MeleeCombatComponent = GetMeleeCombatComponent();
	CachedCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	// Get dodge parameters based on equipment load
	GetDodgeParameters(
		CurrentDodgeDistance,
		CurrentDodgeDuration,
		CurrentIFrameStartTime,
		CurrentIFrameDuration,
		CurrentStaminaCost
	);

	// Consume stamina
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->ConsumeStamina(CurrentStaminaCost);
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::Dodging);
	}

	// Calculate dodge direction based on player input
	ACharacter* Character = CachedCharacter.Get();
	if (Character)
	{
		CurrentDodgeDirection = CalculateDodgeDirection(Character);
		CurrentDodgeVelocity = CalculateDodgeVelocity(Character, CurrentDodgeDirection);

		UE_LOG(LogTemp, Log, TEXT("[Dodge] Direction: X=%.2f, Y=%.2f, Velocity: %s"),
			CurrentDodgeDirection.X, CurrentDodgeDirection.Y, *CurrentDodgeVelocity.ToCompactString());

		// Set dodge direction via combat component (for AnimInstance if needed)
		if (MeleeCombatComponent)
		{
			MeleeCombatComponent->SetDodgeDirection(CurrentDodgeDirection.X, CurrentDodgeDirection.Y);
		}

		// Setup Motion Warping target for directional dodge
		if (bUseMotionWarping)
		{
			SetupMotionWarpingTarget(Character, CurrentDodgeDirection);
		}
		else
		{
			// If not using Motion Warping, apply velocity directly
			ApplyDodgeVelocity(Character, CurrentDodgeVelocity);
		}
	}

	// Start i-frames timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			IFrameStartTimerHandle,
			this,
			&UHarmoniaGameplayAbility_Dodge::StartIFrames,
			CurrentIFrameStartTime,
			false
		);

		// Set timer to end dodge movement
		World->GetTimerManager().SetTimer(
			DodgeEndTimerHandle,
			this,
			&UHarmoniaGameplayAbility_Dodge::OnDodgeMovementEnd,
			CurrentDodgeDuration,
			false
		);
	}

	// Play dodge montage with Motion Warping
	if (DodgeMontage)
	{
		UE_LOG(LogTemp, Log, TEXT("[Dodge] Playing Montage with Motion Warping: %s"), *GetNameSafe(DodgeMontage));
		
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			DodgeMontage,
			1.0f,
			NAME_None,
			false,
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_Dodge::OnMontageCompleted);
			MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_Dodge::OnMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_Dodge::OnMontageInterrupted);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dodge] No DodgeMontage set!"));
	}
}

void UHarmoniaGameplayAbility_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogTemp, Log, TEXT("[Dodge] EndAbility: Cancelled=%d"), bWasCancelled);

	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IFrameStartTimerHandle);
		World->GetTimerManager().ClearTimer(IFrameEndTimerHandle);
		World->GetTimerManager().ClearTimer(DodgeMovementTimerHandle);
		World->GetTimerManager().ClearTimer(DodgeEndTimerHandle);
	}

	// Clear dodge velocity
	ClearDodgeVelocity();

	// Clear dodge direction via combat component
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->ClearDodgeDirection();
	}

	// End i-frames
	if (MeleeCombatComponent && MeleeCombatComponent->IsInvulnerable())
	{
		MeleeCombatComponent->SetInvulnerable(false);
	}

	// Reset defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
	}

	CachedCharacter.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ============================================================================
// Direction Calculation
// ============================================================================

FVector2D UHarmoniaGameplayAbility_Dodge::CalculateDodgeDirection(const ACharacter* Character) const
{
	if (!Character)
	{
		return FVector2D(0.0f, 1.0f); // Default forward
	}

	const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return FVector2D(0.0f, 1.0f);
	}

	// Get input vector in world space
	FVector InputVector = MovementComp->GetLastInputVector();
	
	// If no input, default to forward
	if (InputVector.IsNearlyZero())
	{
		return FVector2D(0.0f, 1.0f);
	}

	// Convert world input to local space relative to character
	const FVector ActorForward = Character->GetActorForwardVector();
	const FVector ActorRight = Character->GetActorRightVector();

	// Dot product to get local direction
	const float ForwardDot = FVector::DotProduct(InputVector.GetSafeNormal(), ActorForward);
	const float RightDot = FVector::DotProduct(InputVector.GetSafeNormal(), ActorRight);

	// Return normalized 2D direction (X = Right, Y = Forward)
	FVector2D LocalDir(RightDot, ForwardDot);
	LocalDir.Normalize();
	
	return LocalDir;
}

FVector UHarmoniaGameplayAbility_Dodge::CalculateDodgeVelocity(const ACharacter* Character, const FVector2D& LocalDirection) const
{
	if (!Character)
	{
		return FVector::ZeroVector;
	}

	const FVector ActorForward = Character->GetActorForwardVector();
	const FVector ActorRight = Character->GetActorRightVector();

	// Convert local 2D direction to world space velocity
	FVector WorldDirection = (ActorForward * LocalDirection.Y) + (ActorRight * LocalDirection.X);
	WorldDirection.Z = 0.0f;
	WorldDirection.Normalize();

	// Calculate velocity magnitude (distance / duration)
	const float Speed = CurrentDodgeDistance / CurrentDodgeDuration;

	return WorldDirection * Speed;
}

// ============================================================================
// Velocity Application
// ============================================================================

void UHarmoniaGameplayAbility_Dodge::ApplyDodgeVelocity(ACharacter* Character, const FVector& Velocity)
{
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Set velocity directly instead of using AddImpulse
	MovementComp->Velocity = Velocity;
	
	// Optionally set to falling mode if we want air control disabled
	// MovementComp->SetMovementMode(MOVE_Falling);
}

void UHarmoniaGameplayAbility_Dodge::ClearDodgeVelocity()
{
	ACharacter* Character = CachedCharacter.Get();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		// Don't completely zero velocity - let physics take over
		// Just ensure we're back in normal movement mode
		if (MovementComp->MovementMode == MOVE_Falling)
		{
			// Will naturally transition to walking when landing
		}
	}
}

// ============================================================================
// Motion Warping
// ============================================================================

void UHarmoniaGameplayAbility_Dodge::SetupMotionWarpingTarget(ACharacter* Character, const FVector2D& Direction)
{
	if (!Character)
	{
		return;
	}

	UMotionWarpingComponent* MotionWarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>();
	if (!MotionWarpingComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Dodge] MotionWarpingComponent not found - falling back to velocity-based dodge"));
		ApplyDodgeVelocity(Character, CurrentDodgeVelocity);
		return;
	}

	// Convert local direction (X=Right, Y=Forward) to world direction
	const FVector ActorForward = Character->GetActorForwardVector();
	const FVector ActorRight = Character->GetActorRightVector();
	FVector WorldDirection = (ActorForward * Direction.Y) + (ActorRight * Direction.X);
	WorldDirection.Z = 0.0f;
	WorldDirection.Normalize();

	// Calculate target location: CurrentPosition + (Direction * Distance)
	const FVector CurrentLocation = Character->GetActorLocation();
	const FVector TargetLocation = CurrentLocation + (WorldDirection * CurrentDodgeDistance);

	// Calculate target rotation (face the dodge direction)
	const FRotator TargetRotation = WorldDirection.Rotation();

	// Add or update motion warping target
	MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
		DodgeWarpTargetName,
		TargetLocation,
		TargetRotation
	);

	UE_LOG(LogTemp, Log, TEXT("[Dodge] Motion Warping target set: %s -> %s (Distance: %.0f)"),
		*DodgeWarpTargetName.ToString(), *TargetLocation.ToCompactString(), CurrentDodgeDistance);
}

// ============================================================================
// Dodge Movement
// ============================================================================

void UHarmoniaGameplayAbility_Dodge::OnDodgeMovementTick()
{
	// Optional: Could implement easing/deceleration here
	// For now, velocity is maintained by movement component
}

void UHarmoniaGameplayAbility_Dodge::OnDodgeMovementEnd()
{
	UE_LOG(LogTemp, Log, TEXT("[Dodge] Movement ended - ability still active until animation completes"));
	
	// Clear velocity but don't end ability yet (wait for montage callbacks)
	ClearDodgeVelocity();
	
	// Note: Ability ends via montage callbacks (OnMontageCompleted/Cancelled/Interrupted)
}

// ============================================================================
// I-Frames
// ============================================================================

void UHarmoniaGameplayAbility_Dodge::StartIFrames()
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetInvulnerable(true);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				IFrameEndTimerHandle,
				this,
				&UHarmoniaGameplayAbility_Dodge::EndIFrames,
				CurrentIFrameDuration,
				false
			);
		}
	}
}

void UHarmoniaGameplayAbility_Dodge::EndIFrames()
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetInvulnerable(false);
	}
}

// ============================================================================
// Animation Callbacks
// ============================================================================

void UHarmoniaGameplayAbility_Dodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Dodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_Dodge::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

// ============================================================================
// Utility
// ============================================================================

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Dodge::GetMeleeCombatComponent() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent;
	}

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Avatar = ActorInfo->AvatarActor.Get())
		{
			return Avatar->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
		}
	}

	return nullptr;
}

EDodgeRollType UHarmoniaGameplayAbility_Dodge::DetermineRollType() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return EDodgeRollType::Medium;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	float EquipLoad = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetEquipLoadAttribute());
	float MaxEquipLoad = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetMaxEquipLoadAttribute());

	if (MaxEquipLoad <= 0.f)
	{
		return EDodgeRollType::Light;
	}

	float LoadRatio = EquipLoad / MaxEquipLoad;

	if (LoadRatio <= 0.3f)
	{
		return EDodgeRollType::Light;
	}
	else if (LoadRatio <= 0.7f)
	{
		return EDodgeRollType::Medium;
	}
	else
	{
		return EDodgeRollType::Heavy;
	}
}

void UHarmoniaGameplayAbility_Dodge::GetDodgeParameters(
	float& OutDistance,
	float& OutDuration,
	float& OutIFrameStart,
	float& OutIFrameDuration,
	float& OutStaminaCost) const
{
	EDodgeRollType RollType = DetermineRollType();

	switch (RollType)
	{
	case EDodgeRollType::Light:
		OutDistance = LightRollDistance;
		OutDuration = LightRollDuration;
		OutIFrameStart = LightRollIFrameStartTime;
		OutIFrameDuration = LightRollIFrameDuration;
		OutStaminaCost = LightRollStaminaCost;
		break;

	case EDodgeRollType::Medium:
		OutDistance = MediumRollDistance;
		OutDuration = MediumRollDuration;
		OutIFrameStart = MediumRollIFrameStartTime;
		OutIFrameDuration = MediumRollIFrameDuration;
		OutStaminaCost = MediumRollStaminaCost;
		break;

	case EDodgeRollType::Heavy:
		OutDistance = HeavyRollDistance;
		OutDuration = HeavyRollDuration;
		OutIFrameStart = HeavyRollIFrameStartTime;
		OutIFrameDuration = HeavyRollIFrameDuration;
		OutStaminaCost = HeavyRollStaminaCost;
		break;

	default:
		OutDistance = MediumRollDistance;
		OutDuration = MediumRollDuration;
		OutIFrameStart = MediumRollIFrameStartTime;
		OutIFrameDuration = MediumRollIFrameDuration;
		OutStaminaCost = MediumRollStaminaCost;
		break;
	}
}
