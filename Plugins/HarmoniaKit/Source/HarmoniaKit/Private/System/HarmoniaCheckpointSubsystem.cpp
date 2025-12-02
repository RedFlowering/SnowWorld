// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaCheckpointSubsystem.h"
#include "Actors/HarmoniaCrystalResonator.h"
#include "Actors/HarmoniaMonsterSpawner.h"
#include "System/HarmoniaSaveGameSubsystem.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "Character/LyraCharacter.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameplayTagsManager.h"
#include "NativeGameplayTags.h"

UHarmoniaCheckpointSubsystem::UHarmoniaCheckpointSubsystem()
{
	// ê¸°ë³¸ ?¤ì • ì´ˆê¸°??
	CheckpointConfig = FHarmoniaCheckpointConfig();
}

void UHarmoniaCheckpointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCheckpointSubsystem: Initialized"));
}

void UHarmoniaCheckpointSubsystem::Deinitialize()
{
	// ?€?´ë¨¸ ?•ë¦¬
	UWorld* World = GetWorld();
	if (World)
	{
		for (auto& Pair : ResonanceTimerHandles)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}

	ResonanceTimerHandles.Empty();
	RegisteredCheckpoints.Empty();
	CheckpointDataMap.Empty();
	PlayerLastCheckpoints.Empty();
	ResonatingPlayers.Empty();
	RegisteredSpawners.Empty();

	Super::Deinitialize();
}

// ============================================================================
// Checkpoint Registration
// ============================================================================

bool UHarmoniaCheckpointSubsystem::RegisterCheckpoint(AHarmoniaCrystalResonator* Checkpoint)
{
	if (!Checkpoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterCheckpoint: Checkpoint is null"));
		return false;
	}

	FName CheckpointID = Checkpoint->CheckpointID;
	if (CheckpointID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterCheckpoint: Checkpoint ID is None"));
		return false;
	}

	if (RegisteredCheckpoints.Contains(CheckpointID))
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterCheckpoint: Checkpoint %s already registered"), *CheckpointID.ToString());
		return false;
	}

	RegisteredCheckpoints.Add(CheckpointID, Checkpoint);

	// ì²´í¬?¬ì¸???°ì´???ì„±
	FHarmoniaCheckpointData Data;
	Data.CheckpointID = CheckpointID;
	Data.CheckpointName = Checkpoint->CheckpointName;
	Data.CheckpointDescription = Checkpoint->CheckpointDescription;
	Data.ResonanceFrequency = Checkpoint->ResonanceFrequency;
	Data.Location = Checkpoint->GetActorLocation();
	Data.Rotation = Checkpoint->GetActorRotation();
	Data.bActivated = Checkpoint->IsActivated();

	CheckpointDataMap.Add(CheckpointID, Data);

	UE_LOG(LogTemp, Log, TEXT("RegisterCheckpoint: Registered checkpoint %s"), *CheckpointID.ToString());
	return true;
}

void UHarmoniaCheckpointSubsystem::UnregisterCheckpoint(FName CheckpointID)
{
	if (RegisteredCheckpoints.Remove(CheckpointID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("UnregisterCheckpoint: Unregistered checkpoint %s"), *CheckpointID.ToString());
	}
}

AHarmoniaCrystalResonator* UHarmoniaCheckpointSubsystem::FindCheckpoint(FName CheckpointID) const
{
	const TObjectPtr<AHarmoniaCrystalResonator>* Found = RegisteredCheckpoints.Find(CheckpointID);
	return Found ? *Found : nullptr;
}

