// Copyright 2025 Snow Game Studio.

#include "Monsters/HarmoniaMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Animation/HarmoniaMonsterAnimationInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/HarmoniaSenseInteractionComponent.h"
#include "SenseStimulusComponent.h"
#include "Components/HarmoniaThreatComponent.h"
#include "SensedStimulStruct.h"
#include "HarmoniaItemPickup.h"
#include "Components/HarmoniaAdvancedAIComponent.h"
#include "Components/HarmoniaAILODComponent.h"
#include "System/HarmoniaTeamManagementSubsystem.h"

AHarmoniaMonsterBase::AHarmoniaMonsterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create attribute set
	AttributeSet = CreateDefaultSubobject<UHarmoniaAttributeSet>(TEXT("AttributeSet"));

	// Create Harmonia Sense Interaction Component for target detection
	// Configured for combat: tracks all actors, not just interactables
	SenseInteractionComponent = CreateDefaultSubobject<UHarmoniaSenseInteractionComponent>(TEXT("SenseInteractionComponent"));
	if (SenseInteractionComponent)
	{
		SenseInteractionComponent->bInteractableOnly = false;  // Track all actors for combat
		SenseInteractionComponent->bEnableAutomaticInteractions = false;  // Disable auto-interactions
		SenseInteractionComponent->bPrioritizeByDistance = true;  // Sort by distance
	}

	// Create Sense Stimulus Component for being detected
	SenseStimulusComponent = CreateDefaultSubobject<USenseStimulusComponent>(TEXT("SenseStimulusComponent"));

	// Create Threat Component for aggro management
	ThreatComponent = CreateDefaultSubobject<UHarmoniaThreatComponent>(TEXT("ThreatComponent"));

	// Create Advanced AI Component (optional, but created by default)
	AdvancedAIComponent = CreateDefaultSubobject<UHarmoniaAdvancedAIComponent>(TEXT("AdvancedAIComponent"));

	// Create AI LOD Component for performance optimization
	AILODComponent = CreateDefaultSubobject<UHarmoniaAILODComponent>(TEXT("AILODComponent"));

	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Network replication
	bReplicates = true;
	SetReplicateMovement(true);
}

void AHarmoniaMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize ability system on server
	if (HasAuthority())
	{
		InitializeAbilitySystem();

		// Initialize with monster data if provided
		if (MonsterData)
		{
			InitializeMonster(MonsterData, MonsterLevel);
		}
	}
}

void AHarmoniaMonsterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Cleanup timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void AHarmoniaMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update animation state
	if (bInitialized)
	{
		UpdateAnimationState();
	}

	// Update attack cooldowns
	TArray<FName> CooldownKeys;
	AttackCooldowns.GetKeys(CooldownKeys);

	for (const FName& AttackID : CooldownKeys)
	{
		float& Cooldown = AttackCooldowns[AttackID];
		Cooldown = FMath::Max(0.0f, Cooldown - DeltaTime);
	}
}

void AHarmoniaMonsterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize ability system when possessed
	if (AbilitySystemComponent && !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		InitializeAbilitySystem();
	}
}

void AHarmoniaMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [BANDWIDTH OPTIMIZATION] MonsterLevel only needs to replicate once at spawn
	// Level doesn't change during gameplay, so COND_InitialOnly saves bandwidth
	DOREPLIFETIME_CONDITION(AHarmoniaMonsterBase, MonsterLevel, COND_InitialOnly);
	DOREPLIFETIME(AHarmoniaMonsterBase, CurrentState);
	DOREPLIFETIME(AHarmoniaMonsterBase, CurrentTarget);

	// [BANDWIDTH OPTIMIZATION] TeamIdentification only needs to replicate once at spawn
	// Team usually doesn't change during gameplay
	DOREPLIFETIME_CONDITION(AHarmoniaMonsterBase, TeamIdentification, COND_InitialOnly);
}

UAbilitySystemComponent* AHarmoniaMonsterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// ============================================================================
// IHarmoniaMonsterInterface Implementation
// ============================================================================

UHarmoniaMonsterData* AHarmoniaMonsterBase::GetMonsterData_Implementation() const
{
	return MonsterData;
}

int32 AHarmoniaMonsterBase::GetMonsterLevel_Implementation() const
{
	return MonsterLevel;
}

