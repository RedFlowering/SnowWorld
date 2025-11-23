# 하모니아 팀 시스템 사용 가이드

## 개요

하모니아 키트의 팀 시스템은 **유동적이고 확장 가능한** 피아식별(Friend-or-Foe) 시스템을 제공합니다. 언리얼 엔진의 표준 `IGenericTeamAgentInterface`와 통합되어 AI 시스템과 완벽하게 연동됩니다.

## 주요 특징

✅ **무제한 팀 생성**: 고정된 Enum 없이 런타임에 팀을 무제한 추가
✅ **유동적 관계 관리**: 게임 중 팀 간 관계를 동적으로 변경 가능
✅ **데이터 드리븐**: 데이터 애셋을 통한 디자이너 친화적 설정
✅ **언리얼 표준 통합**: `IGenericTeamAgentInterface` 완벽 지원
✅ **네트워크 최적화**: 효율적인 복제 및 대역폭 절약

---

## 1. 기본 사용법

### 1.1 팀 생성 (C++)

```cpp
// 월드 서브시스템 가져오기
UHarmoniaTeamManagementSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UHarmoniaTeamManagementSubsystem>();

// 새로운 팀 생성
FHarmoniaTeamIdentification GoblinTeam = TeamSubsystem->CreateFaction(
    FText::FromString("Goblin Tribe"),
    EHarmoniaTeamAttitude::Hostile,  // 기본 태도: 적대적
    FLinearColor::Red                // UI 색상: 빨간색
);

FHarmoniaTeamIdentification OrcTeam = TeamSubsystem->CreateFaction(
    FText::FromString("Orc Clan"),
    EHarmoniaTeamAttitude::Hostile,
    FLinearColor(1.0f, 0.5f, 0.0f)   // 주황색
);

FHarmoniaTeamIdentification PlayerTeam = TeamSubsystem->CreateFaction(
    FText::FromString("Heroes"),
    EHarmoniaTeamAttitude::Defensive,
    FLinearColor::Blue
);
```

### 1.2 팀 관계 설정

```cpp
// 고블린과 오크를 동맹으로 설정
TeamSubsystem->MakeTeamsAllies(GoblinTeam, OrcTeam);

// 고블린과 플레이어를 적으로 설정
TeamSubsystem->MakeTeamsEnemies(GoblinTeam, PlayerTeam);

// 오크와 플레이어도 적으로 설정
TeamSubsystem->MakeTeamsEnemies(OrcTeam, PlayerTeam);
```

### 1.3 몬스터에 팀 할당

```cpp
// 몬스터 스폰 시
AHarmoniaMonsterBase* GoblinMonster = GetWorld()->SpawnActor<AHarmoniaMonsterBase>(...);
GoblinMonster->SetTeamID(GoblinTeam);

AHarmoniaMonsterBase* OrcMonster = GetWorld()->SpawnActor<AHarmoniaMonsterBase>(...);
OrcMonster->SetTeamID(OrcTeam);
```

---

## 2. 고급 사용법

### 2.1 다중 팀 동맹 생성

```cpp
// 여러 팀을 한 번에 동맹으로 설정
TArray<FHarmoniaTeamIdentification> Alliance;
Alliance.Add(GoblinTeam);
Alliance.Add(OrcTeam);
Alliance.Add(TrollTeam);

TeamSubsystem->CreateAlliance(Alliance);
// 이제 고블린, 오크, 트롤은 모두 서로 동맹 관계
```

### 2.2 한 팀을 여러 팀의 적으로 설정

```cpp
TArray<FHarmoniaTeamIdentification> Enemies;
Enemies.Add(GoblinTeam);
Enemies.Add(OrcTeam);
Enemies.Add(UndeadTeam);

TeamSubsystem->SetFactionHostileToMany(PlayerTeam, Enemies);
// 플레이어는 이제 고블린, 오크, 언데드 모두와 적대 관계
```

### 2.3 런타임 관계 변경 (퀘스트/스토리 이벤트)

```cpp
// 퀘스트 완료 시: 고블린 부족이 플레이어와 동맹
void OnQuestCompleted()
{
    TeamSubsystem->ChangeFactionRelationship(
        GoblinTeam,
        PlayerTeam,
        EHarmoniaTeamRelationship::Ally,
        true  // 양방향
    );

    // 고블린 AI에게 전투 중지 명령
    NotifyAllGoblinsOfAlliance();
}

// 배신 이벤트: 동맹이 적으로 변경
void OnBetrayalEvent()
{
    TeamSubsystem->ChangeFactionRelationship(
        GoblinTeam,
        PlayerTeam,
        EHarmoniaTeamRelationship::Enemy,
        true
    );
}
```

