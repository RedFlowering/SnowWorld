// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaTeamManagementSubsystem.h"
#include "Data/HarmoniaTeamSetupData.h"
#include "Definitions/HarmoniaTeamSystemDefinitions.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================================
// Initialization
// ============================================================================

void UHarmoniaTeamManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize with default teams
	InitializeDefaultTeams();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Initialized"));
}

void UHarmoniaTeamManagementSubsystem::Deinitialize()
{
	ClearAllTeams();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Deinitialized"));
}

void UHarmoniaTeamManagementSubsystem::InitializeDefaultTeams()
{
	// Try to load from DefaultTeamSetup data asset
	if (DefaultTeamSetup.IsValid() || !DefaultTeamSetup.IsNull())
	{
		UHarmoniaTeamSetupData* SetupData = DefaultTeamSetup.LoadSynchronous();
		if (SetupData)
		{
			LoadTeamSetup(SetupData, true);
			UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Loaded teams from DefaultTeamSetup"));
			return;
		}
	}

	// Fallback: Hardcoded default teams (will be removed once Data Asset is created)
	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Using fallback hardcoded teams"));

	// Create default Player team
	FHarmoniaTeamIdentification PlayerTeam;
	PlayerTeam.TeamID = FGameplayTag::RequestGameplayTag(FName("Team.Player"));
	PlayerTeam.TeamName = FText::FromString(TEXT("Players"));
	PlayerTeam.TeamColor = FLinearColor::Blue;
	PlayerTeam.DefaultAttitude = EHarmoniaTeamAttitude::Defensive;
	RegisterTeam(PlayerTeam);

	// Create default Neutral team
	FHarmoniaTeamIdentification NeutralTeam;
	NeutralTeam.TeamID = FGameplayTag::RequestGameplayTag(FName("Team.Neutral"));
	NeutralTeam.TeamName = FText::FromString(TEXT("Neutral"));
	NeutralTeam.TeamColor = FLinearColor::Gray;
	NeutralTeam.DefaultAttitude = EHarmoniaTeamAttitude::Neutral;
	RegisterTeam(NeutralTeam);

	// Create Tiger monster team (호랑이)
	FHarmoniaTeamIdentification TigerTeam;
	TigerTeam.TeamID = FGameplayTag::RequestGameplayTag(FName("Team.Monster.Tiger"));
	TigerTeam.TeamName = FText::FromString(TEXT("Tigers"));
	TigerTeam.TeamColor = FLinearColor(1.0f, 0.5f, 0.0f);
	TigerTeam.DefaultAttitude = EHarmoniaTeamAttitude::Hostile;
	RegisterTeam(TigerTeam);
	TeamFriendlyFireMap.Add(TigerTeam.TeamID, true); // 호랑이끼리 공격 가능

	// Create Magpie monster team (까치)
	FHarmoniaTeamIdentification MagpieTeam;
	MagpieTeam.TeamID = FGameplayTag::RequestGameplayTag(FName("Team.Monster.Magpie"));
	MagpieTeam.TeamName = FText::FromString(TEXT("Magpies"));
	MagpieTeam.TeamColor = FLinearColor(0.2f, 0.2f, 0.2f);
	MagpieTeam.DefaultAttitude = EHarmoniaTeamAttitude::Hostile;
	RegisterTeam(MagpieTeam);
	TeamFriendlyFireMap.Add(MagpieTeam.TeamID, false); // 까치끼리 공격 불가

	// Create Generic monster team
	FHarmoniaTeamIdentification GenericMonsterTeam;
	GenericMonsterTeam.TeamID = FGameplayTag::RequestGameplayTag(FName("Team.Monster.Generic"));
	GenericMonsterTeam.TeamName = FText::FromString(TEXT("Monsters"));
	GenericMonsterTeam.TeamColor = FLinearColor::Red;
	GenericMonsterTeam.DefaultAttitude = EHarmoniaTeamAttitude::Hostile;
	RegisterTeam(GenericMonsterTeam);

	// Default Relationships
	MakeTeamsEnemies(PlayerTeam, TigerTeam);
	MakeTeamsEnemies(PlayerTeam, MagpieTeam);
	MakeTeamsEnemies(PlayerTeam, GenericMonsterTeam);
	MakeTeamsEnemies(TigerTeam, MagpieTeam);
}