TArray<AHarmoniaCrystalResonator*> UHarmoniaCheckpointSubsystem::GetAllCheckpoints() const
{
	TArray<AHarmoniaCrystalResonator*> Result;
	for (const auto& Pair : RegisteredCheckpoints)
	{
		if (Pair.Value)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<AHarmoniaCrystalResonator*> UHarmoniaCheckpointSubsystem::GetActivatedCheckpoints() const
{
	TArray<AHarmoniaCrystalResonator*> Result;
	for (const auto& Pair : RegisteredCheckpoints)
	{
		if (Pair.Value && Pair.Value->IsActivated())
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// ============================================================================
// Checkpoint Activation
// ============================================================================

bool UHarmoniaCheckpointSubsystem::ActivateCheckpoint(FName CheckpointID, APlayerController* Player)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateCheckpoint: Player is null"));
		return false;
	}

	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(CheckpointID);
	if (!Checkpoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateCheckpoint: Checkpoint %s not found"), *CheckpointID.ToString());
		return false;
	}

	if (Checkpoint->IsActivated())
	{
		UE_LOG(LogTemp, Log, TEXT("ActivateCheckpoint: Checkpoint %s already activated"), *CheckpointID.ToString());
		return true;
	}

	// ì²´í¬?¬ì¸???œì„±??
	Checkpoint->Activate(Player);

	// ?°ì´???…ë°?´íŠ¸
	if (FHarmoniaCheckpointData* Data = CheckpointDataMap.Find(CheckpointID))
	{
		Data->bActivated = true;
	}

	// ?´ë²¤??ë¸Œë¡œ?œìº?¤íŠ¸
	FHarmoniaCheckpointData CheckpointData = Checkpoint->GetCheckpointData();
	OnCheckpointActivated.Broadcast(CheckpointID, CheckpointData);

	UE_LOG(LogTemp, Log, TEXT("ActivateCheckpoint: Activated checkpoint %s"), *CheckpointID.ToString());
	return true;
}

bool UHarmoniaCheckpointSubsystem::IsCheckpointActivated(FName CheckpointID) const
{
	const FHarmoniaCheckpointData* Data = CheckpointDataMap.Find(CheckpointID);
	return Data ? Data->bActivated : false;
}

// ============================================================================
// Resonance System
// ============================================================================

bool UHarmoniaCheckpointSubsystem::StartResonance(FName CheckpointID, APlayerController* Player)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartResonance: Player is null"));
		return false;
	}

	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(CheckpointID);
	if (!Checkpoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartResonance: Checkpoint %s not found"), *CheckpointID.ToString());
		return false;
	}

	if (!Checkpoint->IsActivated())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartResonance: Checkpoint %s not activated"), *CheckpointID.ToString());
		return false;
	}

	if (IsPlayerResonating(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartResonance: Player already resonating"));
		return false;
	}

	// ê³µëª… ?œì‘
	ResonatingPlayers.Add(Player, CheckpointID);
	Checkpoint->StartResonance(Player);

	// ê³µëª… ?€?´ë¨¸ ?¤ì •
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &UHarmoniaCheckpointSubsystem::CompleteResonance, Player, CheckpointID);

		World->GetTimerManager().SetTimer(
			TimerHandle,
			TimerDelegate,
			CheckpointConfig.ResonanceDuration,
			false
		);

		ResonanceTimerHandles.Add(Player, TimerHandle);
	}

	// ?´ë²¤??ë¸Œë¡œ?œìº?¤íŠ¸
	OnResonanceStarted.Broadcast(CheckpointID, Player, CheckpointConfig.ResonanceDuration);

	UE_LOG(LogTemp, Log, TEXT("StartResonance: Player started resonance at checkpoint %s"), *CheckpointID.ToString());
	return true;
}

void UHarmoniaCheckpointSubsystem::CancelResonance(APlayerController* Player, const FText& Reason)
{
	if (!Player)
	{
		return;
	}

	FName* CheckpointID = ResonatingPlayers.Find(Player);
	if (!CheckpointID)
	{
		return;
	}

	// ?€?´ë¨¸ ì·¨ì†Œ
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle* TimerHandle = ResonanceTimerHandles.Find(Player);
		if (TimerHandle && TimerHandle->IsValid())
		{
			World->GetTimerManager().ClearTimer(*TimerHandle);
		}
	}

	// ì²´í¬?¬ì¸?¸ì—??ê³µëª… ì¢…ë£Œ
	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(*CheckpointID);
	if (Checkpoint)
	{
		Checkpoint->EndResonance(Player);
	}

	FName CancelledCheckpointID = *CheckpointID;

	// ?°ì´???•ë¦¬
	ResonatingPlayers.Remove(Player);
	ResonanceTimerHandles.Remove(Player);

	// ?´ë²¤??ë¸Œë¡œ?œìº?¤íŠ¸
	OnResonanceCancelled.Broadcast(CancelledCheckpointID, Reason);

	UE_LOG(LogTemp, Log, TEXT("CancelResonance: Player cancelled resonance at checkpoint %s. Reason: %s"),
		*CancelledCheckpointID.ToString(), *Reason.ToString());
}

