# Dungeon & Raid System

HarmoniaKit의 던전/레이드 시스템은 다양한 PvE 콘텐츠를 지원하는 종합적인 시스템입니다.

## 목차

1. [개요](#개요)
2. [던전 타입](#던전-타입)
3. [시스템 구조](#시스템-구조)
4. [주요 기능](#주요-기능)
5. [사용 예제](#사용-예제)

## 개요

던전/레이드 시스템은 다음과 같은 콘텐츠를 제공합니다:

- **인스턴스 던전**: 소규모 파티용 던전 (1-5인)
- **레이드**: 대규모 공격대 콘텐츠 (10-40인)
- **무한 던전**: 로그라이크 무한 층 던전
- **챌린지 모드**: 고난이도 변형 던전
- **던전 랭킹**: 클리어 타임 경쟁 시스템

## 던전 타입

### 1. Instance Dungeon (인스턴스 던전)
파티 단위로 진행하는 일반적인 던전입니다.

**특징:**
- 1-5인 파티 구성
- 고정된 던전 구조
- 난이도별 보상
- 일일/주간 입장 제한 가능

### 2. Raid (레이드)
대규모 공격대가 도전하는 엔드게임 콘텐츠입니다.

**특징:**
- 10-40인 공격대 구성
- 역할별 인원 구성 (탱커, 힐러, 딜러, 서포터)
- 다단계 보스 페이즈
- 부활 제한
- 고급 메카닉스

### 3. Infinite Dungeon (무한 던전)
로그라이크 방식의 무한히 이어지는 던전입니다.

**특징:**
- 층수가 올라갈수록 난이도 증가
- 랜덤 생성 레이아웃
- 층별 보상 증가
- 최고 도달 층 기록
- 진행 상황 저장 가능

### 4. Challenge Mode (챌린지 모드)
기존 던전에 특수 규칙을 적용한 고난이도 모드입니다.

**특징:**
- 특수 모디파이어 적용
- 점수 배율 증가
- 특수 보상
- 시즌별 운영 가능

## 시스템 구조

### 핵심 클래스

#### 1. UHarmoniaDungeonComponent
플레이어의 던전 진행을 관리하는 컴포넌트입니다.

**주요 기능:**
- 던전 입장/퇴장
- 상태 관리 (대기, 진행 중, 완료, 실패)
- 타이머 관리
- 점수 계산
- 보상 지급

#### 2. UHarmoniaRaidComponent
레이드 전투를 관리하는 컴포넌트입니다.

**주요 기능:**
- 공격대 멤버 관리
- 역할 구성 검증
- 페이즈 관리
- 부활 시스템
- 전멸 처리

#### 3. UHarmoniaDungeonManager
게임 전체의 던전 시스템을 관리하는 서브시스템입니다.

**주요 기능:**
- 던전 데이터 등록/조회
- 인스턴스 생성/관리
- 매치메이킹
- 글로벌 랭킹
- 이벤트 던전
- 통계 관리

### 데이터 구조

#### UDungeonDataAsset
던전의 기본 정보를 담는 데이터 에셋입니다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
FName DungeonID;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
FText DungeonName;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
EDungeonType DungeonType;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
FDungeonRequirement Requirements;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
TMap<EDungeonDifficulty, FDungeonReward> Rewards;
```

#### URaidDataAsset
레이드의 추가 정보를 담는 데이터 에셋입니다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
int32 MinRaidSize = 10;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
int32 MaxRaidSize = 40;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
TMap<ERaidRole, int32> RecommendedRoleComposition;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
int32 ReviveLimit = 3;
```

## 주요 기능

### 1. 던전 입장 시스템

#### 입장 조건 확인
```cpp
// C++
UHarmoniaDungeonComponent* DungeonComp = GetDungeonComponent();
UDungeonDataAsset* DungeonData = GetDungeonData();

if (DungeonComp->CanEnterDungeon(DungeonData))
{
    DungeonComp->EnterDungeon(DungeonData, EDungeonDifficulty::Normal);
}
```

#### 입장 조건 설정
- 최소/권장 레벨
- 파티 인원 제한
- 필요 아이템 (입장권 등)
- 선행 퀘스트

### 2. 난이도 시스템

5단계 난이도를 지원합니다:
- **Normal**: 일반 난이도
- **Hard**: 어려움
- **Expert**: 전문가
- **Master**: 마스터
- **Nightmare**: 악몽

난이도별로 다른 보상을 설정할 수 있습니다.

### 3. 무한 던전 시스템

#### 층 이동
```cpp
// C++
void OnFloorCleared()
{
    DungeonComponent->AdvanceToNextFloor();
}

int32 CurrentFloor = DungeonComponent->GetCurrentFloor();
int32 HighestFloor = DungeonComponent->GetHighestFloor();
```

#### 층별 난이도 조정
```cpp
// FInfiniteDungeonFloor 구조체
UPROPERTY(EditAnywhere)
float MonsterLevelMultiplier = 1.0f;

UPROPERTY(EditAnywhere)
float MonsterCountMultiplier = 1.0f;

UPROPERTY(EditAnywhere)
bool bHasBoss = false;
```

### 4. 챌린지 모드

#### 모디파이어 적용
```cpp
// C++
FDungeonChallengeModifier Modifier;
Modifier.ModifierName = FText::FromString("Hard Mode");
Modifier.MonsterHealthMultiplier = 2.0f;
Modifier.MonsterDamageMultiplier = 1.5f;
Modifier.ScoreMultiplier = 2.0f;

DungeonComponent->ApplyChallengeModifier(Modifier);
```

### 5. 랭킹 시스템

#### 기록 등록
```cpp
// C++
void OnDungeonComplete()
{
    float ClearTime = DungeonComponent->GetElapsedTime();
    int32 Score = CalculateFinalScore();
    FString PlayerName = GetPlayerName();
    
    DungeonComponent->SubmitRanking(PlayerName, ClearTime, Score);
}
```

#### 랭킹 조회
```cpp
// C++
// 로컬 랭킹
TArray<FDungeonRankingEntry> TopRankings = DungeonComponent->GetDungeonRankings(10);
FDungeonRankingEntry MyBest = DungeonComponent->GetPersonalBest();

// 글로벌 랭킹
UHarmoniaDungeonManager* DungeonManager = GetGameInstance()->GetSubsystem<UHarmoniaDungeonManager>();
TArray<FDungeonRankingEntry> GlobalRankings = DungeonManager->GetGlobalRankings(DungeonID, 100);
```

### 6. 레이드 멤버 관리

#### 멤버 추가/제거
```cpp
// C++
UHarmoniaRaidComponent* RaidComp = GetRaidComponent();

FRaidMemberInfo NewMember;
NewMember.PlayerID = "Player123";
NewMember.PlayerName = "John";
NewMember.Role = ERaidRole::Tank;

RaidComp->AddMember(NewMember);
```

#### 역할 구성 검증
```cpp
// C++
URaidDataAsset* RaidData = GetRaidData();
if (RaidComp->ValidateRoleComposition(RaidData))
{
    StartRaid();
}

TMap<ERaidRole, int32> Distribution = RaidComp->GetRoleDistribution();
```

### 7. 페이즈 시스템

#### 페이즈 전환
```cpp
// C++
void OnBossHealthThreshold(float HealthPercent)
{
    if (HealthPercent <= 75.0f && CurrentPhase == 1)
    {
        RaidComponent->AdvanceToNextPhase();
    }
}
```

### 8. 부활 시스템

#### 멤버 부활
```cpp
// C++
void OnPlayerDeath(const FString& PlayerID)
{
    RaidComponent->OnMemberDeath(PlayerID);
    
    // 전멸 체크
    if (RaidComponent->IsWiped())
    {
        HandleRaidWipe();
    }
}

bool TryRevive(const FString& PlayerID)
{
    return RaidComponent->ReviveMember(PlayerID);
}
```

## 사용 예제

### 예제 1: 기본 던전 생성

```cpp
// C++ - DungeonDataAsset 생성
UDungeonDataAsset* NewDungeon = NewObject<UDungeonDataAsset>();

NewDungeon->DungeonID = FName("Temple01");
NewDungeon->DungeonName = FText::FromString("Ancient Temple");
NewDungeon->DungeonType = EDungeonType::Instance;

// 입장 조건 설정
NewDungeon->Requirements.MinLevel = 10;
NewDungeon->Requirements.RecommendedLevel = 15;
NewDungeon->Requirements.MinPartySize = 3;
NewDungeon->Requirements.MaxPartySize = 5;

// 보상 설정
FDungeonReward NormalReward;
NormalReward.ExperienceReward = 1000;
NormalReward.GoldReward = 500;
NewDungeon->Rewards.Add(EDungeonDifficulty::Normal, NormalReward);

// 던전 매니저에 등록
UHarmoniaDungeonManager* Manager = GetGameInstance()->GetSubsystem<UHarmoniaDungeonManager>();
Manager->RegisterDungeon(NewDungeon);
```

### 예제 2: 무한 던전 설정

```cpp
// C++ - 무한 던전 층 설정
UDungeonDataAsset* InfiniteDungeon = NewObject<UDungeonDataAsset>();
InfiniteDungeon->DungeonType = EDungeonType::Infinite;

for (int32 i = 1; i <= 100; i++)
{
    FInfiniteDungeonFloor Floor;
    Floor.FloorNumber = i;
    Floor.MonsterLevelMultiplier = 1.0f + (i - 1) * 0.1f;
    Floor.MonsterCountMultiplier = 1.0f + (i - 1) * 0.05f;
    Floor.bHasBoss = (i % 10 == 0); // 매 10층마다 보스
    
    InfiniteDungeon->InfiniteFloors.Add(Floor);
}
```

### 예제 3: 레이드 생성 및 시작

```cpp
// C++ - 레이드 생성
URaidDataAsset* NewRaid = NewObject<URaidDataAsset>();

NewRaid->DungeonID = FName("DragonRaid");
NewRaid->DungeonName = FText::FromString("Dragon's Lair Raid");
NewRaid->MinRaidSize = 10;
NewRaid->MaxRaidSize = 25;
NewRaid->ReviveLimit = 5;
NewRaid->BossPhaseCount = 3;

// 권장 역할 구성
NewRaid->RecommendedRoleComposition.Add(ERaidRole::Tank, 2);
NewRaid->RecommendedRoleComposition.Add(ERaidRole::Healer, 3);
NewRaid->RecommendedRoleComposition.Add(ERaidRole::DPS, 15);
NewRaid->RecommendedRoleComposition.Add(ERaidRole::Support, 5);

// 레이드 시작
UHarmoniaRaidComponent* RaidComp = GetRaidComponent();
if (RaidComp->AreAllMembersReady() && RaidComp->ValidateRoleComposition(NewRaid))
{
    StartRaid();
}
```

### 예제 4: 챌린지 모드 적용

```cpp
// C++ - 챌린지 모디파이어 생성
FDungeonChallengeModifier HardcoreMode;
HardcoreMode.ModifierName = FText::FromString("Hardcore");
HardcoreMode.ModifierDescription = FText::FromString("Monster damage increased by 100%");
HardcoreMode.MonsterDamageMultiplier = 2.0f;
HardcoreMode.PlayerHealthMultiplier = 0.5f;
HardcoreMode.ScoreMultiplier = 3.0f;

DungeonComponent->ApplyChallengeModifier(HardcoreMode);
```

### 예제 5: 이벤트 던전

```cpp
// C++ - 이벤트 던전 활성화
UHarmoniaDungeonManager* Manager = GetGameInstance()->GetSubsystem<UHarmoniaDungeonManager>();

FDateTime StartTime = FDateTime::Now();
FDateTime EndTime = StartTime + FTimespan::FromDays(7);

Manager->ActivateEventDungeon(FName("SummerEvent"), StartTime, EndTime);

// 활성 이벤트 던전 조회
TArray<FName> ActiveEvents = Manager->GetActiveEventDungeons();
```

## Blueprint 사용

### 던전 입장
```
Event Graph:
┌─────────────────────────────────────┐
│ OnEnterDungeonButton Clicked        │
└───────────────┬─────────────────────┘
                │
                ▼
┌─────────────────────────────────────┐
│ Get Dungeon Component               │
└───────────────┬─────────────────────┘
                │
                ▼
┌─────────────────────────────────────┐
│ Can Enter Dungeon?                  │
│   └─ Dungeon Data: Temple01        │
│   └─ Difficulty: Normal             │
└───────┬─────────────┬───────────────┘
        │ True        │ False
        ▼             ▼
┌──────────────┐  ┌──────────────────┐
│Enter Dungeon │  │Show Error Message│
└──────────────┘  └──────────────────┘
```

### 랭킹 조회
```
Event Graph:
┌─────────────────────────────────────┐
│ On Show Rankings Button Clicked     │
└───────────────┬─────────────────────┘
                │
                ▼
┌─────────────────────────────────────┐
│ Get Dungeon Component               │
└───────────────┬─────────────────────┘
                │
                ▼
┌─────────────────────────────────────┐
│ Get Dungeon Rankings                │
│   └─ Top Count: 10                  │
└───────────────┬─────────────────────┘
                │
                ▼
┌─────────────────────────────────────┐
│ For Each Loop (Rankings)            │
│   └─ Create Ranking Widget          │
│   └─ Add to Ranking List            │
└─────────────────────────────────────┘
```

## 네트워크 고려사항

### 서버 권한
던전 시스템은 서버에서 관리되어야 합니다:
- 던전 상태 관리
- 보상 지급
- 랭킹 등록
- 인스턴스 생성/삭제

### 복제 설정
다음 속성들은 복제되어야 합니다:
- 현재 던전 상태
- 남은 시간
- 현재 페이즈
- 레이드 멤버 목록

## 성능 최적화

### 인스턴스 관리
- 만료된 인스턴스 자동 정리
- 인스턴스 풀링
- 동적 로딩/언로딩

### 랭킹 시스템
- 상위 N개만 메모리 유지
- 데이터베이스 연동
- 캐싱 시스템

## 확장성

### 커스텀 던전 타입
새로운 던전 타입을 추가할 수 있습니다:
```cpp
enum class EDungeonType : uint8
{
    // ... 기존 타입들
    Survival UMETA(DisplayName = "Survival"),
    TimeAttack UMETA(DisplayName = "Time Attack"),
    Custom UMETA(DisplayName = "Custom")
};
```

### 커스텀 보상
보상 시스템을 확장할 수 있습니다:
```cpp
USTRUCT(BlueprintType)
struct FCustomDungeonReward : public FDungeonReward
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> UniqueItemIDs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AchievementPoints;
};
```

## 관련 시스템

- **Quest System**: 던전 클리어를 퀘스트 목표로 사용
- **Party System**: 파티 구성 및 관리
- **Inventory System**: 보상 아이템 지급
- **Achievement System**: 던전 업적
- **World Generator**: 무한 던전 레이아웃 생성

## 참고 파일

### 헤더 파일
- `HarmoniaDungeonSystemDefinitions.h`: 던전 시스템 정의
- `HarmoniaDungeonComponent.h`: 던전 컴포넌트
- `HarmoniaRaidComponent.h`: 레이드 컴포넌트
- `HarmoniaDungeonManager.h`: 던전 매니저

### 구현 파일
- `HarmoniaDungeonComponent.cpp`
- `HarmoniaRaidComponent.cpp`
- `HarmoniaDungeonManager.cpp`
