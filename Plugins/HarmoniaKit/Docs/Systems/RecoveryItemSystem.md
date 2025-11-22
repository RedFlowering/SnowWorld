# Harmonia Recovery Item System

## 개요

제한된 회복 아이템 시스템 - 다크소울의 에스투스 병에서 영감을 받아 SnowWorld 테마에 맞게 재창조한 독창적인 회복 시스템입니다.

### 핵심 특징

- ✅ **충전 기반 회복**: 체크포인트(크리스탈 공명기)에서 자동 충전
- ✅ **시전 시간**: 즉시 회복이 아닌 1-2초의 시전 시간 (전략적 사용)
- ✅ **다양한 아이템 타입**: 4가지 독특한 회복 아이템
- ✅ **업그레이드 시스템**: 최대 충전 횟수와 효과 증가
- ✅ **네트워크 멀티플레이 지원**: 완전한 리플리케이션
- ✅ **재사용 가능한 아키텍처**: 확장 가능한 컴포넌트 기반 설계

---

## 회복 아이템 종류

### 1. 공명 파편 (Resonance Shards) ⭐ 기본 아이템

크리스탈 공명기에서 떨어져 나온 작은 파편. 각 색상(주파수)별로 다른 효과를 제공합니다.

#### 6가지 주파수 타입

| 주파수 | 색상 | 효과 |
|--------|------|------|
| **Azure** | 푸른색 | 체력 회복 + 짧은 방어력 증가 |
| **Crimson** | 붉은색 | 체력 회복 + 짧은 공격력 증가 |
| **Verdant** | 녹색 | 체력 + 스태미나 동시 회복 |
| **Aurum** | 금색 | 소량 회복 + 경험치 보너스 |
| **Violet** | 보라색 | 체력 회복 + 마나 회복 |
| **Luminous** | 흰색 | 순수 체력 회복 (가장 높은 회복량) |

**특징:**
- 최대 보유: 5개 (업그레이드로 7개, 10개까지 증가)
- 시전 시간: 1.5초
- 체크포인트에서 자동 충전
- 사용 시 고유한 색상의 VFX/SFX

---

### 2. 얼어붙은 시간의 눈송이 (Frozen Time Snowflakes)

시간이 얼어붙은 마법의 눈송이. 시간을 되돌려 최근 피해를 복구합니다.

**특징:**
- 최대 보유: 3개 (매우 희귀)
- 시전 시간: 1초
- 효과: 최근 1-2초간 받은 피해 복구
- 용도: 긴급 회피, 보스전 위기 상황
- 체크포인트에서 충전
- 독특한 시간 역행 VFX

---

### 3. 온천 보온병 (Thermal Spring Flask)

체크포인트 근처의 온천수를 담는 보온병. 지속 회복과 환경 저항을 제공합니다.

**특징:**
- 최대 보유: 4회
- 시전 시간: 2초
- 효과: 3초간 서서히 회복 (HoT - Heal over Time)
- 추가 효과: 얼음 저항 + 추위 무효 (5분 지속)
- 용도: 눈보라 지역 탐험, 지속 전투
- 체크포인트 온천에서 충전

---

### 4. 생명의 루미네센스 (Life Luminescence)

크리스탈 공명 에너지를 압축한 발광체. 설치형 회복 구역을 생성합니다.

**특징:**
- 소모성 아이템 (1회용)
- 설치 시간: 1초
- 효과: 반경 3m 내 60초간 지속 회복
- 용도: 협동 플레이, 보스 전투, 캠핑
- 체크포인트에서 제작 가능
- 범위 내 모든 플레이어 회복

---

## 시스템 아키텍처

### 핵심 컴포넌트

```
┌─────────────────────────────────────────────────────┐
│  UHarmoniaRechargeableItemComponent                 │
│  - 충전 횟수 관리                                    │
│  - 체크포인트 연동                                   │
│  - 네트워크 리플리케이션                            │
│  - 저장/로드                                         │
└─────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────┐
│  UHarmoniaGameplayAbility_UseRecoveryItem            │
│  - 아이템 사용 로직                                  │
│  - 시전 시간 구현                                    │
│  - 회복 효과 적용                                    │
│  - VFX/SFX 재생                                      │
└─────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────┐
│  AHarmoniaRecoveryAreaActor (설치형 전용)            │
│  - 범위 회복 구역                                    │
│  - 주기적 회복 틱                                    │
│  - 시간 제한                                         │
└─────────────────────────────────────────────────────┘
```