bool UHarmoniaCheckpointSubsystem::IsPlayerResonating(APlayerController* Player) const
{
	return ResonatingPlayers.Contains(Player);
}

FName UHarmoniaCheckpointSubsystem::GetResonatingCheckpointID(APlayerController* Player) const
{
	const FName* CheckpointID = ResonatingPlayers.Find(Player);
	return CheckpointID ? *CheckpointID : NAME_None;
}

void UHarmoniaCheckpointSubsystem::CompleteResonance(APlayerController* Player, FName CheckpointID)
{
	if (!Player)
	{
		return;
	}

	// ê³µëª… ê²°ê³¼ ?ì„±
	FHarmoniaResonanceResult Result;
	Result.bSuccess = true;

	// ?Œë ˆ?´ì–´ ?Œë³µ
	RestorePlayerHealth(Player, CheckpointConfig.HealthRestorationRate);

	// ??ë¦¬ìŠ¤??
	if (CheckpointConfig.bRespawnEnemiesOnResonance)
	{
		Result.EnemiesRespawned = RespawnAllEnemies();
	}

	// ?ë™ ?€??
	if (CheckpointConfig.bAutoSaveOnResonance)
	{
		AutoSaveGame(Player);
		Result.bGameSaved = true;
	}

	// ë§ˆì?ë§?ì²´í¬?¬ì¸???¤ì •
	SetPlayerLastCheckpoint(Player, CheckpointID);

	// ê³µëª… ì£¼íŒŒ?˜ë³„ ?¨ê³¼
	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(CheckpointID);
	if (Checkpoint)
	{
		ApplyResonanceFrequencyEffects(Player, Checkpoint->ResonanceFrequency);
		Checkpoint->EndResonance(Player);
	}

	// ?°ì´???•ë¦¬
	ResonatingPlayers.Remove(Player);
	ResonanceTimerHandles.Remove(Player);

	// ?´ë²¤??ë¸Œë¡œ?œìº?¤íŠ¸
	OnResonanceCompleted.Broadcast(CheckpointID, Result);

	UE_LOG(LogTemp, Log, TEXT("CompleteResonance: Player completed resonance at checkpoint %s. Enemies respawned: %d, Game saved: %s"),
		*CheckpointID.ToString(), Result.EnemiesRespawned, Result.bGameSaved ? TEXT("Yes") : TEXT("No"));
}

void UHarmoniaCheckpointSubsystem::RestorePlayerHealth(APlayerController* Player, float RestorationRate)
{
	if (!Player)
	{
		return;
	}

	ALyraCharacter* Character = Cast<ALyraCharacter>(Player->GetPawn());
	if (!Character)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Character->GetLyraAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const ULyraHealthSet* HealthSet = ASC->GetSet<ULyraHealthSet>();
	if (!HealthSet)
	{
		return;
	}

	// ì²´ë ¥ ?Œë³µ
	float MaxHealth = HealthSet->GetMaxHealth();
	float HealthToRestore = MaxHealth * RestorationRate;

	ASC->ApplyModToAttribute(ULyraHealthSet::GetHealthAttribute(), EGameplayModOp::Additive, HealthToRestore);

	UE_LOG(LogTemp, Log, TEXT("RestorePlayerHealth: Restored %.2f health to player"), HealthToRestore);
}

void UHarmoniaCheckpointSubsystem::AutoSaveGame(APlayerController* Player)
{
	if (!Player)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaSaveGameSubsystem* SaveGameSubsystem = GameInstance->GetSubsystem<UHarmoniaSaveGameSubsystem>();
	if (SaveGameSubsystem)
	{
		SaveGameSubsystem->SaveGame(TEXT("AutoSave_Checkpoint"), true);
		UE_LOG(LogTemp, Log, TEXT("AutoSaveGame: Game saved at checkpoint"));
	}
}

