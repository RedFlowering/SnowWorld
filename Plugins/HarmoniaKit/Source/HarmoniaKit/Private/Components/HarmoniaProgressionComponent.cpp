// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaProgressionComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Data/HarmoniaSkillTreeData.h"
#include "Data/HarmoniaClassData.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"
#include "GameplayAbilitySpec.h"

UHarmoniaProgressionComponent::UHarmoniaProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// Default values
	CurrentExperience = 0;
	CurrentLevel = 1;
	AvailableSkillPoints = 0;
	AvailableStatPoints = 0;
	CurrentClass = EHarmoniaCharacterClass::None;
	CurrentAwakeningTier = EHarmoniaAwakeningTier::Base;
	PrestigeLevel = 0;

	// Config defaults
	SkillPointsPerLevel = 1;
	StatPointsPerLevel = 2;
	MaxLevel = 100;
}

void UHarmoniaProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize experience curve if not set
	if (ExperienceCurve.Num() == 0)
	{
		InitializeExperienceCurve();
	}
}

void UHarmoniaProgressionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaProgressionComponent, CurrentExperience);
	DOREPLIFETIME(UHarmoniaProgressionComponent, CurrentLevel);
	DOREPLIFETIME(UHarmoniaProgressionComponent, AvailableSkillPoints);
	DOREPLIFETIME(UHarmoniaProgressionComponent, AvailableStatPoints);
	DOREPLIFETIME(UHarmoniaProgressionComponent, CurrentClass);
	DOREPLIFETIME(UHarmoniaProgressionComponent, CurrentAwakeningTier);
	DOREPLIFETIME(UHarmoniaProgressionComponent, PrestigeLevel);
	DOREPLIFETIME(UHarmoniaProgressionComponent, UnlockedSkillNodes);
	DOREPLIFETIME(UHarmoniaProgressionComponent, AllocatedStats);
}

//~ Experience & Leveling

void UHarmoniaProgressionComponent::AddExperience(int32 Amount)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_AddExperience(Amount);
	}
}

void UHarmoniaProgressionComponent::Server_AddExperience_Implementation(int32 Amount)
{
	if (Amount <= 0 || CurrentLevel >= MaxLevel)
	{
		return;
	}

	// Apply prestige multiplier
	if (PrestigeLevel > 0)
	{
		FHarmoniaPrestigeDefinition PrestigeDef = GetCurrentPrestigeDefinition();
		Amount = FMath::RoundToInt(Amount * PrestigeDef.RewardMultiplier);
	}

	CurrentExperience += Amount;

	const int32 RequiredExp = GetRequiredExperienceForNextLevel();
	OnExperienceGained.Broadcast(Amount, CurrentExperience, RequiredExp);

	// Auto level up if enough experience
	while (CanLevelUp())
	{
		ProcessLevelUp();
	}
}

int32 UHarmoniaProgressionComponent::GetRequiredExperienceForNextLevel() const
{
	if (CurrentLevel >= MaxLevel)
	{
		return 0;
	}

	if (ExperienceCurve.Contains(CurrentLevel))
	{
		return ExperienceCurve[CurrentLevel];
	}

	return CalculateRequiredExperience(CurrentLevel);
}

float UHarmoniaProgressionComponent::GetExperienceProgress() const
{
	const int32 RequiredExp = GetRequiredExperienceForNextLevel();
	if (RequiredExp <= 0)
	{
		return 1.0f;
	}

	return FMath::Clamp(static_cast<float>(CurrentExperience) / RequiredExp, 0.0f, 1.0f);
}

bool UHarmoniaProgressionComponent::CanLevelUp() const
{
	if (CurrentLevel >= MaxLevel)
	{
		return false;
	}

	const int32 RequiredExp = GetRequiredExperienceForNextLevel();
	return CurrentExperience >= RequiredExp;
}

void UHarmoniaProgressionComponent::TryLevelUp()
{
	if (GetOwnerRole() == ROLE_Authority && CanLevelUp())
	{
		ProcessLevelUp();
	}
}

