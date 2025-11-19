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

UHarmoniaCheckpointSubsystem::UHarmoniaCheckpointSubsystem()
{
	// 기본 설정 초기화
	CheckpointConfig = FHarmoniaCheckpointConfig();
}

void UHarmoniaCheckpointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCheckpointSubsystem: Initialized"));
}

void UHarmoniaCheckpointSubsystem::Deinitialize()
{
	// 타이머 정리
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

	// 체크포인트 데이터 생성
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

	// 체크포인트 활성화
	Checkpoint->Activate(Player);

	// 데이터 업데이트
	if (FHarmoniaCheckpointData* Data = CheckpointDataMap.Find(CheckpointID))
	{
		Data->bActivated = true;
	}

	// 이벤트 브로드캐스트
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

	// 공명 시작
	ResonatingPlayers.Add(Player, CheckpointID);
	Checkpoint->StartResonance(Player);

	// 공명 타이머 설정
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

	// 이벤트 브로드캐스트
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

	// 타이머 취소
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle* TimerHandle = ResonanceTimerHandles.Find(Player);
		if (TimerHandle && TimerHandle->IsValid())
		{
			World->GetTimerManager().ClearTimer(*TimerHandle);
		}
	}

	// 체크포인트에서 공명 종료
	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(*CheckpointID);
	if (Checkpoint)
	{
		Checkpoint->EndResonance(Player);
	}

	FName CancelledCheckpointID = *CheckpointID;

	// 데이터 정리
	ResonatingPlayers.Remove(Player);
	ResonanceTimerHandles.Remove(Player);

	// 이벤트 브로드캐스트
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

	// 공명 결과 생성
	FHarmoniaResonanceResult Result;
	Result.bSuccess = true;

	// 플레이어 회복
	RestorePlayerHealth(Player, CheckpointConfig.HealthRestorationRate);

	// 적 리스폰
	if (CheckpointConfig.bRespawnEnemiesOnResonance)
	{
		Result.EnemiesRespawned = RespawnAllEnemies();
	}

	// 자동 저장
	if (CheckpointConfig.bAutoSaveOnResonance)
	{
		AutoSaveGame(Player);
		Result.bGameSaved = true;
	}

	// 마지막 체크포인트 설정
	SetPlayerLastCheckpoint(Player, CheckpointID);

	// 공명 주파수별 효과
	AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(CheckpointID);
	if (Checkpoint)
	{
		ApplyResonanceFrequencyEffects(Player, Checkpoint->ResonanceFrequency);
		Checkpoint->EndResonance(Player);
	}

	// 데이터 정리
	ResonatingPlayers.Remove(Player);
	ResonanceTimerHandles.Remove(Player);

	// 이벤트 브로드캐스트
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

	// 체력 회복
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
	// 각 공명 주파수별 특수 효과 (Blueprint에서 확장 가능)
	// 예: Azure = 마나 회복, Crimson = 공격력 버프, Verdant = 체력 재생 등

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

	// 텔레포트 비용 계산
	FName CurrentCheckpointID = GetPlayerLastCheckpoint(Player);
	Result.ResourceCost = CalculateTeleportCost(CurrentCheckpointID, DestinationCheckpointID);

	// 소울/리소스 차감 로직
	// NOTE: 리소스 시스템이 구현되면 여기서 실제 차감을 수행합니다.
	// TODO: HarmoniaResourceComponent 또는 HarmoniaInventoryComponent에서 리소스 차감
	// Example:
	// if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
	// {
	//     if (UHarmoniaResourceComponent* ResourceComp = PS->FindComponentByClass<UHarmoniaResourceComponent>())
	//     {
	//         if (!ResourceComp->ConsumeResource(EResourceType::Soul, Result.ResourceCost))
	//         {
	//             Result.bSuccess = false;
	//             Result.FailureReason = FText::FromString(TEXT("Not enough souls"));
	//             return Result;
	//         }
	//     }
	// }
	UE_LOG(LogTemp, Log, TEXT("TeleportToCheckpoint: Cost %d souls (not yet deducted - resource system not implemented)"), Result.ResourceCost);

	// 텔레포트 실행
	FVector TeleportLocation = Checkpoint->GetActorLocation();
	FRotator TeleportRotation = Checkpoint->GetActorRotation();

	Pawn->SetActorLocation(TeleportLocation);
	Pawn->SetActorRotation(TeleportRotation);
	Player->SetControlRotation(TeleportRotation);

	Result.bSuccess = true;

	// 이벤트 브로드캐스트
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

	// 전투 중인지 확인
	// NOTE: 전투 상태 시스템이 구현되면 여기서 확인합니다.
	// TODO: HarmoniaCombatComponent 또는 AbilitySystemComponent에서 전투 상태 확인
	// Example:
	// if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
	// {
	//     if (ULyraAbilitySystemComponent* ASC = PS->GetLyraAbilitySystemComponent())
	//     {
	//         if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Combat"))))
	//         {
	//             OutReason = FText::FromString(TEXT("Cannot teleport while in combat"));
	//             return false;
	//         }
	//     }
	// }

	// 리소스 충분한지 확인
	FName CurrentCheckpointID = GetPlayerLastCheckpoint(Player);
	int32 TeleportCost = CalculateTeleportCost(CurrentCheckpointID, DestinationCheckpointID);

	// NOTE: 리소스 시스템이 구현되면 여기서 확인합니다.
	// TODO: HarmoniaResourceComponent 또는 HarmoniaInventoryComponent에서 리소스 확인
	// Example:
	// if (ALyraPlayerState* PS = Player->GetPlayerState<ALyraPlayerState>())
	// {
	//     if (UHarmoniaResourceComponent* ResourceComp = PS->FindComponentByClass<UHarmoniaResourceComponent>())
	//     {
	//         if (!ResourceComp->HasEnoughResource(EResourceType::Soul, TeleportCost))
	//         {
	//             OutReason = FText::Format(
	//                 FText::FromString(TEXT("Not enough souls (need {0})")),
	//                 FText::AsNumber(TeleportCost)
	//             );
	//             return false;
	//         }
	//     }
	// }

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

	// 거리에 따른 비용 계산
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
			// 기존 몬스터 제거
			Spawner->DespawnAllMonsters();

			// 새로 스폰
			// 스포너 모드에 따라 다시 스폰
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
	// 체크포인트 서브시스템의 데이터는 HarmoniaSaveGameSubsystem을 통해 저장됨
	UE_LOG(LogTemp, Log, TEXT("SaveCheckpointData: Checkpoint data saved"));
}

void UHarmoniaCheckpointSubsystem::LoadCheckpointData()
{
	// 체크포인트 서브시스템의 데이터는 HarmoniaSaveGameSubsystem을 통해 로드됨
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

		// 체크포인트 액터에 데이터 적용
		AHarmoniaCrystalResonator* Checkpoint = FindCheckpoint(Data.CheckpointID);
		if (Checkpoint)
		{
			Checkpoint->ApplyCheckpointData(Data);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ApplyCheckpointDataFromLoad: Applied %d checkpoint data"), LoadedData.Num());
}