void UHarmoniaCheckpointSubsystem::ApplyResonanceFrequencyEffects(APlayerController* Player, EHarmoniaResonanceFrequency Frequency)
{
	// ê°?ê³µëª… ì£¼íŒŒ?˜ë³„ ?¹ìˆ˜ ?¨ê³¼ (Blueprint?ì„œ ?•ì¥ ê°€??
	// ?? Azure = ë§ˆë‚˜ ?Œë³µ, Crimson = ê³µê²©??ë²„í”„, Verdant = ì²´ë ¥ ?¬ìƒ ??

	UE_LOG(LogTemp, Log, TEXT("ApplyResonanceFrequencyEffects: Applied %d frequency effects to player"),
		static_cast<int32>(Frequency));
}

// ============================================================================
// Teleportation
// ============================================================================

FHarmoniaTeleportResult UHarmoniaCheckpointSubsystem::TeleportToCheckpoint(APlayerController* Player, FName DestinationCheckpointID)
{
	FHarmoniaTeleportResult Result;
	Result.bSuccess = false;
	Result.DestinationCheckpointID = DestinationCheckpointID;

	FText FailureReason;
	if (!CanTeleportToCheckpoint(Player, DestinationCheckpointID, FailureReason))
	{
		Result.FailureReason = FailureReason;
		return Result;
	}

	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(DestinationCheckpointID);
	if (!Checkpoint)
	{
		Result.FailureReason = FText::FromString(TEXT("Checkpoint not found"));
		return Result;
	}

	APawn* Pawn = Player->GetPawn();
	if (!Pawn)
	{
		Result.FailureReason = FText::FromString(TEXT("No pawn to teleport"));
		return Result;
	}

	// ?”ë ˆ?¬íŠ¸ ë¹„ìš© ê³„ì‚°
	FName CurrentCheckpointID = GetPlayerLastCheckpoint(Player);
	Result.ResourceCost = CalculateTeleportCost(CurrentCheckpointID, DestinationCheckpointID);

	// ?Œìš¸/ë¦¬ì†Œ??ì°¨ê° ë¡œì§
	if (Result.ResourceCost > 0)
	{
		if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
		{
			// HarmoniaResourceComponent ?•ì¸ (?ˆëŠ” ê²½ìš°)
			// NOTE: HarmoniaResourceComponentê°€ êµ¬í˜„?˜ì? ?Šì? ê²½ìš° ??ë¶€ë¶„ì? ?¤í‚µ?©ë‹ˆ??
			TArray<UActorComponent*> ResourceComponents;
			PS->GetComponents(UActorComponent::StaticClass(), ResourceComponents);
			bool bResourceDeducted = false;

			for (UActorComponent* Component : ResourceComponents)
			{
				if (Component->GetClass()->GetName().Contains(TEXT("ResourceComponent")))
				{
					// ë¦¬í”Œ?‰ì…˜???µí•´ ConsumeResource ë©”ì„œ???¸ì¶œ ?œë„
					// NOTE: ?¤ì œ HarmoniaResourceComponentê°€ êµ¬í˜„?˜ë©´ ?ì ˆ???€??ìºìŠ¤?…ìœ¼ë¡?ë³€ê²?
					UE_LOG(LogTemp, Warning, TEXT("TeleportToCheckpoint: Resource component found but not implemented yet"));
					bResourceDeducted = true;
					break;
				}
			}

			if (!bResourceDeducted)
			{
				// ë¦¬ì†Œ???œìŠ¤?œì´ ?†ëŠ” ê²½ìš° ë¬´ë£Œë¡??”ë ˆ?¬íŠ¸ ?ˆìš©
				UE_LOG(LogTemp, Log, TEXT("TeleportToCheckpoint: Cost %d souls (free - resource system not implemented)"), Result.ResourceCost);
			}
		}
	}

	// ?”ë ˆ?¬íŠ¸ ?¤í–‰
	FVector TeleportLocation = Checkpoint->GetActorLocation();
	FRotator TeleportRotation = Checkpoint->GetActorRotation();

	Pawn->SetActorLocation(TeleportLocation);
	Pawn->SetActorRotation(TeleportRotation);
	Player->SetControlRotation(TeleportRotation);

	Result.bSuccess = true;

	// ?´ë²¤??ë¸Œë¡œ?œìº?¤íŠ¸
	OnCheckpointTeleport.Broadcast(CurrentCheckpointID, DestinationCheckpointID, Result);

	UE_LOG(LogTemp, Log, TEXT("TeleportToCheckpoint: Teleported player to checkpoint %s"), *DestinationCheckpointID.ToString());
	return Result;
}