void UHarmoniaProgressionComponent::ProcessLevelUp()
{
	if (CurrentLevel >= MaxLevel)
	{
		return;
	}

	// Deduct experience
	const int32 RequiredExp = GetRequiredExperienceForNextLevel();
	CurrentExperience -= RequiredExp;

	// Increase level
	CurrentLevel++;

	// Award points
	const int32 SkillPointsGained = SkillPointsPerLevel;
	const int32 StatPointsGained = StatPointsPerLevel;

	AvailableSkillPoints += SkillPointsGained;
	AvailableStatPoints += StatPointsGained;

	// Broadcast level up
	OnLevelUp.Broadcast(CurrentLevel, SkillPointsGained);
	OnSkillPointsChanged.Broadcast(AvailableSkillPoints, SkillPointsGained);
	OnStatPointsChanged.Broadcast(AvailableStatPoints, StatPointsGained);

	// Apply class stat growth
	if (ClassDataAsset)
	{
		const FHarmoniaClassDefinition* ClassDef = ClassDataAsset->GetClassDefinition(CurrentClass);
		if (ClassDef)
		{
			for (const auto& StatGrowth : ClassDef->StatGrowthPerLevel)
			{
				ApplyStatAllocation(StatGrowth.Key, FMath::RoundToInt(StatGrowth.Value));
			}
		}
	}
}

void UHarmoniaProgressionComponent::InitializeExperienceCurve()
{
	// Generate default exponential experience curve
	const float BaseExp = 100.0f;
	const float Exponent = 1.5f;
	const float Multiplier = 1.1f;

	ExperienceCurve.Empty();
	for (int32 Level = 1; Level < MaxLevel; ++Level)
	{
		const int32 RequiredExp = CalculateRequiredExperience(Level);
		ExperienceCurve.Add(Level, RequiredExp);
	}
}

int32 UHarmoniaProgressionComponent::CalculateRequiredExperience(int32 Level) const
{
	// Exponential curve: BaseExp * (Level^1.5) * 1.1
	const float BaseExp = 100.0f;
	const float Exponent = 1.5f;
	const float Multiplier = 1.1f;

	return FMath::RoundToInt(BaseExp * FMath::Pow(Level, Exponent) * Multiplier);
}

//~ Skill Tree System

bool UHarmoniaProgressionComponent::UnlockSkillNode(FName NodeID, int32 InvestmentPoints)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_UnlockSkillNode(NodeID, InvestmentPoints);
		return true;
	}
	return false;
}

void UHarmoniaProgressionComponent::Server_UnlockSkillNode_Implementation(FName NodeID, int32 InvestmentPoints)
{
	if (!CanUnlockSkillNode(NodeID))
	{
		return;
	}

	FHarmoniaSkillNode* SkillNode = FindSkillNode(NodeID);
	if (!SkillNode)
	{
		return;
	}

	// Get current investment
	const int32 CurrentInvestment = GetSkillNodeInvestment(NodeID);
	const int32 NewInvestment = CurrentInvestment + InvestmentPoints;

	// Check max investment
	if (NewInvestment > SkillNode->MaxInvestmentPoints)
	{
		return;
	}

	// Check skill points
	const int32 RequiredPoints = SkillNode->RequiredSkillPoints * InvestmentPoints;
	if (AvailableSkillPoints < RequiredPoints)
	{
		return;
	}

	// Deduct skill points
	AvailableSkillPoints -= RequiredPoints;

	// Update investment
	FSkillNodeInvestment* ExistingEntry = UnlockedSkillNodes.FindByPredicate([NodeID](const FSkillNodeInvestment& Entry)
	{
		return Entry.NodeID == NodeID;
	});

	if (ExistingEntry)
	{
		ExistingEntry->InvestedPoints = NewInvestment;
	}
	else
	{
		UnlockedSkillNodes.Add(FSkillNodeInvestment(NodeID, NewInvestment));
	}

	// Apply skill effects
	ApplySkillNodeEffects(*SkillNode);

	// Broadcast
	OnSkillNodeUnlocked.Broadcast(NodeID, InvestmentPoints, NewInvestment);
	OnSkillPointsChanged.Broadcast(AvailableSkillPoints, -RequiredPoints);
}

