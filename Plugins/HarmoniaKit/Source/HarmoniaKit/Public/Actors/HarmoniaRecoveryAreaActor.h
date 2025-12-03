// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaRecoveryItemDefinitions.h"
#include "HarmoniaRecoveryAreaActor.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UAudioComponent;
class UGameplayEffect;

/**
 * @class AHarmoniaRecoveryAreaActor
 * @brief Deployable recovery area actor (Life Luminescence)
 * 
 * Continuously heals players within range.
 *
 * Features:
 * - Range-based healing (Sphere Component)
 * - Periodic recovery ticks (Tick Interval)
 * - Time limit (Duration)
 * - VFX/SFX playback
 * - Multiplayer support
 *
 * Extensibility:
 * - Can be extended for poison zones, buff zones, etc.
 * - Configuration-based operation
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaRecoveryAreaActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaRecoveryAreaActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/**
	 * Initialize recovery area
	 * @param Config Recovery area configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void InitializeRecoveryArea(const FHarmoniaDeployableRecoveryConfig& Config);

	/** Activate the recovery area */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void ActivateRecoveryArea();

	/** Deactivate the recovery area */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void DeactivateRecoveryArea();

	/** Execute a recovery tick */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void PerformRecoveryTick();

	/** Get all actors currently in the recovery area */
	UFUNCTION(BlueprintPure, Category = "Recovery Area")
	TArray<AActor*> GetActorsInRecoveryArea() const;

	/** Expire and destroy the recovery area */
	UFUNCTION(BlueprintCallable, Category = "Recovery Area")
	void ExpireRecoveryArea();

protected:
	/** Called when an actor enters the recovery area */
	UFUNCTION()
	void OnActorEnterRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Called when an actor leaves the recovery area */
	UFUNCTION()
	void OnActorLeaveRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	/** Recovery area sphere component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<USphereComponent> RecoveryAreaSphere;

	/** VFX component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<UNiagaraComponent> AreaVFXComponent;

	/** SFX component (looping) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Area")
	TObjectPtr<UAudioComponent> AreaAudioComponent;

	/** Recovery area configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Area")
	FHarmoniaDeployableRecoveryConfig RecoveryConfig;

	/** Recovery tick timer */
	FTimerHandle RecoveryTickTimerHandle;

	/** Expiration timer */
	FTimerHandle ExpirationTimerHandle;

	/** Whether the area is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Recovery Area")
	bool bIsActive = false;

	/** Actors currently within the recovery area */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> ActorsInArea;
};