### 2.4 팀 관계 조회

```cpp
// 특정 팀의 모든 적 조회
TArray<FHarmoniaTeamIdentification> Enemies = TeamSubsystem->GetEnemyFactions(PlayerTeam);
for (const FHarmoniaTeamIdentification& Enemy : Enemies)
{
    UE_LOG(LogTemp, Log, TEXT("Enemy: %s"), *Enemy.TeamName.ToString());
}

// 특정 팀의 모든 동맹 조회
TArray<FHarmoniaTeamIdentification> Allies = TeamSubsystem->GetAlliedFactions(PlayerTeam);
```

---

## 3. 블루프린트 사용법

### 3.1 팀 생성 (블루프린트)

```
1. Get World Subsystem (Class: HarmoniaTeamManagementSubsystem)
2. Create Faction
   - Faction Name: "Goblin Tribe"
   - Default Attitude: Hostile
   - Team Color: (R=1, G=0, B=0)
3. Return Value를 변수에 저장
```

### 3.2 피아식별 쿼리

```
// 두 액터가 적인지 확인
Get World Subsystem -> Are Enemies (Actor A, Actor B)

// 두 액터가 동맹인지 확인
Get World Subsystem -> Are Allies (Actor A, Actor B)

// 공격 가능 여부
Get World Subsystem -> Can Actor Attack (Source, Target)
```

### 3.3 몬스터 AI에서 사용

Behavior Tree에서:

```
Decorator: Team Query
- Actor A: Self
- Actor B: Target
- Relationship: Enemy
- Success: Execute Attack
```

---

## 4. 데이터 애셋 방식

### 4.1 팀 데이터 애셋 생성

1. Content Browser에서 우클릭
2. `Miscellaneous > Data Asset > HarmoniaTeamConfigData`
3. 팀 설정:
   - Team ID (Numeric ID 또는 GameplayTag)
   - Team Name
   - Default Relationships (다른 팀과의 관계)
   - bAllowFriendlyFire
   - bAutoTargetPlayers

### 4.2 몬스터 데이터에서 사용

`UHarmoniaMonsterData` 애셋에서:

```
AI > Team
- Team ID: 생성한 팀 데이터 애셋 선택
- Use Team System: ✓ (체크)
```

---

## 5. 실전 시나리오 예시

### 시나리오 1: 세력간 전쟁

```cpp
// 게임 시작 시 세력 구조 생성
void InitializeFactionWar()
{
    UHarmoniaTeamManagementSubsystem* TeamSys = GetWorld()->GetSubsystem<UHarmoniaTeamManagementSubsystem>();

    // 세력 생성
    FHarmoniaTeamIdentification Humans = TeamSys->CreateFaction(FText::FromString("Human Alliance"), EHarmoniaTeamAttitude::Defensive);
    FHarmoniaTeamIdentification Elves = TeamSys->CreateFaction(FText::FromString("Elven Kingdom"), EHarmoniaTeamAttitude::Neutral);
    FHarmoniaTeamIdentification Orcs = TeamSys->CreateFaction(FText::FromString("Orc Horde"), EHarmoniaTeamAttitude::Hostile);
    FHarmoniaTeamIdentification Demons = TeamSys->CreateFaction(FText::FromString("Demon Legion"), EHarmoniaTeamAttitude::Hostile);

    // 동맹 구조
    TeamSys->MakeTeamsAllies(Humans, Elves);  // 인간-엘프 동맹

    // 적대 관계
    TArray<FHarmoniaTeamIdentification> GoodFactions = {Humans, Elves};
    TArray<FHarmoniaTeamIdentification> EvilFactions = {Orcs, Demons};

    for (const auto& Good : GoodFactions)
    {
        for (const auto& Evil : EvilFactions)
        {
            TeamSys->MakeTeamsEnemies(Good, Evil);
        }
    }

    // 오크와 데몬도 서로 적대 (내분)
    TeamSys->MakeTeamsEnemies(Orcs, Demons);
}
```

### 시나리오 2: 동적 관계 변화 (스토리 진행)

```cpp
// Chapter 1: 엘프는 중립
// (이미 위에서 설정됨)

// Chapter 2: 플레이어가 엘프의 신뢰를 얻음
void OnChapter2Start()
{
    TeamSys->ChangeFactionRelationship(Elves, PlayerTeam, EHarmoniaTeamRelationship::Ally, true);
}

// Chapter 3: 오크와 데몬이 일시적으로 동맹 (공동의 적)
void OnChapter3BossEvent()
{
    TeamSys->MakeTeamsAllies(Orcs, Demons);
}

// Chapter 4: 동맹 붕괴
void OnChapter4()
{
    TeamSys->BreakAlliance(Orcs, Demons);
    TeamSys->MakeTeamsEnemies(Orcs, Demons);  // 다시 적대
}
```

