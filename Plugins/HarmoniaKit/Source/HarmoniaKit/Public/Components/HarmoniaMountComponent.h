// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaMovementSystemDefinitions.h"
#include "HarmoniaMountComponent.generated.h"

class UHarmoniaAttributeSet;
class UAbilitySystemComponent;
class UDataTable;
class ACharacter;
class USkeletalMeshComponent;

// ============================================================================
// Delegate Declarations
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMountedSignature, EHarmoniaMountType, MountType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDismountedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMountHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMountDiedSignature);

/**
 * Mount Component
 * Manages mount/vehicle system for the character including:
 * - Mount/Dismount mechanics
 * - Mount movement control
 * - Mount combat
 * - Flying mount controls
 * - Mount stamina management
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMountComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaMountComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Mount Management
	// ============================================================================

	/** Get current mount type */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	EHarmoniaMountType GetCurrentMountType() const { return CurrentMountType; }

	/** Set current mount type */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void SetCurrentMountType(EHarmoniaMountType NewMountType);

	/** Get mount data for current mount */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool GetCurrentMountData(FHarmoniaMountData& OutMountData) const;

	/** Get mount data for specific mount type */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool GetMountDataForType(EHarmoniaMountType MountType, FHarmoniaMountData& OutMountData) const;

	// ============================================================================
	// Mount State
	// ============================================================================

	/** Is currently mounted? */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool IsMounted() const { return bIsMounted; }

	/** Is currently mounting/dismounting? */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool IsTransitioning() const { return bIsMounting || bIsDismounting; }

	/** Can mount at current state? */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool CanMount() const;

	/** Can dismount at current state? */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool CanDismount() const;

	/** Is mount sprinting? */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool IsSprinting() const { return bIsSprinting; }

	/** Is mount flying? */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool IsFlying() const { return bIsFlying; }

	// ============================================================================
	// Mount Actions
	// ============================================================================

	/** Begin mounting process */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool StartMount(EHarmoniaMountType MountType);

	/** Begin dismounting process */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	bool StartDismount();

	/** Complete mounting (called after animation) */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void CompleteMounting();

	/** Complete dismounting (called after animation) */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void CompleteDismounting();

	/** Start mount sprinting */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void StartSprint();

	/** Stop mount sprinting */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void StopSprint();

	/** Start flying (for flying mounts) */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void StartFlying();

	/** Stop flying */
	UFUNCTION(BlueprintCallable, Category = "Mount")
	void StopFlying();

	// ============================================================================
	// Mount Combat
	// ============================================================================

	/** Can attack while mounted with current weapon? */
	UFUNCTION(BlueprintCallable, Category = "Mount|Combat")
	bool CanAttackWhileMounted(FGameplayTag WeaponTag) const;

	/** Can use abilities while mounted? */
	UFUNCTION(BlueprintCallable, Category = "Mount|Combat")
	bool CanUseAbilitiesWhileMounted() const;

	/** Execute mount charge attack */
	UFUNCTION(BlueprintCallable, Category = "Mount|Combat")
	bool ExecuteChargeAttack();

	/** Execute mount trample */
	UFUNCTION(BlueprintCallable, Category = "Mount|Combat")
	bool ExecuteTrample();

	/** Execute aerial strike (flying mounts) */
	UFUNCTION(BlueprintCallable, Category = "Mount|Combat")
	bool ExecuteAerialStrike();

	/** Get mount combat data */
	UFUNCTION(BlueprintCallable, Category = "Mount|Combat")
	FHarmoniaMountCombatData GetMountCombatData() const { return CurrentMountCombatData; }

	// ============================================================================
	// Mount Stats
	// ============================================================================

	/** Get current movement speed multiplier */
	UFUNCTION(BlueprintCallable, Category = "Mount|Stats")
	float GetCurrentSpeedMultiplier() const;

	/** Get mount health */
	UFUNCTION(BlueprintCallable, Category = "Mount|Stats")
	float GetMountHealth() const { return CurrentMountHealth; }

	/** Set mount health */
	UFUNCTION(BlueprintCallable, Category = "Mount|Stats")
	void SetMountHealth(float NewHealth);

	/** Damage mount */
	UFUNCTION(BlueprintCallable, Category = "Mount|Stats")
	void DamageMountHealth(float Damage);

	/** Heal mount */
	UFUNCTION(BlueprintCallable, Category = "Mount|Stats")
	void HealMountHealth(float HealAmount);

	// ============================================================================
	// Data Tables
	// ============================================================================

	/** Mount data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Data")
	UDataTable* MountDataTable;

	// ============================================================================
	// Events
	// ============================================================================

	/** Called when successfully mounted */
	UPROPERTY(BlueprintAssignable, Category = "Mount|Events")
	FOnMountedSignature OnMounted;

	/** Called when successfully dismounted */
	UPROPERTY(BlueprintAssignable, Category = "Mount|Events")
	FOnDismountedSignature OnDismounted;

	/** Called when mount health changes */
	UPROPERTY(BlueprintAssignable, Category = "Mount|Events")
	FOnMountHealthChangedSignature OnMountHealthChanged;

	/** Called when mount dies */
	UPROPERTY(BlueprintAssignable, Category = "Mount|Events")
	FOnMountDiedSignature OnMountDied;

protected:
	// ============================================================================
	// Internal State
	// ============================================================================

	/** Current mount type */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	EHarmoniaMountType CurrentMountType;

	/** Is mounted */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	bool bIsMounted;

	/** Is currently mounting */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	bool bIsMounting;

	/** Is currently dismounting */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	bool bIsDismounting;

	/** Is sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	bool bIsSprinting;

	/** Is flying */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	bool bIsFlying;

	/** Current mount health */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	float CurrentMountHealth;

	/** Current mount combat data */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	FHarmoniaMountCombatData CurrentMountCombatData;

	/** Mount mesh component reference */
	UPROPERTY(BlueprintReadOnly, Category = "Mount")
	USkeletalMeshComponent* MountMeshComponent;

	/** Time when mounting started */
	float MountingStartTime;

	/** Time when dismounting started */
	float DismountingStartTime;

	// ============================================================================
	// Cached References
	// ============================================================================

	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	UHarmoniaAttributeSet* AttributeSet;

	// ============================================================================
	// Helper Functions
	// ============================================================================

	/** Initialize cached references */
	void InitializeReferences();

	/** Update movement speed based on mount state */
	void UpdateMovementSpeed();

	/** Spawn mount mesh */
	void SpawnMountMesh(const FHarmoniaMountData& MountData);

	/** Remove mount mesh */
	void RemoveMountMesh();

	/** Apply mounted tags to owner */
	void ApplyMountedTags(const FHarmoniaMountData& MountData);

	/** Remove mounted tags from owner */
	void RemoveMountedTags(const FHarmoniaMountData& MountData);
};