bool UHarmoniaProgressionComponent::CanUnlockSkillNode(FName NodeID) const
{
	FHarmoniaSkillNode* SkillNode = FindSkillNode(NodeID);
	if (!SkillNode)
	{
		return false;
	}

	// Check level requirement
	if (CurrentLevel < SkillNode->RequiredLevel)
	{
		return false;
	}

	// Check class requirement
	if (SkillNode->RequiredClass != EHarmoniaCharacterClass::None &&
		SkillNode->RequiredClass != CurrentClass)
	{
		return false;
	}

	// Check prerequisites
	if (!ArePrerequisitesMet(*SkillNode))
	{
		return false;
	}

	// Check max investment
	const int32 CurrentInvestment = GetSkillNodeInvestment(NodeID);
	if (CurrentInvestment >= SkillNode->MaxInvestmentPoints)
	{
		return false;
	}

	// Check skill points
	if (AvailableSkillPoints < SkillNode->RequiredSkillPoints)
	{
		return false;
	}

	return true;
}

int32 UHarmoniaProgressionComponent::GetSkillNodeInvestment(FName NodeID) const
{
	const FSkillNodeInvestment* Entry = UnlockedSkillNodes.FindByPredicate([NodeID](const FSkillNodeInvestment& Node)
	{
		return Node.NodeID == NodeID;
	});

	return Entry ? Entry->InvestedPoints : 0;
}

bool UHarmoniaProgressionComponent::IsSkillNodeUnlocked(FName NodeID) const
{
	return UnlockedSkillNodes.ContainsByPredicate([NodeID](const FSkillNodeInvestment& Node)
	{
		return Node.NodeID == NodeID;
	});
}

TArray<FName> UHarmoniaProgressionComponent::GetUnlockedSkillNodes() const
{
	TArray<FName> Result;
	for (const FSkillNodeInvestment& Entry : UnlockedSkillNodes)
	{
		Result.Add(Entry.NodeID);
	}
	return Result;
}

void UHarmoniaProgressionComponent::ResetSkillTree(bool bRefundPoints)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_ResetSkillTree(bRefundPoints);
	}
}

void UHarmoniaProgressionComponent::Server_ResetSkillTree_Implementation(bool bRefundPoints)
{
	if (bRefundPoints)
	{
		// Calculate total invested points
		int32 TotalInvestedPoints = 0;
		for (const FSkillNodeInvestment& Entry : UnlockedSkillNodes)
		{
			FHarmoniaSkillNode* SkillNode = FindSkillNode(Entry.NodeID);
			if (SkillNode)
			{
				TotalInvestedPoints += SkillNode->RequiredSkillPoints * Entry.InvestedPoints;
			}
		}

		// Refund points
		AvailableSkillPoints += TotalInvestedPoints;
		OnSkillPointsChanged.Broadcast(AvailableSkillPoints, TotalInvestedPoints);
	}

	// Remove all skill effects
	for (const FSkillNodeInvestment& Entry : UnlockedSkillNodes)
	{
		FHarmoniaSkillNode* SkillNode = FindSkillNode(Entry.NodeID);
		if (SkillNode)
		{
			RemoveSkillNodeEffects(*SkillNode);
		}
	}

	// Clear unlocked nodes
	UnlockedSkillNodes.Empty();
}

//~ Stat Allocation

bool UHarmoniaProgressionComponent::AllocateStatPoints(FGameplayTag StatTag, int32 Points)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_AllocateStatPoints(StatTag, Points);
		return true;
	}
	return false;
}

void UHarmoniaProgressionComponent::Server_AllocateStatPoints_Implementation(FGameplayTag StatTag, int32 Points)
{
	if (Points <= 0 || AvailableStatPoints < Points)
	{
		return;
	}

	// Deduct stat points
	AvailableStatPoints -= Points;

	// Add to allocated stats
	FStatAllocation* ExistingEntry = AllocatedStats.FindByPredicate([StatTag](const FStatAllocation& Entry)
	{
		return Entry.StatTag == StatTag;
	});

	if (ExistingEntry)
	{
		ExistingEntry->AllocatedPoints += Points;
	}
	else
	{
		AllocatedStats.Add(FStatAllocation(StatTag, Points));
	}

	// Apply stat to attribute set
	ApplyStatAllocation(StatTag, Points);

	// Broadcast
	OnStatPointsChanged.Broadcast(AvailableStatPoints, -Points);
}

int32 UHarmoniaProgressionComponent::GetAllocatedStatPoints(FGameplayTag StatTag) const
{
	const FStatAllocation* Entry = AllocatedStats.FindByPredicate([StatTag](const FStatAllocation& Stat)
	{
		return Stat.StatTag == StatTag;
	});

	return Entry ? Entry->AllocatedPoints : 0;
}