### 데이터 구조

```
┌─────────────────────────────────────────────────────┐
│  FHarmoniaRecoveryItemConfig                         │
│  - 아이템 설정 (재사용 가능)                        │
│  - 충전 횟수, 시전 시간, 회복량 등                 │
└─────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────┐
│  FHarmoniaRecoveryItemState                          │
│  - 런타임 상태                                       │
│  - 현재 충전 횟수, 마지막 사용 시간 등            │
└─────────────────────────────────────────────────────┘
```

---

## 사용 방법

### 1. 컴포넌트 추가

플레이어 Character 또는 PlayerController에 `UHarmoniaRechargeableItemComponent`를 추가합니다.

**Blueprint:**
```
1. Character 블루프린트 열기
2. Components 패널에서 "Add Component" 클릭
3. "Harmonia Rechargeable Item Component" 검색 후 추가
```

**C++:**
```cpp
// MyCharacter.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Items")
TObjectPtr<UHarmoniaRechargeableItemComponent> RechargeableItemComponent;

// MyCharacter.cpp
RechargeableItemComponent = CreateDefaultSubobject<UHarmoniaRechargeableItemComponent>(TEXT("RechargeableItemComponent"));
```

---

### 2. 아이템 등록

게임 시작 시 또는 BeginPlay에서 회복 아이템을 등록합니다.

**Blueprint:**
```
Event BeginPlay
  ├─ Get Rechargeable Item Component
  └─ Register Recovery Item
       ├─ Item Type: Resonance Shard
       └─ Config: (설정 구조체)
```

**C++:**
```cpp
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 공명 파편 등록 (Azure)
    FHarmoniaResonanceShardVariant AzureShard;
    AzureShard.Frequency = EHarmoniaResonanceFrequency::Azure;
    AzureShard.ShardName = FText::FromString(TEXT("Azure Resonance Shard"));
    AzureShard.HealthRecoveryPercent = 0.5f; // 50% 회복

    RechargeableItemComponent->RegisterResonanceShard(
        EHarmoniaResonanceFrequency::Azure,
        AzureShard
    );

    // 얼어붙은 시간의 눈송이 등록
    FHarmoniaRecoveryItemConfig SnowflakeConfig;
    SnowflakeConfig.ItemType = EHarmoniaRecoveryItemType::FrozenTimeSnowflake;
    SnowflakeConfig.MaxCharges = 3;
    SnowflakeConfig.UsageDuration = 1.0f;
    SnowflakeConfig.EffectType = EHarmoniaRecoveryEffectType::TimeReversal;

    RechargeableItemComponent->RegisterRecoveryItem(
        EHarmoniaRecoveryItemType::FrozenTimeSnowflake,
        SnowflakeConfig
    );
}
```

---

### 3. Gameplay Ability 설정

회복 아이템 사용을 위한 Gameplay Ability를 생성합니다.

**Blueprint:**
```
1. Content Browser에서 우클릭
2. Blueprint Class → UHarmoniaGameplayAbility_UseRecoveryItem 선택
3. "GA_UseAzureShard" 이름으로 생성
4. 블루프린트 열기
5. Details 패널에서:
   - Item Type: Resonance Shard
   - Shard Frequency: Azure
   - Usage Animation: (애니메이션 몽타주 설정)
```

**Ability 부여:**
```cpp
// AbilitySystemComponent에 Ability 부여
if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
{
    FGameplayAbilitySpec AbilitySpec(GA_UseAzureShard, 1, INDEX_NONE, this);
    ASC->GiveAbility(AbilitySpec);
}
```

---

### 4. 입력 바인딩

Enhanced Input으로 회복 아이템 사용 키를 바인딩합니다.

**Blueprint:**
```
Input Action (IA_UseRecoveryItem)
  ├─ Triggered
  └─ Try Activate Ability By Class
       └─ Ability Class: GA_UseAzureShard
```

