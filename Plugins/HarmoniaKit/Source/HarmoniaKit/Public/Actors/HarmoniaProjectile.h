// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "GameplayEffectTypes.h"
#include "HarmoniaProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UAudioComponent;
class UAbilitySystemComponent;

/**
 * Delegate for projectile hit events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnProjectileHitDelegate, AActor*, HitActor, const FHitResult&, HitResult, const FHarmoniaProjectileData&, ProjectileData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileExplodeDelegate, const FVector&, ExplosionLocation);

/**
 * AHarmoniaProjectile
 *
 * Base class for all projectiles in Harmonia Kit
 * Supports:
 * - Physical projectiles (arrows, bolts, bullets)
 * - Magic projectiles (fireballs, ice shards, etc.)
 * - Explosive projectiles (bombs, grenades)
 * - Homing projectiles
 * - Penetrating projectiles
 * - Bouncing projectiles
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaProjectile : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaProjectile();

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	/**
	 * Initialize projectile with data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Projectile")
	void InitializeProjectile(const FHarmoniaProjectileData& InProjectileData, AActor* InOwner, float DamageMultiplier = 1.0f);

	/**
	 * Launch projectile in direction
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Projectile")
	void LaunchProjectile(const FVector& Direction, float Speed = 0.0f);

	/**
	 * Set homing target
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Projectile")
	void SetHomingTarget(AActor* Target);

	/**
	 * Get projectile data
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Projectile")
	const FHarmoniaProjectileData& GetProjectileData() const { return ProjectileData; }

	/**
	 * Delegates
	 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Projectile")
	FOnProjectileHitDelegate OnProjectileHit;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Projectile")
	FOnProjectileExplodeDelegate OnProjectileExplode;

protected:
	/**
	 * Handle collision
	 */
	UFUNCTION()
	virtual void OnProjectileHit_Internal(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/**
	 * Handle overlap (for explosives)
	 */
	UFUNCTION()
	virtual void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Apply damage to target
	 */
	virtual void ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult);

	/**
	 * Handle penetration
	 */
	virtual bool HandlePenetration(AActor* HitActor);

	/**
	 * Handle bounce
	 */
	virtual void HandleBounce(const FHitResult& HitResult);

	/**
	 * Explode projectile
	 */
	virtual void Explode(const FVector& ExplosionLocation);

	/**
	 * Stick to surface
	 */
	virtual void StickToSurface(const FHitResult& HitResult);

	/**
	 * Destroy projectile
	 */
	virtual void DestroyProjectile();

	/**
	 * Spawn impact effects
	 */
	virtual void SpawnImpactEffects(const FHitResult& HitResult);

	/**
	 * Get ability system component from owner
	 */
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

	// ============================================================================
	// Components
	// ============================================================================

	/** Collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** Mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> MovementComponent;

	/** Trail particle system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> TrailComponent;

	/** Audio component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent;

	// ============================================================================
	// Effects
	// ============================================================================

	/** Impact particle effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> ImpactEffect;

	/** Explosion particle effect */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> ExplosionEffect;

	/** Impact sound */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<USoundBase> ImpactSound;

	/** Explosion sound */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<USoundBase> ExplosionSound;

	// ============================================================================
	// State
	// ============================================================================

	/** Projectile configuration data */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Projectile")
	FHarmoniaProjectileData ProjectileData;

	/** Damage multiplier (from weapon/ability) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Projectile")
	float DamageMultiplier = 1.0f;

	/** Actors already hit (for penetration tracking) */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HitActors;

	/** Remaining penetration count */
	int32 RemainingPenetrations = 0;

	/** Remaining bounce count */
	int32 RemainingBounces = 0;

	/** Is stuck to surface? */
	bool bIsStuck = false;

	/** Projectile owner actor */
	UPROPERTY()
	TObjectPtr<AActor> ProjectileOwner;

	/** Homing target */
	UPROPERTY()
	TWeakObjectPtr<AActor> HomingTarget;

	/** Lifetime timer handle */
	FTimerHandle LifetimeTimerHandle;
};
