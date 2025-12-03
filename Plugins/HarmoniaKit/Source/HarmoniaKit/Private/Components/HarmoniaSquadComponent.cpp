// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSquadComponent.h"
#include "Core/HarmoniaCoreBFL.h"
#include "HarmoniaLogCategories.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

UHarmoniaSquadComponent::UHarmoniaSquadComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
}

void UHarmoniaSquadComponent::InitializeAIComponent()
{
	Super::InitializeAIComponent();

	// Auto-form squad if enabled
	if (bSquadEnabled && GetOwner() && GetOwner()->HasAuthority())
	{
		// Try to find nearby monsters and form a squad
		FTimerHandle UnusedHandle;
		GetWorld()->GetTimerManager().SetTimer(
			UnusedHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				TryFormSquad();
			}),
			1.0f,
			false
		);
	}
}

void UHarmoniaSquadComponent::UpdateAIComponent(float DeltaTime)
{
	Super::UpdateAIComponent(DeltaTime);

	if (!bSquadEnabled || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	TimeSinceFormationUpdate += DeltaTime;

	// Update formation periodically
	if (IsSquadLeader() && TimeSinceFormationUpdate >= FormationUpdateInterval)
	{
		UpdateFormation();
		CheckSquadCohesion();
		TimeSinceFormationUpdate = 0.0f;
	}
}

bool UHarmoniaSquadComponent::IsInCombat() const
{
	// Squad is in combat if any member has a valid target
	if (Super::IsInCombat())
	{
		return true;
	}

	// Check squad members
	for (const FHarmoniaSquadMemberInfo& MemberInfo : SquadMembers)
	{
		if (MemberInfo.Monster)
		{
			if (const UHarmoniaSquadComponent* MemberSquad = MemberInfo.Monster->FindComponentByClass<UHarmoniaSquadComponent>())
			{
				if (MemberSquad->HasValidTarget())
				{
					return true;
				}
			}
		}
	}

	return false;
}

void UHarmoniaSquadComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaSquadComponent, SquadLeader);
	DOREPLIFETIME(UHarmoniaSquadComponent, SquadMembers);
	DOREPLIFETIME(UHarmoniaSquadComponent, CurrentFormation);
	DOREPLIFETIME(UHarmoniaSquadComponent, MyRole);
	DOREPLIFETIME(UHarmoniaSquadComponent, SharedTarget);
}

// ============================================================================
// Public Functions
// ============================================================================

bool UHarmoniaSquadComponent::IsSquadLeader() const
{
	return SquadLeader == nullptr || SquadLeader == GetOwner();
}

int32 UHarmoniaSquadComponent::GetSquadSize() const
{
	if (!IsSquadLeader())
	{
		// Followers don't track squad size
		return 0;
	}

	int32 AliveCount = 0;
	for (const FHarmoniaSquadMemberInfo& Member : SquadMembers)
	{
		if (Member.IsValid())
		{
			AliveCount++;
		}
	}

	return AliveCount;
}

