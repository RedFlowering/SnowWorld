// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "HarmoniaCrystalResonator.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UParticleSystemComponent;
class UNiagaraComponent;
class USphereComponent;
class UAudioComponent;

/**
 * @class AHarmoniaCrystalResonator
 * @brief Crystal Resonator - Checkpoint Actor
 *
 * A creative checkpoint system resembling magical crystals resonating around a bonfire.
 *
 * Key Features:
 * - Checkpoint activation via interaction
 * - Resonance system - rest and recover
 * - Each crystal has a unique "resonance frequency" (color/effect)
 * - Checkpoint upgrade system
 * - Teleport functionality (resonance network)
 *
 * Creative Elements:
 * - Inactive state: Dark, dimly glowing crystal
 * - Active state: Brightly resonating, emitting energy
 * - Unique color/effect per resonance frequency
 * - Resonance effects between nearby crystals
 * - Resonance Echo - display of last resonated location
 */
UCLASS(Blueprintable, ClassGroup = "Harmonia")
class HARMONIAKIT_API AHarmoniaCrystalResonator : public AActor, public IHarmoniaInteractableInterface
{
	GENERATED_BODY()

public:
	AHarmoniaCrystalResonator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ============================================================================
	// Components
	// ============================================================================

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Crystal mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CrystalMesh;

	/** Base platform mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	/** Point light (resonance effect) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> ResonanceLight;

	/** Particle system (resonance effect) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ResonanceEffect;

	/** Interaction range sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	/** Audio component (resonance sound) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> ResonanceAudio;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Checkpoint unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	/** Checkpoint display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	/** Checkpoint description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (MultiLine = true))
	FText CheckpointDescription;

	/** Resonance frequency (color/property) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	/** Auto-activate on start (for debugging) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Debug")
	bool bStartActivated = false;

	/** Interaction range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float InteractionRange = 200.0f;

	// ============================================================================
	// Visual Configuration
	// ============================================================================

	/** Inactive state light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float InactiveLightIntensity = 500.0f;

	/** Active state light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ActiveLightIntensity = 2000.0f;

	/** Resonating state light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonatingLightIntensity = 5000.0f;

	/** Colors per resonance frequency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	TMap<EHarmoniaResonanceFrequency, FLinearColor> FrequencyColors;

	/** Resonance pulse speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonancePulseSpeed = 2.0f;

	// ============================================================================
	// Audio Configuration
	// ============================================================================

	/** Activation sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ActivationSound;

	/** Resonance start sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceStartSound;

	/** Resonance loop sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceLoopSound;

	/** Resonance complete sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceCompleteSound;

	/** Teleport arrival sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> TeleportArrivalSound;

	// ============================================================================
	// State
	// ============================================================================

	/** Current state */
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State", Replicated)
	EHarmoniaCheckpointState CurrentState = EHarmoniaCheckpointState::Inactive;

	/** Currently resonating players */
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State")
	TArray<TObjectPtr<APlayerController>> ResonatingPlayers;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * Activate the checkpoint
	 * @param Player Player activating the checkpoint
	 * @return Whether activation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool Activate(APlayerController* Player);

	/**
	 * Start resonance (rest/recover)
	 * @param Player Player resonating
	 * @return Whether resonance started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool StartResonance(APlayerController* Player);

	/**
	 * End resonance
	 * @param Player Player currently resonating
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void EndResonance(APlayerController* Player);

	/** Change checkpoint state */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void SetCheckpointState(EHarmoniaCheckpointState NewState);

	/** Get checkpoint data */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FHarmoniaCheckpointData GetCheckpointData() const;

	/** Apply checkpoint data */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void ApplyCheckpointData(const FHarmoniaCheckpointData& Data);

	/** Check if activated */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsActivated() const { return CurrentState != EHarmoniaCheckpointState::Inactive; }

	/** Check if currently resonating */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsResonating() const { return CurrentState == EHarmoniaCheckpointState::Resonating; }

	/** Check if player is resonating */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsPlayerResonating(APlayerController* Player) const;

	/** Get frequency color */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FLinearColor GetFrequencyColor() const;

	// ============================================================================
	// Interaction Interface
	// ============================================================================

	virtual void OnInteract_Implementation(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult) override;

	// ============================================================================
	// Replication
	// ============================================================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Register to checkpoint subsystem */
	void RegisterToSubsystem();

	/** Update visual state */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateVisuals();
	virtual void UpdateVisuals_Implementation();

	/** Resonance pulse effect update */
	void UpdateResonancePulse(float DeltaTime);

	/** Update resonance effect with nearby crystals */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateNearbyResonance();
	virtual void UpdateNearbyResonance_Implementation();

	/** Apply frequency-specific effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void ApplyFrequencyEffects();
	virtual void ApplyFrequencyEffects_Implementation();

	/** Play activation visual effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayActivationEffects();
	virtual void PlayActivationEffects_Implementation();

	/** Play resonance start visual effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceStartEffects();
	virtual void PlayResonanceStartEffects_Implementation();

	/** Play resonance complete visual effects */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceCompleteEffects();
	virtual void PlayResonanceCompleteEffects_Implementation();

	/** Initialize default frequency colors */
	void InitializeFrequencyColors();

private:
	/** Resonance pulse timer */
	float ResonancePulseTimer = 0.0f;

	/** Last resonance time */
	FDateTime LastResonanceTime;

	/** Upgrade levels */
	TMap<EHarmoniaCheckpointUpgradeType, int32> UpgradeLevels;

#if WITH_EDITORONLY_DATA
	/** Auto-generate checkpoint ID in editor */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
