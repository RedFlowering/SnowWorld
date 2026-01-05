// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacterWithAbilities.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "Definitions/HarmoniaTeamSystemDefinitions.h"
#include "HarmoniaMonsterBase.generated.h"

class UHarmoniaMonsterData;
class UHarmoniaLootTableData;
class UHarmoniaThreatComponent;
class UHarmoniaAdvancedAIComponent;
class UHarmoniaAILODComponent;
class UMotionWarpingComponent;
class UHarmoniaAttributeSet;
struct FGameplayEffectSpec;

/**
 * Monster Death Delegate
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterDeathDelegate, AHarmoniaMonsterBase*, Monster, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMonsterDamagedDelegate, AHarmoniaMonsterBase*, Monster, float, Damage, AActor*, DamageInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterStateChangedDelegate, EHarmoniaMonsterState, OldState, EHarmoniaMonsterState, NewState);

/**
 * AHarmoniaMonsterBase
 *
 * Base class for all monsters in Harmonia
 * Supports various creature types: humanoid, quadruped, flying, mechanical, etc.
 * Integrates with Gameplay Ability System for combat
 * Can be extended in Blueprint for specific monster types
 *
 * Features:
 * - GAS integration (health, damage, abilities)
 * - AI behavior via AIController
 * - Loot generation and spawning
 * - Level scaling
 * - Animation interface support
 * - Network replication ready
 * - Team-based friend-or-foe identification
 * - Unreal's standard IGenericTeamAgentInterface integration
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaMonsterBase : public ALyraCharacterWithAbilities, public IHarmoniaMonsterInterface, public IHarmoniaTeamAgentInterface
{
	GENERATED_BODY()

public:
	AHarmoniaMonsterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~IHarmoniaMonsterInterface
	virtual UHarmoniaMonsterData* GetMonsterData_Implementation() const override;
	virtual int32 GetMonsterLevel_Implementation() const override;
	virtual FText GetMonsterName_Implementation() const override;
	virtual EHarmoniaMonsterFaction GetFaction_Implementation() const override;
	virtual EHarmoniaMonsterState GetMonsterState_Implementation() const override;
	virtual void SetMonsterState_Implementation(EHarmoniaMonsterState NewState) override;
	virtual bool IsInCombat_Implementation() const override;
	virtual bool IsDead_Implementation() const override;
	virtual void OnDamageTaken_Implementation(float DamageAmount, AActor* DamageInstigator) override;
	virtual void OnDeath_Implementation(AActor* Killer) override;
	virtual AActor* GetCurrentTarget_Implementation() const override;
	virtual void SetCurrentTarget_Implementation(AActor* NewTarget) override;
	virtual TArray<FHarmoniaLootTableRow> GenerateLoot_Implementation(AActor* Killer) override;
	virtual void SpawnLoot_Implementation(const TArray<FHarmoniaLootTableRow>& LootItems, const FVector& SpawnLocation) override;
	virtual EHarmoniaMonsterAggroType GetAggroType_Implementation() const override;
	virtual float GetAggroRange_Implementation() const override;
	virtual void OnTargetDetected_Implementation(AActor* DetectedActor) override;
	virtual void OnTargetLost_Implementation() override;
	//~End of IHarmoniaMonsterInterface

	//~IHarmoniaTeamAgentInterface
	virtual FHarmoniaTeamIdentification GetTeamID_Implementation() const override;
	virtual void SetTeamID_Implementation(const FHarmoniaTeamIdentification& NewTeamID) override;
	virtual EHarmoniaTeamRelationship GetRelationshipWith_Implementation(AActor* OtherActor) const override;
	virtual bool CanAttackActor_Implementation(AActor* OtherActor) const override;
	virtual bool ShouldHelpActor_Implementation(AActor* OtherActor) const override;
	virtual bool IsSameTeamAs_Implementation(AActor* OtherActor) const override;
	virtual bool IsAllyWith_Implementation(AActor* OtherActor) const override;
	virtual bool IsEnemyWith_Implementation(AActor* OtherActor) const override;
	//~End of IHarmoniaTeamAgentInterface

	// ============================================================================
	// Monster Configuration
	// ============================================================================

	/**
	 * Monster data asset
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UHarmoniaMonsterData> MonsterData = nullptr;

	/**
	 * Monster level (affects stats)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster", Replicated, meta = (ExposeOnSpawn = "true", ClampMin = "1"))
	int32 MonsterLevel = 1;

	/**
	 * Override loot table (if different from MonsterData)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Loot")
	TObjectPtr<UHarmoniaLootTableData> OverrideLootTable = nullptr;

	/**
	 * Luck modifier for loot generation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Loot")
	float LootLuckModifier = 0.0f;

	// ============================================================================
	// Monster State
	// ============================================================================

	/**
	 * Current monster state
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Monster|State", ReplicatedUsing = OnRep_MonsterState)
	EHarmoniaMonsterState CurrentState = EHarmoniaMonsterState::Idle;

	/**
	 * Current target actor
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Combat", Replicated)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when monster dies
	 */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterDeathDelegate OnMonsterDeath;

	/**
	 * Called when monster takes damage
	 */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterDamagedDelegate OnMonsterDamaged;

	/**
	 * Called when monster state changes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Monster|Events")
	FOnMonsterStateChangedDelegate OnMonsterStateChanged;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * Initialize monster with data and level
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void InitializeMonster(UHarmoniaMonsterData* InMonsterData, int32 InLevel);

	/**
	 * Apply level scaling to attributes
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void ApplyLevelScaling();

	/**
	 * Get the animation instance (casted to IHarmoniaMonsterAnimationInterface)
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster|Animation")
	TScriptInterface<IHarmoniaMonsterAnimationInterface> GetMonsterAnimationInterface() const;

	/**
	 * Play death animation and handle death
	 */
	UFUNCTION(BlueprintCallable, Category = "Monster")
	virtual void PlayDeathAnimation();