bool UHarmoniaSquadComponent::JoinSquad(AActor* Leader)
{
	if (!Leader || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	// Leave current squad if in one
	if (SquadLeader)
	{
		LeaveSquad();
	}

	// Get leader's squad component
	UHarmoniaSquadComponent* LeaderSquad = Leader->FindComponentByClass<UHarmoniaSquadComponent>();
	if (!LeaderSquad)
	{
		UE_LOG(LogHarmoniaSquad, Warning, TEXT("Leader %s has no squad component"), *Leader->GetName());
		return false;
	}

	// Determine role
	EHarmoniaSquadRole AssignedRole = EHarmoniaSquadRole::DPS;
	if (bAutoAssignRoles)
	{
		AssignedRole = DetermineRole(GetOwner());
	}

	// Add to leader's squad
	if (LeaderSquad->AddSquadMember(GetOwner(), AssignedRole))
	{
		SquadLeader = Leader;
		MyRole = AssignedRole;

		UE_LOG(LogHarmoniaSquad, Log, TEXT("%s joined squad led by %s with role %d"),
			*GetOwner()->GetName(), *Leader->GetName(), (int32)AssignedRole);

		return true;
	}

	return false;
}

void UHarmoniaSquadComponent::LeaveSquad()
{
	HARMONIA_REQUIRE_SERVER(this);

	if (SquadLeader)
	{
		// Remove from leader's squad
		UHarmoniaSquadComponent* LeaderSquad = SquadLeader->FindComponentByClass<UHarmoniaSquadComponent>();
		if (LeaderSquad)
		{
			LeaderSquad->RemoveSquadMember(GetOwner());
		}

		SquadLeader = nullptr;
		MyRole = EHarmoniaSquadRole::DPS;
	}
}

bool UHarmoniaSquadComponent::AddSquadMember(AActor* NewMember, EHarmoniaSquadRole Role)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !NewMember)
	{
		return false;
	}

	// Check if already in squad
	for (const FHarmoniaSquadMemberInfo& Member : SquadMembers)
	{
		if (Member.Monster == NewMember)
		{
			UE_LOG(LogHarmoniaSquad, Warning, TEXT("%s is already in the squad"), *NewMember->GetName());
			return false;
		}
	}

	// Create member info
	FHarmoniaSquadMemberInfo MemberInfo;
	MemberInfo.Monster = NewMember;
	MemberInfo.Role = Role;
	MemberInfo.bIsAlive = true;
	MemberInfo.LastKnownLocation = NewMember->GetActorLocation();
	MemberInfo.FormationOffset = FVector::ZeroVector;

	// Add to squad
	SquadMembers.Add(MemberInfo);

	// Bind to death event if monster implements the interface
	if (NewMember->Implements<UHarmoniaMonsterInterface>())
	{
		AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(NewMember);
		if (Monster)
		{
			Monster->OnMonsterDeath.AddDynamic(this, &UHarmoniaSquadComponent::OnMemberDeath);
		}
	}

	// Broadcast event
	OnSquadMemberAdded.Broadcast(NewMember, Role);
	OnSquadStateChanged.Broadcast(GetOwner(), GetSquadSize());

	// Update formation
	UpdateFormation();

	UE_LOG(LogHarmoniaSquad, Log, TEXT("Added %s to squad with role %d"), *NewMember->GetName(), (int32)Role);

	return true;
}

void UHarmoniaSquadComponent::RemoveSquadMember(AActor* Member)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Member)
	{
		return;
	}

	// Find and remove member
	for (int32 i = SquadMembers.Num() - 1; i >= 0; --i)
	{
		if (SquadMembers[i].Monster == Member)
		{
			SquadMembers.RemoveAt(i);

			OnSquadMemberRemoved.Broadcast(Member);
			OnSquadStateChanged.Broadcast(GetOwner(), GetSquadSize());

			UE_LOG(LogHarmoniaSquad, Log, TEXT("Removed %s from squad"), *Member->GetName());
			break;
		}
	}

	// Update formation
	UpdateFormation();
}

FVector UHarmoniaSquadComponent::GetFormationPosition() const
{
	if (!SquadLeader || IsSquadLeader())
	{
		// Leaders stay at their current position
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	// Find our formation offset from leader's squad
	UHarmoniaSquadComponent* LeaderSquad = SquadLeader->FindComponentByClass<UHarmoniaSquadComponent>();
	if (!LeaderSquad)
	{
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	// Find ourselves in the leader's member list
	for (const FHarmoniaSquadMemberInfo& Member : LeaderSquad->SquadMembers)
	{
		if (Member.Monster == GetOwner())
		{
			// Calculate world position from formation offset
			FVector LeaderLocation = SquadLeader->GetActorLocation();
			FRotator LeaderRotation = SquadLeader->GetActorRotation();
			return LeaderLocation + LeaderRotation.RotateVector(Member.FormationOffset);
		}
	}

	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

void UHarmoniaSquadComponent::SetSquadTarget(AActor* NewTarget)
{
	HARMONIA_REQUIRE_SERVER(this);

	SharedTarget = NewTarget;

	// If leader, propagate to all members
	if (IsSquadLeader())
	{
		for (const FHarmoniaSquadMemberInfo& Member : SquadMembers)
		{
			if (Member.IsValid() && Member.Monster->Implements<UHarmoniaMonsterInterface>())
			{
				IHarmoniaMonsterInterface::Execute_SetCurrentTarget(Member.Monster, NewTarget);
			}
		}
	}
	// If follower, notify leader
	else if (SquadLeader)
	{
		UHarmoniaSquadComponent* LeaderSquad = SquadLeader->FindComponentByClass<UHarmoniaSquadComponent>();
		if (LeaderSquad)
		{
			LeaderSquad->SetSquadTarget(NewTarget);
		}
	}
}

void UHarmoniaSquadComponent::RequestCoordinatedAttack(AActor* Target)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Target)
	{
		return;
	}

	SetSquadTarget(Target);

	UE_LOG(LogHarmoniaSquad, Log, TEXT("Squad coordinated attack on %s"), *Target->GetName());
}

TArray<AActor*> UHarmoniaSquadComponent::GetAliveSquadMembers() const
{
	TArray<AActor*> AliveMembers;

	for (const FHarmoniaSquadMemberInfo& Member : SquadMembers)
	{
		if (Member.IsValid())
		{
			AliveMembers.Add(Member.Monster);
		}
	}

	return AliveMembers;
}

TArray<AActor*> UHarmoniaSquadComponent::FindNearbyMonsters(float SearchRadius) const
{
	TArray<AActor*> NearbyMonsters;

	if (!GetOwner())
	{
		return NearbyMonsters;
	}

	// Find all monsters in range
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHarmoniaMonsterBase::StaticClass(), FoundActors);

	FVector MyLocation = GetOwner()->GetActorLocation();

	for (AActor* Actor : FoundActors)
	{
		if (Actor == GetOwner())
		{
			continue;
		}

		float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
		if (Distance <= SearchRadius)
		{
			// Check if monster is already in a squad
			UHarmoniaSquadComponent* OtherSquad = Actor->FindComponentByClass<UHarmoniaSquadComponent>();
			if (!OtherSquad || (!OtherSquad->SquadLeader && OtherSquad->SquadMembers.Num() == 0))
			{
				NearbyMonsters.Add(Actor);
			}
		}
	}

	return NearbyMonsters;
}

