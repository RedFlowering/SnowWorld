# Harmonia Quest System

**Version:** 2.0 (Enhanced Edition)  
**Last Updated:** 2025-11-23

하모니아 키트 플러그인의 퀘스트 시스템입니다. 모던 RPG에 필요한 모든 퀘스트 기능을 지원하며, 기본 퀘스트부터 복잡한 단계별 스토리 퀘스트까지 구현할 수 있습니다.

## 🎯 핵심 기능

### 1. 기본 퀘스트 시스템
- **8가지 퀘스트 타입**: Main, Side, Daily, Weekly, Repeatable, Story, Tutorial, Achievement
- **14가지 목표 타입**: Kill, Collect, Talk, Reach, Craft, Build, Equip, Use, Gather, Deliver, Escort, Defend, Discover, Custom
- **조건 시스템**: 선행 퀘스트, 레벨, 아이템, 태그, 레시피
- **보상 시스템**: 경험치, 골드, 아이템, 레시피, 태그, 퀘스트 해금

### 2. ⭐ 퀘스트 단계(Phase) 시스템
복잡한 스토리 퀘스트를 여러 단계로 분할:

```cpp
// 예: "마을 방어" 퀘스트
Phase 0: "준비" - NPC와 대화
Phase 1: "1차 공격" - 적 10마리 처치
Phase 2: "보급" - 보급품 3개 전달
Phase 3: "최종 결전" - 보스 처치
```

**데이터 구조:**
```cpp
USTRUCT(BlueprintType)
struct FQuestPhase
{
    int32 PhaseNumber;                      // 단계 번호
    FText PhaseName;                        // 단계 이름
    FText PhaseDescription;                 // 단계 설명
    TArray<FQuestObjective> PhaseObjectives;// 단계 목표
    TArray<FQuestMarker> PhaseMarkers;      // 단계 마커
    TArray<FQuestEvent> PhaseEvents;        // 단계 이벤트
    bool bAutoAdvance;                      // 자동 진행 여부
};
```

### 3. 🗺️ 퀘스트 마커 시스템
맵과 컴패스에 표시되는 시각적 가이드:

```cpp
USTRUCT(BlueprintType)
struct FQuestMarker
{
    EQuestMarkerType MarkerType;            // Objective, Optional, Discover 등
    FVector WorldLocation;                  // 월드 위치
    TWeakObjectPtr<AActor> TargetActor;     // 동적 타겟 (NPC 따라다님)
    bool bShowDistance;                     // 거리 표시 여부
    TSoftObjectPtr<UTexture2D> MarkerIcon;  // 마커 아이콘
    FLinearColor MarkerColor;               // 마커 색상
};
```

**마커 타입:**
- `Objective`: 주요 목표 (노란색)
- `Optional`: 선택 목표 (파란색)
- `Discover`: 탐험 지점 (녹색)
- `Deliver`: 전달 지점 (주황색)
- `Talk`: 대화 NPC (흰색)
- `Gather`: 채집 지점 (갈색)

### 4. 💬 퀘스트 대화 시스템
NPC와의 대화를 퀘스트 상태에 따라 자동 변경:

```cpp
USTRUCT(BlueprintType)
struct FQuestDialogue
{
    TArray<FText> StartDialogues;           // 퀘스트 시작 시
    TArray<FText> InProgressDialogues;      // 진행 중
    TArray<FText> ReadyToCompleteDialogues; // 완료 가능
    TArray<FText> CompletionDialogues;      // 완료 시
    TArray<FText> FailureDialogues;         // 실패 시
};
```

### 5. 💡 퀘스트 힌트 시스템
플레이어가 막혔을 때 자동으로 힌트 제공:

```cpp
USTRUCT(BlueprintType)
struct FQuestHint
{
    FText HintText;                         // 힌트 텍스트
    float DelayBeforeShowing;               // 표시 지연 시간 (초)
    FVector HintLocation;                   // 힌트 위치 (선택)
    bool bHasLocation;                      // 위치 마커 표시 여부
    bool bShown;                            // 이미 표시됨 여부
};
```

### 6. 📊 동적 목표 시스템
플레이어 레벨이나 파티 크기에 따라 목표 수량 자동 조정:

```cpp
USTRUCT(BlueprintType)
struct FDynamicObjectiveCount
{
    int32 BaseCount;                        // 기본 수량
    float CountPerLevel;                    // 레벨당 추가
    float CountPerPartyMember;              // 파티원당 추가
    int32 MaxCount;                         // 최대 제한
};
```

### 7. 👥 파티 퀘스트 시스템
멀티플레이어 환경에서 퀘스트 공유:

```cpp
// FQuestData에 추가된 필드들
bool bCanShare;                             // 공유 가능 여부
bool bSharedProgress;                       // 진행 공유 (한 명이 처치하면 모두 카운트)
int32 MinPartySize;                         // 최소 파티 크기
int32 MaxPartySize;                         // 최대 파티 크기
```

### 8. ⭐ 보너스 목표 시스템
선택적이지만 추가 보상을 주는 목표:

```cpp
// FQuestObjective에 추가
bool bBonus;                                // 보너스 목표 여부

// FQuestData에 추가
TArray<FQuestReward> BonusRewards;          // 보너스 목표 완료 시 추가 보상
```

### 9. ❌ 퀘스트 실패 조건
시간 제한 외 다양한 실패 조건:

```cpp
USTRUCT(BlueprintType)
struct FQuestFailCondition
{
    EQuestFailConditionType ConditionType;  // 실패 조건 타입
    FHarmoniaID TargetId;                   // 대상 ID
    FText FailureMessage;                   // 실패 메시지
    FVector FailLocation;                   // 위치 (LocationLeft용)
    float FailRadius;                       // 반경
};
```

**실패 조건 타입:**
- `TimeLimit`: 시간 초과
- `NPCDied`: NPC 사망
- `ItemLost`: 아이템 손실
- `LocationLeft`: 지역 이탈
- `PlayerDied`: 플레이어 사망

### 10. 🎬 퀘스트 이벤트 시스템
퀘스트 상태 변화 시 게임 이벤트 발생:

```cpp
USTRUCT(BlueprintType)
struct FQuestEvent
{
    EQuestEventTrigger TriggerType;         // 트리거 타입
    TSubclassOf<AActor> ActorToSpawn;       // 스폰할 액터
    FVector SpawnLocation;                  // 스폰 위치
    FGameplayTagContainer EventTags;        // 이벤트 태그
    bool bBroadcastToWorld;                 // 월드 전체 알림
    FName CustomEventName;                  // 커스텀 이벤트
};
```

### 11. 🔔 퀘스트 알림 시스템
UI 알림으로 퀘스트 진행 상황 표시:

```cpp
USTRUCT(BlueprintType)
struct FQuestNotification
{
    EQuestNotificationType NotificationType; // 알림 타입
    FHarmoniaID QuestId;                    // 퀘스트 ID
    FText QuestName;                        // 퀘스트 이름
    FText Message;                          // 알림 메시지
    TSoftObjectPtr<UTexture2D> Icon;        // 알림 아이콘
    float DisplayDuration;                  // 표시 시간 (초)
};
```

### 12. 📈 퀘스트 통계 시스템
플레이어의 퀘스트 활동 추적:

```cpp
USTRUCT(BlueprintType)
struct FQuestStatistics
{
    int32 TotalQuestsCompleted;             // 총 완료 퀘스트
    int32 MainQuestsCompleted;              // 메인 퀘스트 완료
    int32 DailyQuestsCompleted;             // 일일 퀘스트 완료
    float AverageCompletionTime;            // 평균 완료 시간
    TMap<EQuestType, int32> CompletedByType;// 타입별 완료 수
    int32 CurrentStreak;                    // 현재 연속 완료
    int32 BestStreak;                       // 최고 연속 완료
    FDateTime LastCompletionDate;           // 마지막 완료 날짜
};
```

### 13. 📔 퀘스트 로그/저널
플레이어의 퀘스트 기록:

```cpp
USTRUCT(BlueprintType)
struct FQuestLogEntry
{
    FHarmoniaID QuestId;                    // 퀘스트 ID
    FDateTime StartTime;                    // 시작 시간
    FDateTime CompletionTime;               // 완료 시간
    float TotalTime;                        // 소요 시간
    TArray<FString> PlayerNotes;            // 플레이어 메모
    int32 CompletionCount;                  // 완료 횟수 (반복 퀘스트)
    float BestCompletionTime;               // 최단 완료 시간
    bool bFavorited;                        // 즐겨찾기 여부
};
```

### 14. 🎚️ 퀘스트 우선순위 시스템
여러 활성 퀘스트의 UI 표시 순서 관리:

```cpp
// FQuestData에 추가
int32 TrackingPriority;                     // 높을수록 위에 표시

// 우선순위 계산 예:
// Main Quest: 100
// Story Quest: 90
// Daily Quest: 80
// Side Quest: 50
```

## 📊 데이터 테이블 구조

### Quest DataTable 예제

```
[Quest_Tutorial_001]
QuestId: Quest_Tutorial_001
QuestType: Tutorial
QuestName: "전투 튜토리얼"
QuestDescription: "기본 전투 방법을 배웁니다."

Phases:
  [0]:  // Phase 1: 무기 장착
    PhaseName: "무기 장착"
    PhaseObjectives:
      [0]:
        ObjectiveType: Equip
        Description: "나무 검 장착"
        TargetId: Item_WoodenSword
        RequiredCount: 1
    PhaseMarkers:
      [0]:
        MarkerType: Objective
        TargetActor: Tutorial_WeaponChest
    bAutoAdvance: true

  [1]:  // Phase 2: 적 처치
    PhaseName: "전투 실습"
    PhaseObjectives:
      [0]:
        ObjectiveType: Kill
        Description: "훈련용 허수아비 파괴"
        TargetId: Enemy_Dummy
        RequiredCount: 3
      [1]:
        ObjectiveType: Kill
        Description: "보너스: 완벽한 패리 3회"
        TargetId: Enemy_Dummy
        RequiredCount: 3
        bBonus: true
    Markers:
      [0]:
        MarkerType: Objective
        TargetLocation: (1000, 500, 0)

Dialogues:
  StartDialogues:
    [0]: "전투 방법을 알려드리겠습니다."
  InProgressDialogues:
    [0]: "계속 연습하세요!"
  CompletionDialogues:
    [0]: "훌륭합니다! 이제 실전에 나갈 준비가 되었습니다."

Hints:
  [0]:
    HintText: "E 키를 눌러 상자를 열고 무기를 획득하세요."
    DelayBeforeShowing: 10.0
  [1]:
    HintText: "마우스 왼쪽 버튼으로 공격하세요."
    DelayBeforeShowing: 30.0

Events:
  [0]:
    TriggerType: OnStart
    ActorToSpawn: TutorialHelper
    SpawnLocation: (500, 0, 100)
  [1]:
    TriggerType: OnComplete
    EventTags: "Tutorial.Combat.Complete"

Rewards:
  [0]:
    RewardType: Experience
    ExperienceAmount: 100
  [1]:
    RewardType: Gold
    GoldAmount: 50

BonusRewards:
  [0]:
    RewardType: Item
    ItemId: Item_HealthPotion
    ItemAmount: 3

TrackingPriority: 100
bCanShare: false
```

## 🎮 사용 예제

### 1. 단계별 퀘스트 진행

```cpp
// Phase 1 목표 완료 시 자동으로 Phase 2로 진행
void UHarmoniaQuestComponent::OnPhaseObjectivesCompleted(FHarmoniaID QuestId, int32 PhaseNumber)
{
    FQuestData QuestData;
    if (GetQuestData(QuestId, QuestData))
    {
        if (QuestData.Phases.IsValidIndex(PhaseNumber))
        {
            const FQuestPhase& Phase = QuestData.Phases[PhaseNumber];

            if (Phase.bAutoAdvance)
            {
                AdvanceToNextPhase(QuestId);
            }
        }
    }
}
```

### 2. 동적 마커 업데이트

```cpp
// NPC를 따라다니는 마커
FQuestMarker Marker;
Marker.MarkerType = EQuestMarkerType::Talk;
Marker.TargetActor = QuestGiverNPC;
Marker.bShowDistance = true;
Marker.MarkerColor = FLinearColor::White;

// 마커는 NPC가 이동해도 자동으로 따라다님
```