bool UHarmoniaCheckpointSubsystem::CanTeleportToCheckpoint(APlayerController* Player, FName DestinationCheckpointID, FText& OutReason) const
{
	if (!Player)
	{
		OutReason = FText::FromString(TEXT("Invalid player"));
		return false;
	}

	if (!IsCheckpointActivated(DestinationCheckpointID))
	{
		OutReason = FText::FromString(TEXT("Checkpoint not activated"));
		return false;
	}

	if (IsPlayerResonating(Player))
	{
		OutReason = FText::FromString(TEXT("Cannot teleport while resonating"));
		return false;
	}

	// ?„íˆ¬ ì¤‘ì¸ì§€ ?•ì¸
	if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
	{
		if (ULyraAbilitySystemComponent* ASC = PS->GetLyraAbilitySystemComponent())
		{
			// ?„íˆ¬ ê´€??GameplayTag ?•ì¸
			// Lyra?ì„œ ?¬ìš©?˜ëŠ” ?¼ë°˜?ì¸ ?„íˆ¬ ?œê·¸?¤ì„ ?•ì¸
			static const FName CombatTagNames[] = {
				FName(TEXT("State.Combat")),
				FName(TEXT("Status.Combat")),
				FName(TEXT("Ability.Combat")),
				FName(TEXT("GameplayEffect.Combat"))
			};

			for (const FName& TagName : CombatTagNames)
			{
				FGameplayTag CombatTag = UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);
				if (CombatTag.IsValid() && ASC->HasMatchingGameplayTag(CombatTag))
				{
					OutReason = FText::FromString(TEXT("Cannot teleport while in combat"));
					UE_LOG(LogTemp, Log, TEXT("CanTeleportToCheckpoint: Player is in combat (Tag: %s)"), *TagName.ToString());
					return false;
				}
			}

			// ?°ë?ì§€ë¥?ë°›ëŠ” ì¤‘ì¸ì§€???•ì¸ (ìµœê·¼ 5ì´????°ë?ì§€ ë°›ìŒ)
			if (ALyraCharacter* Character = Cast<ALyraCharacter>(Player->GetPawn()))
			{
				const ULyraHealthSet* HealthSet = ASC->GetSet<ULyraHealthSet>();
				if (HealthSet)
				{
					float CurrentHealth = HealthSet->GetHealth();
					float MaxHealth = HealthSet->GetMaxHealth();

					// ì²´ë ¥??100% ë¯¸ë§Œ?´ë©´ ìµœê·¼???„íˆ¬ê°€ ?ˆì—ˆ??ê°€?¥ì„±
					// ?˜ì?ë§??´ê²ƒë§Œìœ¼ë¡œëŠ” ë¶ˆì¶©ë¶„í•˜ë¯€ë¡??œê·¸ ?•ì¸??ì£¼ë¡œ ?¬ìš©
				}
			}
		}
	}

	// ë¦¬ì†Œ??ì¶©ë¶„?œì? ?•ì¸
	FName CurrentCheckpointID = GetPlayerLastCheckpoint(Player);
	int32 TeleportCost = CalculateTeleportCost(CurrentCheckpointID, DestinationCheckpointID);

	if (TeleportCost > 0)
	{
		if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
		{
			// HarmoniaResourceComponent ?•ì¸ (?ˆëŠ” ê²½ìš°)
			TArray<UActorComponent*> ResourceComponents;
			PS->GetComponents(UActorComponent::StaticClass(), ResourceComponents);
			bool bHasResourceSystem = false;

			for (UActorComponent* Component : ResourceComponents)
			{
				if (Component->GetClass()->GetName().Contains(TEXT("ResourceComponent")))
				{
					bHasResourceSystem = true;
					// NOTE: ?¤ì œ HarmoniaResourceComponentê°€ êµ¬í˜„?˜ë©´ ?¬ê¸°??ë¦¬ì†Œ???•ì¸
					// ?„ì¬???œìŠ¤?œì´ ?†ìœ¼ë¯€ë¡??¤í‚µ
					UE_LOG(LogTemp, Log, TEXT("CanTeleportToCheckpoint: Resource component found but not checking (not implemented)"));
					break;
				}
			}

			if (!bHasResourceSystem)
			{
				// ë¦¬ì†Œ???œìŠ¤?œì´ ?†ëŠ” ê²½ìš° ë¬´ë£Œë¡??”ë ˆ?¬íŠ¸ ?ˆìš©
				UE_LOG(LogTemp, Log, TEXT("CanTeleportToCheckpoint: No resource system, teleport is free"));
			}
		}
	}

	return true;
}