FText AHarmoniaMonsterBase::GetMonsterName_Implementation() const
{
	if (MonsterData)
	{
		return MonsterData->MonsterName;
	}
	return FText::FromString(GetName());
}

EHarmoniaMonsterFaction AHarmoniaMonsterBase::GetFaction_Implementation() const
{
	if (MonsterData)
	{
		return MonsterData->FactionSettings.Faction;
	}
	return EHarmoniaMonsterFaction::PlayerHostile;
}

EHarmoniaMonsterState AHarmoniaMonsterBase::GetMonsterState_Implementation() const
{
	return CurrentState;
}

void AHarmoniaMonsterBase::SetMonsterState_Implementation(EHarmoniaMonsterState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EHarmoniaMonsterState OldState = CurrentState;
	CurrentState = NewState;

	// Broadcast state change
	OnMonsterStateChanged.Broadcast(OldState, NewState);

	// Handle state-specific logic
	if (NewState == EHarmoniaMonsterState::Dead && !bDeathStarted)
	{
		PlayDeathAnimation();
	}
}

bool AHarmoniaMonsterBase::IsInCombat_Implementation() const
{
	return CurrentState == EHarmoniaMonsterState::Combat;
}

bool AHarmoniaMonsterBase::IsDead_Implementation() const
{
	return CurrentState == EHarmoniaMonsterState::Dead || bDeathStarted;
}

void AHarmoniaMonsterBase::OnDamageTaken_Implementation(float DamageAmount, AActor* DamageInstigator)
{
	// Broadcast damage event
	OnMonsterDamaged.Broadcast(this, DamageAmount, DamageInstigator);

	// Enter combat state if not already
	if (CurrentState != EHarmoniaMonsterState::Combat && CurrentState != EHarmoniaMonsterState::Dead)
	{
		SetMonsterState_Implementation(EHarmoniaMonsterState::Combat);
	}

	// Set target if we don't have one
	if (!CurrentTarget && DamageInstigator)
	{
		SetCurrentTarget_Implementation(DamageInstigator);
	}
}

void AHarmoniaMonsterBase::OnDeath_Implementation(AActor* Killer)
{
	if (bDeathStarted)
	{
		return;
	}

	bDeathStarted = true;
	SetMonsterState_Implementation(EHarmoniaMonsterState::Dead);

	// Broadcast death event
	OnMonsterDeath.Broadcast(this, Killer);

	// Generate and spawn loot
	if (HasAuthority())
	{
		TArray<FHarmoniaLootTableRow> GeneratedLoot = GenerateLoot_Implementation(Killer);
		SpawnLoot_Implementation(GeneratedLoot, GetActorLocation());
	}

	// Disable collision
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Handle death cleanup
	HandleDeathCleanup();
}

AActor* AHarmoniaMonsterBase::GetCurrentTarget_Implementation() const
{
	return CurrentTarget;
}

void AHarmoniaMonsterBase::SetCurrentTarget_Implementation(AActor* NewTarget)
{
	CurrentTarget = NewTarget;

	if (NewTarget)
	{
		OnTargetDetected_Implementation(NewTarget);
	}
	else
	{
		OnTargetLost_Implementation();
	}
}

TArray<FHarmoniaLootTableRow> AHarmoniaMonsterBase::GenerateLoot_Implementation(AActor* Killer)
{
	UHarmoniaLootTableData* LootTable = OverrideLootTable ? OverrideLootTable : (MonsterData ? MonsterData->LootTable : nullptr);

	if (LootTable)
	{
		return LootTable->GenerateLoot(MonsterLevel, LootLuckModifier);
	}

	return TArray<FHarmoniaLootTableRow>();
}