bool UHarmoniaSquadComponent::TryFormSquad()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !bSquadEnabled)
	{
		return false;
	}

	// Don't form if already in a squad
	if (SquadLeader || SquadMembers.Num() > 0)
	{
		return false;
	}

	// Check if this monster can be a leader
	if (GetOwner()->Implements<UHarmoniaMonsterInterface>())
	{
		UHarmoniaMonsterData* MonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(GetOwner());
		if (!MonsterData || !MonsterData->bCanBeSquadLeader)
		{
			return false;
		}

		// Find nearby monsters
		TArray<AActor*> NearbyMonsters = FindNearbyMonsters(MonsterData->SquadCoordinationRange);

		if (NearbyMonsters.Num() == 0)
		{
			return false;
		}

		// Become the leader and recruit members
		int32 DesiredSize = MonsterData->PreferredSquadSize;
		CurrentFormation = MonsterData->FormationType;

		for (int32 i = 0; i < FMath::Min(NearbyMonsters.Num(), DesiredSize - 1); ++i)
		{
			UHarmoniaSquadComponent* MemberSquad = NearbyMonsters[i]->FindComponentByClass<UHarmoniaSquadComponent>();
			if (MemberSquad)
			{
				MemberSquad->JoinSquad(GetOwner());
			}
		}

		UE_LOG(LogHarmoniaSquad, Log, TEXT("%s formed a squad with %d members"), *GetOwner()->GetName(), GetSquadSize());

		return GetSquadSize() > 0;
	}

	return false;
}

// ============================================================================
// Protected Functions
// ============================================================================

void UHarmoniaSquadComponent::UpdateFormation()
{
	if (!IsSquadLeader() || SquadMembers.Num() == 0)
	{
		return;
	}

	// Calculate formation offsets for each member
	int32 AliveCount = 0;
	for (int32 i = 0; i < SquadMembers.Num(); ++i)
	{
		if (SquadMembers[i].IsValid())
		{
			SquadMembers[i].FormationOffset = CalculateFormationOffset(AliveCount, GetSquadSize());
			SquadMembers[i].LastKnownLocation = SquadMembers[i].Monster->GetActorLocation();
			AliveCount++;
		}
	}
}