// ============================================================================
// Team Registration
// ============================================================================

bool UHarmoniaTeamManagementSubsystem::RegisterTeam(const FHarmoniaTeamIdentification& TeamID, UHarmoniaTeamConfigData* TeamConfig)
{
	if (!TeamID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaTeamManagementSubsystem: Cannot register invalid team ID"));
		return false;
	}

	if (IsTeamRegistered(TeamID))
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaTeamManagementSubsystem: Team %s already registered"),
			*TeamID.TeamName.ToString());
		return false;
	}

	RegisteredTeams.Add(TeamID, TeamConfig);
	OnTeamRegistered.Broadcast(TeamID);

	// If team config provided, load its default relationships
	if (TeamConfig)
	{
		for (const FHarmoniaTeamRelationshipData& RelData : TeamConfig->DefaultRelationships)
		{
			SetTeamRelationship(TeamID, RelData.TargetTeam, RelData.Relationship);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Registered team %s (Tag: %s)"),
		*TeamID.TeamName.ToString(), *TeamID.TeamID.ToString());

	return true;
}

void UHarmoniaTeamManagementSubsystem::UnregisterTeam(const FHarmoniaTeamIdentification& TeamID)
{
	if (!IsTeamRegistered(TeamID))
	{
		return;
	}

	RegisteredTeams.Remove(TeamID);
	RelationshipMatrix.Remove(TeamID);

	// Remove this team from other teams' relationship maps
	for (auto& Pair : RelationshipMatrix)
	{
		Pair.Value.Relationships.Remove(TeamID);
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Unregistered team %s"),
		*TeamID.TeamName.ToString());
}

bool UHarmoniaTeamManagementSubsystem::IsTeamRegistered(const FHarmoniaTeamIdentification& TeamID) const
{
	return RegisteredTeams.Contains(TeamID);
}

UHarmoniaTeamConfigData* UHarmoniaTeamManagementSubsystem::GetTeamConfig(const FHarmoniaTeamIdentification& TeamID) const
{
	if (const TObjectPtr<UHarmoniaTeamConfigData>* ConfigPtr = RegisteredTeams.Find(TeamID))
	{
		return *ConfigPtr;
	}
	return nullptr;
}

TArray<FHarmoniaTeamIdentification> UHarmoniaTeamManagementSubsystem::GetAllTeams() const
{
	TArray<FHarmoniaTeamIdentification> Teams;
	RegisteredTeams.GetKeys(Teams);
	return Teams;
}

bool UHarmoniaTeamManagementSubsystem::LoadTeamConfig(UHarmoniaTeamConfigData* TeamConfig)
{
	if (!TeamConfig)
	{
		return false;
	}

	return RegisterTeam(TeamConfig->TeamID, TeamConfig);
}

void UHarmoniaTeamManagementSubsystem::LoadTeamConfigs(const TArray<UHarmoniaTeamConfigData*>& TeamConfigs)
{
	for (UHarmoniaTeamConfigData* Config : TeamConfigs)
	{
		LoadTeamConfig(Config);
	}
}