**C++:**
```cpp
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(IA_UseRecoveryItem, ETriggerEvent::Triggered, this, &AMyCharacter::UseRecoveryItem);
    }
}

void AMyCharacter::UseRecoveryItem()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // Ability 활성화
        ASC->TryActivateAbilityByClass(GA_UseAzureShard);
    }
}
```

---

### 5. 체크포인트 연동

`UHarmoniaRechargeableItemComponent`는 자동으로 체크포인트 서브시스템과 연동됩니다.

**자동 충전:**
- 플레이어가 체크포인트에서 공명(휴식)하면 자동으로 모든 회복 아이템이 충전됩니다.
- `OnResonanceCompleted` 이벤트에 자동으로 바인딩됨

**수동 충전 (테스트용):**
```cpp
// 특정 아이템 충전
RechargeableItemComponent->RechargeItem(EHarmoniaRecoveryItemType::ResonanceShard);

// 모든 아이템 충전
RechargeableItemComponent->RechargeAllItems();
```

---

### 6. UI 연동

남은 충전 횟수를 UI에 표시합니다.

**Blueprint (Widget):**
```
Event Construct
  ├─ Get Rechargeable Item Component
  ├─ Bind to OnChargesChanged
  └─ Update UI Text
       └─ Get Remaining Charges
```

**C++:**
```cpp
void UMyRecoveryItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UHarmoniaRechargeableItemComponent* ItemComp = GetOwningPlayer()->GetPawn()->FindComponentByClass<UHarmoniaRechargeableItemComponent>())
    {
        ItemComp->OnChargesChanged.AddDynamic(this, &UMyRecoveryItemWidget::UpdateChargesDisplay);

        // 초기 표시
        int32 Charges = ItemComp->GetRemainingCharges(EHarmoniaRecoveryItemType::ResonanceShard);
        UpdateChargesDisplay(EHarmoniaRecoveryItemType::ResonanceShard, Charges);
    }
}

void UMyRecoveryItemWidget::UpdateChargesDisplay(EHarmoniaRecoveryItemType ItemType, int32 NewCharges)
{
    ChargesText->SetText(FText::AsNumber(NewCharges));
}
```

---

## 커스터마이징

### 새로운 회복 아이템 추가

1. `EHarmoniaRecoveryItemType`에 새 타입 추가
2. `FHarmoniaRecoveryItemConfig` 설정
3. 필요 시 새로운 `EHarmoniaRecoveryEffectType` 추가
4. `UHarmoniaGameplayAbility_UseRecoveryItem`에서 효과 로직 구현

**예시: 신성한 물병 (Divine Water)**
```cpp
// Definitions에 추가
enum class EHarmoniaRecoveryItemType : uint8
{
    // ... 기존 타입들
    DivineWater UMETA(DisplayName = "Divine Water")
};

// 등록
FHarmoniaRecoveryItemConfig DivineWaterConfig;
DivineWaterConfig.ItemType = EHarmoniaRecoveryItemType::DivineWater;
DivineWaterConfig.MaxCharges = 3;
DivineWaterConfig.HealthRecoveryPercent = 1.0f; // 100% 회복
DivineWaterConfig.UsageDuration = 3.0f; // 긴 시전 시간
DivineWaterConfig.bRechargeableAtCheckpoint = false; // 체크포인트에서 충전 불가 (희귀)
```

---

### Gameplay Effect 추가

회복 아이템에 버프나 디버프를 추가할 수 있습니다.

**예시: Azure 파편에 방어력 버프 추가**

1. Content Browser에서 Gameplay Effect 생성 (GE_AzureDefenseBonus)
2. Duration Policy: Has Duration (10초)
3. Modifiers 추가:
   - Attribute: Defense
   - Operation: Additive
   - Magnitude: 20

4. 공명 파편 등록 시 추가:
```cpp
AzureShard.AdditionalEffect = GE_AzureDefenseBonus;
```

---

## 고급 기능

### 아이템 업그레이드

체크포인트 강화나 아이템 수집으로 최대 충전 횟수를 증가시킬 수 있습니다.