void UHarmoniaProgressionComponent::ResetStatAllocations(bool bRefundPoints)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_ResetStatAllocations(bRefundPoints);
	}
}

void UHarmoniaProgressionComponent::Server_ResetStatAllocations_Implementation(bool bRefundPoints)
{
	if (bRefundPoints)
	{
		// Calculate total invested points
		int32 TotalInvestedPoints = 0;
		for (const FStatAllocation& Entry : AllocatedStats)
		{
			TotalInvestedPoints += Entry.AllocatedPoints;
		}

		// Refund points
		AvailableStatPoints += TotalInvestedPoints;
		OnStatPointsChanged.Broadcast(AvailableStatPoints, TotalInvestedPoints);
	}

	// Clear allocations
	AllocatedStats.Empty();
}

//~ Class System

bool UHarmoniaProgressionComponent::ChangeClass(EHarmoniaCharacterClass NewClass)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_ChangeClass(NewClass);
		return true;
	}
	return false;
}

void UHarmoniaProgressionComponent::Server_ChangeClass_Implementation(EHarmoniaCharacterClass NewClass)
{
	if (!CanChangeClass(NewClass))
	{
		return;
	}

	const EHarmoniaCharacterClass OldClass = CurrentClass;

	// Remove old class effects
	if (OldClass != EHarmoniaCharacterClass::None)
	{
		RemoveClassEffects(OldClass);
	}

	// Set new class
	CurrentClass = NewClass;

	// Apply new class effects
	if (NewClass != EHarmoniaCharacterClass::None)
	{
		ApplyClassEffects(NewClass);
	}

	// Broadcast
	OnClassChanged.Broadcast(OldClass, NewClass);
}

bool UHarmoniaProgressionComponent::CanChangeClass(EHarmoniaCharacterClass NewClass) const
{
	if (NewClass == CurrentClass)
	{
		return false;
	}

	if (!ClassDataAsset)
	{
		return false;
	}

	const FHarmoniaClassDefinition* NewClassDef = ClassDataAsset->GetClassDefinition(NewClass);
	if (!NewClassDef)
	{
		return false;
	}

	// If changing from a class (advancement), check requirements
	if (CurrentClass != EHarmoniaCharacterClass::None)
	{
		const FHarmoniaClassDefinition* CurrentClassDef = ClassDataAsset->GetClassDefinition(CurrentClass);
		if (CurrentClassDef)
		{
			// Check if new class is in advancement list
			if (!CurrentClassDef->AdvancementClasses.Contains(NewClass))
			{
				return false;
			}

			// Check level requirement
			if (CurrentLevel < CurrentClassDef->RequiredLevelForAdvancement)
			{
				return false;
			}
		}
	}

	return true;
}

TArray<EHarmoniaCharacterClass> UHarmoniaProgressionComponent::GetAvailableAdvancementClasses() const
{
	TArray<EHarmoniaCharacterClass> Result;

	if (!ClassDataAsset || CurrentClass == EHarmoniaCharacterClass::None)
	{
		return Result;
	}

	const FHarmoniaClassDefinition* CurrentClassDef = ClassDataAsset->GetClassDefinition(CurrentClass);
	if (CurrentClassDef && CurrentLevel >= CurrentClassDef->RequiredLevelForAdvancement)
	{
		Result = CurrentClassDef->AdvancementClasses;
	}

	return Result;
}

//~ Awakening System

bool UHarmoniaProgressionComponent::TriggerAwakening(EHarmoniaAwakeningTier NewTier)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_TriggerAwakening(NewTier);
		return true;
	}
	return false;
}

void UHarmoniaProgressionComponent::Server_TriggerAwakening_Implementation(EHarmoniaAwakeningTier NewTier)
{
	if (!CanAwaken(NewTier))
	{
		return;
	}

	CurrentAwakeningTier = NewTier;

	// Apply awakening effects
	ApplyAwakeningEffects(NewTier);

	// Award bonus skill points
	const int32 BonusPoints = 5;
	AvailableSkillPoints += BonusPoints;

	// Broadcast
	OnAwakeningAchieved.Broadcast(NewTier, BonusPoints);
	OnSkillPointsChanged.Broadcast(AvailableSkillPoints, BonusPoints);
}