void UHarmoniaTeamManagementSubsystem::LoadTeamSetup(UHarmoniaTeamSetupData* TeamSetupData, bool bClearExisting)
{
	if (!TeamSetupData)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaTeamManagementSubsystem: LoadTeamSetup called with null data"));
		return;
	}

	if (bClearExisting)
	{
		ClearAllTeams();
	}

	// Register all teams
	for (const FHarmoniaTeamSetupEntry& Entry : TeamSetupData->Teams)
	{
		if (Entry.TeamID.IsValid())
		{
			RegisterTeam(Entry.TeamID, Entry.TeamConfig);

			// Store FriendlyFire setting
			if (Entry.TeamID.TeamID.IsValid())
			{
				TeamFriendlyFireMap.Add(Entry.TeamID.TeamID, Entry.bAllowFriendlyFire);
			}
		}
	}

	// Set all relationships
	for (const FHarmoniaTeamRelationshipEntry& RelEntry : TeamSetupData->Relationships)
	{
		if (!RelEntry.TeamA.IsValid() || !RelEntry.TeamB.IsValid())
		{
			continue;
		}

		// Find team identifications by tag
		FHarmoniaTeamIdentification TeamA, TeamB;
		bool bFoundA = false, bFoundB = false;

		for (const FHarmoniaTeamSetupEntry& Entry : TeamSetupData->Teams)
		{
			if (Entry.TeamID.TeamID == RelEntry.TeamA)
			{
				TeamA = Entry.TeamID;
				bFoundA = true;
			}
			if (Entry.TeamID.TeamID == RelEntry.TeamB)
			{
				TeamB = Entry.TeamID;
				bFoundB = true;
			}
			if (bFoundA && bFoundB) break;
		}

		if (bFoundA && bFoundB)
		{
			if (RelEntry.bBidirectional)
			{
				SetMutualRelationship(TeamA, TeamB, RelEntry.Relationship);
			}
			else
			{
				SetTeamRelationship(TeamA, TeamB, RelEntry.Relationship);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Loaded team setup with %d teams and %d relationships"),
		TeamSetupData->Teams.Num(), TeamSetupData->Relationships.Num());
}

bool UHarmoniaTeamManagementSubsystem::DoesTeamAllowFriendlyFire(FGameplayTag TeamTag) const
{
	if (const bool* bAllowFF = TeamFriendlyFireMap.Find(TeamTag))
	{
		return *bAllowFF;
	}
	return false; // Default: no friendly fire
}

// ============================================================================
// Team Relationship Management
// ============================================================================

void UHarmoniaTeamManagementSubsystem::SetTeamRelationship(const FHarmoniaTeamIdentification& SourceTeam,
	const FHarmoniaTeamIdentification& TargetTeam,
	EHarmoniaTeamRelationship Relationship)
{
	if (!SourceTeam.IsValid() || !TargetTeam.IsValid())
	{
		return;
	}

	// Get or create source team's relationship map
	FHarmoniaTeamRelationshipMap& SourceRelationshipMap =
		RelationshipMatrix.FindOrAdd(SourceTeam);

	// Set relationship
	SourceRelationshipMap.Relationships.Add(TargetTeam, Relationship);

	// Broadcast change
	OnTeamRelationshipChanged.Broadcast(SourceTeam, TargetTeam, Relationship);

	UE_LOG(LogTemp, Verbose, TEXT("HarmoniaTeamManagementSubsystem: Set relationship %s -> %s: %s"),
		*SourceTeam.TeamName.ToString(),
		*TargetTeam.TeamName.ToString(),
		*UEnum::GetValueAsString(Relationship));
}

EHarmoniaTeamRelationship UHarmoniaTeamManagementSubsystem::GetTeamRelationship(
	const FHarmoniaTeamIdentification& SourceTeam,
	const FHarmoniaTeamIdentification& TargetTeam) const
{
	if (!SourceTeam.IsValid() || !TargetTeam.IsValid())
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	// Same team check
	if (SourceTeam.IsSameTeam(TargetTeam))
	{
		// Check if friendly fire is enabled
		if (UHarmoniaTeamConfigData* Config = GetTeamConfig(SourceTeam))
		{
			return Config->bAllowFriendlyFire ? EHarmoniaTeamRelationship::Neutral : EHarmoniaTeamRelationship::Ally;
		}
		return EHarmoniaTeamRelationship::Ally;
	}

	// Check explicit relationship
	if (const FHarmoniaTeamRelationshipMap* SourceRelationshipMap =
		RelationshipMatrix.Find(SourceTeam))
	{
		if (const EHarmoniaTeamRelationship* RelationshipPtr = SourceRelationshipMap->Relationships.Find(TargetTeam))
		{
			return *RelationshipPtr;
		}
	}

	// Fall back to default relationship
	return GetDefaultRelationship(SourceTeam, TargetTeam);
}

bool UHarmoniaTeamManagementSubsystem::AreTeamsAllies(const FHarmoniaTeamIdentification& TeamA,
	const FHarmoniaTeamIdentification& TeamB) const
{
	return GetTeamRelationship(TeamA, TeamB) == EHarmoniaTeamRelationship::Ally;
}

bool UHarmoniaTeamManagementSubsystem::AreTeamsEnemies(const FHarmoniaTeamIdentification& TeamA,
	const FHarmoniaTeamIdentification& TeamB) const
{
	return GetTeamRelationship(TeamA, TeamB) == EHarmoniaTeamRelationship::Enemy;
}

bool UHarmoniaTeamManagementSubsystem::CanTeamAttack(const FHarmoniaTeamIdentification& SourceTeam,
	const FHarmoniaTeamIdentification& TargetTeam) const
{
	EHarmoniaTeamRelationship Relationship = GetTeamRelationship(SourceTeam, TargetTeam);
	return Relationship == EHarmoniaTeamRelationship::Enemy;
}

bool UHarmoniaTeamManagementSubsystem::ShouldTeamHelp(const FHarmoniaTeamIdentification& SourceTeam,
	const FHarmoniaTeamIdentification& TargetTeam) const
{
	EHarmoniaTeamRelationship Relationship = GetTeamRelationship(SourceTeam, TargetTeam);
	return Relationship == EHarmoniaTeamRelationship::Ally;
}

void UHarmoniaTeamManagementSubsystem::SetMutualRelationship(const FHarmoniaTeamIdentification& TeamA,
	const FHarmoniaTeamIdentification& TeamB,
	EHarmoniaTeamRelationship Relationship)
{
	SetTeamRelationship(TeamA, TeamB, Relationship);
	SetTeamRelationship(TeamB, TeamA, Relationship);
}

void UHarmoniaTeamManagementSubsystem::MakeTeamsAllies(const FHarmoniaTeamIdentification& TeamA,
	const FHarmoniaTeamIdentification& TeamB)
{
	SetMutualRelationship(TeamA, TeamB, EHarmoniaTeamRelationship::Ally);
}

void UHarmoniaTeamManagementSubsystem::MakeTeamsEnemies(const FHarmoniaTeamIdentification& TeamA,
	const FHarmoniaTeamIdentification& TeamB)
{
	SetMutualRelationship(TeamA, TeamB, EHarmoniaTeamRelationship::Enemy);
}

// ============================================================================
// Actor-Based Queries
// ============================================================================

FHarmoniaTeamIdentification UHarmoniaTeamManagementSubsystem::GetActorTeamID(AActor* Actor) const
{
	if (!Actor)
	{
		return FHarmoniaTeamIdentification();
	}

	// Check if actor implements team interface
	if (Actor->Implements<UHarmoniaTeamAgentInterface>())
	{
		return IHarmoniaTeamAgentInterface::Execute_GetTeamID(Actor);
	}

	return FHarmoniaTeamIdentification();
}

EHarmoniaTeamRelationship UHarmoniaTeamManagementSubsystem::GetActorRelationship(AActor* SourceActor,
	AActor* TargetActor) const
{
	if (!SourceActor || !TargetActor)
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	FHarmoniaTeamIdentification SourceTeam = GetActorTeamID(SourceActor);
	FHarmoniaTeamIdentification TargetTeam = GetActorTeamID(TargetActor);

	if (!SourceTeam.IsValid() || !TargetTeam.IsValid())
	{
		return EHarmoniaTeamRelationship::Neutral;
	}

	return GetTeamRelationship(SourceTeam, TargetTeam);
}

bool UHarmoniaTeamManagementSubsystem::AreSameTeam(AActor* ActorA, AActor* ActorB) const
{
	if (!ActorA || !ActorB)
	{
		return false;
	}

	FHarmoniaTeamIdentification TeamA = GetActorTeamID(ActorA);
	FHarmoniaTeamIdentification TeamB = GetActorTeamID(ActorB);

	return TeamA.IsValid() && TeamB.IsValid() && TeamA.IsSameTeam(TeamB);
}

bool UHarmoniaTeamManagementSubsystem::AreAllies(AActor* ActorA, AActor* ActorB) const
{
	return GetActorRelationship(ActorA, ActorB) == EHarmoniaTeamRelationship::Ally;
}

bool UHarmoniaTeamManagementSubsystem::AreEnemies(AActor* ActorA, AActor* ActorB) const
{
	return GetActorRelationship(ActorA, ActorB) == EHarmoniaTeamRelationship::Enemy;
}

bool UHarmoniaTeamManagementSubsystem::CanActorAttack(AActor* SourceActor, AActor* TargetActor) const
{
	return GetActorRelationship(SourceActor, TargetActor) == EHarmoniaTeamRelationship::Enemy;
}

bool UHarmoniaTeamManagementSubsystem::ShouldActorHelp(AActor* SourceActor, AActor* TargetActor) const
{
	return GetActorRelationship(SourceActor, TargetActor) == EHarmoniaTeamRelationship::Ally;
}

TArray<AActor*> UHarmoniaTeamManagementSubsystem::GetActorsOnTeam(const FHarmoniaTeamIdentification& TeamID,
	TSubclassOf<AActor> ActorClass) const
{
	TArray<AActor*> Result;

	if (!TeamID.IsValid())
	{
		return Result;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Result;
	}

	// Get all actors of specified class (or all actors if no class specified)
	TArray<AActor*> AllActors;
	TSubclassOf<AActor> SearchClass = ActorClass ? ActorClass : TSubclassOf<AActor>(AActor::StaticClass());
	UGameplayStatics::GetAllActorsOfClass(World, SearchClass, AllActors);

	// Filter by team
	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->Implements<UHarmoniaTeamAgentInterface>())
		{
			FHarmoniaTeamIdentification ActorTeam = IHarmoniaTeamAgentInterface::Execute_GetTeamID(Actor);
			if (ActorTeam.IsSameTeam(TeamID))
			{
				Result.Add(Actor);
			}
		}
	}

	return Result;
}

