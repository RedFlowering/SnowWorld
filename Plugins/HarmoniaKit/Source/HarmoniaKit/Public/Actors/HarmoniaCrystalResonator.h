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
 * AHarmoniaCrystalResonator
 *
 * ?¬ë¦¬?¤íƒˆ ê³µëª…ê¸?- ì²´í¬?¬ì¸???¡í„°
 *
 * ëª¨ë‹¥ë¶??€??ë§ˆë²• ?¬ë¦¬?¤íƒˆ??ê³µëª…?˜ë©° ?ë„ˆì§€ë¥?ë°©ì¶œ?˜ëŠ” ?…ì°½?ì¸ ì²´í¬?¬ì¸???œìŠ¤??
 *
 * ì£¼ìš” ê¸°ëŠ¥:
 * - ?í˜¸?‘ìš©?¼ë¡œ ì²´í¬?¬ì¸???œì„±??
 * - ê³µëª…(Resonance) ?œìŠ¤??- ?´ì‹?˜ì—¬ ?Œë³µ
 * - ê°??¬ë¦¬?¤íƒˆë§ˆë‹¤ ê³ ìœ ??"ê³µëª… ì£¼íŒŒ?? (?‰ìƒ/?Œí–¥)
 * - ì²´í¬?¬ì¸??ê°•í™” ?œìŠ¤??
 * - ?”ë ˆ?¬íŠ¸ ê¸°ëŠ¥ (ê³µëª… ?¤íŠ¸?Œí¬)
 *
 * ?…ì°½???”ì†Œ:
 * - ë¹„í™œ?±í™” ?íƒœ: ?´ë‘¡ê²?ë¹›ë‚˜??? ë“  ?¬ë¦¬?¤íƒˆ
 * - ?œì„±???íƒœ: ë°ê²Œ ê³µëª…?˜ë©° ?ë„ˆì§€ë¥?ë°©ì¶œ
 * - ê³µëª… ì£¼íŒŒ?˜ë³„ ê³ ìœ ???‰ìƒ/?Œí–¥/?¨ê³¼
 * - ê°€ê¹Œìš´ ?¬ë¦¬?¤íƒˆ?¤ë¼ë¦?ê³µëª… ?¨ê³¼ ë°œìƒ
 * - ê³µëª… ?”í–¥ - ë§ˆì?ë§‰ìœ¼ë¡?ê³µëª…???„ì¹˜ ?œì‹œ
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

	/** ë£¨íŠ¸ ì»´í¬?ŒíŠ¸ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** ?¬ë¦¬?¤íƒˆ ë©”ì‹œ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CrystalMesh;

	/** ë°›ì¹¨?€ ë©”ì‹œ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	/** ?¬ì¸???¼ì´??(ê³µëª… ?¨ê³¼) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> ResonanceLight;

	/** ?Œí‹°???œìŠ¤??(ê³µëª… ?¨ê³¼) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ResonanceEffect;

	/** ?í˜¸?‘ìš© ë²”ìœ„ */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	/** ?¤ë””??ì»´í¬?ŒíŠ¸ (ê³µëª… ?Œë¦¬) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> ResonanceAudio;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** ì²´í¬?¬ì¸??ê³ ìœ  ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	/** ì²´í¬?¬ì¸???´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	/** ì²´í¬?¬ì¸???¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (MultiLine = true))
	FText CheckpointDescription;

	/** ê³µëª… ì£¼íŒŒ??(?‰ìƒ/?¹ì„±) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	/** ?œì‘ ???ë™ ?œì„±???¬ë? (?”ë²„ê·¸ìš©) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Debug")
	bool bStartActivated = false;

	/** ?í˜¸?‘ìš© ë²”ìœ„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float InteractionRange = 200.0f;

	// ============================================================================
	// Visual Configuration
	// ============================================================================

	/** ë¹„í™œ?±í™” ?íƒœ ë°œê´‘ ?¸ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float InactiveLightIntensity = 500.0f;

	/** ?œì„±???íƒœ ë°œê´‘ ?¸ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ActiveLightIntensity = 2000.0f;

	/** ê³µëª… ?íƒœ ë°œê´‘ ?¸ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonatingLightIntensity = 5000.0f;

	/** ê³µëª… ì£¼íŒŒ?˜ë³„ ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	TMap<EHarmoniaResonanceFrequency, FLinearColor> FrequencyColors;

	/** ê³µëª… ?„ìŠ¤ ?ë„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonancePulseSpeed = 2.0f;

	// ============================================================================
	// Audio Configuration
	// ============================================================================

	/** ?œì„±???¬ìš´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ActivationSound;

	/** ê³µëª… ?œì‘ ?¬ìš´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceStartSound;

	/** ê³µëª… ë£¨í”„ ?¬ìš´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceLoopSound;

	/** ê³µëª… ?„ë£Œ ?¬ìš´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceCompleteSound;

	/** ?”ë ˆ?¬íŠ¸ ?„ì°© ?¬ìš´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> TeleportArrivalSound;

	// ============================================================================
	// State
	// ============================================================================

	/** ?„ì¬ ?íƒœ */
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State", Replicated)
	EHarmoniaCheckpointState CurrentState = EHarmoniaCheckpointState::Inactive;

	/** ?„ì¬ ê³µëª… ì¤‘ì¸ ?Œë ˆ?´ì–´??*/
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State")
	TArray<TObjectPtr<APlayerController>> ResonatingPlayers;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸???œì„±??
	 * @param Player ?œì„±?”í•˜???Œë ˆ?´ì–´
	 * @return ?œì„±???±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool Activate(APlayerController* Player);

	/**
	 * ê³µëª… ?œì‘ (?´ì‹/?Œë³µ)
	 * @param Player ê³µëª…?˜ëŠ” ?Œë ˆ?´ì–´
	 * @return ê³µëª… ?œì‘ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool StartResonance(APlayerController* Player);

	/**
	 * ê³µëª… ì¢…ë£Œ
	 * @param Player ê³µëª… ì¤‘ì¸ ?Œë ˆ?´ì–´
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void EndResonance(APlayerController* Player);

	/**
	 * ?íƒœ ë³€ê²?
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void SetCheckpointState(EHarmoniaCheckpointState NewState);

	/**
	 * ì²´í¬?¬ì¸???°ì´??ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FHarmoniaCheckpointData GetCheckpointData() const;

	/**
	 * ì²´í¬?¬ì¸???°ì´???ìš©
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void ApplyCheckpointData(const FHarmoniaCheckpointData& Data);

	/**
	 * ?œì„±???¬ë?
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsActivated() const { return CurrentState != EHarmoniaCheckpointState::Inactive; }

	/**
	 * ê³µëª… ì¤‘ì¸ì§€ ?•ì¸
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsResonating() const { return CurrentState == EHarmoniaCheckpointState::Resonating; }

	/**
	 * ?Œë ˆ?´ì–´ê°€ ê³µëª… ì¤‘ì¸ì§€ ?•ì¸
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsPlayerResonating(APlayerController* Player) const;

	/**
	 * ê³µëª… ì£¼íŒŒ???‰ìƒ ê°€?¸ì˜¤ê¸?
	 */
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
	/**
	 * ì²´í¬?¬ì¸???œë¸Œ?œìŠ¤???±ë¡
	 */
	void RegisterToSubsystem();

	/**
	 * ë¹„ì£¼???…ë°?´íŠ¸
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateVisuals();
	virtual void UpdateVisuals_Implementation();

	/**
	 * ê³µëª… ?„ìŠ¤ ?¨ê³¼
	 */
	void UpdateResonancePulse(float DeltaTime);

	/**
	 * ê°€ê¹Œìš´ ?¬ë¦¬?¤íƒˆê³?ê³µëª… ?¨ê³¼
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateNearbyResonance();
	virtual void UpdateNearbyResonance_Implementation();

	/**
	 * ê³µëª… ì£¼íŒŒ?˜ë³„ ?¹ìˆ˜ ?¨ê³¼
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void ApplyFrequencyEffects();
	virtual void ApplyFrequencyEffects_Implementation();

	/**
	 * ?œì„±??ë¹„ì£¼???¨ê³¼
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayActivationEffects();
	virtual void PlayActivationEffects_Implementation();

	/**
	 * ê³µëª… ?œì‘ ë¹„ì£¼???¨ê³¼
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceStartEffects();
	virtual void PlayResonanceStartEffects_Implementation();

	/**
	 * ê³µëª… ?„ë£Œ ë¹„ì£¼???¨ê³¼
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceCompleteEffects();
	virtual void PlayResonanceCompleteEffects_Implementation();

	/**
	 * ì´ˆê¸° ?‰ìƒ ?¤ì •
	 */
	void InitializeFrequencyColors();

private:
	/** ê³µëª… ?„ìŠ¤ ?€?´ë¨¸ */
	float ResonancePulseTimer = 0.0f;

	/** ë§ˆì?ë§?ê³µëª… ?œê°„ */
	FDateTime LastResonanceTime;

	/** ê°•í™” ?ˆë²¨ */
	TMap<EHarmoniaCheckpointUpgradeType, int32> UpgradeLevels;

#if WITH_EDITORONLY_DATA
	/** ?ë””?°ì—??ì²´í¬?¬ì¸??ID ?ë™ ?ì„± */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