void AHarmoniaMonsterBase::SpawnLoot_Implementation(const TArray<FHarmoniaLootTableRow>& LootItems, const FVector& SpawnLocation)
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	// Spawn each loot item as a pickup actor
	for (const FHarmoniaLootTableRow& LootItem : LootItems)
	{
		// Calculate quantity
		int32 Quantity = FMath::RandRange(LootItem.MinQuantity, LootItem.MaxQuantity);
		if (Quantity <= 0)
		{
			continue;
		}

		// Calculate random spawn offset to spread items
		FVector RandomOffset = FVector(
			FMath::RandRange(-100.0f, 100.0f),
			FMath::RandRange(-100.0f, 100.0f),
			50.0f
		);
		FVector ItemSpawnLocation = SpawnLocation + RandomOffset;

		// Spawn pickup actor
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AHarmoniaItemPickup* Pickup = GetWorld()->SpawnActor<AHarmoniaItemPickup>(
			AHarmoniaItemPickup::StaticClass(),
			ItemSpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Pickup)
		{
			// Handle different loot types
			if (LootItem.LootType == EHarmoniaLootItemType::Currency)
			{
				Pickup->InitializeGoldPickup(Quantity);
			}
			else
			{
				Pickup->InitializePickup(LootItem, Quantity);
			}

			UE_LOG(LogTemp, Log, TEXT("Monster dropped: %s x%d at %s"),
				*LootItem.ItemID.ToString(), Quantity, *ItemSpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn item pickup for %s"), *LootItem.ItemID.ToString());
		}
	}

	// Spawn gold if configured
	if (MonsterData && MonsterData->LootTable)
	{
		int32 GoldAmount = FMath::RandRange(MonsterData->LootTable->MinGold, MonsterData->LootTable->MaxGold);
		if (GoldAmount > 0)
		{
			FVector GoldSpawnLocation = SpawnLocation + FVector(0.0f, 0.0f, 50.0f);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AHarmoniaItemPickup* GoldPickup = GetWorld()->SpawnActor<AHarmoniaItemPickup>(
				AHarmoniaItemPickup::StaticClass(),
				GoldSpawnLocation,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (GoldPickup)
			{
				GoldPickup->InitializeGoldPickup(GoldAmount);
				UE_LOG(LogTemp, Log, TEXT("Monster dropped: %d gold"), GoldAmount);
			}
		}
	}
}

EHarmoniaMonsterAggroType AHarmoniaMonsterBase::GetAggroType_Implementation() const
{
	if (MonsterData)
	{
		return MonsterData->AggroType;
	}
	return EHarmoniaMonsterAggroType::Neutral;
}

float AHarmoniaMonsterBase::GetAggroRange_Implementation() const
{
	if (MonsterData)
	{
		return MonsterData->AggroRange;
	}
	return 1000.0f;
}

void AHarmoniaMonsterBase::OnTargetDetected_Implementation(AActor* DetectedActor)
{
	// Enter alert or combat state based on aggro type
	if (CurrentState == EHarmoniaMonsterState::Idle || CurrentState == EHarmoniaMonsterState::Patrol)
	{
		EHarmoniaMonsterAggroType AggroType = GetAggroType_Implementation();

		if (AggroType == EHarmoniaMonsterAggroType::Aggressive)
		{
			SetMonsterState_Implementation(EHarmoniaMonsterState::Combat);
		}
		else
		{
			SetMonsterState_Implementation(EHarmoniaMonsterState::Alert);
		}
	}
}

void AHarmoniaMonsterBase::OnTargetLost_Implementation()
{
	// Return to patrol or idle
	if (CurrentState == EHarmoniaMonsterState::Combat || CurrentState == EHarmoniaMonsterState::Alert)
	{
		SetMonsterState_Implementation(EHarmoniaMonsterState::Patrol);
	}
}

// ============================================================================
// Public Functions
// ============================================================================

void AHarmoniaMonsterBase::InitializeMonster(UHarmoniaMonsterData* InMonsterData, int32 InLevel)
{
	if (bInitialized)
	{
		return;
	}

	MonsterData = InMonsterData;
	MonsterLevel = FMath::Max(1, InLevel);

	if (!MonsterData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster initialized without MonsterData!"));
		return;
	}

	// Apply level scaling
	ApplyLevelScaling();

	// Grant abilities
	GrantAbilities();

	// Set movement speed
	if (GetCharacterMovement())
	{
		float ScaledSpeed = MonsterData->BaseStats.BaseMovementSpeed;
		GetCharacterMovement()->MaxWalkSpeed = ScaledSpeed;
	}

	bInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("Monster initialized: %s (Level %d)"), *GetMonsterName_Implementation().ToString(), MonsterLevel);
}

