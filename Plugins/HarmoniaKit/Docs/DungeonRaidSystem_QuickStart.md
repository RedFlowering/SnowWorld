# 던전/레이드 시스템 사용 가이드

## 시스템 구조 이해하기

던전 시스템은 크게 3가지 요소로 구성됩니다:

### 1. 던전 메타데이터 (UDungeonDataAsset)
던전의 정보만 담고 있는 데이터 에셋입니다.
- 던전 이름, 설명
- 입장 조건, 보상
- 난이도별 설정

### 2. 던전 인스턴스 (AHarmoniaDungeonInstance)
**실제 던전 레벨(맵)에 배치되는 액터**입니다.
- 레벨에 배치하여 던전 공간을 관리
- 플레이어 입장/퇴장 처리
- 몬스터 스폰
- 웨이브 관리

### 3. 던전 컴포넌트 (UHarmoniaDungeonComponent)
**플레이어 캐릭터에 붙는 컴포넌트**입니다.
- 플레이어의 던전 진행 상태 추적
- 타이머 관리
- 점수/보상 계산

## 사용 흐름

```
[플레이어] 
   ↓ 입장 요청
[던전 매니저] → 인스턴스 생성 → [던전 레벨 로드]
   ↓                                      ↓
[던전 컴포넌트]                    [던전 인스턴스 Actor]
   ↓ 상태 추적                           ↓ 몬스터 스폰
[UI 업데이트]                         [전투 진행]
```

## 빠른 시작

### 단계 1: 던전 레벨 만들기

1. **새 레벨 생성**
   - Content Browser → 우클릭 → Level → New Level
   - 이름: `Dungeon_Temple01`

2. **던전 인스턴스 액터 배치**
   ```
   레벨 뷰포트에서:
   - Place Actors → All Classes → HarmoniaDungeonInstance
   - 원하는 위치에 배치 (보통 0,0,0)
   ```

3. **던전 인스턴스 설정**
   ```
   Details 패널에서:
   - Entrance Transform: 입구 위치 설정
   - Exit Transform: 출구 위치 설정
   - Spawn Points: 몬스터 스폰 지점 추가
   ```

### 단계 2: 던전 데이터 에셋 만들기

1. **데이터 에셋 생성**
   ```
   Content Browser:
   - 우클릭 → Miscellaneous → Data Asset
   - Class: DungeonDataAsset
   - 이름: DA_Temple01
   ```

2. **데이터 에셋 설정**
   ```cpp
   Dungeon ID: Temple01
   Dungeon Name: Ancient Temple
   Dungeon Type: Instance
   
   Requirements:
     - Min Level: 10
     - Min Party Size: 3
     - Max Party Size: 5
   
   Rewards (Normal):
     - Experience: 1000
     - Gold: 500
   
   Dungeon Level: Dungeon_Temple01
   ```

3. **던전 매니저에 등록**
   ```cpp
   // GameMode BeginPlay에서
   UHarmoniaDungeonManager* Manager = GetGameInstance()->GetSubsystem<UHarmoniaDungeonManager>();
   Manager->RegisterDungeon(DA_Temple01);
   ```

### 단계 3: 플레이어 캐릭터에 컴포넌트 추가

1. **컴포넌트 추가**
   ```
   플레이어 Blueprint:
   - Components 패널 → Add Component
   - Search: HarmoniaDungeonComponent
   ```

2. **입장 로직 구현**
   ```cpp
   // C++ 예제
   void AMyCharacter::EnterDungeon(FName DungeonID)
   {
       UHarmoniaDungeonManager* Manager = GetGameInstance()->GetSubsystem<UHarmoniaDungeonManager>();
       UDungeonDataAsset* DungeonData = Manager->GetDungeonData(DungeonID);
       
       if (DungeonComponent->CanEnterDungeon(DungeonData))
       {
           // 던전 레벨로 이동
           UGameplayStatics::OpenLevel(this, DungeonData->DungeonLevel.GetAssetName());
       }
   }
   ```

### 단계 4: 던전 입장 처리

**던전 레벨의 Level Blueprint에서:**

```
Event BeginPlay
   ↓
Get All Actors of Class (HarmoniaDungeonInstance)
   ↓
Get Player Controller
   ↓
Call: OnPlayerEnter (던전 인스턴스, 플레이어)
   ↓
Call: StartDungeon (던전 인스턴스)
```

## WorldGenerator와 통합

### 무한 던전 (프로시저럴 생성)

무한 던전은 WorldGenerator를 사용하여 자동으로 레이아웃을 생성합니다.

