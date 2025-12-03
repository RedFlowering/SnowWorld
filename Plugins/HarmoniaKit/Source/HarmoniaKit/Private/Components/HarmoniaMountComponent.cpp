// Copyright RedFlowering. All Rights Reserved.

#include "Components/HarmoniaMountComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "HarmoniaGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"

UHarmoniaMountComponent::UHarmoniaMountComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Initialize state
	CurrentMountType = EHarmoniaMountType::None;
	bIsMounted = false;
	bIsMounting = false;
	bIsDismounting = false;
	bIsSprinting = false;
	bIsFlying = false;
	CurrentMountHealth = 0.0f;
	MountMeshComponent = nullptr;
	MountingStartTime = 0.0f;
	DismountingStartTime = 0.0f;
}

void UHarmoniaMountComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeReferences();
}

void UHarmoniaMountComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsMounted && bIsSprinting && AbilitySystemComponent && AttributeSet)
	{
		// Consume stamina while sprinting on mount
		FHarmoniaMountData MountData;
		if (GetCurrentMountData(MountData))
		{
			float StaminaCost = MountData.SprintStaminaCostPerSecond * DeltaTime;

			// Check if we have enough stamina
			float CurrentStamina = AttributeSet->GetStamina();
			if (CurrentStamina <= 0.0f)
			{
				// Stop sprinting if out of stamina
				StopSprint();
			}
		}
	}
	else if (bIsMounted && !bIsSprinting && AbilitySystemComponent && AttributeSet)
	{
		// Consume stamina while mounted (less than sprinting)
		FHarmoniaMountData MountData;
		if (GetCurrentMountData(MountData))
		{
			float StaminaCost = MountData.StaminaCostPerSecond * DeltaTime;
			// Stamina cost is handled by gameplay effects
		}
	}
}

void UHarmoniaMountComponent::InitializeReferences()
{
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (OwnerCharacter)
	{
		AbilitySystemComponent = OwnerCharacter->FindComponentByClass<UAbilitySystemComponent>();

		if (AbilitySystemComponent)
		{
			AttributeSet = const_cast<UHarmoniaAttributeSet*>(
				AbilitySystemComponent->GetSet<UHarmoniaAttributeSet>()
			);
		}
	}
}

// ============================================================================
// Mount Management
// ============================================================================

void UHarmoniaMountComponent::SetCurrentMountType(EHarmoniaMountType NewMountType)
{
	CurrentMountType = NewMountType;
}

bool UHarmoniaMountComponent::GetCurrentMountData(FHarmoniaMountData& OutMountData) const
{
	return GetMountDataForType(CurrentMountType, OutMountData);
}

bool UHarmoniaMountComponent::GetMountDataForType(EHarmoniaMountType MountType, FHarmoniaMountData& OutMountData) const
{
	if (!MountDataTable)
	{
		return false;
	}

	// Try to find by enum value
	FString ContextString;
	FName RowName = FName(*UEnum::GetValueAsString(MountType));

	FHarmoniaMountData* FoundData = MountDataTable->FindRow<FHarmoniaMountData>(RowName, ContextString);

	if (FoundData)
	{
		OutMountData = *FoundData;
		return true;
	}

	return false;
}

// ============================================================================
// Mount State
// ============================================================================

bool UHarmoniaMountComponent::CanMount() const
{
	if (bIsMounted || bIsMounting || bIsDismounting)
	{
		return false;
	}

	if (!OwnerCharacter || !AbilitySystemComponent)
	{
		return false;
	}

	// Check if in combat (if mount data says we can't mount in combat)
	FHarmoniaMountData MountData;
	if (GetCurrentMountData(MountData))
	{
		if (!MountData.bCanMountInCombat)
		{
			// Check for combat state tags
			if (AbilitySystemComponent->HasMatchingGameplayTag(HarmoniaGameplayTags::State_Combat_Attacking))
			{
				return false;
			}
		}
	}

	// Check for movement restrictions
	if (AbilitySystemComponent->HasMatchingGameplayTag(HarmoniaGameplayTags::Movement_Restricted_NoMount))
	{
		return false;
	}

	return true;
}