### 시나리오 3: 무제한 팀 확장

```cpp
// 게임이 진행되면서 새로운 세력이 계속 추가됨
void AddNewFaction(FText FactionName, TArray<FHarmoniaTeamIdentification> Allies, TArray<FHarmoniaTeamIdentification> Enemies)
{
    UHarmoniaTeamManagementSubsystem* TeamSys = GetWorld()->GetSubsystem<UHarmoniaTeamManagementSubsystem>();

    // 새 팀 생성
    FHarmoniaTeamIdentification NewFaction = TeamSys->CreateFaction(FactionName, EHarmoniaTeamAttitude::Neutral);

    // 동맹 설정
    for (const auto& Ally : Allies)
    {
        TeamSys->MakeTeamsAllies(NewFaction, Ally);
    }

    // 적 설정
    for (const auto& Enemy : Enemies)
    {
        TeamSys->MakeTeamsEnemies(NewFaction, Enemy);
    }

    UE_LOG(LogTemp, Log, TEXT("Added new faction: %s (Total factions: %d)"),
        *FactionName.ToString(), TeamSys->GetFactionCount());
}

// 사용 예시
void OnDiscoverNewContinent()
{
    // 새 대륙의 원주민 세력 추가
    TArray<FHarmoniaTeamIdentification> NoAllies;  // 처음엔 동맹 없음
    TArray<FHarmoniaTeamIdentification> InitialEnemies = {DemonTeam};  // 데몬만 적대

    FHarmoniaTeamIdentification Natives = AddNewFaction(
        FText::FromString("Native Tribes"),
        NoAllies,
        InitialEnemies
    );

    // 나중에 플레이어의 행동에 따라 관계 변경 가능
}
```

---

## 6. 언리얼 표준 AI 시스템 통합

### 6.1 Behavior Tree에서 사용

```
EQS (Environment Query System):
- Distance To (Team: Enemy)
- Distance To (Team: Friendly)

Decorator: Team Query
- Team Attitude: Hostile
- Team Attitude: Friendly
```

### 6.2 AI Controller에서 직접 사용

```cpp
ETeamAttitude::Type Attitude = Monster->GetTeamAttitudeTowards(*Player);

if (Attitude == ETeamAttitude::Hostile)
{
    // 공격 로직
}
else if (Attitude == ETeamAttitude::Friendly)
{
    // 지원 로직
}
```

---

## 7. 네트워크 멀티플레이어

팀 시스템은 자동으로 네트워크 복제를 지원합니다:

```cpp
// 서버에서만 팀 관계 설정
if (HasAuthority())
{
    TeamSubsystem->MakeTeamsEnemies(TeamA, TeamB);
}

// 클라이언트에서는 자동으로 동기화됨
bool bIsEnemy = TeamSubsystem->AreEnemies(MonsterA, MonsterB);  // 올바른 결과 반환
```

---

## 8. 성능 최적화

- **Numeric ID 사용**: GameplayTag 대신 숫자 ID 사용으로 빠른 비교
- **초기 복제**: 팀 ID는 스폰 시 한 번만 복제 (COND_InitialOnly)
- **캐싱**: 자주 조회하는 관계는 캐싱 권장

```cpp
// ❌ 나쁜 예: 매 프레임 조회
void Tick(float DeltaTime)
{
    bool bIsEnemy = TeamSubsystem->AreEnemies(this, Target);  // 느림!
}

// ✅ 좋은 예: 캐싱
void OnTargetChanged(AActor* NewTarget)
{
    bCachedIsEnemy = TeamSubsystem->AreEnemies(this, NewTarget);
}

void Tick(float DeltaTime)
{
    if (bCachedIsEnemy)
    {
        // 빠른 체크
    }
}
```

---

## 정리

하모니아 팀 시스템은:

✅ **확장성**: 무제한 팀 생성 가능
✅ **유연성**: 런타임에 관계 변경 가능
✅ **통합성**: 언리얼 표준 AI 시스템과 완벽 연동
✅ **효율성**: 최적화된 네트워크 복제
✅ **편의성**: 블루프린트 완벽 지원

이제 복잡한 세력 구조와 동적인 관계 변화가 있는 게임을 자유롭게 구현할 수 있습니다!