void AHarmoniaMonsterBase::ApplyLevelScaling()
{
	if (!MonsterData || !AttributeSet || !AbilitySystemComponent)
	{
		return;
	}

	const FHarmoniaMonsterStats& Stats = MonsterData->BaseStats;

	// Calculate scaled stats
	float ScaledMaxHealth = Stats.GetScaledHealth(MonsterLevel);
	float ScaledAttackPower = Stats.GetScaledAttackPower(MonsterLevel);
	float ScaledDefense = Stats.GetScaledDefense(MonsterLevel);

	// Set attributes directly (or use GameplayEffects for better integration)
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetMaxHealthAttribute(), ScaledMaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetHealthAttribute(), ScaledMaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetAttackPowerAttribute(), ScaledAttackPower);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetDefenseAttribute(), ScaledDefense);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetCriticalChanceAttribute(), Stats.CriticalChance);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetCriticalDamageAttribute(), Stats.CriticalDamage);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetMovementSpeedAttribute(), Stats.BaseMovementSpeed);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetAttackSpeedAttribute(), Stats.BaseAttackSpeed);
}

TScriptInterface<IHarmoniaMonsterAnimationInterface> AHarmoniaMonsterBase::GetMonsterAnimationInterface() const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Implements<UHarmoniaMonsterAnimationInterface>())
		{
			return TScriptInterface<IHarmoniaMonsterAnimationInterface>(AnimInstance);
		}
	}
	return nullptr;
}

void AHarmoniaMonsterBase::PlayDeathAnimation()
{
	if (!HasAuthority())
	{
		return;
	}

	// Play death montage if available
	if (MonsterData && MonsterData->DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(MonsterData->DeathMontage);
		}
	}

	// Enable ragdoll if configured
	if (MonsterData && MonsterData->bRagdollOnDeath)
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}

	// Schedule corpse destruction
	if (MonsterData && MonsterData->CorpseLifetime > 0.0f)
	{
		FTimerHandle UnusedHandle;
		GetWorld()->GetTimerManager().SetTimer(
			UnusedHandle,
			FTimerDelegate::CreateLambda([this]() { DestroyCorpse(); }),
			MonsterData->CorpseLifetime,
			false
		);
	}
}

bool AHarmoniaMonsterBase::PerformAttack(FName AttackID)
{
	if (!MonsterData || IsDead_Implementation())
	{
		return false;
	}

	// Check cooldown
	if (AttackCooldowns.Contains(AttackID) && AttackCooldowns[AttackID] > 0.0f)
	{
		return false;
	}

	// Find attack pattern
	const FHarmoniaMonsterAttackPattern* AttackPattern = MonsterData->AttackPatterns.FindByPredicate([AttackID](const FHarmoniaMonsterAttackPattern& Pattern)
	{
		return Pattern.AttackID == AttackID;
	});

	if (!AttackPattern)
	{
		return false;
	}

	// Check state requirement
	if (AttackPattern->RequiredState != EHarmoniaMonsterState::Combat && CurrentState != AttackPattern->RequiredState)
	{
		return false;
	}

	// Play animation
	if (AttackPattern->AttackMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(AttackPattern->AttackMontage);
		}
	}

	// Activate ability if specified
	if (AttackPattern->AttackAbility && AbilitySystemComponent)
	{
		FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AttackPattern->AttackAbility);
		if (Spec)
		{
			AbilitySystemComponent->TryActivateAbility(Spec->Handle);
		}
	}

	// Set cooldown
	AttackCooldowns.Add(AttackID, AttackPattern->Cooldown);

	return true;
}

FHarmoniaMonsterAttackPattern AHarmoniaMonsterBase::SelectRandomAttack() const
{
	if (!MonsterData || MonsterData->AttackPatterns.Num() == 0)
	{
		return FHarmoniaMonsterAttackPattern();
	}

	// Filter available attacks (not on cooldown, state requirements met)
	TArray<FHarmoniaMonsterAttackPattern> AvailableAttacks;
	int32 TotalWeight = 0;

	for (const FHarmoniaMonsterAttackPattern& Pattern : MonsterData->AttackPatterns)
	{
		// Check cooldown
		if (AttackCooldowns.Contains(Pattern.AttackID) && AttackCooldowns[Pattern.AttackID] > 0.0f)
		{
			continue;
		}

		// Check state
		if (Pattern.RequiredState != EHarmoniaMonsterState::Combat && CurrentState != Pattern.RequiredState)
		{
			continue;
		}

		// Check distance if we have a target
		if (CurrentTarget)
		{
			float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
			if (DistanceToTarget < Pattern.MinRange || DistanceToTarget > Pattern.MaxRange)
			{
				continue;
			}
		}

		AvailableAttacks.Add(Pattern);
		TotalWeight += Pattern.SelectionWeight;
	}

	if (AvailableAttacks.Num() == 0)
	{
		return FHarmoniaMonsterAttackPattern();
	}

	// Weighted random selection
	int32 RandomValue = FMath::RandRange(0, TotalWeight - 1);
	int32 CurrentWeight = 0;

	for (const FHarmoniaMonsterAttackPattern& Pattern : AvailableAttacks)
	{
		CurrentWeight += Pattern.SelectionWeight;
		if (RandomValue < CurrentWeight)
		{
			return Pattern;
		}
	}

	return AvailableAttacks[0];
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaMonsterBase::OnRep_MonsterState(EHarmoniaMonsterState OldState)
{
	OnMonsterStateChanged.Broadcast(OldState, CurrentState);
}

void AHarmoniaMonsterBase::OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	// Handle health changes if needed
}

