// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "HarmoniaCheckpointSubsystem.generated.h"

class AHarmoniaCrystalResonator;
class AHarmoniaMonsterSpawner;
class APlayerController;

/**
 * Checkpoint Events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointActivated, FName, CheckpointID, const FHarmoniaCheckpointData&, CheckpointData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResonanceStarted, FName, CheckpointID, APlayerController*, Player, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResonanceCompleted, FName, CheckpointID, const FHarmoniaResonanceResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResonanceCancelled, FName, CheckpointID, FText, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointTeleport, FName, FromCheckpointID, FName, ToCheckpointID, const FHarmoniaTeleportResult&, Result);

/**
 * UHarmoniaCheckpointSubsystem
 *
 * ?¬ë¦¬?¤íƒˆ ê³µëª…ê¸?ì²´í¬?¬ì¸???œìŠ¤??ê´€ë¦?
 *
 * ì£¼ìš” ê¸°ëŠ¥:
 * - ì²´í¬?¬ì¸???±ë¡ ë°??œì„±??
 * - ê³µëª…(?´ì‹) ?œìŠ¤??- ?Œë³µ + ??ë¦¬ìŠ¤??+ ?ë™ ?€??
 * - ì²´í¬?¬ì¸??ê°??”ë ˆ?¬íŠ¸ (ê³µëª… ?¤íŠ¸?Œí¬)
 * - ì²´í¬?¬ì¸??ê°•í™” ?œìŠ¤??
 * - ì£½ì—ˆ????ë§ˆì?ë§?ì²´í¬?¬ì¸?¸ì—??ë¦¬ìŠ¤??
 * - ê³µëª… ì£¼íŒŒ?˜ë³„ ê³ ìœ ???¨ê³¼
 *
 * ?…ì°½???”ì†Œ:
 * - ê°??¬ë¦¬?¤íƒˆë§ˆë‹¤ ê³ ìœ ??"ê³µëª… ì£¼íŒŒ?? (?‰ìƒ/?Œí–¥)
 * - ì²´í¬?¬ì¸??ê°•í™”ë¡??Œë³µ??ì¦ê?
 * - ê°€ê¹Œìš´ ì²´í¬?¬ì¸?¸ë“¤?¼ë¦¬ ê³µëª… ?¨ê³¼
 * - ê³µëª… ?¤íŠ¸?Œí¬ë¥??µí•œ ?”ë ˆ?¬íŠ¸
 */
