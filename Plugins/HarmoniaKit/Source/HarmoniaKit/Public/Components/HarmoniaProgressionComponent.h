// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "HarmoniaProgressionComponent.generated.h"

class UHarmoniaSkillTreeData;
class UHarmoniaClassData;
class ULyraAbilitySystemComponent;
class UGameplayEffect;

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExperienceGained, int32, Amount, int32, CurrentExp, int32, RequiredExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, SkillPointsGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillPointsChanged, int32, NewAmount, int32, DeltaAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatPointsChanged, int32, NewAmount, int32, DeltaAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillNodeUnlocked, FName, NodeID, int32, InvestedPoints, int32, TotalInvested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClassChanged, EHarmoniaCharacterClass, OldClass, EHarmoniaCharacterClass, NewClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAwakeningAchieved, EHarmoniaAwakeningTier, NewTier, int32, BonusSkillPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPrestigeLevelChanged, int32, NewPrestigeLevel, int32, BonusStatPoints, int32, BonusSkillPoints);

/**
 * Component that manages character progression system
 * Handles experience, levels, skill points, stat points, class system, and skill trees
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaProgressionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//~ Configuration
	/** Skill tree data assets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Config")
	TArray<TObjectPtr<UHarmoniaSkillTreeData>> SkillTreeDataAssets;

	/** Class data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Config")
	TObjectPtr<UHarmoniaClassData> ClassDataAsset;

	/** Experience curve (Level -> Required Exp for next level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Config")
	TMap<int32, int32> ExperienceCurve;

	/** Skill points gained per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Config")
	int32 SkillPointsPerLevel = 1;

	/** Stat points gained per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Config")
	int32 StatPointsPerLevel = 2;

	/** Max level cap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Config")
	int32 MaxLevel = 100;

	//~ Current State (Replicated)
	/** Current experience */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience, BlueprintReadOnly, Category = "Progression")
	int32 CurrentExperience;

	/** Current level */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentLevel, BlueprintReadOnly, Category = "Progression")
	int32 CurrentLevel;

	/** Available skill points */
	UPROPERTY(ReplicatedUsing = OnRep_AvailableSkillPoints, BlueprintReadOnly, Category = "Progression")
	int32 AvailableSkillPoints;

	/** Available stat points */
	UPROPERTY(ReplicatedUsing = OnRep_AvailableStatPoints, BlueprintReadOnly, Category = "Progression")
	int32 AvailableStatPoints;

	/** Current character class */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentClass, BlueprintReadOnly, Category = "Class")
	EHarmoniaCharacterClass CurrentClass;

	/** Current awakening tier */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAwakeningTier, BlueprintReadOnly, Category = "Awakening")
	EHarmoniaAwakeningTier CurrentAwakeningTier;

	/** Prestige level (New Game+ level) */
	UPROPERTY(ReplicatedUsing = OnRep_PrestigeLevel, BlueprintReadOnly, Category = "Prestige")
	int32 PrestigeLevel;

	/** Unlocked skill nodes and their investment levels */
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedSkillNodes, BlueprintReadOnly, Category = "Skills")
	TArray<FSkillNodeInvestment> UnlockedSkillNodes;

	/** Allocated primary stats (Vitality, Strength, etc) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stats")
	TArray<FStatAllocation> AllocatedStats;

	//~ Delegates
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnExperienceGained OnExperienceGained;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnSkillPointsChanged OnSkillPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnStatPointsChanged OnStatPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnSkillNodeUnlocked OnSkillNodeUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnClassChanged OnClassChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnAwakeningAchieved OnAwakeningAchieved;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnPrestigeLevelChanged OnPrestigeLevelChanged;

	//~ Experience & Leveling
	/** Add experience points */
	UFUNCTION(BlueprintCallable, Category = "Progression|Experience")
	void AddExperience(int32 Amount);

	/** Get required experience for next level */
	UFUNCTION(BlueprintPure, Category = "Progression|Experience")
	int32 GetRequiredExperienceForNextLevel() const;

	/** Get experience progress (0.0 - 1.0) */
	UFUNCTION(BlueprintPure, Category = "Progression|Experience")
	float GetExperienceProgress() const;

	/** Check if can level up */
	UFUNCTION(BlueprintPure, Category = "Progression|Experience")
	bool CanLevelUp() const;

	/** Manually trigger level up (if has enough exp) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Experience")
	void TryLevelUp();

	//~ Skill Tree System
	/** Unlock/invest in a skill node */
	UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
	bool UnlockSkillNode(FName NodeID, int32 InvestmentPoints = 1);

	/** Check if can unlock a skill node */
	UFUNCTION(BlueprintPure, Category = "Progression|Skills")
	bool CanUnlockSkillNode(FName NodeID) const;

	/** Get investment level in a skill node */
	UFUNCTION(BlueprintPure, Category = "Progression|Skills")
	int32 GetSkillNodeInvestment(FName NodeID) const;

	/** Check if skill node is unlocked */
	UFUNCTION(BlueprintPure, Category = "Progression|Skills")
	bool IsSkillNodeUnlocked(FName NodeID) const;

	/** Get all unlocked skill nodes */
	UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
	TArray<FName> GetUnlockedSkillNodes() const;

	/** Reset skill tree (refund all points) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
	void ResetSkillTree(bool bRefundPoints = true);

	//~ Stat Allocation
	/** Allocate stat points to primary attributes */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	bool AllocateStatPoints(FGameplayTag StatTag, int32 Points);

	/** Get allocated points for a stat */
	UFUNCTION(BlueprintPure, Category = "Progression|Stats")
	int32 GetAllocatedStatPoints(FGameplayTag StatTag) const;

	/** Reset all stat allocations */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void ResetStatAllocations(bool bRefundPoints = true);

	//~ Class System
	/** Change character class */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	bool ChangeClass(EHarmoniaCharacterClass NewClass);

	/** Check if can change to a class */
	UFUNCTION(BlueprintPure, Category = "Progression|Class")
	bool CanChangeClass(EHarmoniaCharacterClass NewClass) const;

	/** Get available advancement classes */
	UFUNCTION(BlueprintCallable, Category = "Progression|Class")
	TArray<EHarmoniaCharacterClass> GetAvailableAdvancementClasses() const;

	//~ Awakening System
	/** Trigger awakening to next tier */
	UFUNCTION(BlueprintCallable, Category = "Progression|Awakening")
	bool TriggerAwakening(EHarmoniaAwakeningTier NewTier);

	/** Check if can awaken to next tier */
	UFUNCTION(BlueprintPure, Category = "Progression|Awakening")
	bool CanAwaken(EHarmoniaAwakeningTier TargetTier) const;

	//~ Prestige / New Game+ System
	/** Enter New Game+ (prestige) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Prestige")
	bool EnterNewGamePlus();

	/** Check if can enter New Game+ */
	UFUNCTION(BlueprintPure, Category = "Progression|Prestige")
	bool CanEnterNewGamePlus() const;

	/** Get prestige bonuses */
	UFUNCTION(BlueprintPure, Category = "Progression|Prestige")
	FHarmoniaPrestigeDefinition GetCurrentPrestigeDefinition() const;

	//~ Save/Load Support
	/** Export progression data for save game */
	UFUNCTION(BlueprintCallable, Category = "Progression|Save")
	FHarmoniaProgressionSaveData ExportProgressionData() const;

	/** Import progression data from save game */
	UFUNCTION(BlueprintCallable, Category = "Progression|Save")
	void ImportProgressionData(const FHarmoniaProgressionSaveData& SavedData);