### 3. 힌트 시스템 활용

```cpp
// 30초 동안 진행 없으면 힌트 표시
if (ElapsedTimeSinceLastProgress > Hint.DelayBeforeShowing && !Hint.bShown)
{
    ShowHintToPlayer(Hint.HintText);

    if (Hint.bHasLocation)
    {
        ShowHintMarker(Hint.HintLocation);
    }

    Hint.bShown = true;
}
```

### 4. 파티 퀘스트

```cpp
// 파티원과 퀘스트 공유
void ShareQuestWithParty(FHarmoniaID QuestId)
{
    FQuestData QuestData;
    if (GetQuestData(QuestId, QuestData) && QuestData.bCanShare)
    {
        TArray<APlayerController*> PartyMembers = GetPartyMembers();

        for (APlayerController* Member : PartyMembers)
        {
            UHarmoniaQuestComponent* QuestComp = GetQuestComponent(Member);
            if (QuestComp)
            {
                QuestComp->StartQuest(QuestId);
            }
        }
    }
}
```

### 5. 통계 추적

```cpp
// 퀘스트 완료 시 통계 업데이트
void UpdateStatistics(FHarmoniaID QuestId, const FQuestData& QuestData, float CompletionTime)
{
    Statistics.TotalQuestsCompleted++;

    switch (QuestData.QuestType)
    {
        case EQuestType::Main:
            Statistics.MainQuestsCompleted++;
            break;
        case EQuestType::Daily:
            Statistics.DailyQuestsCompleted++;
            UpdateDailyStreak();
            break;
    }

    // 평균 완료 시간 갱신
    float TotalTime = Statistics.AverageCompletionTime * (Statistics.TotalQuestsCompleted - 1);
    Statistics.AverageCompletionTime = (TotalTime + CompletionTime) / Statistics.TotalQuestsCompleted;

    // 최장 시간 퀘스트 갱신
    if (CompletionTime > Statistics.LongestQuestTime)
    {
        Statistics.LongestQuestTime = CompletionTime;
        Statistics.LongestQuestCompleted = QuestId;
    }
}
```

## 🎨 UI 연동 예제

### 퀘스트 트래커 위젯

```cpp
// 우선순위 순으로 정렬
TArray<FActiveQuestProgress> SortedQuests = ActiveQuests;
SortedQuests.Sort([this](const FActiveQuestProgress& A, const FActiveQuestProgress& B)
{
    FQuestData DataA, DataB;
    GetQuestData(A.QuestId, DataA);
    GetQuestData(B.QuestId, DataB);

    return DataA.TrackingPriority > DataB.TrackingPriority;
});

// UI에 표시
for (const FActiveQuestProgress& Quest : SortedQuests)
{
    AddQuestToTracker(Quest);
}
```

### 알림 시스템

```cpp
// 목표 완료 알림
FQuestNotification Notification;
Notification.NotificationType = EQuestNotificationType::ObjectiveComplete;
Notification.QuestId = QuestId;
Notification.QuestName = QuestData.QuestName;
Notification.Message = FText::Format(
    LOCTEXT("ObjectiveComplete", "[목표 완료] {0}"),
    Objective.Description
);
Notification.DisplayDuration = 3.0f;

ShowNotification(Notification);
```

## 🎓 튜토리얼

### 첫 번째 단계별 퀘스트 만들기

1. **DataTable 생성**
   - Content Browser → 우클릭 → Miscellaneous → Data Table
   - Row Structure: `FQuestData`

2. **퀘스트 설정**
   ```
   Row Name: Quest_FirstPhased

   Phases:
     Phase 0: "시작" - NPC 대화
     Phase 1: "수집" - 아이템 3개 수집
     Phase 2: "완료" - NPC에게 돌아가기
   ```

3. **컴포넌트 추가**
   - 플레이어 BP → Add Component → HarmoniaQuestComponent
   - Quest Data Table 설정

4. **테스트**
   - 퀘스트 시작
   - Phase별 목표 확인
   - 자동 진행 확인

완성입니다! 이제 모던 RPG급 퀘스트 시스템을 사용할 수 있습니다. 🎉