UCLASS(Config=Game)
class HARMONIAKIT_API UHarmoniaCheckpointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaCheckpointSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End of USubsystem interface

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸???¤ì • ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	const FHarmoniaCheckpointConfig& GetCheckpointConfig() const { return CheckpointConfig; }

	/**
	 * ì²´í¬?¬ì¸???¤ì • ë³€ê²?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void SetCheckpointConfig(const FHarmoniaCheckpointConfig& NewConfig) { CheckpointConfig = NewConfig; }

	// ============================================================================
	// Checkpoint Registration
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸???±ë¡
	 * @param Checkpoint ?±ë¡??ì²´í¬?¬ì¸???¡í„°
	 * @return ?±ë¡ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool RegisterCheckpoint(AHarmoniaCrystalResonator* Checkpoint);

	/**
	 * ì²´í¬?¬ì¸???±ë¡ ?´ì œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void UnregisterCheckpoint(FName CheckpointID);

	/**
	 * ì²´í¬?¬ì¸??ì°¾ê¸°
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	AHarmoniaCrystalResonator* FindCheckpoint(FName CheckpointID) const;

	/**
	 * ëª¨ë“  ì²´í¬?¬ì¸??ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	TArray<AHarmoniaCrystalResonator*> GetAllCheckpoints() const;

	/**
	 * ?œì„±?”ëœ ì²´í¬?¬ì¸?¸ë§Œ ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	TArray<AHarmoniaCrystalResonator*> GetActivatedCheckpoints() const;

	// ============================================================================
	// Checkpoint Activation
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸???œì„±??
	 * @param CheckpointID ?œì„±?”í•  ì²´í¬?¬ì¸??ID
	 * @param Player ?œì„±?”í•˜???Œë ˆ?´ì–´
	 * @return ?œì„±???±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool ActivateCheckpoint(FName CheckpointID, APlayerController* Player);

	/**
	 * ì²´í¬?¬ì¸???œì„±???¬ë? ?•ì¸
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	bool IsCheckpointActivated(FName CheckpointID) const;

	// ============================================================================
	// Resonance System (Rest/Recovery)
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸?¸ì—??ê³µëª… ?œì‘ (?´ì‹/?Œë³µ)
	 * @param CheckpointID ê³µëª…??ì²´í¬?¬ì¸??ID
	 * @param Player ê³µëª…?˜ëŠ” ?Œë ˆ?´ì–´
	 * @return ê³µëª… ?œì‘ ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool StartResonance(FName CheckpointID, APlayerController* Player);

	/**
	 * ê³µëª… ì·¨ì†Œ
	 * @param Player ê³µëª… ì¤‘ì¸ ?Œë ˆ?´ì–´
	 * @param Reason ì·¨ì†Œ ?¬ìœ 
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void CancelResonance(APlayerController* Player, const FText& Reason);

	/**
	 * ?Œë ˆ?´ì–´ê°€ ?„ì¬ ê³µëª… ì¤‘ì¸ì§€ ?•ì¸
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	bool IsPlayerResonating(APlayerController* Player) const;

	/**
	 * ?Œë ˆ?´ì–´ê°€ ê³µëª… ì¤‘ì¸ ì²´í¬?¬ì¸??ID ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	FName GetResonatingCheckpointID(APlayerController* Player) const;

	// ============================================================================
	// Teleportation
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸?¸ë¡œ ?”ë ˆ?¬íŠ¸
	 * @param Player ?”ë ˆ?¬íŠ¸???Œë ˆ?´ì–´
	 * @param DestinationCheckpointID ëª©ì ì§€ ì²´í¬?¬ì¸??ID
	 * @return ?”ë ˆ?¬íŠ¸ ê²°ê³¼
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	FHarmoniaTeleportResult TeleportToCheckpoint(APlayerController* Player, FName DestinationCheckpointID);

	/**
	 * ?”ë ˆ?¬íŠ¸ ê°€???¬ë? ?•ì¸
	 * @param Player ?”ë ˆ?¬íŠ¸???Œë ˆ?´ì–´
	 * @param DestinationCheckpointID ëª©ì ì§€ ì²´í¬?¬ì¸??ID
	 * @param OutReason ë¶ˆê??¥í•œ ?´ìœ  (?¤íŒ¨ ??
	 * @return ?”ë ˆ?¬íŠ¸ ê°€???¬ë?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	bool CanTeleportToCheckpoint(APlayerController* Player, FName DestinationCheckpointID, FText& OutReason) const;

	/**
	 * ?”ë ˆ?¬íŠ¸ ë¹„ìš© ê³„ì‚°
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	int32 CalculateTeleportCost(FName FromCheckpointID, FName ToCheckpointID) const;

	// ============================================================================
	// Checkpoint Upgrades
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸??ê°•í™”
	 * @param CheckpointID ê°•í™”??ì²´í¬?¬ì¸??ID
	 * @param UpgradeType ê°•í™” ?€??
	 * @return ê°•í™” ?±ê³µ ?¬ë?
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool UpgradeCheckpoint(FName CheckpointID, EHarmoniaCheckpointUpgradeType UpgradeType);

	/**
	 * ì²´í¬?¬ì¸??ê°•í™” ?ˆë²¨ ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	int32 GetCheckpointUpgradeLevel(FName CheckpointID, EHarmoniaCheckpointUpgradeType UpgradeType) const;

	// ============================================================================
	// Respawn System
	// ============================================================================

	/**
	 * ?Œë ˆ?´ì–´ ë§ˆì?ë§?ê³µëª… ì²´í¬?¬ì¸???¤ì •
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void SetPlayerLastCheckpoint(APlayerController* Player, FName CheckpointID);

	/**
	 * ?Œë ˆ?´ì–´ ë§ˆì?ë§?ê³µëª… ì²´í¬?¬ì¸??ê°€?¸ì˜¤ê¸?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	FName GetPlayerLastCheckpoint(APlayerController* Player) const;

	/**
	 * ?Œë ˆ?´ì–´ë¥?ë§ˆì?ë§?ì²´í¬?¬ì¸?¸ì—??ë¦¬ìŠ¤??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	bool RespawnPlayerAtLastCheckpoint(APlayerController* Player);

	// ============================================================================
	// Monster Respawn
	// ============================================================================

	/**
	 * ëª¬ìŠ¤???¤í¬???±ë¡ (ê³µëª… ??ë¦¬ì…‹??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void RegisterMonsterSpawner(AHarmoniaMonsterSpawner* Spawner);

	/**
	 * ëª¬ìŠ¤???¤í¬???±ë¡ ?´ì œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void UnregisterMonsterSpawner(AHarmoniaMonsterSpawner* Spawner);

	/**
	 * ëª¨ë“  ??ë¦¬ìŠ¤??(ê³µëª… ???¸ì¶œ)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	int32 RespawnAllEnemies();

	// ============================================================================
	// Data Persistence
	// ============================================================================

	/**
	 * ì²´í¬?¬ì¸???°ì´???€??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void SaveCheckpointData();

	/**
	 * ì²´í¬?¬ì¸???°ì´??ë¡œë“œ
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void LoadCheckpointData();

	/**
	 * ì²´í¬?¬ì¸???°ì´??ê°€?¸ì˜¤ê¸?(?¸ì´ë¸??œìŠ¤???°ë™??
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Checkpoint")
	TArray<FHarmoniaCheckpointData> GetCheckpointDataForSave() const;

	/**
	 * ì²´í¬?¬ì¸???°ì´???ìš© (ë¡œë“œ ?œìŠ¤???°ë™??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Checkpoint")
	void ApplyCheckpointDataFromLoad(const TArray<FHarmoniaCheckpointData>& LoadedData);

	// ============================================================================
	// Events
	// ============================================================================

	/** ì²´í¬?¬ì¸???œì„±???´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnCheckpointActivated OnCheckpointActivated;

	/** ê³µëª… ?œì‘ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnResonanceStarted OnResonanceStarted;

	/** ê³µëª… ?„ë£Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnResonanceCompleted OnResonanceCompleted;

	/** ê³µëª… ì·¨ì†Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnResonanceCancelled OnResonanceCancelled;

	/** ?”ë ˆ?¬íŠ¸ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Checkpoint|Events")
	FOnCheckpointTeleport OnCheckpointTeleport;

protected:
	/**
	 * ê³µëª… ?„ë£Œ ì²˜ë¦¬
	 */
	void CompleteResonance(APlayerController* Player, FName CheckpointID);

	/**
	 * ?Œë ˆ?´ì–´ ?Œë³µ
	 */
	void RestorePlayerHealth(APlayerController* Player, float RestorationRate);

	/**
	 * ê²Œì„ ?ë™ ?€??
	 */
	void AutoSaveGame(APlayerController* Player);

	/**
	 * ê³µëª… ì£¼íŒŒ?˜ì— ?°ë¥¸ ì¶”ê? ?¨ê³¼
	 */
	void ApplyResonanceFrequencyEffects(APlayerController* Player, EHarmoniaResonanceFrequency Frequency);

