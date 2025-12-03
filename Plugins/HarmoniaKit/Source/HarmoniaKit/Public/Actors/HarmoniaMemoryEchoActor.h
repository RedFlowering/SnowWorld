// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaMemoryEchoActor.generated.h"

class UStaticMeshComponent;
class UParticleSystemComponent;
class UAudioComponent;
class USphereComponent;
class UHarmoniaDeathPenaltyConfigAsset;

/**
 * Actor representing a player's dropped currencies on death
 * Acts like the "bloodstain" in Dark Souls or "rune pile" in Elden Ring
 * Features time decay and memory resonance mechanics
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaMemoryEchoActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaMemoryEchoActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//~ Components
	/** Visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Interaction trigger */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	/** Memory resonance effect radius */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> ResonanceSphere;

	/** Visual effect */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> EffectComponent;

	/** Ambient sound */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AmbientSound;

	//~ Currency Data
	/** Currencies stored in this memory echo */
	UPROPERTY(ReplicatedUsing = OnRep_StoredCurrencies, BlueprintReadOnly, Category = "Currency")
	TArray<FHarmoniaCurrencyAmount> StoredCurrencies;

	/** Player who owns this memory echo */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Currency")
	TObjectPtr<APlayerController> OwningPlayerController;

	/** Player state ID for verification */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Currency")
	FString OwningPlayerStateId;

	//~ Time Decay
	/** Time since this memory echo was created */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Time Decay")
	float TimeSinceCreation;

	/** Total decay percentage applied so far */
	UPROPERTY(ReplicatedUsing = OnRep_TotalDecayPercentage, BlueprintReadOnly, Category = "Time Decay")
	float TotalDecayPercentage;

	/** Last time decay was applied */
	UPROPERTY(BlueprintReadOnly, Category = "Time Decay")
	float LastDecayTime;

	//~ Configuration
	/** Death penalty config reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TObjectPtr<UHarmoniaDeathPenaltyConfigAsset> DeathPenaltyConfig;

	//~ Initialization
	/** Initialize the memory echo with currencies and owner */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	void Initialize(const TArray<FHarmoniaCurrencyAmount>& Currencies, APlayerController* OwnerController, UHarmoniaDeathPenaltyConfigAsset* Config);

	/** Get total value in a specific currency */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	int32 GetCurrencyAmount(EHarmoniaCurrencyType CurrencyType) const;

	/** Check if this echo belongs to the specified player */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	bool BelongsToPlayer(APlayerController* PlayerController) const;

	/** Check if player can interact with this echo */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	bool CanPlayerInteract(APlayerController* PlayerController) const;

	/** Attempt to recover currencies (returns actual amounts recovered) */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo")
	TArray<FHarmoniaCurrencyAmount> RecoverCurrencies(APlayerController* PlayerController);

	/** Calculate fast recovery bonus if applicable */
	UFUNCTION(BlueprintPure, Category = "Memory Echo")
	bool IsFastRecovery() const;

	/** Get fast recovery bonus multiplier */
	UFUNCTION(BlueprintPure, Category = "Memory Echo")
	float GetFastRecoveryMultiplier() const;

	//~ Time Decay System
	/** Apply time-based decay to stored currencies */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Time Decay")
	void ApplyTimeDecay(float DeltaTime);

	/** Get current decay percentage */
	UFUNCTION(BlueprintPure, Category = "Memory Echo|Time Decay")
	float GetCurrentDecayPercentage() const { return TotalDecayPercentage; }

	//~ Memory Resonance System
	/** Update memory resonance effects on nearby enemies */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Resonance")
	void UpdateMemoryResonance();

	/** Get all enemies within resonance radius */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Resonance")
	TArray<AActor*> GetEnemiesInResonanceRange() const;

	/** Apply resonance buff to an enemy */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Resonance")
	void ApplyResonanceBuffToEnemy(AActor* Enemy);

	/** Remove resonance buff from an enemy */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Resonance")
	void RemoveResonanceBuffFromEnemy(AActor* Enemy);

	//~ Visual/Audio Updates
	/** Update visual effects based on current state */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Visual")
	void UpdateVisualEffects();

	/** Update audio based on decay state */
	UFUNCTION(BlueprintCallable, Category = "Memory Echo|Audio")
	void UpdateAudio();

protected:
	//~ Internal
	/** Timer handle for decay updates */
	FTimerHandle DecayTimerHandle;

	/** Timer handle for resonance updates */
	FTimerHandle ResonanceTimerHandle;

	/** Enemies currently affected by resonance */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> BuffedEnemies;

	/** Apply decay to currencies */
	void ApplyDecayToCurrencies(float DecayPercentage);

	/** Cleanup when all currencies are exhausted */
	void HandleCurrenciesExhausted();

	//~ Interaction Callbacks
	UFUNCTION()
	void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//~ Replication
	UFUNCTION()
	void OnRep_StoredCurrencies();

	UFUNCTION()
	void OnRep_TotalDecayPercentage();
};