void AHarmoniaMonsterBase::OnOutOfHealth(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	if (HasAuthority() && !bDeathStarted)
	{
		OnDeath_Implementation(EffectInstigator);
	}
}

void AHarmoniaMonsterBase::OnDamageReceived(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	if (HasAuthority())
	{
		// Add threat for damage dealer
		if (ThreatComponent && EffectInstigator)
		{
			ThreatComponent->AddThreat(EffectInstigator, EffectMagnitude);

			// Update target based on threat if no current target or threat system takes priority
			AActor* HighestThreat = ThreatComponent->GetHighestThreatActor();
			if (HighestThreat && (!CurrentTarget || HighestThreat != CurrentTarget))
			{
				SetCurrentTarget_Implementation(HighestThreat);
			}
		}

		OnDamageTaken_Implementation(EffectMagnitude, EffectInstigator);
	}
}

void AHarmoniaMonsterBase::InitializeAbilitySystem()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Bind to attribute changes
	AttributeSet->OnHealthChanged.AddUObject(this, &AHarmoniaMonsterBase::OnHealthChanged);
	AttributeSet->OnOutOfHealth.AddUObject(this, &AHarmoniaMonsterBase::OnOutOfHealth);
	AttributeSet->OnDamageReceived.AddUObject(this, &AHarmoniaMonsterBase::OnDamageReceived);
}