bool UHarmoniaMountComponent::CanDismount() const
{
	if (!bIsMounted || bIsMounting || bIsDismounting)
	{
		return false;
	}

	return true;
}

// ============================================================================
// Mount Actions
// ============================================================================

bool UHarmoniaMountComponent::StartMount(EHarmoniaMountType MountType)
{
	if (!CanMount())
	{
		return false;
	}

	FHarmoniaMountData MountData;
	if (!GetMountDataForType(MountType, MountData))
	{
		return false;
	}

	// Set mount type
	CurrentMountType = MountType;
	bIsMounting = true;
	MountingStartTime = GetWorld()->GetTimeSeconds();

	// Apply mounting tag
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(HarmoniaGameplayTags::State_Mounting);
	}

	// Set mount health
	CurrentMountHealth = MountData.MountHealth;

	// Spawn mount mesh
	SpawnMountMesh(MountData);

	return true;
}

bool UHarmoniaMountComponent::StartDismount()
{
	if (!CanDismount())
	{
		return false;
	}

	FHarmoniaMountData MountData;
	if (!GetCurrentMountData(MountData))
	{
		return false;
	}

	bIsDismounting = true;
	DismountingStartTime = GetWorld()->GetTimeSeconds();

	// Remove mounted tags
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Mounted);
		AbilitySystemComponent->AddLooseGameplayTag(HarmoniaGameplayTags::State_Dismounting);
	}

	// Stop sprint if sprinting
	if (bIsSprinting)
	{
		StopSprint();
	}

	// Stop flying if flying
	if (bIsFlying)
	{
		StopFlying();
	}

	return true;
}

void UHarmoniaMountComponent::CompleteMounting()
{
	bIsMounting = false;
	bIsMounted = true;

	FHarmoniaMountData MountData;
	if (GetCurrentMountData(MountData))
	{
		// Apply mounted tags
		ApplyMountedTags(MountData);

		// Update movement speed
		UpdateMovementSpeed();

		// Remove mounting tag
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Mounting);
			AbilitySystemComponent->AddLooseGameplayTag(HarmoniaGameplayTags::State_Mounted);
		}

		// Send gameplay event
		if (AbilitySystemComponent)
		{
			FGameplayEventData EventData;
			AbilitySystemComponent->HandleGameplayEvent(
				HarmoniaGameplayTags::GameplayEvent_Mount_Mounted,
				&EventData
			);
		}

		// Broadcast event
		OnMounted.Broadcast(CurrentMountType);
	}
}

void UHarmoniaMountComponent::CompleteDismounting()
{
	FHarmoniaMountData MountData;
	if (GetCurrentMountData(MountData))
	{
		// Remove mounted tags
		RemoveMountedTags(MountData);
	}

	bIsDismounting = false;
	bIsMounted = false;

	// Remove mount mesh
	RemoveMountMesh();

	// Reset mount type
	CurrentMountType = EHarmoniaMountType::None;

	// Remove dismounting tag
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Dismounting);
	}

	// Restore normal movement speed
	UpdateMovementSpeed();

	// Send gameplay event
	if (AbilitySystemComponent)
	{
		FGameplayEventData EventData;
		AbilitySystemComponent->HandleGameplayEvent(
			HarmoniaGameplayTags::GameplayEvent_Mount_Dismounted,
			&EventData
		);
	}

	// Broadcast event
	OnDismounted.Broadcast();
}

void UHarmoniaMountComponent::StartSprint()
{
	if (!bIsMounted || bIsSprinting)
	{
		return;
	}

	FHarmoniaMountData MountData;
	if (!GetCurrentMountData(MountData))
	{
		return;
	}

	// Check stamina
	if (AttributeSet)
	{
		float CurrentStamina = AttributeSet->GetStamina();
		if (CurrentStamina <= 0.0f)
		{
			return;
		}
	}

	bIsSprinting = true;

	// Apply sprinting tag
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(HarmoniaGameplayTags::State_Mount_Sprinting);
	}

	// Update movement speed
	UpdateMovementSpeed();
}

void UHarmoniaMountComponent::StopSprint()
{
	if (!bIsSprinting)
	{
		return;
	}

	bIsSprinting = false;

	// Remove sprinting tag
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Mount_Sprinting);
	}

	// Update movement speed
	UpdateMovementSpeed();
}

