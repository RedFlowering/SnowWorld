// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_SwarmBehavior.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_SwarmBehavior::UHarmoniaGameplayAbility_SwarmBehavior()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	SwarmDetectionRadius = 500.0f;
	AttackBonusPerMember = 0.15f;
	DefenseBonusPerMember = 0.1f;
	SpeedBonusPerMember = 0.05f;
	FearDuration = 3.0f;
	MinimumSwarmSize = 2;
	MaximumSwarmSize = 10;
	UpdateInterval = 1.0f;
	bInFear = false;
}

void UHarmoniaGameplayAbility_SwarmBehavior::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	// Auto-activate when avatar is set
	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}

void UHarmoniaGameplayAbility_SwarmBehavior::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Start periodic swarm updates
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle,
		this,
		&UHarmoniaGameplayAbility_SwarmBehavior::UpdateSwarmBonuses,
		UpdateInterval,
		true
	);

	// Initial update
	UpdateSwarmBonuses();

	// Bind to death events
	BindToAllyDeathEvents();
}

void UHarmoniaGameplayAbility_SwarmBehavior::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear timer
	if (UpdateTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	// Remove bonuses
	RemoveSwarmBonuses();
	RemoveFear();

	// Unbind events
	UnbindFromAllyDeathEvents();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_SwarmBehavior::UpdateSwarmBonuses()
{
	if (bInFear)
	{
		return; // Don't update bonuses while in fear
	}

	TArray<AActor*> NearbyMembers = GetNearbySwarmMembers();
	int32 MemberCount = NearbyMembers.Num();

	// Store for death event tracking
	SwarmMembers = NearbyMembers;

	// Apply bonuses if enough members
	if (MemberCount >= MinimumSwarmSize)
	{
		ApplySwarmBonuses(MemberCount);
	}
	else
	{
		RemoveSwarmBonuses();
	}
}

TArray<AActor*> UHarmoniaGameplayAbility_SwarmBehavior::GetNearbySwarmMembers() const
{
	TArray<AActor*> Members;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return Members;
	}

	AHarmoniaMonsterBase* MyMonster = Cast<AHarmoniaMonsterBase>(Avatar);
	if (!MyMonster)
	{
		return Members;
	}

	FVector MyLocation = Avatar->GetActorLocation();
	EHarmoniaMonsterFaction MyFaction = MyMonster->GetFaction();

	// Find all monsters of same faction nearby
	TArray<AActor*> AllMonsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHarmoniaMonsterBase::StaticClass(), AllMonsters);

	for (AActor* Actor : AllMonsters)
	{
		if (Actor == Avatar)
		{
			continue;
		}

		AHarmoniaMonsterBase* OtherMonster = Cast<AHarmoniaMonsterBase>(Actor);
		if (!OtherMonster)
		{
			continue;
		}

		// Same faction check
		if (OtherMonster->GetFaction() != MyFaction)
		{
			continue;
		}

		// Distance check
		float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
		if (Distance <= SwarmDetectionRadius)
		{
			Members.Add(Actor);
		}
	}

	return Members;
}

void UHarmoniaGameplayAbility_SwarmBehavior::ApplySwarmBonuses(int32 MemberCount)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Remove existing bonuses first
	RemoveSwarmBonuses();

	// Cap member count
	int32 EffectiveMembers = FMath::Min(MemberCount, MaximumSwarmSize);

	// Apply effect if we have one
	if (SwarmBonusEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SwarmBonusEffect, EffectiveMembers, EffectContext);
		if (SpecHandle.IsValid())
		{
			SwarmBonusHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Manually apply stat bonuses
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet)
	{
		float AttackBonus = 1.0f + (EffectiveMembers * AttackBonusPerMember);
		float DefenseBonus = 1.0f + (EffectiveMembers * DefenseBonusPerMember);
		float SpeedBonus = 1.0f + (EffectiveMembers * SpeedBonusPerMember);

		// Store base values if first time (per instance, not static)
		if (!bBaseStatsStored)
		{
			BaseAttackPower = AttributeSet->GetAttackPower();
			BaseDefense = AttributeSet->GetDefense();
			BaseMovementSpeed = AttributeSet->GetMovementSpeed();
			bBaseStatsStored = true;
		}

		// Apply bonuses using stored base values
		ASC->SetNumericAttributeBase(AttributeSet->GetAttackPowerAttribute(), BaseAttackPower * AttackBonus);
		ASC->SetNumericAttributeBase(AttributeSet->GetDefenseAttribute(), BaseDefense * DefenseBonus);
		ASC->SetNumericAttributeBase(AttributeSet->GetMovementSpeedAttribute(), BaseMovementSpeed * SpeedBonus);
	}

	UE_LOG(LogTemp, Log, TEXT("Swarm bonuses applied: %d members (Attack: +%.0f%%, Defense: +%.0f%%, Speed: +%.0f%%)"),
		EffectiveMembers,
		EffectiveMembers * AttackBonusPerMember * 100.0f,
		EffectiveMembers * DefenseBonusPerMember * 100.0f,
		EffectiveMembers * SpeedBonusPerMember * 100.0f);
}

void UHarmoniaGameplayAbility_SwarmBehavior::RemoveSwarmBonuses()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	if (SwarmBonusHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SwarmBonusHandle);
		SwarmBonusHandle.Invalidate();
	}
}

void UHarmoniaGameplayAbility_SwarmBehavior::OnNearbyAllyDied(AActor* DeadAlly)
{
	if (bInFear)
	{
		return; // Already in fear
	}

	// Check if ally was in our swarm
	if (SwarmMembers.Contains(DeadAlly))
	{
		ApplyFear();

		// Auto-remove fear after duration
		FTimerHandle FearTimer;
		GetWorld()->GetTimerManager().SetTimer(
			FearTimer,
			this,
			&UHarmoniaGameplayAbility_SwarmBehavior::RemoveFear,
			FearDuration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_SwarmBehavior::ApplyFear()
{
	if (bInFear)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	bInFear = true;

	// Remove swarm bonuses while in fear
	RemoveSwarmBonuses();

	// Apply fear effect
	if (FearEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(FearEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			FearEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Set monster to retreating state
	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(GetAvatarActorFromActorInfo());
	if (Monster)
	{
		Monster->SetMonsterState(EHarmoniaMonsterState::Retreating);
	}

	UE_LOG(LogTemp, Log, TEXT("Swarm member entered fear state"));
}

void UHarmoniaGameplayAbility_SwarmBehavior::RemoveFear()
{
	if (!bInFear)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && FearEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(FearEffectHandle);
		FearEffectHandle.Invalidate();
	}

	bInFear = false;

	// Resume normal swarm behavior
	UpdateSwarmBonuses();

	UE_LOG(LogTemp, Log, TEXT("Swarm member recovered from fear"));
}

void UHarmoniaGameplayAbility_SwarmBehavior::BindToAllyDeathEvents()
{
	// This would need to be called whenever swarm composition changes
	// For now, we'll check on update
	// In a full implementation, bind to each swarm member's death event
}

void UHarmoniaGameplayAbility_SwarmBehavior::UnbindFromAllyDeathEvents()
{
	// Unbind from all death events
	SwarmMembers.Empty();
}