private:
	/** ì²´í¬?¬ì¸???¤ì • */
	UPROPERTY(Config)
	FHarmoniaCheckpointConfig CheckpointConfig;

	/** ?±ë¡??ì²´í¬?¬ì¸?¸ë“¤ (ID -> Actor) */
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<AHarmoniaCrystalResonator>> RegisteredCheckpoints;

	/** ì²´í¬?¬ì¸???°ì´??(ID -> Data) */
	UPROPERTY(Transient)
	TMap<FName, FHarmoniaCheckpointData> CheckpointDataMap;

	/** ?Œë ˆ?´ì–´ë³?ë§ˆì?ë§?ì²´í¬?¬ì¸??(PlayerController -> CheckpointID) */
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerController>, FName> PlayerLastCheckpoints;

	/** ê³µëª… ì¤‘ì¸ ?Œë ˆ?´ì–´ (PlayerController -> CheckpointID) */
	UPROPERTY(Transient)
	TMap<TObjectPtr<APlayerController>, FName> ResonatingPlayers;

	/** ê³µëª… ?€?´ë¨¸ ?¸ë“¤ (PlayerController -> TimerHandle) */
	TMap<APlayerController*, FTimerHandle> ResonanceTimerHandles;

	/** ?±ë¡??ëª¬ìŠ¤???¤í¬?ˆë“¤ */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AHarmoniaMonsterSpawner>> RegisteredSpawners;
};
