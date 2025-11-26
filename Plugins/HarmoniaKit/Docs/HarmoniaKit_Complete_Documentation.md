# HarmoniaKit 플러그인 - 통합 문서

**Version:** 1.0  
**Last Updated:** 2025-11-26  
**Unreal Engine:** 5.3+

---

## 📖 목차

1. [소개](#1-소개)
2. [빠른 시작](#2-빠른-시작)
3. [전투 시스템](#3-전투-시스템)
   - 3.6 [락온 타게팅 시스템](#36-락온-타게팅-시스템-lock-on-targeting)
   - 3.7 [회피 구르기 시스템](#37-회피-구르기-시스템-dodge-roll)
   - 3.8 [관련 GameplayTag 정의](#38-관련-gameplaytag-정의)
4. [캐릭터 시스템](#4-캐릭터-시스템)
5. [생활 컨텐츠 시스템](#5-생활-컨텐츠-시스템)
6. [온라인 및 팀 시스템](#6-온라인-및-팀-시스템)
7. [던전 및 월드 시스템](#7-던전-및-월드-시스템)
8. [퀘스트 시스템](#8-퀘스트-시스템)
9. [인벤토리 및 제작 시스템](#9-인벤토리-및-제작-시스템)
10. [저장 시스템](#10-저장-시스템)
11. [모드 시스템](#11-모드-시스템)
12. [개발 도구](#12-개발-도구)
13. [언리얼 엔진 통합](#13-언리얼-엔진-통합)
14. [리팩토링 히스토리](#14-리팩토링-히스토리)
15. [API 레퍼런스](#15-api-레퍼런스)
16. [데이터 드리븐 태그 시스템](#16-데이터-드리븐-태그-시스템)
17. [Gameplay Ability 태그 설정 가이드](#17-gameplay-ability-태그-설정-가이드)

---

## 1. 소개

### 1.1 HarmoniaKit이란?

HarmoniaKit은 Unreal Engine 5용 **고성능 멀티플레이어 RPG 플러그인**입니다. 소울라이크 전투 시스템부터 생활 컨텐츠까지 다양한 게임 시스템을 모듈식으로 제공합니다.

### 1.2 주요 특징

| 특징 | 설명 |
|------|------|
| ⚔️ **소울라이크 전투** | Sense System 기반 멀티스레드 히트 감지, 11종 무기 타입 |
| 🎮 **GAS 통합** | Gameplay Ability System 기반 어빌리티 프레임워크 |
| 🌍 **절차적 월드 생성** | 청크 기반 무한 세계 생성 |
| 🎣 **생활 컨텐츠** | 낚시, 채집, 요리, 농사, 음악 연주 시스템 |
| 🌐 **온라인 서브시스템** | Steam, EOS 통합, 세션/매치메이킹 |
| 🔧 **모드 시스템** | 사용자 정의 콘텐츠 지원 |
| 📜 **퀘스트 시스템** | 다단계 퀘스트, 동적 목표/보상 |
| 💾 **저장 시스템** | 로컬/클라우드 저장, 비동기 처리 |

### 1.3 설계 철학

1. **모듈화**: 각 시스템은 독립 컴포넌트로 필요한 것만 선택 사용
2. **확장성**: 데이터 주도 설계(Data-Driven), 블루프린트 친화적
3. **상호작용**: 시스템 간 크로스 시너지 (요리 ← 낚시/채집/농사, 음악 → 모든 시스템 버프)
4. **깊이 있는 게임플레이**: 미니게임, 전략적 선택, 수집/성장 요소

---

## 2. 빠른 시작

### 2.1 근접 전투 시스템 (5분 설정)

#### Step 1: 무기 데이터 정의
```cpp
// DataTable에 무기 정의
FHarmoniaMeleeWeaponData SwordData;
SwordData.WeaponType = EHarmoniaWeaponType::Sword;
SwordData.BaseDamage = 50.0f;
SwordData.AttackSpeed = 1.2f;
SwordData.StaminaCost = 15.0f;
```

#### Step 2: 캐릭터에 컴포넌트 추가
```cpp
UPROPERTY(VisibleAnywhere)
UHarmoniaMeleeCombatComponent* MeleeCombatComp;

// 생성자에서
MeleeCombatComp = CreateDefaultSubobject<UHarmoniaMeleeCombatComponent>(TEXT("MeleeCombatComp"));
```

#### Step 3: 입력 바인딩
```cpp
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction("LightAttack", IE_Pressed, MeleeCombatComp, &UHarmoniaMeleeCombatComponent::TryLightAttack);
    PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, MeleeCombatComp, &UHarmoniaMeleeCombatComponent::TryHeavyAttack);
    PlayerInputComponent->BindAction("Block", IE_Pressed, MeleeCombatComp, &UHarmoniaMeleeCombatComponent::StartBlock);
    PlayerInputComponent->BindAction("Dodge", IE_Pressed, MeleeCombatComp, &UHarmoniaMeleeCombatComponent::TryDodge);
}
```

### 2.2 회복 아이템 시스템

#### Step 1: 회복 아이템 데이터 정의
```cpp
FRechargeableItemData EstusData;
EstusData.ItemID = "Estus_Flask";
EstusData.MaxCharges = 5;
EstusData.HealAmount = 200.0f;
EstusData.HealDuration = 2.0f;
EstusData.UseCooldown = 1.0f;
```

#### Step 2: 컴포넌트 추가 및 사용
```cpp
UHarmoniaRechargeableItemComponent* RecoveryComp = Character->FindComponentByClass<UHarmoniaRechargeableItemComponent>();
RecoveryComp->AddItem(EstusData);
RecoveryComp->UseItem(0); // 슬롯 0번 사용
```

### 2.3 사망 패널티 시스템 (다크소울 스타일)

#### Step 1: 통화 관리자 설정
```cpp
UHarmoniaCurrencyManagerComponent* CurrencyComp = Character->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
CurrencyComp->AddCurrency("Souls", 1000);
```

#### Step 2: 사망 패널티 컴포넌트 설정
```cpp
UHarmoniaDeathPenaltyComponent* DeathPenaltyComp = Character->FindComponentByClass<UHarmoniaDeathPenaltyComponent>();
DeathPenaltyComp->SetDropPercentage(1.0f); // 100% 드랍
DeathPenaltyComp->SetRecoveryTimeLimit(300.0f); // 5분 내 회수
```

---

## 3. 전투 시스템

### 3.1 개요

HarmoniaKit의 전투 시스템은 **Gameplay Ability System(GAS)**과 **SenseSystem**을 기반으로 한 고성능 멀티스레드 히트 감지 시스템입니다.

### 3.2 핵심 컴포넌트

| 컴포넌트 | 역할 |
|---------|------|
| `UHarmoniaBaseCombatComponent` | 전투 시스템 기본 클래스 |
| `UHarmoniaMeleeCombatComponent` | 근접 전투 시스템 |
| `UHarmoniaSenseAttackComponent` | Sense 기반 히트 감지 |
| `UHarmoniaGameplayAbility_ComboAttack` | 콤보 어빌리티 |

### 3.3 근접 전투 시스템

#### 3.3.1 지원 무기 타입 (11종)

```cpp
enum class EHarmoniaWeaponType : uint8
{
    Sword,           // 검
    Greatsword,      // 대검
    Katana,          // 카타나
    Axe,             // 도끼
    Mace,            // 철퇴
    Spear,           // 창
    Halberd,         // 할버드
    Dagger,          // 단검 (이중 무기)
    Fists,           // 맨손
    Scythe,          // 낫
    Staff            // 지팡이
};
```

#### 3.3.2 전투 액션

| 액션 | 함수 | 설명 |
|------|------|------|
| 약공격 | `TryLightAttack()` | 빠른 공격, 낮은 데미지 |
| 강공격 | `TryHeavyAttack()` | 느린 공격, 높은 데미지 |
| 방어 | `StartBlock()` / `StopBlock()` | 데미지 감소 |
| 패리 | `TryParry()` | 정확한 타이밍에 공격 튕겨내기 |
| 회피 | `TryDodge()` | 무적 프레임 포함 회피 |
| 리포스트 | `TryRiposte()` | 패리 성공 후 반격 |
| 백스탭 | `TryBackstab()` | 후방 기습 공격 |

#### 3.3.3 콤보 시스템

DataTable 기반 콤보 정의:

```cpp
// FComboAttackData 구조
USTRUCT(BlueprintType)
struct FComboAttackData : public FTableRowBase
{
    UPROPERTY(EditAnywhere)
    FName ComboID;

    UPROPERTY(EditAnywhere)
    TArray<FComboAttackStep> Steps;

    UPROPERTY(EditAnywhere)
    float ComboWindowTime = 0.5f;
};

// FComboAttackStep 구조
USTRUCT(BlueprintType)
struct FComboAttackStep
{
    UPROPERTY(EditAnywhere)
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere)
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere)
    TArray<FComboAttackBranch> Branches; // 분기 공격
};
```

### 3.4 데미지 시스템

#### 3.4.1 데미지 타입

```cpp
enum class EHarmoniaDamageType : uint8
{
    Instant,        // 즉시 데미지
    DoT,            // 도트 데미지
    Explosion,      // 폭발 데미지 (범위)
    Percentage,     // 퍼센트 데미지
    TrueDamage      // 방어 무시
};
```

#### 3.4.2 Attribute Set

```cpp
// UHarmoniaAttributeSet의 주요 속성
UPROPERTY() FGameplayAttributeData Health;
UPROPERTY() FGameplayAttributeData MaxHealth;
UPROPERTY() FGameplayAttributeData Stamina;
UPROPERTY() FGameplayAttributeData MaxStamina;
UPROPERTY() FGameplayAttributeData Mana;
UPROPERTY() FGameplayAttributeData MaxMana;
UPROPERTY() FGameplayAttributeData Poise;         // 강인함 (경직 저항)
UPROPERTY() FGameplayAttributeData AttackPower;
UPROPERTY() FGameplayAttributeData DefensePower;
```

### 3.5 보스 전투 설정

#### 3.5.1 보스 몬스터 클래스

```cpp
UCLASS()
class AHarmoniaBossMonster : public AHarmoniaMonsterBase
{
    // Phase 시스템
    UPROPERTY(EditDefaultsOnly)
    TArray<FBossPhaseData> Phases;

    // 현재 Phase
    UPROPERTY(ReplicatedUsing=OnRep_CurrentPhase)
    int32 CurrentPhaseIndex;

    // Phase 전환
    UFUNCTION()
    void OnHealthThresholdReached(float HealthPercent);
};
```

#### 3.5.2 Boss Phase 데이터

```cpp
USTRUCT(BlueprintType)
struct FBossPhaseData
{
    UPROPERTY(EditAnywhere)
    float HealthThreshold; // 이 체력 이하에서 활성화

    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<UGameplayAbility>> PhaseAbilities;

    UPROPERTY(EditAnywhere)
    float DamageMultiplier;

    UPROPERTY(EditAnywhere)
    UAnimMontage* PhaseTransitionMontage;
};
```

### 3.6 락온 타게팅 시스템 (Lock-On Targeting)

소울라이크 전투 시스템의 핵심 기능인 락온 타게팅 시스템입니다.

#### 3.6.1 주요 기능

| 기능 | 설명 |
|------|------|
| **자동 타겟 찾기** | 카메라 중심에서 가장 가까운 적을 자동으로 찾아 락온 |
| **타겟 전환** | 좌우로 타겟을 전환할 수 있는 기능 |
| **자동 카메라 회전** | 락온된 적을 향해 카메라가 자동으로 회전 |
| **거리 기반 해제** | 적이 너무 멀어지면 자동으로 락온 해제 |
| **Sense System 통합** | 장애물에 가려진 적은 타겟팅 불가 |

#### 3.6.2 핵심 클래스

```cpp
// 락온 컴포넌트 사용
UPROPERTY(VisibleAnywhere)
UHarmoniaLockOnComponent* LockOnComp;

// 생성자에서
LockOnComp = CreateDefaultSubobject<UHarmoniaLockOnComponent>(TEXT("LockOnComp"));

// 입력 바인딩
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AMyCharacter::ToggleLockOn);
    PlayerInputComponent->BindAction("SwitchTargetLeft", IE_Pressed, LockOnComp, &UHarmoniaLockOnComponent::SwitchTargetLeft);
    PlayerInputComponent->BindAction("SwitchTargetRight", IE_Pressed, LockOnComp, &UHarmoniaLockOnComponent::SwitchTargetRight);
}

void AMyCharacter::ToggleLockOn()
{
    LockOnComp->ToggleLockOn();
}
```

#### 3.6.3 설정 가능한 파라미터

| 파라미터 | 타입 | 기본값 | 설명 |
|---------|------|--------|------|
| `MaxLockOnDistance` | float | 1500.0 | 락온 가능한 최대 거리 |
| `MaxLockOnAngle` | float | 60.0 | 초기 락온 가능한 최대 각도 (도) |
| `LockOnBreakDistance` | float | 2000.0 | 락온 자동 해제 거리 |
| `CameraRotationSpeed` | float | 10.0 | 카메라 회전 속도 |
| `TargetHeightOffset` | float | 100.0 | 타겟 높이 오프셋 |
| `TargetSocketName` | FName | None | 정밀 락온용 소켓 이름 |
| `bUseSenseSystem` | bool | true | Sense System 사용 여부 |
| `TargetableTag` | FGameplayTag | - | 타겟팅 가능 액터 태그 |
| `bBreakLockOnTargetDeath` | bool | true | 타겟 사망 시 락온 해제 |
| `bSmoothCameraRotation` | bool | true | 부드러운 카메라 회전 |

#### 3.6.4 이벤트 델리게이트

```cpp
// 타겟 변경 이벤트
UPROPERTY(BlueprintAssignable)
FOnLockOnTargetChangedDelegate OnLockOnTargetChanged;

// 락온 상태 변경 이벤트
UPROPERTY(BlueprintAssignable)
FOnLockOnStateChangedDelegate OnLockOnStateChanged;

// 사용 예
LockOnComp->OnLockOnTargetChanged.AddDynamic(this, &AMyCharacter::OnLockOnTargetChanged);
LockOnComp->OnLockOnStateChanged.AddDynamic(this, &AMyCharacter::OnLockOnStateChanged);

void AMyCharacter::OnLockOnTargetChanged(AActor* OldTarget, AActor* NewTarget)
{
    // UI 업데이트 등
}

void AMyCharacter::OnLockOnStateChanged(bool bIsLockedOn)
{
    // 카메라 모드 전환 등
}
```

#### 3.6.5 블루프린트에서 사용

```cpp
// 락온 컴포넌트 접근
UHarmoniaLockOnComponent* LockOn = Character->FindComponentByClass<UHarmoniaLockOnComponent>();

// 락온 상태 확인
bool bIsLocked = LockOn->IsLockedOn();

// 현재 타겟 가져오기
AActor* Target = LockOn->GetCurrentTarget();

// 타겟까지의 거리
float Distance = LockOn->GetDistanceToTarget();

// 락온 토글
LockOn->ToggleLockOn();

// 타겟 전환
LockOn->SwitchTargetLeft();
LockOn->SwitchTargetRight();

// 수동 타겟 설정
LockOn->SetLockOnTarget(SpecificEnemy);
```

### 3.7 회피 구르기 시스템 (Dodge Roll)

타이밍 기반 회피 메커니즘과 무적 프레임(I-Frame)이 구현된 회피 시스템입니다.

#### 3.7.1 주요 기능

| 기능 | 설명 |
|------|------|
| **방향 기반 회피** | 입력 방향으로 구르기 (입력 없으면 후방) |
| **무적 프레임 (I-frames)** | 회피 중 일정 시간 동안 무적 상태 |
| **장비 무게 시스템** | 장비 무게에 따라 롤 타입 변경 (Light/Medium/Heavy) |
| **스태미나 소모** | 롤 타입별 스태미나 소모량 차이 |
| **네트워크 지원** | 멀티플레이어 환경에서 작동 |

#### 3.7.2 롤 타입별 파라미터

| 롤 타입 | 장비 무게 | 거리 | 지속시간 | I-Frame 시작 | I-Frame 지속 | 스태미나 |
|--------|----------|------|----------|--------------|--------------|----------|
| **Light** | 0-30% | 500 | 0.5초 | 0.1초 | 0.4초 | 15 |
| **Medium** | 30-70% | 400 | 0.6초 | 0.1초 | 0.3초 | 20 |
| **Heavy** | 70%+ | 300 | 0.8초 | 0.15초 | 0.2초 | 30 |

#### 3.7.3 핵심 클래스

```cpp
// EDodgeRollType 열거형
enum class EDodgeRollType : uint8
{
    Light,   // 0-30% 장비 무게
    Medium,  // 30-70% 장비 무게
    Heavy    // 70%+ 장비 무게
};

// UHarmoniaGameplayAbility_Dodge 주요 속성
UPROPERTY(EditDefaultsOnly)
TObjectPtr<UAnimMontage> DodgeMontage;

// Light Roll 설정
UPROPERTY(EditDefaultsOnly, Category = "Dodge|Light Roll")
float LightRollDistance = 500.0f;
float LightRollDuration = 0.5f;
float LightRollIFrameStartTime = 0.1f;
float LightRollIFrameDuration = 0.4f;
float LightRollStaminaCost = 15.0f;

// Medium Roll 설정
UPROPERTY(EditDefaultsOnly, Category = "Dodge|Medium Roll")
float MediumRollDistance = 400.0f;
float MediumRollDuration = 0.6f;
float MediumRollIFrameStartTime = 0.1f;
float MediumRollIFrameDuration = 0.3f;
float MediumRollStaminaCost = 20.0f;

// Heavy Roll 설정
UPROPERTY(EditDefaultsOnly, Category = "Dodge|Heavy Roll")
float HeavyRollDistance = 300.0f;
float HeavyRollDuration = 0.8f;
float HeavyRollIFrameStartTime = 0.15f;
float HeavyRollIFrameDuration = 0.2f;
float HeavyRollStaminaCost = 30.0f;
```

#### 3.7.4 Gameplay Ability 블루프린트 생성

1. **블루프린트 생성**: `UHarmoniaGameplayAbility_Dodge`를 부모로 하는 블루프린트 생성
2. **애니메이션 설정**: `DodgeMontage`에 회피 애니메이션 할당
3. **파라미터 조정**: 롤 타입별 거리/지속시간/I-Frame 설정
4. **태그 설정**: 아래 태그 설정 참고

#### 3.7.5 회피 어빌리티 태그 설정

```
AbilityTags:
  - Ability.Dodge
  - Ability.Combat.Dodge

ActivationOwnedTags:
  - State.Dodging

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking

Related Tags (I-Frame 중 자동 적용):
  - State.Invincible
```

#### 3.7.6 캐릭터에 Ability 부여

Experience 또는 AbilitySet에서 Dodge Ability를 캐릭터에 부여:

1. 해당 Experience/AbilitySet 열기
2. Abilities 배열에 GA_Dodge 블루프린트 추가
3. Input Tag를 `InputTag.Dodge`로 설정

### 3.8 관련 GameplayTag 정의

Lock-On과 Dodge 시스템에서 사용하는 GameplayTag들입니다.

#### 3.8.1 Lock-On 시스템 태그

| 태그 | 설명 |
|------|------|
| `Ability.LockOn` | 락온 어빌리티 |
| `InputTag.LockOn` | 락온 입력 태그 |
| `InputTag.SwitchTargetLeft` | 왼쪽 타겟 전환 |
| `InputTag.SwitchTargetRight` | 오른쪽 타겟 전환 |
| `State.LockOn.Active` | 락온 활성 상태 |
| `State.LockOn.HasTarget` | 타겟 존재 상태 |
| `Status.LockedOn` | 락온 상태 (레거시) |
| `Character.Type.Enemy` | 적 캐릭터 |
| `Character.Type.Ally` | 아군 캐릭터 |
| `Character.Type.Neutral` | 중립 캐릭터 |

#### 3.8.2 Dodge 시스템 태그

| 태그 | 설명 |
|------|------|
| `Ability.Dodge` | 회피 어빌리티 |
| `InputTag.Dodge` | 회피 입력 태그 |
| `State.Dodging` | 회피 중 상태 |
| `State.Invincible` | 무적 상태 |
| `Character.State.Dodging` | 캐릭터 회피 상태 |
| `Character.State.Attacking` | 캐릭터 공격 상태 |
| `Character.State.Blocking` | 캐릭터 방어 상태 |

---

## 4. 캐릭터 시스템

### 4.1 스태미나 시스템

#### 4.1.1 행동별 스태미나 소비

| 행동 | 기본 소비량 |
|------|------------|
| 달리기 | 10/초 |
| 약공격 | 15 |
| 강공격 | 25 |
| 방어 (히트 시) | 20 |
| 구르기 | 20 |
| 점프 | 10 |

#### 4.1.2 스태미나 회복

```cpp
// 자동 회복 설정
UPROPERTY(EditAnywhere)
float StaminaRegenRate = 30.0f; // 초당 회복량

UPROPERTY(EditAnywhere)
float StaminaRegenDelay = 1.5f; // 소비 후 회복 시작 딜레이
```

### 4.2 회복 아이템 시스템 (에스투스 스타일)

#### 4.2.1 핵심 클래스

```cpp
// 충전식 회복 아이템 컴포넌트
UCLASS()
class UHarmoniaRechargeableItemComponent : public UActorComponent
{
    UFUNCTION(BlueprintCallable)
    bool UseItem(int32 SlotIndex);

    UFUNCTION(BlueprintCallable)
    void RechargeItem(int32 SlotIndex, int32 Charges);

    UFUNCTION(BlueprintCallable)
    void RechargeAllItems(); // 체크포인트에서 호출
};
```

#### 4.2.2 Resonance Shard (충전 아이템)

```cpp
// 체크포인트 또는 특정 조건에서 회복 아이템 충전
void ACheckpointActor::OnPlayerOverlap(ACharacter* Player)
{
    UHarmoniaRechargeableItemComponent* RecoveryComp = 
        Player->FindComponentByClass<UHarmoniaRechargeableItemComponent>();
    
    if (RecoveryComp)
    {
        RecoveryComp->RechargeAllItems();
    }
}
```

### 4.3 사망 패널티 시스템

#### 4.3.1 핵심 흐름

```
[플레이어 사망]
       ↓
[통화(Souls) 드랍]
       ↓
[Memory Echo(혈흔) 스폰]
       ↓
[체크포인트에서 리스폰]
       ↓
[시간 내 회수 시도]
       ↓
[성공: 통화 회수] / [실패: 영구 손실]
```

#### 4.3.2 Memory Echo 액터

```cpp
UCLASS()
class AHarmoniaMemoryEchoActor : public AActor
{
    // 저장된 통화
    UPROPERTY(Replicated)
    TMap<FName, int32> StoredCurrencies;

    // 회수 가능 시간
    UPROPERTY(Replicated)
    float RemainingTime;

    // 상호작용으로 회수
    UFUNCTION()
    void OnInteract(ACharacter* Interactor);
};
```

---

## 5. 생활 컨텐츠 시스템

### 5.1 낚시 시스템

#### 5.1.1 핵심 컴포넌트

```cpp
UCLASS()
class UHarmoniaFishingComponent : public UActorComponent
{
    UFUNCTION(BlueprintCallable)
    void StartFishing(const FFishingSpotData& SpotData);

    UFUNCTION(BlueprintCallable)
    void CastLine();

    UFUNCTION(BlueprintCallable)
    void ReelIn();

    // 미니게임 입력
    UFUNCTION(BlueprintCallable)
    void ProcessMinigameInput(EFishingInputType InputType);
};
```

#### 5.1.2 물고기 희귀도

| 희귀도 | 설명 |
|--------|------|
| Common | 일반 |
| Uncommon | 고급 |
| Rare | 희귀 |
| Epic | 영웅 |
| Legendary | 전설 |

#### 5.1.3 미니게임 타입

```cpp
enum class EFishingMinigameType : uint8
{
    Timing,        // 타이밍 맞추기
    BarBalance,    // 바 균형 유지
    QTE,           // Quick Time Event
    ReelTension    // 릴 장력 관리
};
```

### 5.2 채집 시스템

#### 5.2.1 자원 타입

```cpp
enum class EGatheringResourceType : uint8
{
    Mineral,    // 광물
    Herb,       // 약초
    Wood,       // 나무
    Fiber,      // 섬유
    Stone,      // 돌
    Crystal,    // 크리스탈
    Flower,     // 꽃
    Mushroom    // 버섯
};
```

#### 5.2.2 도구 시스템

```cpp
// 도구 데이터
USTRUCT(BlueprintType)
struct FGatheringToolData
{
    UPROPERTY(EditAnywhere)
    EGatheringToolType ToolType; // 곡괭이, 도끼, 낫, 망치

    UPROPERTY(EditAnywhere)
    int32 ToolTier; // 도구 등급 (1-5)

    UPROPERTY(EditAnywhere)
    float Durability;

    UPROPERTY(EditAnywhere)
    float GatheringSpeed;
};
```

#### 5.2.3 사용 예시

```cpp
UHarmoniaGatheringComponent* GatherComp = Character->FindComponentByClass<UHarmoniaGatheringComponent>();
GatherComp->EquipTool(PickaxeData);
GatherComp->StartGathering(MineralResourceID);
// 채집 완료 시 OnGatheringComplete 델리게이트 발생
```

### 5.3 요리 시스템

#### 5.3.1 조리 방법

```cpp
enum class ECookingMethod : uint8
{
    Boiling,    // 끓이기
    Frying,     // 굽기
    Steaming,   // 찌기
    Baking,     // 베이킹
    Roasting,   // 로스팅
    Mixing      // 혼합
};
```

#### 5.3.2 요리 품질 및 버프

```cpp
enum class ECookingQuality : uint8
{
    Failed,       // 실패작 - 디버프 가능
    Poor,         // 저급
    Normal,       // 일반
    Good,         // 우수
    Excellent,    // 최우수
    Masterpiece   // 걸작 - 추가 버프
};

// 버프 효과 예시
USTRUCT(BlueprintType)
struct FCookingBuffEffect
{
    UPROPERTY(EditAnywhere)
    EBuffType BuffType; // 체력회복, 마나회복, 공격력, 방어력, 속도 등

    UPROPERTY(EditAnywhere)
    float Value;

    UPROPERTY(EditAnywhere)
    float Duration;
};
```

#### 5.3.3 사용 예시

```cpp
UHarmoniaCookingComponent* CookingComp = Character->FindComponentByClass<UHarmoniaCookingComponent>();
CookingComp->DiscoverRecipe(RecipeID);
CookingComp->StartCooking(RecipeID);
// 요리 완료 후
CookingComp->ConsumeFood(FoodID, Quality);
```

### 5.4 농사 시스템

#### 5.4.1 작물 성장 단계

```cpp
enum class ECropGrowthStage : uint8
{
    Seed,       // 씨앗
    Sprout,     // 싹
    Growing,    // 성장 중
    Mature,     // 성숙
    Harvest,    // 수확 가능
    Withered    // 시듦
};
```

#### 5.4.2 농사 시스템 흐름

```
[밭 생성] → [씨앗 심기] → [물주기] → [비료 주기(선택)]
                              ↓
[성장 (시간 경과)] → [수확] → [재수확 가능 작물은 반복]
```

#### 5.4.3 비료 효과

```cpp
USTRUCT(BlueprintType)
struct FFertilizerData
{
    UPROPERTY(EditAnywhere)
    float GrowthSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere)
    float YieldMultiplier = 1.0f;

    UPROPERTY(EditAnywhere)
    float QualityBonus = 0.0f;
};
```

### 5.5 음악/연주 시스템

#### 5.5.1 악기 타입

```cpp
enum class EInstrumentType : uint8
{
    Lute,       // 류트
    Flute,      // 플루트
    Harp,       // 하프
    Drum,       // 드럼
    Horn,       // 호른
    Violin,     // 바이올린
    Piano,      // 피아노
    Guitar      // 기타
};
```

#### 5.5.2 리듬 미니게임

```cpp
// 노트 입력 처리
UFUNCTION(BlueprintCallable)
void UHarmoniaMusicComponent::HitRhythmNote(int32 NoteIndex, float InputTime, bool bPerfectHit)
{
    // 타이밍에 따른 점수 계산
    float TimingAccuracy = CalculateTimingAccuracy(NoteIndex, InputTime);
    
    if (bPerfectHit || TimingAccuracy > 0.95f)
    {
        // Perfect!
        AddPerformanceScore(100);
    }
    else if (TimingAccuracy > 0.8f)
    {
        // Great!
        AddPerformanceScore(75);
    }
    // ...
}
```

#### 5.5.3 범위 버프 적용

```cpp
// 연주 품질에 따른 범위 버프
void UHarmoniaMusicComponent::ApplyPerformanceBuff()
{
    EPerformanceQuality Quality = CalculatePerformanceQuality();
    float BuffRange = GetBuffRange(Quality);
    
    TArray<AActor*> NearbyActors;
    UKismetSystemLibrary::SphereOverlapActors(
        this, GetOwner()->GetActorLocation(), BuffRange, ...);
    
    for (AActor* Actor : NearbyActors)
    {
        if (IsFriendly(Actor))
        {
            ApplyBuffToActor(Actor, CurrentMusicBuffs);
        }
    }
}
```

---

## 6. 온라인 및 팀 시스템

### 6.1 온라인 서브시스템

#### 6.1.1 지원 플랫폼

| 플랫폼 | 기능 |
|--------|------|
| Steam | 친구, 매치메이킹, 업적, 클라우드 저장 |
| Epic Online Services (EOS) | 크로스 플랫폼, 세션 관리 |

#### 6.1.2 핵심 기능

```cpp
UCLASS()
class UHarmoniaOnlineSubsystem : public UGameInstanceSubsystem
{
    // 세션 관리
    UFUNCTION(BlueprintCallable)
    void CreateSession(const FSessionSettings& Settings);

    UFUNCTION(BlueprintCallable)
    void JoinSession(const FOnlineSessionSearchResult& SearchResult);

    // 친구 관리
    UFUNCTION(BlueprintCallable)
    TArray<FHarmoniaFriendInfo> GetFriendsList();

    // 초대
    UFUNCTION(BlueprintCallable)
    void InviteFriend(const FUniqueNetId& FriendId);
};
```

### 6.2 팀 시스템

#### 6.2.1 핵심 특징

- **무제한 팀 생성**: 런타임에 동적으로 팀 추가
- **유동적 관계 관리**: 게임 중 동맹↔적대 관계 변경
- **언리얼 표준 통합**: `IGenericTeamAgentInterface` 완벽 지원

#### 6.2.2 팀 생성

```cpp
UHarmoniaTeamManagementSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UHarmoniaTeamManagementSubsystem>();

// 팀 생성
FHarmoniaTeamIdentification GoblinTeam = TeamSubsystem->CreateFaction(
    FText::FromString("Goblin Tribe"),
    EHarmoniaTeamAttitude::Hostile,
    FLinearColor::Red
);

FHarmoniaTeamIdentification PlayerTeam = TeamSubsystem->CreateFaction(
    FText::FromString("Heroes"),
    EHarmoniaTeamAttitude::Defensive,
    FLinearColor::Blue
);
```

#### 6.2.3 관계 설정

```cpp
// 동맹 설정
TeamSubsystem->MakeTeamsAllies(Team1, Team2);

// 적대 설정
TeamSubsystem->MakeTeamsEnemies(Team1, Team2);

// 런타임 관계 변경 (퀘스트 완료 시)
TeamSubsystem->ChangeFactionRelationship(
    GoblinTeam,
    PlayerTeam,
    EHarmoniaTeamRelationship::Ally,
    true  // 양방향
);
```

#### 6.2.4 피아식별 쿼리

```cpp
// 두 액터가 적인지 확인
bool bAreEnemies = TeamSubsystem->AreEnemies(ActorA, ActorB);

// 공격 가능 여부
bool bCanAttack = TeamSubsystem->CanActorAttack(Source, Target);
```

---

## 7. 던전 및 월드 시스템

### 7.1 던전 레이드 시스템

#### 7.1.1 핵심 클래스

```cpp
UCLASS()
class UHarmoniaDungeonComponent : public UActorComponent
{
    // 던전 입장
    UFUNCTION(BlueprintCallable)
    bool TryEnterDungeon(const FHarmoniaDungeonData& DungeonData);

    // 던전 완료
    UFUNCTION(BlueprintCallable)
    void CompleteDungeon();

    // 던전 퇴장
    UFUNCTION(BlueprintCallable)
    void ExitDungeon();
};
```

#### 7.1.2 던전 데이터

```cpp
USTRUCT(BlueprintType)
struct FHarmoniaDungeonData : public FTableRowBase
{
    UPROPERTY(EditAnywhere)
    FName DungeonID;

    UPROPERTY(EditAnywhere)
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere)
    int32 MaxPlayers;

    UPROPERTY(EditAnywhere)
    TArray<FDungeonStageData> Stages;

    UPROPERTY(EditAnywhere)
    FDungeonRewardData Rewards;
};
```

### 7.2 월드 생성 시스템

#### 7.2.1 절차적 생성

```cpp
UCLASS()
class UHarmoniaWorldGeneratorSubsystem : public UWorldSubsystem
{
    // 청크 생성
    UFUNCTION(BlueprintCallable)
    void GenerateChunk(FIntVector ChunkCoord);

    // 시드 설정
    UFUNCTION(BlueprintCallable)
    void SetWorldSeed(int32 Seed);

    // 바이옴 조회
    UFUNCTION(BlueprintCallable)
    EBiomeType GetBiomeAt(FVector WorldLocation);
};
```

#### 7.2.2 바이옴 시스템

```cpp
enum class EBiomeType : uint8
{
    Plains,     // 평원
    Forest,     // 숲
    Desert,     // 사막
    Snow,       // 눈
    Swamp,      // 늪
    Mountain,   // 산
    Volcanic    // 화산
};
```

---

## 8. 퀘스트 시스템

### 8.1 핵심 컴포넌트

```cpp
UCLASS()
class UHarmoniaQuestComponent : public UActorComponent
{
    // 퀘스트 수락
    UFUNCTION(BlueprintCallable)
    bool AcceptQuest(FName QuestID);

    // 퀘스트 진행 업데이트
    UFUNCTION(BlueprintCallable)
    void UpdateQuestProgress(FName QuestID, FName ObjectiveID, int32 Progress);

    // 퀘스트 완료
    UFUNCTION(BlueprintCallable)
    bool CompleteQuest(FName QuestID);
};
```

### 8.2 퀘스트 데이터 구조

```cpp
USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
    UPROPERTY(EditAnywhere)
    FName QuestID;

    UPROPERTY(EditAnywhere)
    FText QuestName;

    UPROPERTY(EditAnywhere)
    FText Description;

    UPROPERTY(EditAnywhere)
    TArray<FQuestPhase> Phases;

    UPROPERTY(EditAnywhere)
    TArray<FQuestCondition> Prerequisites;

    UPROPERTY(EditAnywhere)
    FQuestReward Rewards;
};
```

### 8.3 다단계 퀘스트 (Phase)

```cpp
USTRUCT(BlueprintType)
struct FQuestPhase
{
    UPROPERTY(EditAnywhere)
    int32 PhaseIndex;

    UPROPERTY(EditAnywhere)
    FText PhaseDescription;

    UPROPERTY(EditAnywhere)
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere)
    bool bAllObjectivesRequired; // AND 또는 OR 조건
};
```

---

## 9. 인벤토리 및 제작 시스템

### 9.1 제작 시스템

#### 9.1.1 핵심 컴포넌트

```cpp
UCLASS()
class UHarmoniaCraftingComponent : public UActorComponent
{
    // 레시피 목록 조회
    UFUNCTION(BlueprintCallable)
    TArray<FCraftingRecipeData> GetAvailableRecipes();

    // 제작 시작
    UFUNCTION(BlueprintCallable)
    bool StartCrafting(FName RecipeID);

    // 제작 취소
    UFUNCTION(BlueprintCallable)
    void CancelCrafting();
};
```

#### 9.1.2 레시피 데이터

```cpp
USTRUCT(BlueprintType)
struct FCraftingRecipeData : public FTableRowBase
{
    UPROPERTY(EditAnywhere)
    FName RecipeID;

    UPROPERTY(EditAnywhere)
    TArray<FCraftingIngredient> Ingredients;

    UPROPERTY(EditAnywhere)
    FCraftingOutput Output;

    UPROPERTY(EditAnywhere)
    float CraftingTime;

    UPROPERTY(EditAnywhere)
    int32 RequiredCraftingLevel;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AHarmoniaCraftingStation> RequiredStation;
};
```

### 9.2 Lyra 인벤토리 통합

#### 9.2.1 커스텀 인벤토리 Fragment

```cpp
// 내구도 Fragment
UCLASS()
class UHarmoniaInventoryFragment_Durability : public ULyraInventoryItemFragment
{
    UPROPERTY(EditAnywhere)
    float MaxDurability = 100.0f;

    UPROPERTY()
    float CurrentDurability;
};

// 스탯 수정자 Fragment
UCLASS()
class UHarmoniaInventoryFragment_StatModifiers : public ULyraInventoryItemFragment
{
    UPROPERTY(EditAnywhere)
    TArray<FHarmoniaStatModifier> Modifiers;
};
```

---

## 10. 저장 시스템

### 10.1 핵심 서브시스템

```cpp
UCLASS()
class UHarmoniaSaveGameSubsystem : public UGameInstanceSubsystem
{
    // 저장
    UFUNCTION(BlueprintCallable)
    void SaveGame(int32 SlotIndex, FOnSaveCompleted OnCompleted);

    // 불러오기
    UFUNCTION(BlueprintCallable)
    void LoadGame(int32 SlotIndex, FOnLoadCompleted OnCompleted);

    // 슬롯 정보
    UFUNCTION(BlueprintCallable)
    TArray<FSaveSlotInfo> GetSaveSlots();

    // 클라우드 저장 (Steam)
    UFUNCTION(BlueprintCallable)
    void SyncToCloud();
};
```

### 10.2 Saveable 인터페이스

```cpp
UINTERFACE()
class UHarmoniaSaveableInterface : public UInterface
{
    GENERATED_BODY()
};

class IHarmoniaSaveableInterface
{
    GENERATED_BODY()

public:
    // 저장 데이터 수집
    virtual void CollectSaveData(FHarmoniaSaveData& OutData) = 0;

    // 저장 데이터 적용
    virtual void ApplySaveData(const FHarmoniaSaveData& InData) = 0;
};
```

---

## 11. 모드 시스템

### 11.1 핵심 서브시스템

```cpp
UCLASS()
class UHarmoniaModSubsystem : public UGameInstanceSubsystem
{
    // 모드 로드
    UFUNCTION(BlueprintCallable)
    bool LoadMod(const FString& ModPath);

    // 모드 언로드
    UFUNCTION(BlueprintCallable)
    void UnloadMod(const FString& ModID);

    // 활성 모드 목록
    UFUNCTION(BlueprintCallable)
    TArray<FHarmoniaModInfo> GetActiveMods();
};
```

### 11.2 에셋 오버라이드

```cpp
UCLASS()
class UHarmoniaAssetOverrideSubsystem : public UGameInstanceSubsystem
{
    // 에셋 오버라이드 등록
    UFUNCTION(BlueprintCallable)
    void RegisterAssetOverride(const FSoftObjectPath& OriginalAsset, const FSoftObjectPath& OverrideAsset);
};
```

### 11.3 DataTable 패칭

```cpp
UCLASS()
class UHarmoniaDataTablePatcher : public UObject
{
    // DataTable에 행 추가
    UFUNCTION(BlueprintCallable)
    void AddRowToDataTable(UDataTable* DataTable, FName RowName, const FTableRowBase& RowData);

    // DataTable 행 수정
    UFUNCTION(BlueprintCallable)
    void ModifyDataTableRow(UDataTable* DataTable, FName RowName, const FTableRowBase& NewRowData);
};
```

---

## 12. 개발 도구

### 12.1 치트 시스템

> ⚠️ **주의**: Editor/Development 빌드에서만 사용 가능. Shipping 빌드에서는 자동 비활성화됨.

#### 12.1.1 콘솔 명령어

| 명령어 | 설명 |
|--------|------|
| `Harmonia.God` | 무적 모드 |
| `Harmonia.Ghost` | 노클립 모드 |
| `Harmonia.GiveItem <ItemID> <Count>` | 아이템 지급 |
| `Harmonia.SetLevel <Level>` | 레벨 설정 |
| `Harmonia.AddCurrency <Type> <Amount>` | 통화 추가 |
| `Harmonia.TeleportTo <X> <Y> <Z>` | 텔레포트 |
| `Harmonia.SpawnMonster <MonsterID>` | 몬스터 스폰 |
| `Harmonia.CompleteQuest <QuestID>` | 퀘스트 완료 |

#### 12.1.2 치트 라이브러리 (블루프린트)

```cpp
UCLASS()
class UHarmoniaCheatLibrary : public UBlueprintFunctionLibrary
{
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheats")
    static void SetGodMode(ACharacter* Character, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheats")
    static void GiveAllItems(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheats")
    static void UnlockAllRecipes(ACharacter* Character);
};
```

### 12.2 애니메이션 이펙트 시스템

#### 12.2.1 GameplayTag 기반 VFX/SFX 관리

```cpp
// DataTable에 이펙트 정의
USTRUCT(BlueprintType)
struct FAnimationEffectEntry : public FTableRowBase
{
    UPROPERTY(EditAnywhere)
    FGameplayTag EffectTag;

    UPROPERTY(EditAnywhere)
    UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere)
    USoundBase* SoundEffect;

    UPROPERTY(EditAnywhere)
    FTransform SpawnOffset;
};
```

#### 12.2.2 AnimNotify로 이펙트 재생

```cpp
// AnimNotify_PlayTaggedEffect
// 애니메이션 몽타주에서 특정 프레임에 이펙트 재생
UCLASS()
class UAnimNotify_PlayTaggedEffect : public UAnimNotify
{
    UPROPERTY(EditAnywhere)
    FGameplayTag EffectTag;

    UPROPERTY(EditAnywhere)
    FName SocketName;
};
```

---

## 13. 언리얼 엔진 통합

### 13.1 AI 시스템 통합

#### 13.1.1 Blackboard 헬퍼

```cpp
// UHarmoniaBaseAIComponent에 추가된 헬퍼
UBlackboardComponent* GetBlackboardComponent() const;
void SetBlackboardValueAsObject(FName KeyName, UObject* ObjectValue);
UObject* GetBlackboardValueAsObject(FName KeyName) const;
void SetBlackboardValueAsVector(FName KeyName, FVector VectorValue);
void SetBlackboardValueAsBool(FName KeyName, bool BoolValue);
void SetBlackboardValueAsFloat(FName KeyName, float FloatValue);
```

#### 13.1.2 Behavior Tree Task/Service

```cpp
// BT Task로 AI 컴포넌트 활성화/비활성화
UCLASS()
class UBTTask_SetAIComponentEnabled : public UBTTaskNode
{
    UPROPERTY(EditAnywhere)
    TSubclassOf<UHarmoniaBaseAIComponent> ComponentClass;

    UPROPERTY(EditAnywhere)
    bool bEnable = true;
};

// BT Service로 AI 상태를 Blackboard에 자동 동기화
UCLASS()
class UBTService_SyncAIComponentToBlackboard : public UBTService
{
    UPROPERTY(EditAnywhere)
    TSubclassOf<UHarmoniaBaseAIComponent> ComponentClass;

    UPROPERTY(EditAnywhere)
    FBlackboardKeySelector IsEnabledKey;
};
```

### 13.2 Lyra 호환성

#### 13.2.1 Equipment 어댑터

```cpp
UCLASS()
class UHarmoniaLyraEquipmentAdapter : public UObject
{
    // Lyra Equipment를 Harmonia 시스템에 연결
    UFUNCTION(BlueprintCallable)
    void SyncLyraEquipment(ULyraEquipmentManagerComponent* EquipmentManager);

    // Harmonia 무기 데이터를 Lyra 형식으로 변환
    UFUNCTION(BlueprintCallable)
    ULyraInventoryItemDefinition* ConvertToLyraItem(const FHarmoniaMeleeWeaponData& WeaponData);
};
```

---

## 14. 리팩토링 히스토리

### 14.1 2025-11-22: Combat System 리팩토링

**목적**: 전투 시스템 코드 중복 제거 및 유지보수성 향상

**주요 작업**:
1. `UHarmoniaBaseCombatComponent` 생성 (공통 코드 추출)
2. `HarmoniaMeleeCombatDefinitions.h` 분리 (Definition 파일 크기 축소)
3. `UHarmoniaCombatLibrary` 생성 (유틸리티 함수 중앙화)

**성과**:
- 코드 중복: 400줄+ → 200줄
- 파일 크기: 43KB → 분할된 작은 파일들

### 14.2 2025-11-22: 치트 시스템 빌드 제한

**목적**: Shipping 빌드에서 치트 기능 완전 제거

**주요 작업**:
- `#if !UE_BUILD_SHIPPING` 전처리기 적용
- 조건부 컴파일로 치트 코드 물리적 제거

**대상 파일**:
- `HarmoniaCheatManager.h/cpp`
- `HarmoniaCheatLibrary.h/cpp`

---

## 15. API 레퍼런스

### 15.1 주요 컴포넌트 목록

| 카테고리 | 컴포넌트 | 헤더 파일 |
|----------|---------|----------|
| 전투 | `UHarmoniaBaseCombatComponent` | `HarmoniaBaseCombatComponent.h` |
| 전투 | `UHarmoniaMeleeCombatComponent` | `HarmoniaMeleeCombatComponent.h` |
| 전투 | `UHarmoniaSenseAttackComponent` | `HarmoniaSenseAttackComponent.h` |
| 캐릭터 | `UHarmoniaRechargeableItemComponent` | `HarmoniaRechargeableItemComponent.h` |
| 캐릭터 | `UHarmoniaCurrencyManagerComponent` | `HarmoniaCurrencyManagerComponent.h` |
| 캐릭터 | `UHarmoniaDeathPenaltyComponent` | `HarmoniaDeathPenaltyComponent.h` |
| 생활 | `UHarmoniaFishingComponent` | `HarmoniaFishingComponent.h` |
| 생활 | `UHarmoniaGatheringComponent` | `HarmoniaGatheringComponent.h` |
| 생활 | `UHarmoniaCookingComponent` | `HarmoniaCookingComponent.h` |
| 생활 | `UHarmoniaFarmingComponent` | `HarmoniaFarmingComponent.h` |
| 생활 | `UHarmoniaMusicComponent` | `HarmoniaMusicComponent.h` |
| 퀘스트 | `UHarmoniaQuestComponent` | `HarmoniaQuestComponent.h` |
| 던전 | `UHarmoniaDungeonComponent` | `HarmoniaDungeonComponent.h` |
| 제작 | `UHarmoniaCraftingComponent` | `HarmoniaCraftingComponent.h` |

### 15.2 주요 서브시스템 목록

| 서브시스템 | 범위 | 역할 |
|-----------|------|------|
| `UHarmoniaTeamManagementSubsystem` | World | 팀/진영 관리 |
| `UHarmoniaWorldGeneratorSubsystem` | World | 절차적 월드 생성 |
| `UHarmoniaEffectCacheSubsystem` | World | VFX/SFX 캐싱 |
| `UHarmoniaOnlineSubsystem` | GameInstance | 온라인 기능 |
| `UHarmoniaSaveGameSubsystem` | GameInstance | 저장/불러오기 |
| `UHarmoniaModSubsystem` | GameInstance | 모드 관리 |

### 15.3 주요 데이터 구조체

| 구조체 | 용도 |
|--------|------|
| `FHarmoniaMeleeWeaponData` | 근접 무기 데이터 |
| `FComboAttackData` | 콤보 공격 정의 |
| `FQuestData` | 퀘스트 정의 |
| `FCraftingRecipeData` | 제작 레시피 |
| `FHarmoniaDungeonData` | 던전 정의 |
| `FRechargeableItemData` | 회복 아이템 정의 |
| `FHarmoniaTeamIdentification` | 팀 식별자 |

---

## 16. 데이터 드리븐 태그 시스템

### 16.1 개요

HarmoniaKit은 GameplayTag를 **데이터 드리븐 방식**으로 관리합니다. 코드 수정 없이 INI 파일이나 DataTable을 통해 새 태그를 추가할 수 있습니다.

### 16.2 태그 정의 방식

#### 방식 1: INI 파일 (Config/HarmoniaGameplayTags.ini)
```ini
; 전투 상태 태그
+GameplayTags=(Tag="State.Combat.Attacking",DevComment="공격 중")
+GameplayTags=(Tag="State.Combat.Blocking",DevComment="방어 중")
+GameplayTags=(Tag="State.Combat.Dodging",DevComment="회피 중")

; 속성 태그
+GameplayTags=(Tag="Stat.Primary.Strength",DevComment="힘")
+GameplayTags=(Tag="Stat.Combat.AttackPower",DevComment="공격력")
```

#### 방식 2: DataTable (FHarmoniaTagDefinition)
| TagName | Category | DisplayName | DevComment |
|---------|----------|-------------|------------|
| State.Combat.Attacking | State | 공격 중 | 캐릭터가 공격 중 |
| Damage.Type.Physical | Damage | 물리 피해 | 물리 속성 피해 |

### 16.3 주요 태그 카테고리

| 카테고리 | 패턴 | 용도 | 예시 |
|----------|------|------|------|
| **InputTag** | `InputTag.*` | Enhanced Input 입력 매핑 | `InputTag.Attack.Light`, `InputTag.Mount` |
| **Ability** | `Ability.*` | 어빌리티 클래스 식별 | `Ability.Melee.Slash`, `Ability.Magic.Fireball` |
| **State** | `State.*` | 캐릭터/시스템 상태 | `State.Combat.Attacking`, `State.Mounted` |
| **GameplayEvent** | `GameplayEvent.*` | 이벤트 트리거 | `GameplayEvent.HitReaction`, `GameplayEvent.Death` |
| **Damage** | `Damage.Type.*` | 데미지 타입 | `Damage.Type.Physical`, `Damage.Type.Fire` |
| **Effect** | `Effect.*` | 시각/사운드 효과 | `Effect.Hit.Slash`, `Effect.Trail.Sword` |
| **Stat** | `Stat.*` | 어트리뷰트 매핑 | `Stat.Primary.Strength`, `Stat.Combat.AttackPower` |
| **StatusEffect** | `StatusEffect.*` | 상태이상 | `StatusEffect.Burn`, `StatusEffect.Stun` |
| **Mount** | `Mount.*` | 탈것 시스템 | `Mount.Type.Ground`, `Mount.Type.Flying` |
| **Waypoint** | `Waypoint.*` | 패스트 트래블 | `Waypoint.Discovered`, `Waypoint.Active` |
| **Surface** | `Surface.*` | 등반/파쿠르 | `Surface.Climbable.Ladder`, `Surface.Climbable.Rough` |
| **Water** | `Water.*` | 수영 시스템 | `Water.Environment.Fresh`, `Water.Environment.Toxic` |
| **Anim** | `Anim.*` | 애니메이션 트리거 | `Anim.Player.Attack.Sword.Light` |
| **Sound** | `Sound.*` | 사운드 트리거 | `Sound.SFX.Hit.Metal`, `Sound.Music.Combat` |
| **Item** | `Item.*` | 아이템 분류 | `Item.Type.Weapon`, `Item.Rarity.Legendary` |
| **AI** | `AI.*` | AI 행동 | `AI.State.Alert`, `AI.Behavior.Patrol` |
| **Quest** | `Quest.*` | 퀘스트 분류 | `Quest.Type.Main`, `Quest.State.Active` |

### 16.4 HarmoniaTagRegistrySubsystem

런타임에 태그를 등록하고 조회하는 엔진 서브시스템입니다.

```cpp
// 서브시스템 접근
UHarmoniaTagRegistrySubsystem* TagRegistry = GEngine->GetEngineSubsystem<UHarmoniaTagRegistrySubsystem>();

// 태그 검색
FGameplayTag AttackTag = TagRegistry->FindTagByName(TEXT("State.Combat.Attacking"));

// 카테고리별 태그 조회
TArray<FGameplayTag> StateTags = TagRegistry->GetTagsByCategory(TEXT("State"));

// 모든 태그 가져오기
const TMap<FName, FGameplayTag>& AllTags = TagRegistry->GetAllTags();
```

### 16.5 태그-어트리뷰트 매핑

`FAttributeNameMap` 싱글톤을 통해 태그와 GAS 어트리뷰트를 매핑합니다.

```cpp
// 태그로 어트리뷰트 조회
FGameplayAttribute Attr = UHarmoniaAbilitySystemLibrary::GetAttributeByName(TEXT("Stat.Primary.Strength"));
if (Attr.IsValid())
{
    float Value = AbilitySystemComponent->GetNumericAttribute(Attr);
}

// 매핑된 어트리뷰트 목록
// Stat.Primary.*: Strength, Dexterity, Intelligence, Constitution, Wisdom, Luck
// Stat.Combat.*: AttackPower, Defense, CriticalRate, CriticalDamage, AttackSpeed, CooldownReduction
// Stat.Survival.*: MaxHealth, MaxMana, MaxStamina, HealthRegen, ManaRegen, StaminaRegen
// Stat.Resistance.*: FireResistance, IceResistance, LightningResistance, PoisonResistance
```

### 16.6 INI 파일 구조

`Config/HarmoniaGameplayTags.ini` 파일의 전체 구조:

```ini
[/Script/GameplayTags.GameplayTagsSettings]
ImportTagsFromConfig=True

; ============================================================================
; Input Tags (Enhanced Input 연동)
; ============================================================================
+GameplayTags=(Tag="InputTag.Attack.Light",DevComment="경 공격 입력")
+GameplayTags=(Tag="InputTag.Attack.Heavy",DevComment="강 공격 입력")
...

; ============================================================================
; State Tags (캐릭터/시스템 상태)
; ============================================================================
+GameplayTags=(Tag="State.Combat.Attacking",DevComment="공격 중")
+GameplayTags=(Tag="State.Mounted",DevComment="탈것 탑승 중")
...

; ============================================================================
; Stat Tags (어트리뷰트 매핑)
; ============================================================================
+GameplayTags=(Tag="Stat.Primary.Strength",DevComment="힘")
+GameplayTags=(Tag="Stat.Combat.AttackPower",DevComment="공격력")
...
```

### 16.7 새 태그 추가 방법

#### 즉시 적용 (Hot Reload 지원)
1. `Config/HarmoniaGameplayTags.ini` 파일 열기
2. 적절한 섹션에 새 태그 추가
3. 에디터 재시작 (또는 프로젝트 세팅에서 Reload)

#### C++ 코드에서 사용
```cpp
// 1. HarmoniaGameplayTags.h에 선언 추가
UE_DECLARE_GAMEPLAY_TAG_EXTERN(MyNewTag)

// 2. HarmoniaGameplayTags.cpp에 정의 추가
UE_DEFINE_GAMEPLAY_TAG(MyNewTag, "Custom.MyNewTag")

// 3. INI에도 문서화용으로 추가
+GameplayTags=(Tag="Custom.MyNewTag",DevComment="내 커스텀 태그")
```

---

## 17. Gameplay Ability 태그 설정 가이드

### 17.1 개요

HarmoniaKit의 모든 Gameplay Ability 클래스는 **블루프린트 또는 파생 클래스에서 태그를 설정**하도록 설계되었습니다. 
이를 통해 태그 설정의 유연성을 확보하고, 프로젝트별 커스터마이징이 가능합니다.

> ⚠️ **중요**: 태그 추가 전 반드시 `Config/DefaultGameplayTags.ini`에서 중복 여부를 확인하세요!

### 17.2 태그 컨테이너 설명

| 컨테이너 | 용도 | 예시 |
|---------|------|------|
| **AbilityTags** | 어빌리티를 식별하는 태그 | `Ability.Combat.Dodge` |
| **ActivationOwnedTags** | 어빌리티 활성화 중 소유자에게 적용되는 태그 | `State.Dodging` |
| **ActivationRequiredTags** | 어빌리티 활성화에 필요한 태그 | `State.Combat.RiposteWindow` |
| **ActivationBlockedTags** | 이 태그가 있으면 어빌리티 활성화 불가 | `State.HitReaction` |
| **BlockAbilitiesWithTag** | 어빌리티 활성화 중 이 태그를 가진 다른 어빌리티 차단 | `State.Combat.Attacking` |
| **CancelAbilitiesWithTag** | 어빌리티 활성화 시 이 태그를 가진 다른 어빌리티 취소 | `State.Blocking` |

### 17.3 전투 어빌리티 태그 설정

#### 17.3.1 UHarmoniaGameplayAbility_Dodge (회피)

```
AbilityTags:
  - Ability.Combat.Dodge

ActivationOwnedTags:
  - State.Dodging

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking

Related Tags (I-Frame 중 MeleeCombatComponent에서 적용):
  - State.Invincible
```

#### 17.3.2 UHarmoniaGameplayAbility_MeleeAttack (근접 공격)

```
AbilityTags:
  - Ability.Combat.Attack.Melee
  - Ability.Combat.Attack.Light (경공격용)
  - Ability.Combat.Attack.Heavy (강공격용)

ActivationOwnedTags:
  - State.Combat.Attacking

ActivationBlockedTags:
  - State.Combat.Attacking (콤보가 아닌 경우)
  - State.Blocking
  - State.Dodging
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Blocking
  - State.Dodging

Related Tags:
  - State.Combat.ComboWindow (콤보 입력 윈도우 중 적용)
```

#### 17.3.3 UHarmoniaGameplayAbility_Block (방어)

```
AbilityTags:
  - Ability.Combat.Block

ActivationOwnedTags:
  - State.Blocking

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Dodging
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Dodging
```

#### 17.3.4 UHarmoniaGameplayAbility_Parry (패리)

```
AbilityTags:
  - Ability.Combat.Parry

ActivationOwnedTags:
  - State.Parrying

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging
```

#### 17.3.5 UHarmoniaGameplayAbility_Riposte (리포스트)

```
AbilityTags:
  - Ability.Combat.Riposte

ActivationOwnedTags:
  - State.Combat.Riposting
  - State.Invincible

ActivationRequiredTags:
  - State.Combat.RiposteWindow (패리 성공 후 적용됨)

ActivationBlockedTags:
  - State.HitReaction
  - State.Dodging

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging

CancelAbilitiesWithTag:
  - State.Blocking
```

#### 17.3.6 UHarmoniaGameplayAbility_HitReaction (피격 반응)

```
AbilityTags:
  - Ability.Combat.HitReaction

AbilityTriggers:
  - TriggerTag: GameplayEvent.HitReaction
  - TriggerSource: GameplayEvent

ActivationOwnedTags:
  - State.HitReaction
  - State.HitStunned

ActivationBlockedTags:
  - State.Invincible

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging

CancelAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking
```

#### 17.3.7 UHarmoniaGameplayAbility_ComboAttack (콤보 공격)

```
AbilityTags:
  - Ability.Combat.Attack.Combo

ActivationOwnedTags:
  - State.Combat.Attacking

ActivationBlockedTags:
  - State.Combat.Attacking (콤보 윈도우가 아닐 때만)
  - State.Blocking
  - State.Dodging
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Blocking
  - State.Dodging

Related Tags:
  - State.Combat.ComboWindow (콤보 연속 허용)

Related Gameplay Events:
  - GameplayEvent.Attack.ComboNext
  - GameplayEvent.Attack.ComboReset
```

#### 17.3.8 UHarmoniaGameplayAbility_RangedAttack (원거리 공격)

```
AbilityTags:
  - Ability.Combat.Attack.Ranged

ActivationOwnedTags:
  - State.Combat.Attacking
  - State.Combat.Aiming (조준/차징 중)

ActivationBlockedTags:
  - State.Dead
  - State.Stunned
  - State.HitReaction
  - State.Dodging

BlockAbilitiesWithTag:
  - State.Blocking
  - State.Dodging

CancelAbilitiesWithTag:
  - Ability.Attack.Melee
```

### 17.4 이동 어빌리티 태그 설정

#### 17.4.1 UHarmoniaGameplayAbility_Climb (등반)

```
AbilityTags:
  - Ability.Movement.Climb

ActivationOwnedTags:
  - State.Climbing

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Mounted
  - State.Swimming
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Mounted
  - State.Swimming

Movement Restriction Check:
  - Movement.Restricted.NoClimb

Related Gameplay Events:
  - GameplayEvent.Climbing.Started
  - GameplayEvent.Climbing.Stopped
```

#### 17.4.2 UHarmoniaGameplayAbility_Swim (수영)

```
AbilityTags:
  - Ability.Movement.Swim

ActivationOwnedTags:
  - State.Swimming

ActivationBlockedTags:
  - State.Mounted
  - State.Climbing
  - State.HitReaction

BlockAbilitiesWithTag:
  - State.Mounted
  - State.Climbing

Movement Restriction Check:
  - Movement.Restricted.NoSwim

Related Gameplay Events:
  - GameplayEvent.Swimming.Started
  - GameplayEvent.Swimming.Stopped
  - GameplayEvent.Diving.Started
  - GameplayEvent.Oxygen.Depleted
```

#### 17.4.3 UHarmoniaGameplayAbility_Vault (볼트/파쿠르)

```
AbilityTags:
  - Ability.Movement.Vault

ActivationOwnedTags:
  - State.Vaulting

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Blocking
  - State.Dodging
  - State.HitReaction
  - State.Swimming

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking

Related Gameplay Events:
  - GameplayEvent.Parkour.Vault
```

#### 17.4.4 UHarmoniaGameplayAbility_Mount (탈것)

```
AbilityTags:
  - Ability.Movement.Mount

ActivationOwnedTags:
  - State.Mounting

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Dodging
  - State.Mounted
  - State.Swimming
  - State.Climbing

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Dodging
  - State.Mounted

Movement Restriction Check:
  - Movement.Restricted.NoMount

Related Gameplay Events:
  - GameplayEvent.Mount.Mounted
  - GameplayEvent.Mount.Dismounted
```

#### 17.4.5 UHarmoniaGameplayAbility_FastTravel (빠른 이동)

```
AbilityTags:
  - Ability.Movement.FastTravel

ActivationOwnedTags:
  - State.FastTraveling

ActivationRequiredTags:
  - State.CanFastTravel (또는 Waypoint.Discovered)

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.HitReaction
  - State.Swimming
  - State.Climbing

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Movement.Sprint

CancelAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Blocking

Movement Restriction Check:
  - Movement.Restricted.NoFastTravel

Related Gameplay Events:
  - GameplayEvent.FastTravel.Started
  - GameplayEvent.FastTravel.Completed
```

### 17.5 아이템/상호작용 어빌리티 태그 설정

#### 17.5.1 UHarmoniaGameplayAbility_UseRecoveryItem (회복 아이템 사용)

```
AbilityTags:
  - Ability.Item.UseRecovery

ActivationOwnedTags:
  - State.UsingItem
  - State.Casting (시전 중)

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.Dodging
  - State.HitReaction
  - State.UsingItem

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Dodging

Note: 아이템 사용은 다음 상황에서 취소됨:
  - 시전 중 이동
  - 시전 중 피격
```

#### 17.5.2 UHarmoniaGameplayAbility_Interact (상호작용)

```
AbilityTags:
  - Ability.Interaction

AbilityTriggers:
  - TriggerTag: Event.Interaction.TryInteract
  - TriggerSource: GameplayEvent

ActivationOwnedTags:
  - State.Interacting

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.HitReaction
  - State.Dodging

BlockAbilitiesWithTag:
  - State.Combat.Attacking
```

### 17.6 몬스터 전용 어빌리티 태그 설정

#### 17.6.1 UHarmoniaGameplayAbility_Boss (보스 기본)

```
AbilityTags:
  - Ability.Boss.[AbilityName]

ActivationOwnedTags:
  - State.Boss.Casting (시전 중)
  - State.Boss.Enraged (분노 어빌리티용)

ActivationBlockedTags:
  - State.Boss.PhaseTransition
  - State.HitReaction (일부 보스는 무시 가능)

ValidPhases:
  - 빈 배열 = 모든 페이즈
  - [1, 2] = 1, 2 페이즈만
  - [3] = 3 페이즈(분노 페이즈)만
```

#### 17.6.2 UHarmoniaGameplayAbility_Stealth (은신)

```
AbilityTags:
  - Ability.Combat.Stealth

ActivationOwnedTags:
  - State.Stealthed

ActivationBlockedTags:
  - State.Combat.Attacking
  - State.HitReaction
  - State.Stealthed

Note: 은신 해제 조건:
  - 공격 시 (기습 데미지 적용 후)
  - 피격 시
  - 지속 시간 만료 시 (무한이 아닌 경우)
```

#### 17.6.3 UHarmoniaGameplayAbility_Summon (소환)

```
AbilityTags:
  - Ability.Combat.Summon

ActivationOwnedTags:
  - State.Summoning
  - State.Casting

ActivationBlockedTags:
  - State.HitReaction
  - State.Dodging
  - State.Summoning

BlockAbilitiesWithTag:
  - State.Combat.Attacking
  - State.Dodging
```

#### 17.6.4 UHarmoniaGameplayAbility_ElitePassive (엘리트 패시브)

```
AbilityTags:
  - Ability.Monster.Elite.Passive

ActivationPolicy: OnSpawn (자동 활성화)
ActivationGroup: Exclusive_Blocking

ActivationOwnedTags:
  - State.Elite

Note: 패시브 어빌리티로 항상 활성 상태
```

#### 17.6.5 UHarmoniaGameplayAbility_SwarmBehavior (군집 행동)

```
AbilityTags:
  - Ability.Monster.Swarm.Passive

ActivationPolicy: OnSpawn (자동 활성화)
ActivationGroup: Independent (다른 어빌리티와 병행 가능)

ActivationOwnedTags:
  - State.Swarm.Active (활성 중 항상)
  - State.Swarm.Empowered (군집 보너스 활성 시)
  - State.Swarm.Fearful (인근 아군 사망 시)

Note: 패시브 어빌리티로 항상 활성 상태
```

### 17.7 태그 네이밍 규칙

#### 17.7.1 권장 패턴

| 패턴 | 용도 | 예시 |
|------|------|------|
| `Ability.[Category].[Name]` | 어빌리티 식별 | `Ability.Combat.Dodge` |
| `State.[StateName]` | 상태 태그 | `State.Dodging`, `State.HitReaction` |
| `State.Combat.[StateName]` | 전투 관련 상태 | `State.Combat.Attacking` |
| `State.Movement.[StateName]` | 이동 관련 상태 | `State.Movement.Sprint` |
| `GameplayEvent.[EventName]` | 이벤트 트리거 | `GameplayEvent.HitReaction` |
| `Movement.Restricted.[Type]` | 이동 제한 | `Movement.Restricted.NoClimb` |

#### 17.7.2 태그 통일 권장사항

현재 일부 어빌리티에서 `State.X`와 `Character.State.X` 패턴이 혼용되고 있습니다.

**권장**: `State.X` 패턴으로 통일
- ✅ `State.Dodging`
- ❌ `Character.State.Dodging`

### 17.8 블루프린트에서 태그 설정 방법

1. **어빌리티 블루프린트 생성**: C++ 클래스를 부모로 블루프린트 생성
2. **Class Defaults 열기**: 블루프린트 에디터에서 Class Defaults 선택
3. **Tags 섹션 찾기**: Details 패널에서 "Tags" 검색
4. **각 컨테이너에 태그 추가**:
   - Ability Tags
   - Activation Owned Tags
   - Activation Required Tags
   - Activation Blocked Tags
   - Block Abilities With Tag
   - Cancel Abilities With Tag

### 17.9 태그 중복 방지

> ⚠️ **경고**: 태그 추가 전 반드시 확인!

#### 확인 순서
1. `Config/DefaultGameplayTags.ini` 검색
2. DataTable 태그 정의 확인 (예: `DT_HarmoniaCharacterTypes`)
3. 기존 어빌리티의 태그 설정 확인

#### 중복 발생 시
- 동일한 태그가 여러 곳에서 정의되면 경고 발생 가능
- INI 파일에서 정의된 태그가 우선권을 가짐
- 태그 정의는 한 곳에서만 관리 권장

---

## 📝 라이선스

Copyright © 2025 Snow Game Studio.  
All rights reserved.

---

**제작:** Antigravity AI & Snow Game Studio  
**최종 업데이트:** 2025-11-26