protected:
	//~ Internal Helpers
	/** Initialize default experience curve */
	void InitializeExperienceCurve();

	/** Calculate required experience for a level */
	int32 CalculateRequiredExperience(int32 Level) const;

	/** Process level up */
	void ProcessLevelUp();

	/** Get ability system component */
	ULyraAbilitySystemComponent* GetAbilitySystemComponent() const;

	/** Find skill node by ID */
	FHarmoniaSkillNode* FindSkillNode(FName NodeID) const;

	/** Check if prerequisites are met for a skill node */
	bool ArePrerequisitesMet(const FHarmoniaSkillNode& Node) const;

	/** Apply skill node effects */
	void ApplySkillNodeEffects(const FHarmoniaSkillNode& Node);

	/** Remove skill node effects */
	void RemoveSkillNodeEffects(const FHarmoniaSkillNode& Node);

	/** Apply stat allocation to attribute set */
	void ApplyStatAllocation(FGameplayTag StatTag, int32 Points);

	/** Apply class effects */
	void ApplyClassEffects(EHarmoniaCharacterClass Class);

	/** Remove class effects */
	void RemoveClassEffects(EHarmoniaCharacterClass Class);

	/** Apply awakening effects */
	void ApplyAwakeningEffects(EHarmoniaAwakeningTier Tier);

	/** Apply prestige effects */
	void ApplyPrestigeEffects();

	//~ Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_AddExperience(int32 Amount);

	UFUNCTION(Server, Reliable)
	void Server_UnlockSkillNode(FName NodeID, int32 InvestmentPoints);

	UFUNCTION(Server, Reliable)
	void Server_AllocateStatPoints(FGameplayTag StatTag, int32 Points);

	UFUNCTION(Server, Reliable)
	void Server_ChangeClass(EHarmoniaCharacterClass NewClass);

	UFUNCTION(Server, Reliable)
	void Server_TriggerAwakening(EHarmoniaAwakeningTier NewTier);

	UFUNCTION(Server, Reliable)
	void Server_EnterNewGamePlus();

	UFUNCTION(Server, Reliable)
	void Server_ResetSkillTree(bool bRefundPoints);

	UFUNCTION(Server, Reliable)
	void Server_ResetStatAllocations(bool bRefundPoints);

	//~ Replication
	UFUNCTION()
	void OnRep_CurrentExperience();

	UFUNCTION()
	void OnRep_CurrentLevel();

	UFUNCTION()
	void OnRep_AvailableSkillPoints();

	UFUNCTION()
	void OnRep_AvailableStatPoints();

	UFUNCTION()
	void OnRep_UnlockedSkillNodes();

	UFUNCTION()
	void OnRep_CurrentClass();

	UFUNCTION()
	void OnRep_CurrentAwakeningTier();

	UFUNCTION()
	void OnRep_PrestigeLevel();

private:
	//~ Cached References
	UPROPERTY()
	TObjectPtr<ULyraAbilitySystemComponent> CachedAbilitySystemComponent;

	/** Active gameplay effect handles from skills/class/awakening */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> ActiveProgressionEffects;
};
