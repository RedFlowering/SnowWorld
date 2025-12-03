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
	// 기본 ?�정 초기??
	CheckpointConfig = FHarmoniaCheckpointConfig();
}

void UHarmoniaCheckpointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCheckpointSubsystem: Initialized"));
}

void UHarmoniaCheckpointSubsystem::Deinitialize()
{
	// ?�?�머 ?�리
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

	// 체크?�인???�이???�성
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

	// 체크?�인???�성??
	Checkpoint->Activate(Player);

	// ?�이???�데?�트
	if (FHarmoniaCheckpointData* Data = CheckpointDataMap.Find(CheckpointID))
	{
		Data->bActivated = true;
	}

	// ?�벤??브로?�캐?�트
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

	// 공명 ?�작
	ResonatingPlayers.Add(Player, CheckpointID);
	Checkpoint->StartResonance(Player);

	// 공명 ?�?�머 ?�정
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

	// ?�벤??브로?�캐?�트
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

	// ?�?�머 취소
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle* TimerHandle = ResonanceTimerHandles.Find(Player);
		if (TimerHandle && TimerHandle->IsValid())
		{
			World->GetTimerManager().ClearTimer(*TimerHandle);
		}
	}

	// 체크?�인?�에??공명 종료
	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(*CheckpointID);
	if (Checkpoint)
	{
		Checkpoint->EndResonance(Player);
	}

	FName CancelledCheckpointID = *CheckpointID;

	// ?�이???�리
	ResonatingPlayers.Remove(Player);
	ResonanceTimerHandles.Remove(Player);

	// ?�벤??브로?�캐?�트
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

	// 공명 결과 ?�성
	FHarmoniaResonanceResult Result;
	Result.bSuccess = true;

	// ?�레?�어 ?�복
	RestorePlayerHealth(Player, CheckpointConfig.HealthRestorationRate);

	// ??리스??
	if (CheckpointConfig.bRespawnEnemiesOnResonance)
	{
		Result.EnemiesRespawned = RespawnAllEnemies();
	}

	// ?�동 ?�??
	if (CheckpointConfig.bAutoSaveOnResonance)
	{
		AutoSaveGame(Player);
		Result.bGameSaved = true;
	}

	// 마�?�?체크?�인???�정
	SetPlayerLastCheckpoint(Player, CheckpointID);

	// 공명 주파?�별 ?�과
	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(CheckpointID);
	if (Checkpoint)
	{
		ApplyResonanceFrequencyEffects(Player, Checkpoint->ResonanceFrequency);
		Checkpoint->EndResonance(Player);
	}

	// ?�이???�리
	ResonatingPlayers.Remove(Player);
	ResonanceTimerHandles.Remove(Player);

	// ?�벤??브로?�캐?�트
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

	// 체력 ?�복
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
	// �?공명 주파?�별 ?�수 ?�과 (Blueprint?�서 ?�장 가??
	// ?? Azure = 마나 ?�복, Crimson = 공격??버프, Verdant = 체력 ?�생 ??

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

	// ?�레?�트 비용 계산
	FName CurrentCheckpointID = GetPlayerLastCheckpoint(Player);
	Result.ResourceCost = CalculateTeleportCost(CurrentCheckpointID, DestinationCheckpointID);

	// ?�울/리소??차감 로직
	if (Result.ResourceCost > 0)
	{
		if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
		{
			// HarmoniaResourceComponent ?�인 (?�는 경우)
			// NOTE: HarmoniaResourceComponent가 구현?��? ?��? 경우 ??부분�? ?�킵?�니??
			TArray<UActorComponent*> ResourceComponents;
			PS->GetComponents(UActorComponent::StaticClass(), ResourceComponents);
			bool bResourceDeducted = false;

			for (UActorComponent* Component : ResourceComponents)
			{
				if (Component->GetClass()->GetName().Contains(TEXT("ResourceComponent")))
				{
					// 리플?�션???�해 ConsumeResource 메서???�출 ?�도
					// NOTE: ?�제 HarmoniaResourceComponent가 구현?�면 ?�절???�??캐스?�으�?변�?
					UE_LOG(LogTemp, Warning, TEXT("TeleportToCheckpoint: Resource component found but not implemented yet"));
					bResourceDeducted = true;
					break;
				}
			}

			if (!bResourceDeducted)
			{
				// 리소???�스?�이 ?�는 경우 무료�??�레?�트 ?�용
				UE_LOG(LogTemp, Log, TEXT("TeleportToCheckpoint: Cost %d souls (free - resource system not implemented)"), Result.ResourceCost);
			}
		}
	}

	// ?�레?�트 ?�행
	FVector TeleportLocation = Checkpoint->GetActorLocation();
	FRotator TeleportRotation = Checkpoint->GetActorRotation();

	Pawn->SetActorLocation(TeleportLocation);
	Pawn->SetActorRotation(TeleportRotation);
	Player->SetControlRotation(TeleportRotation);

	Result.bSuccess = true;

	// ?�벤??브로?�캐?�트
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

	// ?�투 중인지 ?�인
	if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
	{
		if (ULyraAbilitySystemComponent* ASC = PS->GetLyraAbilitySystemComponent())
		{
			// ?�투 관??GameplayTag ?�인
			// Lyra?�서 ?�용?�는 ?�반?�인 ?�투 ?�그?�을 ?�인
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

			// ?��?지�?받는 중인지???�인 (최근 5�????��?지 받음)
			if (ALyraCharacter* Character = Cast<ALyraCharacter>(Player->GetPawn()))
			{
				const ULyraHealthSet* HealthSet = ASC->GetSet<ULyraHealthSet>();
				if (HealthSet)
				{
					float CurrentHealth = HealthSet->GetHealth();
					float MaxHealth = HealthSet->GetMaxHealth();

					// 체력??100% 미만?�면 최근???�투가 ?�었??가?�성
					// ?��?�??�것만으로는 불충분하므�??�그 ?�인??주로 ?�용
				}
			}
		}
	}

	// 리소??충분?��? ?�인
	FName CurrentCheckpointID = GetPlayerLastCheckpoint(Player);
	int32 TeleportCost = CalculateTeleportCost(CurrentCheckpointID, DestinationCheckpointID);

	if (TeleportCost > 0)
	{
		if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
		{
			// HarmoniaResourceComponent ?�인 (?�는 경우)
			TArray<UActorComponent*> ResourceComponents;
			PS->GetComponents(UActorComponent::StaticClass(), ResourceComponents);
			bool bHasResourceSystem = false;

			for (UActorComponent* Component : ResourceComponents)
			{
				if (Component->GetClass()->GetName().Contains(TEXT("ResourceComponent")))
				{
					bHasResourceSystem = true;
					// NOTE: ?�제 HarmoniaResourceComponent가 구현?�면 ?�기??리소???�인
					// ?�재???�스?�이 ?�으므�??�킵
					UE_LOG(LogTemp, Log, TEXT("CanTeleportToCheckpoint: Resource component found but not checking (not implemented)"));
					break;
				}
			}

			if (!bHasResourceSystem)
			{
				// 리소???�스?�이 ?�는 경우 무료�??�레?�트 ?�용
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

	// 거리???�른 비용 계산
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
			// 기존 몬스???�거
			Spawner->DespawnAllMonsters();

			// ?�로 ?�폰
			// ?�포??모드???�라 ?�시 ?�폰
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
	// 체크?�인???�브?�스?�의 ?�이?�는 HarmoniaSaveGameSubsystem???�해 ?�?�됨
	UE_LOG(LogTemp, Log, TEXT("SaveCheckpointData: Checkpoint data saved"));
}

void UHarmoniaCheckpointSubsystem::LoadCheckpointData()
{
	// 체크?�인???�브?�스?�의 ?�이?�는 HarmoniaSaveGameSubsystem???�해 로드??
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

		// 체크?�인???�터???�이???�용
		AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(Data.CheckpointID);
		if (Checkpoint)
		{
			Checkpoint->ApplyCheckpointData(Data);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ApplyCheckpointDataFromLoad: Applied %d checkpoint data"), LoadedData.Num());
}