void AHarmoniaMonsterBase::GrantAbilities()
{
	if (!AbilitySystemComponent || !MonsterData || !HasAuthority())
	{
		return;
	}

	// Grant abilities from attack patterns
	for (const FHarmoniaMonsterAttackPattern& Pattern : MonsterData->AttackPatterns)
	{
		if (Pattern.AttackAbility)
		{
			FGameplayAbilitySpec AbilitySpec(Pattern.AttackAbility, MonsterLevel, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void AHarmoniaMonsterBase::UpdateAnimationState()
{
	TScriptInterface<IHarmoniaMonsterAnimationInterface> AnimInterface = GetMonsterAnimationInterface();
	if (!AnimInterface)
	{
		return;
	}

	// Update movement parameters
	if (GetCharacterMovement())
	{
		float Speed = GetVelocity().Size();
		AnimInterface->UpdateMovementSpeed(Speed);

		bool bInAir = GetCharacterMovement()->IsFalling();
		AnimInterface->UpdateIsInAir(bInAir);
	}

	// Update combat state
	AnimInterface->UpdateIsInCombat(IsInCombat_Implementation());

	// Update monster state
	AnimInterface->UpdateMonsterState(CurrentState);
}

void AHarmoniaMonsterBase::HandleDeathCleanup()
{
	// Disable AI
	if (GetController())
	{
		GetController()->UnPossess();
	}

	// Stop movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	// Cancel all abilities
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}
}

void AHarmoniaMonsterBase::DestroyCorpse()
{
	Destroy();
}

// ============================================================================
// Sense System Integration
// ============================================================================

TArray<AActor*> AHarmoniaMonsterBase::GetSensedTargets(FName SensorTag) const
{
	TArray<AActor*> SensedActors;

	if (!SenseInteractionComponent)
	{
		return SensedActors;
	}

	// Use Harmonia Sense Interaction Component's convenient target tracking
	TArray<FInteractableTargetInfo> TrackedTargets;

	// If sensor tag specified, get only those
	if (SensorTag != NAME_None)
	{
		TrackedTargets = SenseInteractionComponent->GetInteractablesBySensorTag(SensorTag);
	}
	else
	{
		// Get all tracked targets
		TrackedTargets = SenseInteractionComponent->GetAllInteractableTargets();
	}

	// Extract actors from target info
	for (const FInteractableTargetInfo& TargetInfo : TrackedTargets)
	{
		if (TargetInfo.IsValid())
		{
			AActor* Actor = TargetInfo.TargetActor;
			if (Actor && !Actor->IsPendingKillPending())
			{
				// Filter out self
				if (Actor != this)
				{
					SensedActors.AddUnique(Actor);
				}
			}
		}
	}

	return SensedActors;
}

AActor* AHarmoniaMonsterBase::SelectBestTarget() const
{
	// First, check threat system if available
	if (ThreatComponent)
	{
		AActor* HighestThreat = ThreatComponent->GetHighestThreatActor();
		if (HighestThreat)
		{
			return HighestThreat;
		}
	}

	// Use Harmonia Sense Interaction Component for target selection
	// It automatically prioritizes by distance and manages tracked targets
	if (!SenseInteractionComponent)
	{
		return nullptr;
	}

	// Get best target from sense interaction component
	// It's already sorted by priority (distance-based by default)
	FInteractableTargetInfo BestTargetInfo = SenseInteractionComponent->GetBestInteractionTarget();

	if (!BestTargetInfo.IsValid())
	{
		return nullptr;
	}

	AActor* BestTarget = BestTargetInfo.TargetActor;

	// Validate target
	if (!BestTarget)
	{
		return nullptr;
	}

	// Check if it's a valid pawn
	APawn* TargetPawn = Cast<APawn>(BestTarget);
	if (!TargetPawn || !TargetPawn->GetController())
	{
		return nullptr;
	}

	// Faction-based targeting (check if target is another monster)
	if (BestTarget->Implements<UHarmoniaMonsterInterface>())
	{
		// If target is a monster, check faction
		if (MonsterData)
		{
			// Get target's faction
			UHarmoniaMonsterData* TargetMonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(BestTarget);
			if (TargetMonsterData)
			{
				// Check faction relationship
				const FHarmoniaFactionSettings& MyFaction = MonsterData->FactionSettings;
				const FHarmoniaFactionSettings& TargetFaction = TargetMonsterData->FactionSettings;

				// Can we attack this faction?
				if (!MyFaction.CanAttack(TargetFaction.Faction))
				{
					// Not hostile, skip this target
					return nullptr;
				}
			}
			else
			{
				// No faction data, assume neutral (don't attack)
				return nullptr;
			}
		}
		else
		{
			// No faction data, assume neutral (don't attack other monsters)
			return nullptr;
		}
	}

	return BestTarget;
}

// ============================================================================
// Gameplay Ability Integration
// ============================================================================

bool AHarmoniaMonsterBase::ActivateAttackAbility(FName AttackID)
{
	if (!MonsterData || !AbilitySystemComponent || IsDead_Implementation())
	{
		return false;
	}

	// Check cooldown
	if (AttackCooldowns.Contains(AttackID) && AttackCooldowns[AttackID] > 0.0f)
	{
		return false;
	}

	// Find attack pattern
	const FHarmoniaMonsterAttackPattern* AttackPattern = MonsterData->AttackPatterns.FindByPredicate([AttackID](const FHarmoniaMonsterAttackPattern& Pattern)
	{
		return Pattern.AttackID == AttackID;
	});

	if (!AttackPattern || !AttackPattern->AttackAbility)
	{
		return false;
	}

	// Check state requirement
	if (AttackPattern->RequiredState != EHarmoniaMonsterState::Combat && CurrentState != AttackPattern->RequiredState)
	{
		return false;
	}

	// Find and activate the ability
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AttackPattern->AttackAbility);
	if (!Spec)
	{
		return false;
	}

	bool bSuccess = AbilitySystemComponent->TryActivateAbility(Spec->Handle);

	if (bSuccess)
	{
		// Set cooldown
		AttackCooldowns.Add(AttackID, AttackPattern->Cooldown);
	}

	return bSuccess;
}

bool AHarmoniaMonsterBase::IsAttackOnCooldown(FName AttackID) const
{
	if (AttackCooldowns.Contains(AttackID))
	{
		return AttackCooldowns[AttackID] > 0.0f;
	}
	return false;
}

// ============================================================================
// IHarmoniaTeamAgentInterface Implementation
// ============================================================================

FHarmoniaTeamIdentification AHarmoniaMonsterBase::GetTeamID_Implementation() const
{
	// If using legacy faction system, convert to team ID
	if (bUseLegacyFactionSystem && MonsterData)
	{
		// Convert legacy faction to numeric team ID
		FHarmoniaTeamIdentification LegacyTeamID;
		LegacyTeamID.TeamNumericID = static_cast<int32>(MonsterData->FactionSettings.Faction) + 1000; // Offset to avoid conflict
		LegacyTeamID.TeamName = FText::FromString(UEnum::GetValueAsString(MonsterData->FactionSettings.Faction));
		return LegacyTeamID;
	}

	// Use new team system
	if (TeamIdentification.IsValid())
	{
		return TeamIdentification;
	}

	// Fall back to team from monster data
	if (MonsterData && MonsterData->TeamID.IsValid())
	{
		return MonsterData->TeamID;
	}

	return FHarmoniaTeamIdentification();
}

void AHarmoniaMonsterBase::SetTeamID_Implementation(const FHarmoniaTeamIdentification& NewTeamID)
{
	TeamIdentification = NewTeamID;
	bUseLegacyFactionSystem = false;
}

EHarmoniaTeamRelationship AHarmoniaMonsterBase::GetRelationshipWith_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	// Get team management subsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	UHarmoniaTeamManagementSubsystem* TeamSubsystem = World->GetSubsystem<UHarmoniaTeamManagementSubsystem>();
	if (!TeamSubsystem)
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	return TeamSubsystem->GetActorRelationship(const_cast<AHarmoniaMonsterBase*>(this), OtherActor);
}

bool AHarmoniaMonsterBase::CanAttackActor_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(this, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

bool AHarmoniaMonsterBase::ShouldHelpActor_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(this, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

bool AHarmoniaMonsterBase::IsSameTeamAs_Implementation(AActor* OtherActor) const
{
	if (!OtherActor || !OtherActor->Implements<UHarmoniaTeamAgentInterface>())
	{
		return false;
	}

	FHarmoniaTeamIdentification MyTeam = Execute_GetTeamID(this);
	FHarmoniaTeamIdentification OtherTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(OtherActor);

	return MyTeam.IsValid() && OtherTeam.IsValid() && MyTeam.IsSameTeam(OtherTeam);
}

bool AHarmoniaMonsterBase::IsAllyWith_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(this, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

bool AHarmoniaMonsterBase::IsEnemyWith_Implementation(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(this, OtherActor);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

// ============================================================================
// IGenericTeamAgentInterface Implementation (Unreal's Standard Team System)
// ============================================================================

FGenericTeamId AHarmoniaMonsterBase::GetGenericTeamId() const
{
	// Convert our team ID to Unreal's FGenericTeamId
	FHarmoniaTeamIdentification HarmoniaTeam = Execute_GetTeamID(this);

	if (HarmoniaTeam.IsValid() && HarmoniaTeam.TeamNumericID > 0)
	{
		// Use numeric ID for Unreal's team system
		// FGenericTeamId supports values 0-255
		uint8 GenericTeamID = static_cast<uint8>(HarmoniaTeam.TeamNumericID % 256);
		return FGenericTeamId(GenericTeamID);
	}

	// Return NoTeam if no valid team
	return FGenericTeamId::NoTeam;
}

void AHarmoniaMonsterBase::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	// Convert Unreal's FGenericTeamId to our team system
	if (TeamID != FGenericTeamId::NoTeam)
	{
		FHarmoniaTeamIdentification NewTeamID;
		NewTeamID.TeamNumericID = TeamID.GetId();
		NewTeamID.TeamName = FText::FromString(FString::Printf(TEXT("Team %d"), TeamID.GetId()));
		Execute_SetTeamID(this, NewTeamID);
	}
}

ETeamAttitude::Type AHarmoniaMonsterBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	// Convert our relationship system to Unreal's ETeamAttitude
	EHarmoniaTeamRelationship Relationship = Execute_GetRelationshipWith(this, const_cast<AActor*>(&Other));

	switch (Relationship)
	{
	case EHarmoniaTeamRelationship::Ally:
		return ETeamAttitude::Friendly;

	case EHarmoniaTeamRelationship::Enemy:
		return ETeamAttitude::Hostile;

	case EHarmoniaTeamRelationship::Neutral:
	default:
		return ETeamAttitude::Neutral;
	}
}