```cpp
// C++ 예제
void GenerateInfiniteDungeonFloor(int32 FloorNumber)
{
    // 1. 던전 생성기 생성
    UHarmoniaDungeonGenerator* Generator = NewObject<UHarmoniaDungeonGenerator>();
    
    // 2. 층 정보 가져오기
    FInfiniteDungeonFloor FloorInfo;
    FloorInfo.FloorNumber = FloorNumber;
    FloorInfo.MonsterLevelMultiplier = 1.0f + (FloorNumber - 1) * 0.1f;
    FloorInfo.bHasBoss = (FloorNumber % 10 == 0);
    
    // 3. 레이아웃 생성
    FDungeonLayout Layout = Generator->GenerateInfiniteDungeonFloor(FloorNumber, FloorInfo);
    
    // 4. 월드에 스폰
    AHarmoniaDungeonInstance* Instance = Generator->SpawnDungeonFromLayout(
        this, Layout, InfiniteDungeonData
    );
    
    // 5. 던전 시작
    Instance->StartDungeon();
}
```

### POI 던전 (WorldGenerator POI 시스템)

WorldGenerator의 POI 시스템과 통합하여 오픈 월드에 던전 입구를 배치할 수 있습니다.

```cpp
// WorldGenerator POI 설정에서
POI Settings:
  - POI Type: Dungeon
  - Actor Class: BP_DungeonEntrance
  - Difficulty: by Distance
  
// BP_DungeonEntrance Actor
class ABP_DungeonEntrance : public AActor
{
    UPROPERTY(EditAnywhere)
    FName DungeonID;
    
    void OnPlayerInteract()
    {
        // 던전 입장 로직
        EnterDungeon(DungeonID);
    }
};
```

## Blueprint 예제

### 예제 1: 던전 입장 UI

```
┌─────────────────────────────────┐
│  Widget: Dungeon Entrance UI    │
└─────────────────────────────────┘

On Construct:
   ↓
Get Dungeon Data (from DungeonEntrance Actor)
   ↓
Display Dungeon Info
   ├─ Dungeon Name
   ├─ Recommended Level
   ├─ Required Party Size
   └─ Rewards

On "Enter" Button Clicked:
   ↓
Get Dungeon Component (from Player)
   ↓
Can Enter Dungeon?
   ├─ Yes → Open Dungeon Level
   └─ No → Show Error Message
```

### 예제 2: 던전 진행 UI

```
┌─────────────────────────────────┐
│  Widget: Dungeon Progress HUD   │
└─────────────────────────────────┘

Event Tick:
   ↓
Get Dungeon Component
   ↓
Update UI
   ├─ Remaining Time
   ├─ Current Wave
   ├─ Monster Count
   └─ Current Score

Bind Event: OnDungeonTimeUpdate
   ↓
Update Timer Display

Bind Event: OnWaveCompleted
   ↓
Show "Wave Completed!" Message
```

### 예제 3: 무한 던전 층 진행

```
Bind Event: OnInfiniteDungeonFloorChanged
   ↓
Play Floor Clear Animation
   ↓
Wait 3 seconds
   ↓
Call: GenerateNextFloor (Dungeon Instance)
   ↓
Teleport Player to Start Position
```

## 레이드 예제

### 레이드 준비 화면

```cpp
// C++ 예제 - 레이드 준비
void SetupRaid()
{
    UHarmoniaRaidComponent* RaidComp = GetRaidComponent();
    
    // 멤버 추가
    for (APlayerController* PC : AllPlayers)
    {
        FRaidMemberInfo Member;
        Member.PlayerID = PC->GetUniqueID();
        Member.PlayerName = PC->GetPlayerState()->GetPlayerName();
        Member.Role = DetermineRole(PC); // 탱커, 힐러, DPS 등
        Member.bIsReady = false;
        
        RaidComp->AddMember(Member);
    }
    
    // 역할 구성 검증
    if (RaidComp->ValidateRoleComposition(RaidData))
    {
        UE_LOG(LogTemp, Log, TEXT("Raid composition is valid!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Raid composition needs adjustment"));
        ShowRoleWarning();
    }
}
```

### 레이드 페이즈 전환

```cpp
// Raid Boss Blueprint
On Boss Health Changed:
   ↓
Get Current Health Percentage
   ↓
If Health <= 75% AND Current Phase == 1:
   ↓
   Call: Advance To Next Phase (Raid Component)
   ↓
   Play Phase Transition Cutscene
   ↓
   Change Boss Attack Pattern
```

## 실전 예제: 완전한 던전 시스템

### 1. 던전 입구 Blueprint (BP_DungeonEntrance)

```cpp
class ABP_DungeonEntrance : public AActor
{
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DungeonID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UWidgetComponent* InfoWidget;
    
    UFUNCTION(BlueprintCallable)
    void OnPlayerEnter()
    {
        // UI 표시
        ShowDungeonInfo();
    }
    
    UFUNCTION(BlueprintCallable)
    void EnterDungeon(APlayerController* Player)
    {
        UHarmoniaDungeonManager* Manager = GetGameInstance()->GetSubsystem<UHarmoniaDungeonManager>();
        
        // 인스턴스 생성 또는 기존 인스턴스에 참여
        FGuid InstanceID = Manager->CreateDungeonInstance(DungeonData, Difficulty);
        Manager->AddPlayerToInstance(InstanceID, Player->GetUniqueID());
        
        // 던전 레벨로 이동
        UGameplayStatics->OpenLevel(Player, DungeonLevelName);
    }
};
```