bool UHarmoniaProgressionComponent::CanAwaken(EHarmoniaAwakeningTier TargetTier) const
{
	// Must be next tier
	if (static_cast<int32>(TargetTier) != static_cast<int32>(CurrentAwakeningTier) + 1)
	{
		return false;
	}

	// Check level requirement
	const int32 RequiredLevel = 50 * static_cast<int32>(TargetTier);
	if (CurrentLevel < RequiredLevel)
	{
		return false;
	}

	return true;
}

//~ Prestige / New Game+ System

bool UHarmoniaProgressionComponent::EnterNewGamePlus()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Server_EnterNewGamePlus();
		return true;
	}
	return false;
}

void UHarmoniaProgressionComponent::Server_EnterNewGamePlus_Implementation()
{
	if (!CanEnterNewGamePlus())
	{
		return;
	}

	// Increment prestige level
	PrestigeLevel++;

	// Get prestige bonuses
	FHarmoniaPrestigeDefinition PrestigeDef = GetCurrentPrestigeDefinition();

	// Award bonus points
	AvailableStatPoints += PrestigeDef.BonusStatPoints;
	AvailableSkillPoints += PrestigeDef.BonusSkillPoints;

	// Reset level but keep skills/stats
	CurrentLevel = 1;
	CurrentExperience = 0;

	// Apply prestige effects
	ApplyPrestigeEffects();

	// Broadcast
	OnPrestigeLevelChanged.Broadcast(PrestigeLevel, PrestigeDef.BonusStatPoints, PrestigeDef.BonusSkillPoints);
	OnStatPointsChanged.Broadcast(AvailableStatPoints, PrestigeDef.BonusStatPoints);
	OnSkillPointsChanged.Broadcast(AvailableSkillPoints, PrestigeDef.BonusSkillPoints);
}

bool UHarmoniaProgressionComponent::CanEnterNewGamePlus() const
{
	// Check if reached max level
	if (CurrentLevel < MaxLevel)
	{
		return false;
	}

	return true;
}

FHarmoniaPrestigeDefinition UHarmoniaProgressionComponent::GetCurrentPrestigeDefinition() const
{
	FHarmoniaPrestigeDefinition PrestigeDef;
	PrestigeDef.PrestigeLevel = PrestigeLevel;
	PrestigeDef.EnemyDifficultyMultiplier = 1.0f + (PrestigeLevel * 0.5f);
	PrestigeDef.RewardMultiplier = 1.0f + (PrestigeLevel * 1.0f);
	PrestigeDef.BonusStatPoints = 10 * PrestigeLevel;
	PrestigeDef.BonusSkillPoints = 10 * PrestigeLevel;

	return PrestigeDef;
}

//~ Save/Load Support

FHarmoniaProgressionSaveData UHarmoniaProgressionComponent::ExportProgressionData() const
{
	FHarmoniaProgressionSaveData SaveData;

	SaveData.CurrentExperience = CurrentExperience;
	SaveData.CurrentLevel = CurrentLevel;
	SaveData.AvailableSkillPoints = AvailableSkillPoints;
	SaveData.AvailableStatPoints = AvailableStatPoints;
	SaveData.CurrentClass = CurrentClass;
	SaveData.CurrentAwakeningTier = CurrentAwakeningTier;
	SaveData.PrestigeLevel = PrestigeLevel;
	SaveData.UnlockedSkillNodes = UnlockedSkillNodes;
	SaveData.AllocatedStats = AllocatedStats;
	SaveData.TotalExperienceEarned = CurrentExperience;
	SaveData.TotalLevelsGained = CurrentLevel - 1;

	return SaveData;
}

void UHarmoniaProgressionComponent::ImportProgressionData(const FHarmoniaProgressionSaveData& SavedData)
{
	CurrentExperience = SavedData.CurrentExperience;
	CurrentLevel = SavedData.CurrentLevel;
	AvailableSkillPoints = SavedData.AvailableSkillPoints;
	AvailableStatPoints = SavedData.AvailableStatPoints;
	CurrentClass = SavedData.CurrentClass;
	CurrentAwakeningTier = SavedData.CurrentAwakeningTier;
	PrestigeLevel = SavedData.PrestigeLevel;
	UnlockedSkillNodes = SavedData.UnlockedSkillNodes;
	AllocatedStats = SavedData.AllocatedStats;

	// Reapply all effects
	if (CurrentClass != EHarmoniaCharacterClass::None)
	{
		ApplyClassEffects(CurrentClass);
	}

	for (const FSkillNodeInvestment& Entry : UnlockedSkillNodes)
	{
		FHarmoniaSkillNode* SkillNode = FindSkillNode(Entry.NodeID);
		if (SkillNode)
		{
			ApplySkillNodeEffects(*SkillNode);
		}
	}

	ApplyAwakeningEffects(CurrentAwakeningTier);
	ApplyPrestigeEffects();
}