protected:
	// ============================================================================
	// Components
	// ============================================================================

	/**
	 * Attribute Set (monster-specific attributes)
	 * Note: ASC comes from ALyraCharacter via PawnExtension
	 */
	UPROPERTY()
	TObjectPtr<UHarmoniaAttributeSet> AttributeSet = nullptr;

	/**
	 * Motion Warping Component (for leap attacks)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Movement")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent = nullptr;

	/**
	 * Threat Component (for MMO-style aggro management)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHarmoniaThreatComponent> ThreatComponent = nullptr;

	/**
	 * Advanced AI Component (for emotion, combos, tactical positioning)
	 * Optional - enables advanced AI behaviors when added
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UHarmoniaAdvancedAIComponent> AdvancedAIComponent = nullptr;

	/**
	 * AI LOD Component (for performance optimization)
	 * Manages update frequency based on distance and visibility
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UHarmoniaAILODComponent> AILODComponent = nullptr;

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Whether monster has been initialized
	 */
	bool bInitialized = false;

	/**
	 * Whether death sequence has started
	 */
	bool bDeathStarted = false;

	// ============================================================================
	// Replication Callbacks
	// ============================================================================

	UFUNCTION()
	virtual void OnRep_MonsterState(EHarmoniaMonsterState OldState);

	// ============================================================================
	// Attribute Callbacks
	// ============================================================================

	/**
	 * Called when health changes
	 */
	virtual void OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);

	/**
	 * Called when health reaches zero
	 */
	virtual void OnOutOfHealth(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);

	/**
	 * Called when damage is received
	 */
	virtual void OnDamageReceived(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);

	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Initialize ability system
	 */
	virtual void InitializeAbilitySystem();

	/**
	 * Grant abilities from monster data
	 */
	virtual void GrantAbilities();

	/**
	 * Update animation state
	 */
	virtual void UpdateAnimationState();

	/**
	 * Handle death cleanup
	 */
	virtual void HandleDeathCleanup();

	/**
	 * Destroy corpse after lifetime
	 */
	UFUNCTION()
	virtual void DestroyCorpse();
};