### 2. 던전 완료 처리

```cpp
// Dungeon Instance
void OnAllMonstersDefeated()
{
    // 던전 완료
    EndDungeon(true);
    
    // 모든 플레이어에게 보상 지급
    for (APlayerController* PC : ActivePlayers)
    {
        if (AActor* PlayerPawn = PC->GetPawn())
        {
            if (UHarmoniaDungeonComponent* DungeonComp = PlayerPawn->FindComponentByClass<UHarmoniaDungeonComponent>())
            {
                // 보상 계산 및 지급
                DungeonComp->GrantDungeonReward();
                
                // 랭킹 등록
                FString PlayerName = PC->GetPlayerState()->GetPlayerName();
                float ClearTime = DungeonComp->GetElapsedTime();
                int32 Score = CalculateScore();
                
                DungeonComp->SubmitRanking(PlayerName, ClearTime, Score);
            }
        }
    }
    
    // 3초 후 플레이어들을 원래 위치로
    FTimerHandle TeleportTimer;
    GetWorld()->GetTimerManager().SetTimer(TeleportTimer, [this]()
    {
        TeleportPlayersToExit();
    }, 3.0f, false);
}
```

### 3. 던전 UI 시스템

```cpp
// Dungeon HUD Widget
class UDungeonHUDWidget : public UUserWidget
{
public:
    UFUNCTION(BlueprintImplementableEvent)
    void UpdateTimer(float RemainingTime, float TotalTime);
    
    UFUNCTION(BlueprintImplementableEvent)
    void UpdateWave(int32 WaveNumber);
    
    UFUNCTION(BlueprintImplementableEvent)
    void UpdateMonsterCount(int32 RemainingMonsters);
    
    UFUNCTION(BlueprintImplementableEvent)
    void ShowCompletionScreen(bool bSuccess, const FDungeonReward& Reward);
    
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        
        // 플레이어의 던전 컴포넌트 찾기
        APlayerController* PC = GetOwningPlayer();
        if (AActor* Pawn = PC->GetPawn())
        {
            UHarmoniaDungeonComponent* DungeonComp = Pawn->FindComponentByClass<UHarmoniaDungeonComponent>();
            
            // 이벤트 바인딩
            DungeonComp->OnDungeonTimeUpdate.AddDynamic(this, &UDungeonHUDWidget::UpdateTimer);
            DungeonComp->OnDungeonCompleted.AddDynamic(this, &UDungeonHUDWidget::OnDungeonComplete);
        }
    }
};
```

## 최적화 팁

### 1. 던전 인스턴스 관리
```cpp
// 매니저에서 만료된 인스턴스 자동 정리
void UHarmoniaDungeonManager::Tick(float DeltaTime)
{
    // 24시간 이상 된 인스턴스 제거
    CleanupExpiredInstances();
}
```

### 2. 몬스터 스폰 최적화
```cpp
// 플레이어 거리 기반 스폰
void SpawnMonstersNearPlayers()
{
    for (const FDungeonSpawnPoint& SpawnPoint : SpawnPoints)
    {
        bool bNearPlayer = false;
        for (APlayerController* PC : ActivePlayers)
        {
            float Distance = FVector::Dist(PC->GetPawn()->GetActorLocation(), SpawnPoint.SpawnTransform.GetLocation());
            if (Distance < 5000.0f) // 5000 유닛 이내
            {
                bNearPlayer = true;
                break;
            }
        }
        
        if (bNearPlayer)
        {
            SpawnMonster(SpawnPoint);
        }
    }
}
```

## 문제 해결

### Q: 던전에 입장했는데 시작이 안됩니다
A: Level Blueprint에서 `StartDungeon()` 호출했는지 확인하세요.

### Q: 몬스터가 스폰되지 않습니다
A: `SpawnPoints` 배열이 비어있는지, `MonsterClass`가 설정되어 있는지 확인하세요.

### Q: 무한 던전 층 이동이 안됩니다
A: `DungeonType`이 `Infinite`로 설정되어 있는지 확인하세요.

### Q: 랭킹이 저장되지 않습니다
A: `SubmitGlobalRanking()`을 서버에서 호출해야 합니다 (네트워크 게임의 경우).

## 다음 단계

- [전투 시스템 통합](CombatSystem_README.md)
- [퀘스트 시스템 통합](QuestSystem_README.md)
- [파티 시스템 구현](PartySystem.md)
- [업적 시스템 연동](AchievementSystem.md)