//~ Internal Helpers

ULyraAbilitySystemComponent* UHarmoniaProgressionComponent::GetAbilitySystemComponent() const
{
	if (CachedAbilitySystemComponent)
	{
		return CachedAbilitySystemComponent;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Try to get from IAbilitySystemInterface
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		CachedAbilitySystemComponent = Cast<ULyraAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}
	else if (AActor* OwnerActor = Owner->GetOwner())
	{
		if (IAbilitySystemInterface* ASI2 = Cast<IAbilitySystemInterface>(OwnerActor))
		{
			CachedAbilitySystemComponent = Cast<ULyraAbilitySystemComponent>(ASI2->GetAbilitySystemComponent());
		}
	}

	return CachedAbilitySystemComponent;
}

FHarmoniaSkillNode* UHarmoniaProgressionComponent::FindSkillNode(FName NodeID) const
{
	for (UHarmoniaSkillTreeData* SkillTreeData : SkillTreeDataAssets)
	{
		if (SkillTreeData)
		{
			FHarmoniaSkillNode* Node = SkillTreeData->FindSkillNode(NodeID);
			if (Node)
			{
				return Node;
			}
		}
	}

	return nullptr;
}

bool UHarmoniaProgressionComponent::ArePrerequisitesMet(const FHarmoniaSkillNode& Node) const
{
	for (const FName& PrereqID : Node.PrerequisiteNodeIDs)
	{
		if (!IsSkillNodeUnlocked(PrereqID))
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaProgressionComponent::GrantAbilitySetToASC(const ULyraAbilitySet* AbilitySet, ULyraAbilitySystemComponent* ASC, FHarmoniaGrantedHandles* OutGrantedHandles)
{
	if (!AbilitySet || !ASC)
	{
		return;
	}

	// Check authority - abilities should only be granted on the server
	if (!ASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Access protected members using reflection
	UClass* AbilitySetClass = ULyraAbilitySet::StaticClass();

	// Get GrantedAttributes array using reflection
	FArrayProperty* AttributesProperty = FindFProperty<FArrayProperty>(AbilitySetClass, TEXT("GrantedAttributes"));
	if (AttributesProperty)
	{
		const TArray<FLyraAbilitySet_AttributeSet>* GrantedAttributes = AttributesProperty->ContainerPtrToValuePtr<TArray<FLyraAbilitySet_AttributeSet>>(AbilitySet);
		if (GrantedAttributes)
		{
			for (int32 SetIndex = 0; SetIndex < GrantedAttributes->Num(); ++SetIndex)
			{
				const FLyraAbilitySet_AttributeSet& SetToGrant = (*GrantedAttributes)[SetIndex];

				if (!IsValid(SetToGrant.AttributeSet))
				{
					continue;
				}

				UAttributeSet* NewSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet);
				ASC->AddAttributeSetSubobject(NewSet);

				if (OutGrantedHandles)
				{
					OutGrantedHandles->GrantedAttributeSets.Add(NewSet);
				}
			}
		}
	}

	// Get GrantedGameplayAbilities array using reflection
	FArrayProperty* AbilitiesProperty = FindFProperty<FArrayProperty>(AbilitySetClass, TEXT("GrantedGameplayAbilities"));
	if (AbilitiesProperty)
	{
		const TArray<FLyraAbilitySet_GameplayAbility>* GrantedAbilities = AbilitiesProperty->ContainerPtrToValuePtr<TArray<FLyraAbilitySet_GameplayAbility>>(AbilitySet);
		if (GrantedAbilities)
		{
			for (int32 AbilityIndex = 0; AbilityIndex < GrantedAbilities->Num(); ++AbilityIndex)
			{
				const FLyraAbilitySet_GameplayAbility& AbilityToGrant = (*GrantedAbilities)[AbilityIndex];

				if (!IsValid(AbilityToGrant.Ability))
				{
				}
			}
		}
	}

	// Get GrantedGameplayEffects array using reflection
	FArrayProperty* EffectsProperty = FindFProperty<FArrayProperty>(AbilitySetClass, TEXT("GrantedGameplayEffects"));
	if (EffectsProperty)
	{
		const TArray<FLyraAbilitySet_GameplayEffect>* GrantedEffects = EffectsProperty->ContainerPtrToValuePtr<TArray<FLyraAbilitySet_GameplayEffect>>(AbilitySet);
		if (GrantedEffects)
		{
			for (int32 EffectIndex = 0; EffectIndex < GrantedEffects->Num(); ++EffectIndex)
			{
				const FLyraAbilitySet_GameplayEffect& EffectToGrant = (*GrantedEffects)[EffectIndex];

				if (!IsValid(EffectToGrant.GameplayEffect))
				{
					continue;
				}

				const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
				const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext());

				if (OutGrantedHandles)
				{
					OutGrantedHandles->GameplayEffectHandles.Add(GameplayEffectHandle);
				}
			}
		}
	}
}

void UHarmoniaProgressionComponent::ApplySkillNodeEffects(const FHarmoniaSkillNode& Node)
{
	ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply GameplayEffects
	for (TSubclassOf<UGameplayEffect> EffectClass : Node.GrantedEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				ActiveProgressionEffects.Add(ActiveHandle);
			}
		}
	}

	// Grant AbilitySet using our direct implementation
	if (Node.GrantedAbilitySet)
	{
		FHarmoniaGrantedHandles GrantedHandles;
		GrantAbilitySetToASC(Node.GrantedAbilitySet, ASC, &GrantedHandles);
		GrantedHandlesList.Add(GrantedHandles);
	}
}