void UHarmoniaMountComponent::StartFlying()
{
	if (!bIsMounted || bIsFlying)
	{
		return;
	}

	FHarmoniaMountData MountData;
	if (!GetCurrentMountData(MountData))
	{
		return;
	}

	// Check if mount can fly
	if (MountData.MovementType != EHarmoniaMountMovementType::Flying &&
		MountData.MovementType != EHarmoniaMountMovementType::Versatile)
	{
		return;
	}

	bIsFlying = true;

	// Apply flying tag
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(HarmoniaGameplayTags::State_Mount_Flying);
	}

	// Change movement mode to flying
	if (OwnerCharacter)
	{
		UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
		if (MovementComponent)
		{
			MovementComponent->SetMovementMode(MOVE_Flying);
		}
	}

	// Update movement speed
	UpdateMovementSpeed();
}

void UHarmoniaMountComponent::StopFlying()
{
	if (!bIsFlying)
	{
		return;
	}

	bIsFlying = false;

	// Remove flying tag
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Mount_Flying);
	}

	// Change movement mode to walking
	if (OwnerCharacter)
	{
		UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
		if (MovementComponent)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}

	// Update movement speed
	UpdateMovementSpeed();
}

// ============================================================================
// Mount Combat
// ============================================================================

bool UHarmoniaMountComponent::CanAttackWhileMounted(FGameplayTag WeaponTag) const
{
	if (!bIsMounted)
	{
		return false;
	}

	FHarmoniaMountData MountData;
	if (!GetCurrentMountData(MountData))
	{
		return false;
	}

	if (!MountData.bCanAttackWhileMounted)
	{
		return false;
	}

	// Check if weapon type is allowed
	if (MountData.AllowedWeaponTags.Num() > 0)
	{
		return MountData.AllowedWeaponTags.HasTag(WeaponTag);
	}

	return true;
}

bool UHarmoniaMountComponent::CanUseAbilitiesWhileMounted() const
{
	if (!bIsMounted)
	{
		return false;
	}

	FHarmoniaMountData MountData;
	if (!GetCurrentMountData(MountData))
	{
		return false;
	}

	return MountData.bCanUseAbilitiesWhileMounted;
}

bool UHarmoniaMountComponent::ExecuteChargeAttack()
{
	if (!bIsMounted || !AbilitySystemComponent)
	{
		return false;
	}

	// Check if moving fast enough
	if (OwnerCharacter)
	{
		UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
		if (MovementComponent)
		{
			float CurrentSpeed = MovementComponent->Velocity.Size();
			if (CurrentSpeed < CurrentMountCombatData.ChargeSpeedThreshold)
			{
				return false;
			}
		}
	}

	// Check stamina
	if (AttributeSet)
	{
		float CurrentStamina = AttributeSet->GetStamina();
		if (CurrentStamina < CurrentMountCombatData.ChargeStaminaCost)
		{
			return false;
		}
	}

	// Send gameplay event
	FGameplayEventData EventData;
	EventData.EventMagnitude = CurrentMountCombatData.ChargeAttackDamage;
	AbilitySystemComponent->HandleGameplayEvent(
		HarmoniaGameplayTags::GameplayEvent_Mount_ChargeAttack,
		&EventData
	);

	return true;
}

bool UHarmoniaMountComponent::ExecuteTrample()
{
	if (!bIsMounted || !AbilitySystemComponent)
	{
		return false;
	}

	// Send gameplay event
	FGameplayEventData EventData;
	EventData.EventMagnitude = CurrentMountCombatData.TrampleDamage;
	AbilitySystemComponent->HandleGameplayEvent(
		HarmoniaGameplayTags::GameplayEvent_Mount_Trample,
		&EventData
	);

	return true;
}

bool UHarmoniaMountComponent::ExecuteAerialStrike()
{
	if (!bIsMounted || !bIsFlying || !AbilitySystemComponent)
	{
		return false;
	}

	// Send gameplay event
	FGameplayEventData EventData;
	EventData.EventMagnitude = CurrentMountCombatData.AerialStrikeDamage;
	AbilitySystemComponent->HandleGameplayEvent(
		HarmoniaGameplayTags::GameplayEvent_Mount_AerialStrike,
		&EventData
	);

	return true;
}

