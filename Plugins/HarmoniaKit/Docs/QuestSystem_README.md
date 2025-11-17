# Harmonia Quest System

하모니아 키트 플러그인의 퀘스트 시스템입니다. 다양한 종류의 퀘스트를 쉽게 관리하고 시나리오를 설계할 수 있습니다.

## 주요 기능

### 1. 퀘스트 타입
- **Main Quest**: 메인 스토리 퀘스트
- **Side Quest**: 사이드 퀘스트
- **Daily Quest**: 일일 퀘스트 (자동 리셋)
- **Weekly Quest**: 주간 퀘스트 (자동 리셋)
- **Repeatable**: 반복 가능한 퀘스트
- **Story**: 스토리 퀘스트
- **Tutorial**: 튜토리얼 퀘스트
- **Achievement**: 업적 퀘스트

### 2. 퀘스트 목표 타입
- **Kill**: 적 처치 (예: "늑대 10마리 처치")
- **Collect**: 아이템 수집 (예: "나무 열매 5개 수집")
- **Talk**: NPC 대화 (예: "마을 촌장과 대화")
- **Reach**: 위치 도달 (예: "동굴 입구 도달")
- **Craft**: 아이템 제작 (예: "철 검 제작")
- **Build**: 구조물 건설 (예: "작업대 건설")
- **Equip**: 아이템 장착 (예: "철 갑옷 장착")
- **Use**: 아이템 사용 (예: "치유 물약 사용")
- **Gather**: 자원 채집 (예: "철광석 10개 채집")
- **Deliver**: 아이템 전달 (예: "편지를 상인에게 전달")
- **Escort**: NPC 호위 (예: "상인을 마을까지 호위")
- **Defend**: 위치 방어 (예: "마을을 5분간 방어")
- **Discover**: 지역 탐험 (예: "고대 유적 발견")
- **Custom**: 커스텀 목표 (게임플레이 태그 기반)

### 3. 퀘스트 조건 시스템
- **QuestCompleted**: 선행 퀘스트 완료 조건
- **QuestNotCompleted**: 특정 퀘스트 미완료 조건
- **Level**: 레벨 요구사항
- **HasItem**: 아이템 소지 조건
- **HasTag**: 게임플레이 태그 소지 조건
- **RecipeKnown**: 레시피 습득 조건
- **Custom**: 커스텀 조건

### 4. 퀘스트 보상
- **Experience**: 경험치
- **Gold**: 골드
- **Item**: 아이템
- **Recipe**: 레시피 해금
- **Tag**: 게임플레이 태그 부여
- **UnlockQuest**: 퀘스트 해금
- **Custom**: 커스텀 보상

## 컴포넌트

### HarmoniaQuestComponent
플레이어 캐릭터에 부착하여 퀘스트를 관리하는 컴포넌트입니다.

**주요 기능:**
- 퀘스트 시작/완료/포기/실패
- 퀘스트 목표 진행상황 추적
- 퀘스트 조건 검증
- 퀘스트 보상 지급
- 시간 제한 퀘스트 관리
- 퀘스트 체인 자동 진행

**주요 함수:**
```cpp
// 퀘스트 시작
bool StartQuest(FHarmoniaID QuestId);

// 퀘스트 완료 (선택적 보상 선택 가능)
bool CompleteQuest(FHarmoniaID QuestId, const TArray<int32>& SelectedOptionalRewards);

// 퀘스트 포기
bool AbandonQuest(FHarmoniaID QuestId);

// 퀘스트 목표 업데이트 (타입과 타겟으로)
void UpdateQuestObjectivesByType(EQuestObjectiveType ObjectiveType, FHarmoniaID TargetId, int32 Progress = 1);

// 퀘스트 상태 확인
bool IsQuestActive(FHarmoniaID QuestId) const;
bool IsQuestCompleted(FHarmoniaID QuestId) const;
bool IsQuestAvailable(FHarmoniaID QuestId) const;

// 사용 가능한 퀘스트 목록 가져오기
TArray<FHarmoniaID> GetAvailableQuests() const;
```

**델리게이트:**
```cpp
// 퀘스트 시작 시
UPROPERTY(BlueprintAssignable)
FOnQuestStarted OnQuestStarted;

// 퀘스트 목표 업데이트 시
UPROPERTY(BlueprintAssignable)
FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

// 퀘스트 완료 시
UPROPERTY(BlueprintAssignable)
FOnQuestCompleted OnQuestCompleted;

// 퀘스트 완료 가능 시
UPROPERTY(BlueprintAssignable)
FOnQuestReadyToComplete OnQuestReadyToComplete;
```

### HarmoniaQuestSubsystem
게임 전체의 퀘스트 데이터를 관리하는 서브시스템입니다.

**주요 기능:**
- 퀘스트 데이터 캐싱 및 빠른 조회
- 퀘스트 체인 검증
- 일일/주간 퀘스트 리셋 관리
- 퀘스트 검색 및 필터링
- 퀘스트 통계 및 분석

**주요 함수:**
```cpp
// 퀘스트 데이터 가져오기
bool GetQuestData(FHarmoniaID QuestId, FQuestData& OutQuestData) const;

// 타입별 퀘스트 가져오기
TArray<FQuestData> GetQuestsByType(EQuestType QuestType) const;

// 퀘스트 체인 가져오기
void GetQuestChain(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutQuestChain, bool bIncludeStartQuest = true) const;

// 퀘스트 체인 검증 (순환 참조 체크)
bool ValidateQuestChain(FHarmoniaID QuestId) const;

// 레벨에 맞는 퀘스트 추천
TArray<FQuestData> GetRecommendedQuestsForLevel(int32 PlayerLevel, int32 LevelRange = 5) const;
```

