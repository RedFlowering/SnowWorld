// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/HarmoniaBaseCombatComponent.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "GameplayTagContainer.h"
#include "HarmoniaRangedCombatComponent.generated.h"

class UHarmoniaLockOnComponent;
class AHarmoniaProjectile;
class UAbilitySystemComponent;
class USplineComponent;

/**
 * Delegate for ranged attack events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRangedAttackDelegate, AActor*, Projectile, const FHarmoniaProjectileData&, ProjectileData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponDrawnDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponHolsteredDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAimingStateChangedDelegate, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, int32, CurrentAmmo, int32, MaxAmmo);

/**
 * UHarmoniaRangedCombatComponent
 *
 * Manages ranged combat including:
 * - Bow/Crossbow aiming and firing with trajectory calculation
 * - Magic spell casting with mana management
 * - Throwing weapons
 * - Firearms (dark fantasy style)
 * - Ammo/mana management
 * - Aiming modes and reticle
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRangedCombatComponent : public UHarmoniaBaseCombatComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRangedCombatComponent();

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	// ============================================================================
	// Weapon Management
	// ============================================================================

	/**
	 * Get current ranged weapon type
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	EHarmoniaRangedWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

	/**
	 * Set current ranged weapon type
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void SetCurrentWeaponType(EHarmoniaRangedWeaponType NewWeaponType);

	/**
	 * Draw/ready weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void DrawWeapon();

	/**
	 * Holster weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void HolsterWeapon();

	/**
	 * Is weapon currently drawn?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	bool IsWeaponDrawn() const { return bWeaponDrawn; }

	// ============================================================================
	// Aiming
	// ============================================================================

	/**
	 * Start aiming
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void StartAiming();

	/**
	 * Stop aiming
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void StopAiming();

	/**
	 * Toggle aiming
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void ToggleAiming();

	/**
	 * Is currently aiming?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	bool IsAiming() const { return bIsAiming; }

	/**
	 * Get aim direction (world space)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	FVector GetAimDirection() const;

	/**
	 * Get aim location (where the projectile will spawn)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	FVector GetAimLocation() const;

	/**
	 * Get aim target location (where we're aiming at)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	FVector GetAimTargetLocation() const;

	/**
	 * Calculate trajectory path for projectiles
	 * @param OutPathPositions Output array of positions along the trajectory
	 * @param TimeStep Time between each position sample
	 * @param MaxSimTime Maximum simulation time
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void CalculateTrajectoryPath(TArray<FVector>& OutPathPositions, float TimeStep = 0.1f, float MaxSimTime = 5.0f);

	// ============================================================================
	// Firing
	// ============================================================================

	/**
	 * Request fire (for bows, starts drawing)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	bool RequestFire();

	/**
	 * Request release (for bows, releases arrow)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	bool RequestRelease();

	/**
	 * Can fire?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	bool CanFire() const;

	/**
	 * Is currently drawing/charging?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	bool IsDrawing() const { return bIsDrawing; }

	/**
	 * Get current draw time
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	float GetCurrentDrawTime() const { return CurrentDrawTime; }

	/**
	 * Get draw progress (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	float GetDrawProgress() const;

	/**
	 * Get damage multiplier based on draw time
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	float GetDrawDamageMultiplier() const;

	/**
	 * Is currently reloading?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	bool IsReloading() const { return bIsReloading; }

	/**
	 * Request reload
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void RequestReload();

	// ============================================================================
	// Ammo Management
	// ============================================================================

	/**
	 * Get current ammo in magazine/quiver
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	/**
	 * Get total ammo (reserve)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	int32 GetTotalAmmo() const { return TotalAmmo; }

	/**
	 * Get magazine/quiver size
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	int32 GetMagazineSize() const;

	/**
	 * Add ammo
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void AddAmmo(int32 Amount);

	/**
	 * Set ammo
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	void SetAmmo(int32 InCurrentAmmo, int32 InTotalAmmo);

	/**
	 * Has ammo?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat")
	bool HasAmmo() const;

	// ============================================================================
	// Spell Management (Magic Weapons)
	// ============================================================================

	/**
	 * Set current spell
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat|Magic")
	void SetCurrentSpell(FName SpellRowName);

	/**
	 * Get current spell data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat|Magic")
	bool GetCurrentSpellData(FHarmoniaSpellData& OutSpellData) const;

	/**
	 * Can cast spell?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat|Magic")
	bool CanCastSpell() const;

	/**
	 * Get spell cooldown remaining
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ranged Combat|Magic")
	float GetSpellCooldownRemaining() const;

	// ============================================================================
	// Projectile Spawning
	// ============================================================================

	/**
	 * Spawn projectile
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ranged Combat")
	AActor* SpawnProjectile(const FHarmoniaProjectileData& ProjectileData, const FVector& SpawnLocation, const FVector& Direction, float DamageMultiplier = 1.0f);

	// ============================================================================
	// Delegates
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Ranged Combat")
	FOnRangedAttackDelegate OnProjectileFired;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Ranged Combat")
	FOnWeaponDrawnDelegate OnWeaponDrawn;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Ranged Combat")
	FOnWeaponHolsteredDelegate OnWeaponHolstered;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Ranged Combat")
	FOnAimingStateChangedDelegate OnAimingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Ranged Combat")
	FOnAmmoChangedDelegate OnAmmoChanged;

protected:
	/**
	 * Internal fire logic
	 */
	void FireProjectile();

	/**
	 * Perform reload
	 */
	void PerformReload();

	/**
	 * Apply recoil
	 */
	void ApplyRecoil();

	/**
	 * Consume resources (ammo/mana/stamina)
	 */
	bool ConsumeResources();



	/**
	 * Get lock-on component
	 */
	UHarmoniaLockOnComponent* GetLockOnComponent() const;

	// ============================================================================
	// Server RPCs
	// ============================================================================

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireProjectile(const FVector& SpawnLocation, const FVector& Direction, float DamageMultiplier);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartAiming();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopAiming();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartDrawing();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReleaseDrawing();

	// ============================================================================
	// Replication Callbacks
	// ============================================================================

	UFUNCTION()
	void OnRep_CurrentWeaponType();

	UFUNCTION()
	void OnRep_IsAiming();

	UFUNCTION()
	void OnRep_CurrentAmmo();

