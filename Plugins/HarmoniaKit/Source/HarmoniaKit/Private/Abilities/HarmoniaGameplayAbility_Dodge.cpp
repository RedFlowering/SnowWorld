// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Dodge.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_Dodge::UHarmoniaGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Setup default tags
	DodgingTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dodging")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Blocking")));
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
		return false;
	}

	if (UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent())
	{
		if (!MeleeComp->CanDodge())
		{
			return false;
		}

		// Get stamina cost based on equipment load
		float Distance, Duration, IFrameStart, IFrameDuration, StaminaCost;
		GetDodgeParameters(Distance, Duration, IFrameStart, IFrameDuration, StaminaCost);

		// Check stamina
		if (!MeleeComp->HasEnoughStamina(StaminaCost))
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_Dodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MeleeCombatComponent = GetMeleeCombatComponent();

	// Get dodge parameters based on equipment load
	GetDodgeParameters(
		CurrentDodgeDistance,
		CurrentDodgeDuration,
		CurrentIFrameStartTime,
		CurrentIFrameDuration,
		CurrentStaminaCost
	);

	// Determine roll type for logging
	EDodgeRollType RollType = DetermineRollType();
	const TCHAR* RollTypeName =
		(RollType == EDodgeRollType::Light) ? TEXT("Light") :
		(RollType == EDodgeRollType::Medium) ? TEXT("Medium") : TEXT("Heavy");

	UE_LOG(LogTemp, Log, TEXT("Dodge: Roll type = %s (Distance=%.0f, Duration=%.2f, IFrame=%.2f, Stamina=%.0f)"),
		RollTypeName, CurrentDodgeDistance, CurrentDodgeDuration, CurrentIFrameDuration, CurrentStaminaCost);

	// Consume stamina
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->ConsumeStamina(CurrentStaminaCost);
	}

	// Apply dodging tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTags(DodgingTags);
	}

	// Set defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::Dodging);
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
	}

	// Play dodge animation
	if (DodgeMontage)
	{
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

	// Apply dodge movement
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			// Get dodge direction (based on input or forward)
			FVector DodgeDirection = Character->GetActorForwardVector();

			// Apply launch velocity
			const FVector LaunchVelocity = DodgeDirection * (CurrentDodgeDistance / CurrentDodgeDuration);
			MovementComp->AddImpulse(LaunchVelocity, true);
		}
	}
}

void UHarmoniaGameplayAbility_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IFrameStartTimerHandle);
		World->GetTimerManager().ClearTimer(IFrameEndTimerHandle);
	}

	// End i-frames
	if (MeleeCombatComponent && MeleeCombatComponent->IsInvulnerable())
	{
		MeleeCombatComponent->SetInvulnerable(false);
	}

	// Remove dodging tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTags(DodgingTags);
	}

	// Reset defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

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

void UHarmoniaGameplayAbility_Dodge::StartIFrames()
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetInvulnerable(true);

		// Set timer to end i-frames
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

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Dodge::GetMeleeCombatComponent() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent;
	}

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Owner = ActorInfo->OwnerActor.Get())
		{
			return Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
		}
	}

	return nullptr;
}

EDodgeRollType UHarmoniaGameplayAbility_Dodge::DetermineRollType() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return EDodgeRollType::Medium; // Default to medium
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	// Get EquipLoad and MaxEquipLoad attributes
	float EquipLoad = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetEquipLoadAttribute());
	float MaxEquipLoad = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetMaxEquipLoadAttribute());

	if (MaxEquipLoad <= 0.f)
	{
		return EDodgeRollType::Light; // No equipment = light roll
	}

	// Calculate load ratio
	float LoadRatio = EquipLoad / MaxEquipLoad;

	// Determine roll type based on load ratio
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
		// Fallback to medium
		OutDistance = MediumRollDistance;
		OutDuration = MediumRollDuration;
		OutIFrameStart = MediumRollIFrameStartTime;
		OutIFrameDuration = MediumRollIFrameDuration;
		OutStaminaCost = MediumRollStaminaCost;
		break;
	}
}