## 사용 예제

### 1. 퀘스트 데이터 테이블 생성
DataTable을 생성하고 Row Type을 `FQuestData`로 설정합니다.

### 2. 퀘스트 설정 예제

```
[Quest_MainStory_001]
QuestId: Quest_MainStory_001
QuestType: Main
QuestName: "숲의 위협"
QuestDescription: "마을을 위협하는 늑대들을 처치하세요."

Objectives:
  [0]:
    ObjectiveType: Kill
    Description: "늑대 10마리 처치"
    TargetId: Enemy_Wolf
    RequiredCount: 10

UnlockConditions:
  [0]:
    ConditionType: Level
    RequiredLevel: 5

Rewards:
  [0]:
    RewardType: Experience
    ExperienceAmount: 500
  [1]:
    RewardType: Gold
    GoldAmount: 100
  [2]:
    RewardType: Item
    ItemId: Item_HealthPotion
    ItemAmount: 3

NextQuests:
  [0]: Quest_MainStory_002
```

### 3. C++ 사용 예제

```cpp
// 퀘스트 컴포넌트 가져오기
UHarmoniaQuestComponent* QuestComponent = GetOwner()->FindComponentByClass<UHarmoniaQuestComponent>();

// 퀘스트 시작
QuestComponent->StartQuest(FHarmoniaID(TEXT("Quest_MainStory_001")));

// 목표 업데이트 (늑대 처치 시)
QuestComponent->UpdateQuestObjectivesByType(
    EQuestObjectiveType::Kill,
    FHarmoniaID(TEXT("Enemy_Wolf")),
    1  // 1마리 처치
);

// 퀘스트 완료
QuestComponent->CompleteQuest(FHarmoniaID(TEXT("Quest_MainStory_001")));
```

### 4. Blueprint 사용 예제

```
1. 캐릭터 블루프린트에 HarmoniaQuestComponent 추가
2. Quest Data Table 설정
3. 이벤트 그래프에서:
   - Start Quest 노드로 퀘스트 시작
   - Update Quest Objectives By Type 노드로 목표 업데이트
   - Complete Quest 노드로 퀘스트 완료
```

### 5. 퀘스트 체인 설정

```
Quest_Tutorial_001 (튜토리얼: 기본 조작)
  └─> Quest_Tutorial_002 (튜토리얼: 전투)
       └─> Quest_Tutorial_003 (튜토리얼: 제작)
            └─> Quest_MainStory_001 (메인 스토리 시작)
```

### 6. 선택적 보상 설정

```
OptionalRewards:
  [0]:
    RewardType: Item
    ItemId: Item_IronSword
  [1]:
    RewardType: Item
    ItemId: Item_IronShield
  [2]:
    RewardType: Item
    ItemId: Item_IronHelmet

MaxOptionalRewardChoices: 1  // 플레이어는 3개 중 1개 선택 가능
```

## 시스템 통합

### 인벤토리 시스템 연동
```cpp
// Collect 목표 자동 업데이트
void OnItemAdded(FHarmoniaID ItemId, int32 Amount)
{
    QuestComponent->UpdateQuestObjectivesByType(
        EQuestObjectiveType::Collect,
        ItemId,
        Amount
    );
}
```

### 제작 시스템 연동
```cpp
// Craft 목표 자동 업데이트
void OnItemCrafted(FHarmoniaID ItemId, int32 Amount)
{
    QuestComponent->UpdateQuestObjectivesByType(
        EQuestObjectiveType::Craft,
        ItemId,
        Amount
    );
}
```

### 전투 시스템 연동
```cpp
// Kill 목표 자동 업데이트
void OnEnemyKilled(AActor* Enemy)
{
    FHarmoniaID EnemyId = GetEnemyId(Enemy);
    QuestComponent->UpdateQuestObjectivesByType(
        EQuestObjectiveType::Kill,
        EnemyId,
        1
    );
}
```

## 저장/로드

```cpp
// 퀘스트 데이터 저장
FQuestSaveData SaveData = QuestComponent->GetQuestSaveData();
// SaveData를 세이브 게임에 저장

// 퀘스트 데이터 로드
QuestComponent->LoadQuestFromSaveData(SaveData);
```

## 디버그 기능

```cpp
#if WITH_EDITOR
// 퀘스트 목표 즉시 완료
QuestComponent->Debug_CompleteQuestObjectives(QuestId);

// 모든 퀘스트 해금
QuestComponent->Debug_UnlockAllQuests();

// 모든 퀘스트 초기화
QuestComponent->Debug_ResetAllQuests();

// 퀘스트 체인 검증
Subsystem->ValidateAllQuests(OutErrors);

// 퀘스트 의존성 그래프 출력
Subsystem->PrintQuestDependencyGraph();
#endif
```

## 보안 기능

- 서버 권한 검증 (Server RPC)
- 속도 제한 (Rate Limiting)
- 최대 활성 퀘스트 제한
- 퀘스트 조건 서버 측 검증
- 보상 지급 서버 측 처리

## 성능 최적화

- 퀘스트 데이터 캐싱
- 효율적인 목표 업데이트 (타입 및 태그 기반 자동 매칭)
- 네트워크 복제 최적화
- 시간 제한 퀘스트만 틱 활성화

## 확장 가능성

시스템을 확장하려면:
1. `EQuestObjectiveType`에 새로운 목표 타입 추가
2. `EQuestConditionType`에 새로운 조건 타입 추가
3. `EQuestRewardType`에 새로운 보상 타입 추가
4. 해당 로직을 Component와 Subsystem에 구현

## 라이센스

Copyright 2025 Snow Game Studio.