TArray<AActor*> UHarmoniaTeamManagementSubsystem::GetAlliesOf(AActor* Actor, float SearchRadius,
	TSubclassOf<AActor> ActorClass) const
{
	TArray<AActor*> Result;

	if (!Actor)
	{
		return Result;
	}

	FHarmoniaTeamIdentification ActorTeam = GetActorTeamID(Actor);
	if (!ActorTeam.IsValid())
	{
		return Result;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Result;
	}

	TArray<AActor*> AllActors;
	TSubclassOf<AActor> SearchClass = ActorClass ? ActorClass : TSubclassOf<AActor>(AActor::StaticClass());
	UGameplayStatics::GetAllActorsOfClass(World, SearchClass, AllActors);

	FVector ActorLocation = Actor->GetActorLocation();
	float SearchRadiusSqr = SearchRadius * SearchRadius;

	for (AActor* OtherActor : AllActors)
	{
		if (!OtherActor || OtherActor == Actor)
		{
			continue;
		}

		// Check distance if radius specified
		if (SearchRadius > 0.0f)
		{
			float DistanceSqr = FVector::DistSquared(ActorLocation, OtherActor->GetActorLocation());
			if (DistanceSqr > SearchRadiusSqr)
			{
				continue;
			}
		}

		// Check if ally
		if (AreAllies(Actor, OtherActor))
		{
			Result.Add(OtherActor);
		}
	}

	return Result;
}