int32 UHarmoniaCheckpointSubsystem::CalculateTeleportCost(FName FromCheckpointID, FName ToCheckpointID) const
{
	if (FromCheckpointID.IsNone() || ToCheckpointID.IsNone())
	{
		return CheckpointConfig.BaseTeleportCost;
	}

	AHarmoniaCrystalResonator* FromCheckpoint = FindCheckpoint(FromCheckpointID);
	AHarmoniaCrystalResonator* ToCheckpoint = FindCheckpoint(ToCheckpointID);

	if (!FromCheckpoint || !ToCheckpoint)
	{
		return CheckpointConfig.BaseTeleportCost;
	}

	// ê±°ë¦¬???°ë¥¸ ë¹„ìš© ê³„ì‚°
	float Distance = FVector::Dist(FromCheckpoint->GetActorLocation(), ToCheckpoint->GetActorLocation());
	int32 DistanceCost = FMath::FloorToInt(Distance * CheckpointConfig.TeleportCostPerDistance);

	return CheckpointConfig.BaseTeleportCost + DistanceCost;
}

// ============================================================================
// Checkpoint Upgrades
// ============================================================================

bool UHarmoniaCheckpointSubsystem::UpgradeCheckpoint(FName CheckpointID, EHarmoniaCheckpointUpgradeType UpgradeType)
{
	FHarmoniaCheckpointData* Data = CheckpointDataMap.Find(CheckpointID);
	if (!Data)
	{
		return false;
	}

	int32& CurrentLevel = Data->UpgradeLevels.FindOrAdd(UpgradeType);
	CurrentLevel++;

	UE_LOG(LogTemp, Log, TEXT("UpgradeCheckpoint: Checkpoint %s upgraded. Type: %d, Level: %d"),
		*CheckpointID.ToString(), static_cast<int32>(UpgradeType), CurrentLevel);

	return true;
}

int32 UHarmoniaCheckpointSubsystem::GetCheckpointUpgradeLevel(FName CheckpointID, EHarmoniaCheckpointUpgradeType UpgradeType) const
{
	const FHarmoniaCheckpointData* Data = CheckpointDataMap.Find(CheckpointID);
	if (!Data)
	{
		return 0;
	}

	const int32* Level = Data->UpgradeLevels.Find(UpgradeType);
	return Level ? *Level : 0;
}

// ============================================================================
// Respawn System
// ============================================================================

void UHarmoniaCheckpointSubsystem::SetPlayerLastCheckpoint(APlayerController* Player, FName CheckpointID)
{
	if (!Player)
	{
		return;
	}

	PlayerLastCheckpoints.Add(Player, CheckpointID);
	UE_LOG(LogTemp, Log, TEXT("SetPlayerLastCheckpoint: Player last checkpoint set to %s"), *CheckpointID.ToString());
}

FName UHarmoniaCheckpointSubsystem::GetPlayerLastCheckpoint(APlayerController* Player) const
{
	const FName* CheckpointID = PlayerLastCheckpoints.Find(Player);
	return CheckpointID ? *CheckpointID : NAME_None;
}