private:
	// ============================================================================
	// Configuration
	// ============================================================================
	/** Data table containing spell configurations */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Magic")
	TObjectPtr<UDataTable> SpellDataTable = nullptr;

	/** Default projectile class (if weapon doesn't specify) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat")
	TSubclassOf<AActor> DefaultProjectileClass;

	/** Trajectory visualization component (optional) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat")
	TObjectPtr<USplineComponent> TrajectorySpline;

	/** Show trajectory preview? */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat")
	bool bShowTrajectoryPreview = true;

	/** Aim assist strength (0-1) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Aiming")
	float AimAssistStrength = 0.3f;

	/** Aim assist radius */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Aiming")
	float AimAssistRadius = 500.0f;

	// ============================================================================
	// Default Weapon Configuration (replaces WeaponDataTable)
	// ============================================================================

	/** Default projectile type */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	EHarmoniaProjectileType DefaultProjectileType = EHarmoniaProjectileType::Arrow;

	/** Draw/Charge time for bows (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float DrawTime = 1.0f;

	/** Maximum draw time for damage bonus */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float MaxDrawTime = 2.0f;

	/** Damage multiplier at max draw (min draw is always 1.0) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float MaxDrawDamageMultiplier = 2.0f;

	/** Reload time (seconds, 0 = no reload) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float ReloadTime = 0.0f;

	/** Magazine/Quiver size (0 = infinite) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	int32 MagazineSize = 0;

	/** Maximum ammo count (0 = infinite) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	int32 MaxAmmoCount = 0;

	/** Accuracy (0-1, 1 = perfect accuracy) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float Accuracy = 0.95f;

	/** Spread angle (degrees, 0 = no spread) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float SpreadAngle = 0.0f;

	/** Number of projectiles per shot (shotgun) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	int32 ProjectilesPerShot = 1;

	/** Can move while aiming? */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	bool bCanMoveWhileAiming = true;

	/** Can fire while moving? */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	bool bCanFireWhileMoving = true;

	/** Recoil amount */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float RecoilAmount = 1.0f;

	/** Fire rate (shots per second, 0 = manual) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float FireRate = 0.0f;

	/** Aiming movement speed multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float AimingMovementSpeedMultiplier = 0.5f;

	/** Maximum range (cm, 0 = infinite) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float MaxRange = 10000.0f;

	/** Muzzle socket name for projectile spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	FName MuzzleSocketName = FName("Muzzle");

	/** Base damage multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float BaseDamageMultiplier = 1.0f;

	/** Stamina cost per shot */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float StaminaCostPerShot = 15.0f;

	/** Mana cost per shot (for magic weapons) */
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Ranged Combat|Weapon")
	float ManaCostPerShot = 0.0f;

	// ============================================================================
	// State
	// ============================================================================

	/** Current weapon type */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponType)
	EHarmoniaRangedWeaponType CurrentWeaponType = EHarmoniaRangedWeaponType::None;

	/** Is weapon drawn? */
	UPROPERTY(Replicated)
	bool bWeaponDrawn = false;

	/** Is currently aiming? */
	UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
	bool bIsAiming = false;

	/** Is currently drawing/charging? */
	UPROPERTY(Replicated)
	bool bIsDrawing = false;

	/** Current draw time */
	UPROPERTY(Replicated)
	float CurrentDrawTime = 0.0f;

	/** Is currently reloading? */
	UPROPERTY(Replicated)
	bool bIsReloading = false;

	/** Reload time remaining */
	float ReloadTimeRemaining = 0.0f;

	/** Current ammo in magazine/quiver */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo = 0;

	/** Total ammo (reserve) */
	UPROPERTY(Replicated)
	int32 TotalAmmo = 0;

	/** Last fire time (for fire rate) */
	float LastFireTime = 0.0f;

	/** Current spell row name */
	UPROPERTY(Replicated)
	FName CurrentSpellRowName;

	/** Last spell cast time */
	float LastSpellCastTime = 0.0f;

	UPROPERTY()
	TObjectPtr<UHarmoniaLockOnComponent> CachedLockOnComponent;
};