```cpp
// 최대 충전 횟수 +2 업그레이드
RechargeableItemComponent->UpgradeItemMaxCharges(
    EHarmoniaRecoveryItemType::ResonanceShard,
    2
);
```

---

### 저장/로드

아이템 상태를 자동으로 저장/로드할 수 있습니다.

**저장:**
```cpp
TArray<FHarmoniaRecoveryItemState> ItemStates = RechargeableItemComponent->GetItemStatesForSave();
// SaveGame에 ItemStates 저장
```

**로드:**
```cpp
// SaveGame에서 ItemStates 로드
RechargeableItemComponent->LoadItemStates(LoadedItemStates);
```

---

## Gameplay Tags

회복 아이템 사용 시 권장되는 Gameplay Tags:

### 상태 태그
```
State.Item.Using              // 아이템 사용 중
State.Item.CannotUse          // 아이템 사용 불가 상태
```

### 블록 태그 (사용 중 제한)
```
Ability.Melee                 // 근접 공격 불가
Ability.Ranged                // 원거리 공격 불가
Ability.Skill                 // 스킬 사용 불가
Movement.Sprint               // 달리기 불가
```

### 아이템 타입 태그
```
Item.Recovery.ResonanceShard
Item.Recovery.FrozenTimeSnowflake
Item.Recovery.ThermalSpringFlask
Item.Recovery.LifeLuminescence
```

---

## 네트워크 멀티플레이

모든 시스템은 네트워크 리플리케이션을 지원합니다.

### 서버 권한
- 아이템 사용: 서버에서 검증
- 충전 횟수 변경: 서버에서만 가능
- 회복 효과 적용: 서버에서 적용 후 클라이언트에 리플리케이션

### 클라이언트 예측
- VFX/SFX: 즉시 재생 (예측)
- UI 업데이트: 리플리케이션 후 업데이트

---

## 성능 최적화

### 메모리
- 사용하지 않는 아이템은 등록하지 않기
- VFX/SFX 애셋은 필요 시 로드 (Lazy Loading)

### 네트워크
- 상태 변경 시에만 리플리케이션
- 배치 처리로 패킷 최적화

---

## 디버깅

### 콘솔 명령어

```
// 충전 횟수 확인
ShowDebug RecoveryItems

// 특정 아이템 충전
Cheat.RechargeItem ResonanceShard 5

// 모든 아이템 충전
Cheat.RechargeAllItems
```

### 로그 카테고리
```cpp
LogTemp                       // 일반 로그
LogHarmoniaRecovery          // 회복 시스템 로그
LogHarmoniaCheckpoint        // 체크포인트 연동 로그
```

---

## 확장 아이디어

### 1. 제작 시스템 연동
- 체크포인트에서 회복 아이템 제작
- 재료 수집 및 레시피 시스템

### 2. 희귀 아이템
- 보스 드랍 전용 회복 아이템
- 일회용 완전 회복 아이템

### 3. 아이템 조합
- 여러 공명 파편 조합으로 강력한 효과
- 시너지 시스템

### 4. 플레이어 선택
- 체크포인트에서 회복 vs 경험치 선택
- 트레이드오프 시스템

---

## 문제 해결

### Q: 아이템이 충전되지 않아요
A: `UHarmoniaCheckpointSubsystem`의 `OnResonanceCompleted` 이벤트가 제대로 바인딩되었는지 확인하세요. 컴포넌트의 `BeginPlay`에서 자동으로 바인딩됩니다.

### Q: 네트워크에서 충전 횟수가 동기화되지 않아요
A: `UHarmoniaRechargeableItemComponent`의 `SetIsReplicatedByDefault(true)`가 호출되었는지 확인하세요.

### Q: VFX가 재생되지 않아요
A: Niagara System 애셋이 올바르게 설정되었는지, 그리고 `UsageVFX` 프로퍼티에 할당되었는지 확인하세요.

---

## 라이선스

Copyright 2025 Snow Game Studio.

---

## 기여

버그 리포트나 기능 제안은 프로젝트 이슈 트래커에 등록해주세요.

**제작:** SnowWorld Development Team
**버전:** 1.0.0
**최종 수정:** 2025-11-18