void UHarmoniaProgressionComponent::RemoveSkillNodeEffects(const FHarmoniaSkillNode& Node)
{
	// Simplified - would need to track which handles belong to which node
}

void UHarmoniaProgressionComponent::ApplyStatAllocation(FGameplayTag StatTag, int32 Points)
{
	// Apply stat increase via GameplayEffect or direct attribute modification
}

void UHarmoniaProgressionComponent::ApplyClassEffects(EHarmoniaCharacterClass Class)
{
	if (!ClassDataAsset)
	{
		return;
	}

	const FHarmoniaClassDefinition* ClassDef = ClassDataAsset->GetClassDefinition(Class);
	if (!ClassDef)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Apply class effects
	for (TSubclassOf<UGameplayEffect> EffectClass : ClassDef->ClassEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				ActiveProgressionEffects.Add(ActiveHandle);
			}
		}
	}

	// Grant class abilities using our direct implementation
	if (ClassDef->ClassAbilitySet)
	{
		FHarmoniaGrantedHandles GrantedHandles;
		GrantAbilitySetToASC(ClassDef->ClassAbilitySet, ASC, &GrantedHandles);
		GrantedHandlesList.Add(GrantedHandles);
	}

	// Apply starting stat bonuses
	for (const auto& StatBonus : ClassDef->StartingStatBonuses)
	{
		ApplyStatAllocation(StatBonus.Key, FMath::RoundToInt(StatBonus.Value));
	}
}

void UHarmoniaProgressionComponent::RemoveClassEffects(EHarmoniaCharacterClass Class)
{
	// Simplified
}

void UHarmoniaProgressionComponent::ApplyAwakeningEffects(EHarmoniaAwakeningTier Tier)
{
	// Apply awakening bonuses
}

void UHarmoniaProgressionComponent::ApplyPrestigeEffects()
{
	// Apply prestige bonuses
}

//~ Replication

void UHarmoniaProgressionComponent::OnRep_CurrentExperience()
{
}

void UHarmoniaProgressionComponent::OnRep_CurrentLevel()
{
}

void UHarmoniaProgressionComponent::OnRep_AvailableSkillPoints()
{
}

void UHarmoniaProgressionComponent::OnRep_AvailableStatPoints()
{
}

void UHarmoniaProgressionComponent::OnRep_UnlockedSkillNodes()
{
}

void UHarmoniaProgressionComponent::OnRep_CurrentClass()
{
}

void UHarmoniaProgressionComponent::OnRep_CurrentAwakeningTier()
{
}

void UHarmoniaProgressionComponent::OnRep_PrestigeLevel()
{
}