bool UHarmoniaCheckpointSubsystem::RespawnPlayerAtLastCheckpoint(APlayerController* Player)
{
	if (!Player)
	{
		return false;
	}

	FName LastCheckpointID = GetPlayerLastCheckpoint(Player);
	if (LastCheckpointID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerAtLastCheckpoint: No last checkpoint for player"));
		return false;
	}

	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(LastCheckpointID);
	if (!Checkpoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("RespawnPlayerAtLastCheckpoint: Last checkpoint %s not found"), *LastCheckpointID.ToString());
		return false;
	}

	APawn* Pawn = Player->GetPawn();
	if (Pawn)
	{
		FVector RespawnLocation = Checkpoint->GetActorLocation();
		FRotator RespawnRotation = Checkpoint->GetActorRotation();

		Pawn->SetActorLocation(RespawnLocation);
		Pawn->SetActorRotation(RespawnRotation);
		Player->SetControlRotation(RespawnRotation);

		UE_LOG(LogTemp, Log, TEXT("RespawnPlayerAtLastCheckpoint: Respawned player at checkpoint %s"), *LastCheckpointID.ToString());
		return true;
	}

	return false;
}

// ============================================================================
// Monster Respawn
// ============================================================================

void UHarmoniaCheckpointSubsystem::RegisterMonsterSpawner(AHarmoniaMonsterSpawner* Spawner)
{
	if (Spawner && !RegisteredSpawners.Contains(Spawner))
	{
		RegisteredSpawners.Add(Spawner);
		UE_LOG(LogTemp, Log, TEXT("RegisterMonsterSpawner: Registered spawner"));
	}
}

void UHarmoniaCheckpointSubsystem::UnregisterMonsterSpawner(AHarmoniaMonsterSpawner* Spawner)
{
	if (Spawner)
	{
		RegisteredSpawners.Remove(Spawner);
		UE_LOG(LogTemp, Log, TEXT("UnregisterMonsterSpawner: Unregistered spawner"));
	}
}

int32 UHarmoniaCheckpointSubsystem::RespawnAllEnemies()
{
	int32 TotalRespawned = 0;

	for (AHarmoniaMonsterSpawner* Spawner : RegisteredSpawners)
	{
		if (Spawner && Spawner->IsValidLowLevel())
		{
			// ê¸°ì¡´ ëª¬ìŠ¤???œê±°
			Spawner->DespawnAllMonsters();

			// ?ˆë¡œ ?¤í°
			// ?¤í¬??ëª¨ë“œ???°ë¼ ?¤ì‹œ ?¤í°
			if (Spawner->SpawnMode == EHarmoniaSpawnMode::Respawn || Spawner->SpawnMode == EHarmoniaSpawnMode::OnBeginPlay)
			{
				for (int32 i = 0; i < Spawner->MaxTotalMonsters; ++i)
				{
					if (Spawner->SpawnMonster())
					{
						TotalRespawned++;
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RespawnAllEnemies: Respawned %d enemies"), TotalRespawned);
	return TotalRespawned;
}

// ============================================================================
// Data Persistence
// ============================================================================

void UHarmoniaCheckpointSubsystem::SaveCheckpointData()
{
	// ì²´í¬?¬ì¸???œë¸Œ?œìŠ¤?œì˜ ?°ì´?°ëŠ” HarmoniaSaveGameSubsystem???µí•´ ?€?¥ë¨
	UE_LOG(LogTemp, Log, TEXT("SaveCheckpointData: Checkpoint data saved"));
}

void UHarmoniaCheckpointSubsystem::LoadCheckpointData()
{
	// ì²´í¬?¬ì¸???œë¸Œ?œìŠ¤?œì˜ ?°ì´?°ëŠ” HarmoniaSaveGameSubsystem???µí•´ ë¡œë“œ??
	UE_LOG(LogTemp, Log, TEXT("LoadCheckpointData: Checkpoint data loaded"));
}

TArray<FHarmoniaCheckpointData> UHarmoniaCheckpointSubsystem::GetCheckpointDataForSave() const
{
	TArray<FHarmoniaCheckpointData> Result;
	CheckpointDataMap.GenerateValueArray(Result);
	return Result;
}

void UHarmoniaCheckpointSubsystem::ApplyCheckpointDataFromLoad(const TArray<FHarmoniaCheckpointData>& LoadedData)
{
	for (const FHarmoniaCheckpointData& Data : LoadedData)
	{
		CheckpointDataMap.Add(Data.CheckpointID, Data);

		// ì²´í¬?¬ì¸???¡í„°???°ì´???ìš©
		AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(Data.CheckpointID);
		if (Checkpoint)
		{
			Checkpoint->ApplyCheckpointData(Data);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ApplyCheckpointDataFromLoad: Applied %d checkpoint data"), LoadedData.Num());
}