FVector UHarmoniaSquadComponent::CalculateFormationOffset(int32 MemberIndex, int32 TotalMembers) const
{
	if (TotalMembers <= 0)
	{
		return FVector::ZeroVector;
	}

	const float BaseDistance = 300.0f;

	switch (CurrentFormation)
	{
	case EHarmoniaSquadFormationType::Tight:
		{
			// Close formation, small offsets
			float Angle = (2.0f * PI * MemberIndex) / TotalMembers;
			return FVector(FMath::Cos(Angle) * BaseDistance * 0.5f, FMath::Sin(Angle) * BaseDistance * 0.5f, 0.0f);
		}

	case EHarmoniaSquadFormationType::Circle:
		{
			// Circle around leader
			float Angle = (2.0f * PI * MemberIndex) / TotalMembers;
			return FVector(FMath::Cos(Angle) * BaseDistance, FMath::Sin(Angle) * BaseDistance, 0.0f);
		}

	case EHarmoniaSquadFormationType::Line:
		{
			// Line formation
			float Offset = (MemberIndex - TotalMembers / 2.0f) * BaseDistance;
			return FVector(0.0f, Offset, 0.0f);
		}

	case EHarmoniaSquadFormationType::Wedge:
		{
			// V-formation
			int32 Row = FMath::Sqrt((float)MemberIndex);
			int32 PosInRow = MemberIndex - (Row * Row);
			float XOffset = Row * BaseDistance;
			float YOffset = (PosInRow - Row / 2.0f) * BaseDistance;
			return FVector(-XOffset, YOffset, 0.0f);
		}

	case EHarmoniaSquadFormationType::Flanking:
		{
			// Flank on sides
			float Side = (MemberIndex % 2 == 0) ? 1.0f : -1.0f;
			float Depth = FMath::Floor(MemberIndex / 2.0f);
			return FVector(-Depth * BaseDistance * 0.5f, Side * BaseDistance, 0.0f);
		}

	case EHarmoniaSquadFormationType::Loose:
	default:
		{
			// Loose spread formation
			float Angle = (2.0f * PI * MemberIndex) / TotalMembers;
			return FVector(FMath::Cos(Angle) * BaseDistance * 1.5f, FMath::Sin(Angle) * BaseDistance * 1.5f, 0.0f);
		}
	}
}

EHarmoniaSquadRole UHarmoniaSquadComponent::DetermineRole(AActor* Monster) const
{
	if (!Monster || !Monster->Implements<UHarmoniaMonsterInterface>())
	{
		return EHarmoniaSquadRole::DPS;
	}

	// Get ability system component
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Monster);
	if (!ASI)
	{
		return EHarmoniaSquadRole::DPS;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EHarmoniaSquadRole::DPS;
	}

	// Get attribute set
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (!AttributeSet)
	{
		return EHarmoniaSquadRole::DPS;
	}

	// Determine role based on stats
	float Health = AttributeSet->GetMaxHealth();
	float Attack = AttributeSet->GetAttackPower();
	float Defense = AttributeSet->GetDefense();

	// Tank: High health and defense
	if (Health > 500.0f && Defense > 30.0f)
	{
		return EHarmoniaSquadRole::Tank;
	}

	// DPS: High attack
	if (Attack > 50.0f)
	{
		return EHarmoniaSquadRole::DPS;
	}

	// Default to DPS
	return EHarmoniaSquadRole::DPS;
}

void UHarmoniaSquadComponent::CheckSquadCohesion()
{
	if (!IsSquadLeader() || !GetOwner())
	{
		return;
	}

	FVector LeaderLocation = GetOwner()->GetActorLocation();

	// Check each member's distance
	for (int32 i = SquadMembers.Num() - 1; i >= 0; --i)
	{
		if (!SquadMembers[i].IsValid())
		{
			continue;
		}

		float Distance = FVector::Dist(LeaderLocation, SquadMembers[i].Monster->GetActorLocation());

		// If too far, remove from squad
		if (Distance > MaxSquadDistance)
		{
			UE_LOG(LogHarmoniaSquad, Warning, TEXT("Squad member %s is too far (%.1f > %.1f), removing from squad"),
				*SquadMembers[i].Monster->GetName(), Distance, MaxSquadDistance);

			RemoveSquadMember(SquadMembers[i].Monster);
		}
	}
}

void UHarmoniaSquadComponent::OnMemberDeath(AHarmoniaMonsterBase* DeadMember, AActor* Killer)
{
	HARMONIA_REQUIRE_SERVER(this);

	// Mark member as dead
	for (FHarmoniaSquadMemberInfo& Member : SquadMembers)
	{
		if (Member.Monster == DeadMember)
		{
			Member.bIsAlive = false;
			UE_LOG(LogHarmoniaSquad, Log, TEXT("Squad member %s died"), *DeadMember->GetName());
			break;
		}
	}

	// Update squad state
	OnSquadStateChanged.Broadcast(GetOwner(), GetSquadSize());
}