TArray<AActor*> UHarmoniaTeamManagementSubsystem::GetEnemiesOf(AActor* Actor, float SearchRadius,
	TSubclassOf<AActor> ActorClass) const
{
	TArray<AActor*> Result;

	if (!Actor)
	{
		return Result;
	}

	FHarmoniaTeamIdentification ActorTeam = GetActorTeamID(Actor);
	if (!ActorTeam.IsValid())
	{
		return Result;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Result;
	}

	TArray<AActor*> AllActors;
	TSubclassOf<AActor> SearchClass = ActorClass ? ActorClass : TSubclassOf<AActor>(AActor::StaticClass());
	UGameplayStatics::GetAllActorsOfClass(World, SearchClass, AllActors);

	FVector ActorLocation = Actor->GetActorLocation();
	float SearchRadiusSqr = SearchRadius * SearchRadius;

	for (AActor* OtherActor : AllActors)
	{
		if (!OtherActor || OtherActor == Actor)
		{
			continue;
		}

		// Check distance if radius specified
		if (SearchRadius > 0.0f)
		{
			float DistanceSqr = FVector::DistSquared(ActorLocation, OtherActor->GetActorLocation());
			if (DistanceSqr > SearchRadiusSqr)
			{
				continue;
			}
		}

		// Check if enemy
		if (AreEnemies(Actor, OtherActor))
		{
			Result.Add(OtherActor);
		}
	}

	return Result;
}