// ============================================================================
// Mount Stats
// ============================================================================

float UHarmoniaMountComponent::GetCurrentSpeedMultiplier() const
{
	if (!bIsMounted)
	{
		return 1.0f;
	}

	FHarmoniaMountData MountData;
	if (!GetCurrentMountData(MountData))
	{
		return 1.0f;
	}

	float Multiplier = MountData.SpeedMultiplier;

	if (bIsSprinting)
	{
		Multiplier *= MountData.SprintMultiplier;
	}

	if (bIsFlying)
	{
		Multiplier = MountData.FlyingSpeedMultiplier;
	}

	return Multiplier;
}

void UHarmoniaMountComponent::SetMountHealth(float NewHealth)
{
	float OldHealth = CurrentMountHealth;
	CurrentMountHealth = FMath::Clamp(NewHealth, 0.0f, 999999.0f);

	FHarmoniaMountData MountData;
	if (GetCurrentMountData(MountData))
	{
		OnMountHealthChanged.Broadcast(CurrentMountHealth, MountData.MountHealth);

		if (CurrentMountHealth <= 0.0f && OldHealth > 0.0f)
		{
			// Mount died
			OnMountDied.Broadcast();

			// Force dismount
			if (bIsMounted)
			{
				StartDismount();
				CompleteDismounting();
			}
		}
	}
}

void UHarmoniaMountComponent::DamageMountHealth(float Damage)
{
	SetMountHealth(CurrentMountHealth - Damage);
}

void UHarmoniaMountComponent::HealMountHealth(float HealAmount)
{
	SetMountHealth(CurrentMountHealth + HealAmount);
}

// ============================================================================
// Helper Functions
// ============================================================================

void UHarmoniaMountComponent::UpdateMovementSpeed()
{
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	float BaseSpeed = 600.0f; // Default base speed
	float Multiplier = GetCurrentSpeedMultiplier();

	MovementComponent->MaxWalkSpeed = BaseSpeed * Multiplier;

	if (bIsFlying)
	{
		MovementComponent->MaxFlySpeed = BaseSpeed * Multiplier;
	}
}

void UHarmoniaMountComponent::SpawnMountMesh(const FHarmoniaMountData& MountData)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Remove existing mount mesh if any
	RemoveMountMesh();

	// Create new skeletal mesh component for mount
	MountMeshComponent = NewObject<USkeletalMeshComponent>(OwnerCharacter);
	if (MountMeshComponent)
	{
		MountMeshComponent->RegisterComponent();
		MountMeshComponent->AttachToComponent(
			OwnerCharacter->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetIncludingScale
		);

		// Load and set mount mesh
		if (!MountData.MountMesh.IsNull())
		{
			USkeletalMesh* LoadedMesh = MountData.MountMesh.LoadSynchronous();
			if (LoadedMesh)
			{
				MountMeshComponent->SetSkeletalMesh(LoadedMesh);
			}
		}

		// Load and set animation blueprint
		if (!MountData.AnimationBlueprint.IsNull())
		{
			TSubclassOf<UAnimInstance> AnimClass = MountData.AnimationBlueprint.LoadSynchronous();
			if (AnimClass)
			{
				MountMeshComponent->SetAnimInstanceClass(AnimClass);
			}
		}
	}
}

void UHarmoniaMountComponent::RemoveMountMesh()
{
	if (MountMeshComponent)
	{
		MountMeshComponent->DestroyComponent();
		MountMeshComponent = nullptr;
	}
}

void UHarmoniaMountComponent::ApplyMountedTags(const FHarmoniaMountData& MountData)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Apply mounted tags from mount data
	for (const FGameplayTag& Tag : MountData.MountedTags)
	{
		AbilitySystemComponent->AddLooseGameplayTag(Tag);
	}
}

void UHarmoniaMountComponent::RemoveMountedTags(const FHarmoniaMountData& MountData)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Remove mounted tags from mount data
	for (const FGameplayTag& Tag : MountData.MountedTags)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(Tag);
	}
}