// ============================================================================
// Utility Functions
// ============================================================================

FHarmoniaTeamIdentification UHarmoniaTeamManagementSubsystem::CreateTeamID(FGameplayTag TeamTag, FText TeamName)
{
	FHarmoniaTeamIdentification TeamID;
	TeamID.TeamID = TeamTag;
	TeamID.TeamName = TeamName.IsEmpty() ? FText::FromString(TeamTag.ToString()) : TeamName;
	return TeamID;
}

void UHarmoniaTeamManagementSubsystem::ResetAllRelationships()
{
	RelationshipMatrix.Empty();
	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Reset all relationships"));
}

void UHarmoniaTeamManagementSubsystem::ClearAllTeams()
{
	RegisteredTeams.Empty();
	RelationshipMatrix.Empty();
	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Cleared all teams"));
}

// ============================================================================
// Debug Functions
// ============================================================================

void UHarmoniaTeamManagementSubsystem::DebugPrintTeams() const
{
	UE_LOG(LogTemp, Log, TEXT("========== REGISTERED TEAMS =========="));
	for (const auto& Pair : RegisteredTeams)
	{
		const FHarmoniaTeamIdentification& TeamID = Pair.Key;
		UE_LOG(LogTemp, Log, TEXT("Team: %s (Tag: %s)"),
			*TeamID.TeamName.ToString(),
			*TeamID.TeamID.ToString());
	}
	UE_LOG(LogTemp, Log, TEXT("======================================"));
}

void UHarmoniaTeamManagementSubsystem::DebugPrintRelationshipMatrix() const
{
	UE_LOG(LogTemp, Log, TEXT("========== RELATIONSHIP MATRIX =========="));
	for (const auto& SourcePair : RelationshipMatrix)
	{
		const FHarmoniaTeamIdentification& SourceTeam = SourcePair.Key;
		UE_LOG(LogTemp, Log, TEXT("Source Team: %s"), *SourceTeam.TeamName.ToString());

		for (const auto& TargetPair : SourcePair.Value.Relationships)
		{
			const FHarmoniaTeamIdentification& TargetTeam = TargetPair.Key;
			const EHarmoniaTeamRelationship& Relationship = TargetPair.Value;
			UE_LOG(LogTemp, Log, TEXT("  -> %s: %s"),
				*TargetTeam.TeamName.ToString(),
				*UEnum::GetValueAsString(Relationship));
		}
	}
	UE_LOG(LogTemp, Log, TEXT("========================================="));
}

// ============================================================================
// Advanced Team Management (Dynamic Faction System)
// ============================================================================

FHarmoniaTeamIdentification UHarmoniaTeamManagementSubsystem::CreateFaction(FText FactionName,
	EHarmoniaTeamAttitude DefaultAttitude,
	FLinearColor TeamColor)
{
	FHarmoniaTeamIdentification NewFaction;
	// 팩션은 동적으로 생성되므로 태그가 없을 수 있음 - TeamName으로 식별
	NewFaction.TeamName = FactionName;
	NewFaction.TeamColor = TeamColor;
	NewFaction.DefaultAttitude = DefaultAttitude;

	// Register the faction
	RegisterTeam(NewFaction);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Created faction '%s'"),
		*FactionName.ToString());

	return NewFaction;
}

TArray<FHarmoniaTeamIdentification> UHarmoniaTeamManagementSubsystem::CreateFactionNetwork(
	const TArray<UHarmoniaTeamConfigData*>& FactionDefinitions)
{
	TArray<FHarmoniaTeamIdentification> CreatedFactions;

	// First pass: Create all factions
	for (UHarmoniaTeamConfigData* FactionConfig : FactionDefinitions)
	{
		if (FactionConfig && RegisterTeam(FactionConfig->TeamID, FactionConfig))
		{
			CreatedFactions.Add(FactionConfig->TeamID);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Created faction network with %d factions"),
		CreatedFactions.Num());

	return CreatedFactions;
}

void UHarmoniaTeamManagementSubsystem::SetFactionHostileToMany(const FHarmoniaTeamIdentification& SourceTeam,
	const TArray<FHarmoniaTeamIdentification>& HostileFactions)
{
	for (const FHarmoniaTeamIdentification& HostileTeam : HostileFactions)
	{
		MakeTeamsEnemies(SourceTeam, HostileTeam);
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Set faction '%s' hostile to %d factions"),
		*SourceTeam.TeamName.ToString(), HostileFactions.Num());
}

void UHarmoniaTeamManagementSubsystem::CreateAlliance(const TArray<FHarmoniaTeamIdentification>& AllianceFactions)
{
	// Make all factions allies with each other
	for (int32 i = 0; i < AllianceFactions.Num(); ++i)
	{
		for (int32 j = i + 1; j < AllianceFactions.Num(); ++j)
		{
			MakeTeamsAllies(AllianceFactions[i], AllianceFactions[j]);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Created alliance with %d factions"),
		AllianceFactions.Num());
}

void UHarmoniaTeamManagementSubsystem::BreakAlliance(const FHarmoniaTeamIdentification& TeamA,
	const FHarmoniaTeamIdentification& TeamB)
{
	SetMutualRelationship(TeamA, TeamB, EHarmoniaTeamRelationship::Neutral);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Broke alliance between '%s' and '%s'"),
		*TeamA.TeamName.ToString(), *TeamB.TeamName.ToString());
}

TArray<FHarmoniaTeamIdentification> UHarmoniaTeamManagementSubsystem::GetEnemyFactions(
	const FHarmoniaTeamIdentification& Team) const
{
	TArray<FHarmoniaTeamIdentification> Enemies;

	if (const FHarmoniaTeamRelationshipMap* TeamRelationshipMap =
		RelationshipMatrix.Find(Team))
	{
		for (const auto& Pair : TeamRelationshipMap->Relationships)
		{
			if (Pair.Value == EHarmoniaTeamRelationship::Enemy)
			{
				Enemies.Add(Pair.Key);
			}
		}
	}

	return Enemies;
}

TArray<FHarmoniaTeamIdentification> UHarmoniaTeamManagementSubsystem::GetAlliedFactions(
	const FHarmoniaTeamIdentification& Team) const
{
	TArray<FHarmoniaTeamIdentification> Allies;

	if (const FHarmoniaTeamRelationshipMap* TeamRelationshipMap =
		RelationshipMatrix.Find(Team))
	{
		for (const auto& Pair : TeamRelationshipMap->Relationships)
		{
			if (Pair.Value == EHarmoniaTeamRelationship::Ally)
			{
				Allies.Add(Pair.Key);
			}
		}
	}

	return Allies;
}

void UHarmoniaTeamManagementSubsystem::ChangeFactionRelationship(const FHarmoniaTeamIdentification& FactionA,
	const FHarmoniaTeamIdentification& FactionB,
	EHarmoniaTeamRelationship NewRelationship,
	bool bBidirectional)
{
	if (bBidirectional)
	{
		SetMutualRelationship(FactionA, FactionB, NewRelationship);
	}
	else
	{
		SetTeamRelationship(FactionA, FactionB, NewRelationship);
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaTeamManagementSubsystem: Changed relationship between '%s' and '%s' to %s (%s)"),
		*FactionA.TeamName.ToString(),
		*FactionB.TeamName.ToString(),
		*UEnum::GetValueAsString(NewRelationship),
		bBidirectional ? TEXT("bidirectional") : TEXT("unidirectional"));
}

int32 UHarmoniaTeamManagementSubsystem::GetFactionCount() const
{
	return RegisteredTeams.Num();
}

// ============================================================================
// Helper Functions
// ============================================================================

EHarmoniaTeamRelationship UHarmoniaTeamManagementSubsystem::GetDefaultRelationship(
	const FHarmoniaTeamIdentification& SourceTeam,
	const FHarmoniaTeamIdentification& TargetTeam) const
{
	// Check team config for default attitude
	if (UHarmoniaTeamConfigData* Config = GetTeamConfig(SourceTeam))
	{
		return Config->GetRelationshipWith(TargetTeam);
	}

	// Default to neutral
	return EHarmoniaTeamRelationship::Neutral;
}
