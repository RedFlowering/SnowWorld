# HarmoniaKit 플러그인 - 통합 문서

**Version:** 1.0  
**Last Updated:** 2025-11-27  
**Unreal Engine:** 5.7  
**Framework:** Lyra Starter Game

---

## 📖 목차

1. [소개](#1-소개)
2. [플러그인 아키텍처](#2-플러그인-아키텍처)
3. [빠른 시작](#3-빠른-시작)
4. [전투 시스템](#4-전투-시스템)
   - 4.6 [락온 타게팅 시스템](#46-락온-타게팅-시스템-lock-on-targeting)
   - 4.7 [회피 구르기 시스템](#47-회피-구르기-시스템-dodge-roll)
   - 4.8 [관련 GameplayTag 정의](#48-관련-gameplaytag-정의)
5. [캐릭터 시스템](#5-캐릭터-시스템)
6. [생활 컨텐츠 시스템](#6-생활-컨텐츠-시스템)
7. [온라인 및 팀 시스템](#7-온라인-및-팀-시스템)
8. [던전 및 월드 시스템](#8-던전-및-월드-시스템)
9. [퀘스트 시스템](#9-퀘스트-시스템)
10. [인벤토리 및 제작 시스템](#10-인벤토리-및-제작-시스템)
11. [저장 시스템](#11-저장-시스템)
12. [모드 시스템](#12-모드-시스템)
13. [개발 도구](#13-개발-도구)
14. [언리얼 엔진 통합](#14-언리얼-엔진-통합)
15. [리팩토링 히스토리](#15-리팩토링-히스토리)
16. [API 레퍼런스](#16-api-레퍼런스)
17. [데이터 드리븐 태그 시스템](#17-데이터-드리븐-태그-시스템)
18. [Gameplay Ability 태그 설정 가이드](#18-gameplay-ability-태그-설정-가이드)
19. [오브젝트 풀링 시스템](#19-오브젝트-풀링-시스템)
20. [스킬 트리 시스템](#20-스킬-트리-시스템)
21. [동적 난이도 조절 (DDA) 시스템](#21-동적-난이도-조절-dda-시스템)
22. [미니맵 및 월드 마커 시스템](#22-미니맵-및-월드-마커-시스템)
23. [버프/디버프 UI 시스템](#23-버프디버프-ui-시스템)
24. [업데이트 레이트 최적화 시스템](#24-업데이트-레이트-최적화-시스템)
25. [AI 스로틀링 시스템](#25-ai-스로틀링-시스템)
26. [네트워크 최적화 시스템](#26-네트워크-최적화-시스템)

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

## 2. 플러그인 아키텍처

### 2.1 Lyra 프레임워크 기반

HarmoniaKit은 **Lyra Starter Game** 프레임워크를 기반으로 확장되었습니다.

| 항목 | 설명 |
|------|------|
| **핵심 모듈** | LyraGame |
| **소스 경로** | `Engine/Plugins/Runtime/Lyra/Source/LyraGame/` |

#### Lyra 통합 클래스

| HarmoniaKit 클래스 | Lyra 부모 클래스 | 설명 |
|-------------------|------------------|------|
| `UHarmoniaAttributeSet` | `ULyraAttributeSet` | 확장된 RPG 어트리뷰트 |
| `UHarmoniaGameplayAbility` | `ULyraGameplayAbility` | 커스텀 어빌리티 기반 |
| `UHarmoniaInventoryFragment_*` | `ULyraInventoryItemFragment` | 인벤토리 프래그먼트 |

### 2.2 플러그인 종속성

#### 엔진 모듈
```
Core, CoreUObject, Engine, Slate, SlateCore, UMG,
GameplayAbilities, GameplayTags, GameplayTasks,
EnhancedInput, InputCore, CommonUI, NavigationSystem,
AIModule, Niagara, LevelSequence, MovieScene
```

#### 외부 플러그인 (필수)
| 플러그인 | 버전 | 용도 |
|----------|------|------|
| **SenseSystem** | 최신 | 멀티스레드 히트 감지 |
| **ALS (Advanced Locomotion System)** | 최신 | 캐릭터 이동/애니메이션 |

#### 온라인 서브시스템 (선택)
| 플러그인 | 용도 |
|----------|------|
| **OnlineSubsystem** | 온라인 기능 기본 |
| **OnlineSubsystemSteam** | Steam 통합 |
| **OnlineSubsystemEOS** | Epic Online Services |

### 2.3 모듈 구조

HarmoniaKit 플러그인은 **10개의 모듈**로 구성됩니다.

| 모듈 | 타입 | 로드 시점 | 설명 |
|------|------|----------|------|
| **HarmoniaKit** | Runtime | Default | 핵심 런타임 모듈 |
| **HarmoniaEditor** | Editor | Default | 에디터 전용 도구 |
| **HarmoniaLoadManager** | Runtime | Default | 비동기 로딩 관리 |
| **HarmoniaLoadManagerEditor** | Editor | Default | 로드 매니저 에디터 |
| **HarmoniaOnlineSubsystem** | Runtime | Default | 온라인 서브시스템 |
| **HarmoniaStory** | Runtime | Default | 스토리/다이얼로그 시스템 |
| **HarmoniaWorldGenerator** | Runtime | Default | 절차적 월드 생성 |
| **HarmoniaModSystem** | Runtime | Default | 모드 지원 시스템 |
| **HarmoniaLocalizationSystem** | Runtime | Default | 로컬라이제이션 |
| **HarmoniaGameService** | Runtime | Default | 게임 서비스 통합 |

### 2.4 개발 환경

| 항목 | 경로/값 |
|------|--------|
| **엔진 경로** | `C:\Epic\UE_5.7` |
| **프로젝트 경로** | `C:\Projects\SnowWorld` |
| **플러그인 경로** | `C:\Projects\SnowWorld\Plugins\HarmoniaKit` |
| **빌드 시스템** | UnrealBuildTool (UBT) |
| **IDE** | Visual Studio 2022 / Rider 권장 |

---

## 3. 빠른 시작

### 3.1 근접 전투 시스템 (5분 설정)

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

### 3.2 회복 아이템 시스템

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

### 3.3 사망 패널티 시스템 (다크소울 스타일)

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

## 4. 전투 시스템

### 4.1 개요

HarmoniaKit의 전투 시스템은 **Gameplay Ability System(GAS)**과 **SenseSystem**을 기반으로 한 고성능 멀티스레드 히트 감지 시스템입니다.

### 4.2 핵심 컴포넌트

| 컴포넌트 | 역할 |
|---------|------|
| `UHarmoniaBaseCombatComponent` | 전투 시스템 기본 클래스 |
| `UHarmoniaMeleeCombatComponent` | 근접 전투 시스템 |
| `UHarmoniaSenseAttackComponent` | Sense 기반 히트 감지 |
| `UHarmoniaGameplayAbility_ComboAttack` | 콤보 어빌리티 |

### 4.3 근접 전투 시스템

#### 4.3.1 지원 무기 타입 (11종)

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

#### 4.3.2 전투 액션

| 액션 | 함수 | 설명 |
|------|------|------|
| 약공격 | `TryLightAttack()` | 빠른 공격, 낮은 데미지 |
| 강공격 | `TryHeavyAttack()` | 느린 공격, 높은 데미지 |
| 방어 | `StartBlock()` / `StopBlock()` | 데미지 감소 |
| 패리 | `TryParry()` | 정확한 타이밍에 공격 튕겨내기 |
| 회피 | `TryDodge()` | 무적 프레임 포함 회피 |
| 리포스트 | `TryRiposte()` | 패리 성공 후 반격 |
| 백스탭 | `TryBackstab()` | 후방 기습 공격 |

#### 4.3.3 콤보 시스템

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

### 4.4 데미지 시스템

#### 4.4.1 데미지 타입

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

#### 4.4.2 Attribute Set

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

### 4.5 보스 전투 설정

#### 4.5.1 보스 몬스터 클래스

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

#### 4.5.2 Boss Phase 데이터

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

### 4.6 락온 타게팅 시스템 (Lock-On Targeting)

소울라이크 전투 시스템의 핵심 기능인 락온 타게팅 시스템입니다.

#### 4.6.1 주요 기능

| 기능 | 설명 |
|------|------|
| **자동 타겟 찾기** | 카메라 중심에서 가장 가까운 적을 자동으로 찾아 락온 |
| **타겟 전환** | 좌우로 타겟을 전환할 수 있는 기능 |
| **자동 카메라 회전** | 락온된 적을 향해 카메라가 자동으로 회전 |
| **거리 기반 해제** | 적이 너무 멀어지면 자동으로 락온 해제 |
| **Sense System 통합** | 장애물에 가려진 적은 타겟팅 불가 |

#### 4.6.2 핵심 클래스

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

#### 4.6.3 설정 가능한 파라미터

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

#### 4.6.4 이벤트 델리게이트

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

#### 4.6.5 블루프린트에서 사용

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

### 4.7 회피 구르기 시스템 (Dodge Roll)

타이밍 기반 회피 메커니즘과 무적 프레임(I-Frame)이 구현된 회피 시스템입니다.

#### 4.7.1 주요 기능

| 기능 | 설명 |
|------|------|
| **방향 기반 회피** | 입력 방향으로 구르기 (입력 없으면 후방) |
| **무적 프레임 (I-frames)** | 회피 중 일정 시간 동안 무적 상태 |
| **장비 무게 시스템** | 장비 무게에 따라 롤 타입 변경 (Light/Medium/Heavy) |
| **스태미나 소모** | 롤 타입별 스태미나 소모량 차이 |
| **네트워크 지원** | 멀티플레이어 환경에서 작동 |

#### 4.7.2 롤 타입별 파라미터

| 롤 타입 | 장비 무게 | 거리 | 지속시간 | I-Frame 시작 | I-Frame 지속 | 스태미나 |
|--------|----------|------|----------|--------------|--------------|----------|
| **Light** | 0-30% | 500 | 0.5초 | 0.1초 | 0.4초 | 15 |
| **Medium** | 30-70% | 400 | 0.6초 | 0.1초 | 0.3초 | 20 |
| **Heavy** | 70%+ | 300 | 0.8초 | 0.15초 | 0.2초 | 30 |

#### 4.7.3 핵심 클래스

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

#### 4.7.4 Gameplay Ability 블루프린트 생성

1. **블루프린트 생성**: `UHarmoniaGameplayAbility_Dodge`를 부모로 하는 블루프린트 생성
2. **애니메이션 설정**: `DodgeMontage`에 회피 애니메이션 할당
3. **파라미터 조정**: 롤 타입별 거리/지속시간/I-Frame 설정
4. **태그 설정**: 아래 태그 설정 참고

#### 4.7.5 회피 어빌리티 태그 설정

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

#### 4.7.6 캐릭터에 Ability 부여

Experience 또는 AbilitySet에서 Dodge Ability를 캐릭터에 부여:

1. 해당 Experience/AbilitySet 열기
2. Abilities 배열에 GA_Dodge 블루프린트 추가
3. Input Tag를 `InputTag.Dodge`로 설정

### 4.8 관련 GameplayTag 정의

Lock-On과 Dodge 시스템에서 사용하는 GameplayTag들입니다.

#### 4.8.1 Lock-On 시스템 태그

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

#### 4.8.2 Dodge 시스템 태그

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

## 5. 캐릭터 시스템

### 5.1 스태미나 시스템

#### 5.1.1 행동별 스태미나 소비

| 행동 | 기본 소비량 |
|------|------------|
| 달리기 | 10/초 |
| 약공격 | 15 |
| 강공격 | 25 |
| 방어 (히트 시) | 20 |
| 구르기 | 20 |
| 점프 | 10 |

#### 5.1.2 스태미나 회복

```cpp
// 자동 회복 설정
UPROPERTY(EditAnywhere)
float StaminaRegenRate = 30.0f; // 초당 회복량

UPROPERTY(EditAnywhere)
float StaminaRegenDelay = 1.5f; // 소비 후 회복 시작 딜레이
```

### 5.2 회복 아이템 시스템 (에스투스 스타일)

#### 5.2.1 핵심 클래스

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

#### 5.2.2 Resonance Shard (충전 아이템)

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

### 5.3 사망 패널티 시스템

#### 5.3.1 핵심 흐름

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

#### 5.3.2 Memory Echo 액터

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

## 6. 생활 컨텐츠 시스템

### 6.1 낚시 시스템

#### 6.1.1 핵심 컴포넌트

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

#### 6.1.2 물고기 희귀도

| 희귀도 | 설명 |
|--------|------|
| Common | 일반 |
| Uncommon | 고급 |
| Rare | 희귀 |
| Epic | 영웅 |
| Legendary | 전설 |

#### 6.1.3 미니게임 타입

```cpp
enum class EFishingMinigameType : uint8
{
    Timing,        // 타이밍 맞추기
    BarBalance,    // 바 균형 유지
    QTE,           // Quick Time Event
    ReelTension    // 릴 장력 관리
};
```

### 6.2 채집 시스템

#### 6.2.1 자원 타입

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

#### 6.2.2 도구 시스템

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

#### 6.2.3 사용 예시

```cpp
UHarmoniaGatheringComponent* GatherComp = Character->FindComponentByClass<UHarmoniaGatheringComponent>();
GatherComp->EquipTool(PickaxeData);
GatherComp->StartGathering(MineralResourceID);
// 채집 완료 시 OnGatheringComplete 델리게이트 발생
```

### 6.3 요리 시스템

#### 6.3.1 조리 방법

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

#### 6.3.2 요리 품질 및 버프

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

#### 6.3.3 사용 예시

```cpp
UHarmoniaCookingComponent* CookingComp = Character->FindComponentByClass<UHarmoniaCookingComponent>();
CookingComp->DiscoverRecipe(RecipeID);
CookingComp->StartCooking(RecipeID);
// 요리 완료 후
CookingComp->ConsumeFood(FoodID, Quality);
```

### 6.4 농사 시스템

#### 6.4.1 작물 성장 단계

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

#### 6.4.2 농사 시스템 흐름

```
[밭 생성] → [씨앗 심기] → [물주기] → [비료 주기(선택)]
                              ↓
[성장 (시간 경과)] → [수확] → [재수확 가능 작물은 반복]
```

#### 6.4.3 비료 효과

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

### 6.5 음악/연주 시스템

#### 6.5.1 악기 타입

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

#### 6.5.2 리듬 미니게임

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

#### 6.5.3 범위 버프 적용

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

## 7. 온라인 및 팀 시스템

### 7.1 온라인 서브시스템

#### 7.1.1 지원 플랫폼

| 플랫폼 | 기능 |
|--------|------|
| Steam | 친구, 매치메이킹, 업적, 클라우드 저장 |
| Epic Online Services (EOS) | 크로스 플랫폼, 세션 관리 |

#### 7.1.2 핵심 기능

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

### 7.2 팀 시스템

#### 7.2.1 핵심 특징

- **무제한 팀 생성**: 런타임에 동적으로 팀 추가
- **유동적 관계 관리**: 게임 중 동맹↔적대 관계 변경
- **언리얼 표준 통합**: `IGenericTeamAgentInterface` 완벽 지원

#### 7.2.2 팀 생성

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

#### 7.2.3 관계 설정

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

#### 7.2.4 피아식별 쿼리

```cpp
// 두 액터가 적인지 확인
bool bAreEnemies = TeamSubsystem->AreEnemies(ActorA, ActorB);

// 공격 가능 여부
bool bCanAttack = TeamSubsystem->CanActorAttack(Source, Target);
```

---

## 8. 던전 및 월드 시스템

### 8.1 던전 레이드 시스템

#### 8.1.1 핵심 클래스

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

#### 8.1.2 던전 데이터

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

### 8.2 월드 생성 시스템

#### 8.2.1 절차적 생성

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

#### 8.2.2 바이옴 시스템

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

## 9. 퀘스트 시스템

### 9.1 핵심 컴포넌트

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

### 9.2 퀘스트 데이터 구조

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

### 9.3 다단계 퀘스트 (Phase)

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

## 10. 인벤토리 및 제작 시스템

### 10.1 제작 시스템

#### 10.1.1 핵심 컴포넌트

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

#### 10.1.2 레시피 데이터

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

### 10.2 Lyra 인벤토리 통합

#### 10.2.1 커스텀 인벤토리 Fragment

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

## 11. 저장 시스템

### 11.1 핵심 서브시스템

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

### 11.2 Saveable 인터페이스

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

## 12. 모드 시스템

### 12.1 핵심 서브시스템

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

### 12.2 에셋 오버라이드

```cpp
UCLASS()
class UHarmoniaAssetOverrideSubsystem : public UGameInstanceSubsystem
{
    // 에셋 오버라이드 등록
    UFUNCTION(BlueprintCallable)
    void RegisterAssetOverride(const FSoftObjectPath& OriginalAsset, const FSoftObjectPath& OverrideAsset);
};
```

### 12.3 DataTable 패칭

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

## 13. 개발 도구

### 13.1 치트 시스템

> ⚠️ **주의**: Editor/Development 빌드에서만 사용 가능. Shipping 빌드에서는 자동 비활성화됨.

#### 13.1.1 콘솔 명령어

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

#### 13.1.2 치트 라이브러리 (블루프린트)

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

### 13.2 애니메이션 이펙트 시스템

#### 13.2.1 GameplayTag 기반 VFX/SFX 관리

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

#### 13.2.2 AnimNotify로 이펙트 재생

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

## 14. 언리얼 엔진 통합

### 14.1 AI 시스템 통합

#### 14.1.1 Blackboard 헬퍼

```cpp
// UHarmoniaBaseAIComponent에 추가된 헬퍼
UBlackboardComponent* GetBlackboardComponent() const;
void SetBlackboardValueAsObject(FName KeyName, UObject* ObjectValue);
UObject* GetBlackboardValueAsObject(FName KeyName) const;
void SetBlackboardValueAsVector(FName KeyName, FVector VectorValue);
void SetBlackboardValueAsBool(FName KeyName, bool BoolValue);
void SetBlackboardValueAsFloat(FName KeyName, float FloatValue);
```

#### 14.1.2 Behavior Tree Task/Service

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

### 14.2 Lyra 호환성

#### 14.2.1 Equipment 어댑터

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

## 15. 리팩토링 히스토리

### 15.1 2025-11-22: Combat System 리팩토링

**목적**: 전투 시스템 코드 중복 제거 및 유지보수성 향상

**주요 작업**:
1. `UHarmoniaBaseCombatComponent` 생성 (공통 코드 추출)
2. `HarmoniaMeleeCombatDefinitions.h` 분리 (Definition 파일 크기 축소)
3. `UHarmoniaCombatLibrary` 생성 (유틸리티 함수 중앙화)

**성과**:
- 코드 중복: 400줄+ → 200줄
- 파일 크기: 43KB → 분할된 작은 파일들

### 15.2 2025-11-22: 치트 시스템 빌드 제한

**목적**: Shipping 빌드에서 치트 기능 완전 제거

**주요 작업**:
- `#if !UE_BUILD_SHIPPING` 전처리기 적용
- 조건부 컴파일로 치트 코드 물리적 제거

**대상 파일**:
- `HarmoniaCheatManager.h/cpp`
- `HarmoniaCheatLibrary.h/cpp`

---

## 16. API 레퍼런스

### 16.1 주요 컴포넌트 목록

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

### 16.2 주요 서브시스템 목록

| 서브시스템 | 범위 | 역할 |
|-----------|------|------|
| `UHarmoniaTeamManagementSubsystem` | World | 팀/진영 관리 |
| `UHarmoniaWorldGeneratorSubsystem` | World | 절차적 월드 생성 |
| `UHarmoniaEffectCacheSubsystem` | World | VFX/SFX 캐싱 |
| `UHarmoniaOnlineSubsystem` | GameInstance | 온라인 기능 |
| `UHarmoniaSaveGameSubsystem` | GameInstance | 저장/불러오기 |
| `UHarmoniaModSubsystem` | GameInstance | 모드 관리 |

### 16.3 주요 데이터 구조체

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

## 17. 데이터 드리븐 태그 시스템

### 17.1 개요

HarmoniaKit은 GameplayTag를 **데이터 드리븐 방식**으로 관리합니다. 코드 수정 없이 INI 파일이나 DataTable을 통해 새 태그를 추가할 수 있습니다.

### 17.2 태그 정의 방식

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

### 17.3 주요 태그 카테고리

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

### 17.4 HarmoniaTagRegistrySubsystem

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

### 17.5 태그-어트리뷰트 매핑

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

### 17.6 INI 파일 구조

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

### 17.7 새 태그 추가 방법

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

## 18. Gameplay Ability 태그 설정 가이드

### 18.1 개요

HarmoniaKit의 모든 Gameplay Ability 클래스는 **블루프린트 또는 파생 클래스에서 태그를 설정**하도록 설계되었습니다. 
이를 통해 태그 설정의 유연성을 확보하고, 프로젝트별 커스터마이징이 가능합니다.

> ⚠️ **중요**: 태그 추가 전 반드시 `Config/DefaultGameplayTags.ini`에서 중복 여부를 확인하세요!

### 18.2 태그 컨테이너 설명

| 컨테이너 | 용도 | 예시 |
|---------|------|------|
| **AbilityTags** | 어빌리티를 식별하는 태그 | `Ability.Combat.Dodge` |
| **ActivationOwnedTags** | 어빌리티 활성화 중 소유자에게 적용되는 태그 | `State.Dodging` |
| **ActivationRequiredTags** | 어빌리티 활성화에 필요한 태그 | `State.Combat.RiposteWindow` |
| **ActivationBlockedTags** | 이 태그가 있으면 어빌리티 활성화 불가 | `State.HitReaction` |
| **BlockAbilitiesWithTag** | 어빌리티 활성화 중 이 태그를 가진 다른 어빌리티 차단 | `State.Combat.Attacking` |
| **CancelAbilitiesWithTag** | 어빌리티 활성화 시 이 태그를 가진 다른 어빌리티 취소 | `State.Blocking` |

### 18.3 전투 어빌리티 태그 설정

#### 18.3.1 UHarmoniaGameplayAbility_Dodge (회피)

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

#### 18.3.2 UHarmoniaGameplayAbility_MeleeAttack (근접 공격)

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

#### 18.3.3 UHarmoniaGameplayAbility_Block (방어)

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

#### 18.3.4 UHarmoniaGameplayAbility_Parry (패리)

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

#### 18.3.5 UHarmoniaGameplayAbility_Riposte (리포스트)

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

#### 18.3.6 UHarmoniaGameplayAbility_HitReaction (피격 반응)

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

#### 18.3.7 UHarmoniaGameplayAbility_ComboAttack (콤보 공격)

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

#### 18.3.8 UHarmoniaGameplayAbility_RangedAttack (원거리 공격)

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

### 18.4 이동 어빌리티 태그 설정

#### 18.4.1 UHarmoniaGameplayAbility_Climb (등반)

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

#### 18.4.2 UHarmoniaGameplayAbility_Swim (수영)

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

#### 18.4.3 UHarmoniaGameplayAbility_Vault (볼트/파쿠르)

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

#### 18.4.4 UHarmoniaGameplayAbility_Mount (탈것)

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

#### 18.4.5 UHarmoniaGameplayAbility_FastTravel (빠른 이동)

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

### 18.5 아이템/상호작용 어빌리티 태그 설정

#### 18.5.1 UHarmoniaGameplayAbility_UseRecoveryItem (회복 아이템 사용)

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

#### 18.5.2 UHarmoniaGameplayAbility_Interact (상호작용)

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

### 18.6 몬스터 전용 어빌리티 태그 설정

#### 18.6.1 UHarmoniaGameplayAbility_Boss (보스 기본)

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

#### 18.6.2 UHarmoniaGameplayAbility_Stealth (은신)

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

#### 18.6.3 UHarmoniaGameplayAbility_Summon (소환)

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

#### 18.6.4 UHarmoniaGameplayAbility_ElitePassive (엘리트 패시브)

```
AbilityTags:
  - Ability.Monster.Elite.Passive

ActivationPolicy: OnSpawn (자동 활성화)
ActivationGroup: Exclusive_Blocking

ActivationOwnedTags:
  - State.Elite

Note: 패시브 어빌리티로 항상 활성 상태
```

#### 18.6.5 UHarmoniaGameplayAbility_SwarmBehavior (군집 행동)

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

### 18.7 태그 네이밍 규칙

#### 18.7.1 권장 패턴

| 패턴 | 용도 | 예시 |
|------|------|------|
| `Ability.[Category].[Name]` | 어빌리티 식별 | `Ability.Combat.Dodge` |
| `State.[StateName]` | 상태 태그 | `State.Dodging`, `State.HitReaction` |
| `State.Combat.[StateName]` | 전투 관련 상태 | `State.Combat.Attacking` |
| `State.Movement.[StateName]` | 이동 관련 상태 | `State.Movement.Sprint` |
| `GameplayEvent.[EventName]` | 이벤트 트리거 | `GameplayEvent.HitReaction` |
| `Movement.Restricted.[Type]` | 이동 제한 | `Movement.Restricted.NoClimb` |

#### 18.7.2 태그 통일 권장사항

현재 일부 어빌리티에서 `State.X`와 `Character.State.X` 패턴이 혼용되고 있습니다.

**권장**: `State.X` 패턴으로 통일
- ✅ `State.Dodging`
- ❌ `Character.State.Dodging`

### 18.8 블루프린트에서 태그 설정 방법

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

### 18.9 태그 중복 방지

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

## 19. 오브젝트 풀링 시스템

고성능 Actor 풀링 시스템으로 메모리 할당/해제 오버헤드를 최소화합니다.

### 19.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaObjectPoolSubsystem` | WorldSubsystem으로 풀 관리 |
| `IHarmoniaPoolableInterface` | 풀링 대상 Actor 인터페이스 |
| `UHarmoniaObjectPoolConfigDataAsset` | 풀 설정 데이터 에셋 |

### 19.2 빠른 시작

#### Step 1: 데이터 에셋 생성
Content Browser → 우클릭 → Miscellaneous → Data Asset → `HarmoniaObjectPoolConfigDataAsset` 선택

#### Step 2: 풀 등록 (C++)
```cpp
UHarmoniaObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UHarmoniaObjectPoolSubsystem>();

FHarmoniaPoolConfig Config;
Config.ActorClass = AMyProjectile::StaticClass();
Config.InitialSize = 50;
Config.MaxSize = 200;
Config.bCanGrow = true;
Config.GrowthAmount = 10;
Config.ShrinkTimeout = 60.0f;

PoolSubsystem->RegisterPool(Config);
```

#### Step 3: Actor 획득/반환
```cpp
// 풀에서 가져오기
AActor* Projectile = PoolSubsystem->AcquireActor(AMyProjectile::StaticClass(), SpawnTransform);

// 사용 후 반환
PoolSubsystem->ReleaseActor(Projectile);
```

#### Step 4: Poolable Interface 구현 (선택)
```cpp
class AMyProjectile : public AActor, public IHarmoniaPoolableInterface
{
    virtual void OnAcquiredFromPool_Implementation() override
    {
        // 활성화 시 초기화
        SetActorEnableCollision(true);
    }

    virtual void OnReleasedToPool_Implementation() override
    {
        // 비활성화 시 정리
        SetActorEnableCollision(false);
        StopAllParticles();
    }
};
```

### 19.3 데이터 에셋 설정

| 설정 | 설명 | 기본값 |
|------|------|--------|
| `bEnablePooling` | 전역 풀링 활성화 | true |
| `bCollectStatistics` | 통계 수집 (디버그) | true |
| `ShrinkCheckInterval` | 축소 검사 주기 (초) | 30.0 |
| `bWarmUpOnStart` | 시작 시 예열 | true |
| `HiddenLocation` | 비활성 Actor 위치 | (0, 0, -100000) |

---

## 20. 스킬 트리 시스템

노드 기반 스킬 포인트 할당 시스템입니다.

### 20.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaSkillTreeSubsystem` | GameInstanceSubsystem으로 스킬 트리 관리 |
| `UHarmoniaSkillTreeData` | 스킬 트리 구조 데이터 에셋 |

### 20.2 노드 타입

| 타입 | 설명 |
|------|------|
| `Passive` | 패시브 능력치 보너스 |
| `Active` | 새로운 어빌리티 언락 |
| `Upgrade` | 기존 어빌리티 강화 |
| `Keystone` | 강력한 고유 효과 (트리당 1개만 선택 가능) |

### 20.3 빠른 시작

```cpp
UHarmoniaSkillTreeSubsystem* SkillTreeSubsystem = 
    GetGameInstance()->GetSubsystem<UHarmoniaSkillTreeSubsystem>();

// 스킬 트리 데이터 로드
SkillTreeSubsystem->LoadSkillTree(SkillTreeDataAsset);

// 노드 언락
if (SkillTreeSubsystem->CanUnlockNode(PlayerID, NodeID))
{
    SkillTreeSubsystem->UnlockNode(PlayerID, NodeID);
}

// 스킬 포인트 추가
SkillTreeSubsystem->AddSkillPoints(PlayerID, 5);

// 리스펙 (비용 지불)
SkillTreeSubsystem->RespecSkillTree(PlayerID, RespecCost);
```

---

## 21. 동적 난이도 조절 (DDA) 시스템

플레이어 성능을 분석하여 자동으로 게임 난이도를 조절합니다.

### 21.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaDynamicDifficultySubsystem` | DDA 메인 서브시스템 |
| `UHarmoniaDDAConfigDataAsset` | DDA 설정 데이터 에셋 |

### 21.2 추적 메트릭

| 메트릭 | 설명 |
|--------|------|
| 사망 횟수 | 총/시간당/연속 사망 |
| 승리 시 체력 | 전투 종료 시 남은 HP% |
| 패링/회피 성공률 | 방어 기술 숙련도 |
| 명중률 | 공격 정확도 |
| 피해 비율 | 준 피해 / 받은 피해 |

### 21.3 DDA 프로필

| 프로필 | 스킬 레이팅 | 설명 |
|--------|-------------|------|
| **Beginner** | 0-25 | 가장 쉬움, 높은 보조 |
| **Learning** | 26-45 | 쉬움, 적당한 보조 |
| **Standard** | 46-65 | 기본 (1.0x 배율) |
| **Skilled** | 66-85 | 어려움, 적 강화 |
| **Master** | 86-100 | 가장 어려움, 최대 도전 |

### 21.4 빠른 시작

#### Step 1: 데이터 에셋 생성
Content Browser → 우클릭 → Data Asset → `HarmoniaDDAConfigDataAsset` 선택

#### Step 2: 프로필 설정
```cpp
// 데이터 에셋에서 프로필 커스터마이즈
DDAConfig->BeginnerProfile.Parameters.EnemyDamageMultiplier = 0.5f;
DDAConfig->BeginnerProfile.Parameters.PlayerDefenseMultiplier = 1.5f;
```

#### Step 3: 서브시스템 초기화
```cpp
UHarmoniaDynamicDifficultySubsystem* DDASubsystem = 
    GetGameInstance()->GetSubsystem<UHarmoniaDynamicDifficultySubsystem>();

DDASubsystem->SetConfigDataAsset(DDAConfigAsset);
```

#### Step 4: 이벤트 기록
```cpp
// 플레이어 사망 시
DDASubsystem->RecordPlayerDeath(PlayerID);

// 전투 승리 시
DDASubsystem->RecordVictory(PlayerID, RemainingHealthPercent, CombatDuration);

// 패링 시도
DDASubsystem->RecordParryAttempt(PlayerID, bSuccess, bPerfect);
```

#### Step 5: 현재 파라미터 적용
```cpp
FHarmoniaDDAParameters Params = DDASubsystem->GetCurrentParameters(PlayerID);

// 적 스폰 시 적용
Enemy->SetHealthMultiplier(Params.EnemyHealthMultiplier);
Enemy->SetDamageMultiplier(Params.EnemyDamageMultiplier);
```

---

## 22. 미니맵 및 월드 마커 시스템

실시간 미니맵 렌더링과 3D 월드 마커를 제공합니다.

### 22.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaMinimapComponent` | Scene Capture 기반 미니맵 |
| `UHarmoniaWorldMarkerComponent` | 3D→2D 월드 마커 |
| `UHarmoniaUIConfigDataAsset` | UI 색상/설정 데이터 에셋 |

### 22.2 미니맵 설정

#### Step 1: 컴포넌트 추가
```cpp
UPROPERTY(VisibleAnywhere)
UHarmoniaMinimapComponent* MinimapComp;

// 생성자
MinimapComp = CreateDefaultSubobject<UHarmoniaMinimapComponent>(TEXT("Minimap"));
```

#### Step 2: 아이콘 추가
```cpp
// Actor 추적 아이콘
FGuid IconID = MinimapComp->AddActorIcon(Enemy, EnemyIconTexture, FLinearColor::Red, true);

// 정적 위치 아이콘
MinimapComp->AddLocationIcon(TreasureLocation, TreasureIcon, FLinearColor::Yellow);
```

#### Step 3: 줌 제어
```cpp
MinimapComp->ZoomIn(0.5f);
MinimapComp->ZoomOut(0.5f);
MinimapComp->SetZoomLevel(2.0f);
```

### 22.3 월드 마커 설정

```cpp
UPROPERTY(VisibleAnywhere)
UHarmoniaWorldMarkerComponent* WorldMarkerComp;

// 퀘스트 마커 추가
FGuid MarkerID = WorldMarkerComp->AddQuestMarker(ObjectiveLocation, 
    NSLOCTEXT("Quest", "Obj1", "마을로 이동"), true);

// 액터 추적 마커
WorldMarkerComp->AddActorMarker(TargetNPC, 
    NSLOCTEXT("NPC", "Name", "상인"), ShopIcon, FLinearColor::Green);
```

### 22.4 데이터 에셋 색상 설정

| 설정 그룹 | 항목 | 설명 |
|-----------|------|------|
| **MinimapColors** | `PlayerIconColor` | 플레이어 아이콘 |
| | `AllyIconColor` | 아군 아이콘 |
| | `EnemyIconColor` | 적 아이콘 |
| | `QuestObjectiveColor` | 퀘스트 목표 |
| **WorldMarkerColors** | `PrimaryQuestColor` | 주요 퀘스트 (골드) |
| | `SecondaryQuestColor` | 보조 퀘스트 (실버) |
| | `DangerColor` | 위험 지역 |

---

## 23. 버프/디버프 UI 시스템

GAS(Gameplay Ability System)와 통합된 상태 효과 UI 컴포넌트입니다.

### 23.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaStatusEffectComponent` | 상태 효과 추적 컴포넌트 |
| `UHarmoniaUIConfigDataAsset` | UI 색상/타이밍 설정 |

### 23.2 효과 타입

| 타입 | 색상 (기본) | 설명 |
|------|------------|------|
| `Buff` | 초록 | 긍정적 효과 |
| `Debuff` | 빨강 | 부정적 효과 |
| `Neutral` | 회색 | 중립 효과 |

### 23.3 효과 카테고리

| 카테고리 | 설명 |
|----------|------|
| `Combat` | 공격력, 방어력 등 |
| `Movement` | 이동 속도, 점프 등 |
| `Resource` | HP/MP 재생 등 |
| `Control` | 스턴, 슬로우 등 |
| `Elemental` | 화상, 빙결 등 |
| `Special` | 특수 효과 |

### 23.4 빠른 시작

#### Step 1: 데이터 에셋 설정
```cpp
// UHarmoniaUIConfigDataAsset 생성 후 설정
UIConfig->StatusEffectColors.BuffBorderColor = FLinearColor::Green;
UIConfig->StatusEffectColors.DebuffBorderColor = FLinearColor::Red;
UIConfig->StatusEffectTiming.ExpiringSoonThreshold = 5.0f;
```

#### Step 2: 컴포넌트 초기화
```cpp
UHarmoniaStatusEffectComponent* StatusComp = 
    Character->FindComponentByClass<UHarmoniaStatusEffectComponent>();

// 데이터 에셋 적용
StatusComp->SetUIConfigDataAsset(UIConfigAsset);

// GAS 연동 (자동으로 BeginPlay에서 시도)
StatusComp->InitializeWithASC(AbilitySystemComponent);
```

#### Step 3: 효과 설정 등록
```cpp
FHarmoniaStatusEffectConfig PoisonConfig;
PoisonConfig.EffectTag = FGameplayTag::RequestGameplayTag("Effect.Debuff.Poison");
PoisonConfig.DisplayName = NSLOCTEXT("Effect", "Poison", "중독");
PoisonConfig.EffectType = EHarmoniaStatusEffectType::Debuff;
PoisonConfig.Category = EHarmoniaStatusEffectCategory::Elemental;
PoisonConfig.Icon = PoisonIconTexture;

StatusComp->RegisterEffectConfig(PoisonConfig);
```

#### Step 4: UI에서 효과 표시
```cpp
// 모든 버프 가져오기
TArray<FHarmoniaStatusEffectUIData> Buffs = StatusComp->GetBuffs();

// 모든 디버프 가져오기
TArray<FHarmoniaStatusEffectUIData> Debuffs = StatusComp->GetDebuffs();

// UI 위젯에서 렌더링
for (const FHarmoniaStatusEffectUIData& Effect : Buffs)
{
    // Effect.IconTexture, Effect.RemainingDuration, Effect.StackCount 사용
}
```

#### Step 5: 이벤트 바인딩
```cpp
StatusComp->OnEffectAdded.AddDynamic(this, &UMyWidget::HandleEffectAdded);
StatusComp->OnEffectRemoved.AddDynamic(this, &UMyWidget::HandleEffectRemoved);
StatusComp->OnEffectExpiring.AddDynamic(this, &UMyWidget::HandleEffectExpiring);
```

---

## 24. 업데이트 레이트 최적화 시스템

거리 및 가시성 기반 업데이트 주기 관리 시스템입니다.

### 24.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaUpdateRateSubsystem` | 월드 서브시스템 |
| `FHarmoniaUpdateRateConfig` | 액터별 설정 |
| `EHarmoniaUpdateTier` | 업데이트 티어 |

### 24.2 업데이트 티어

| 티어 | 거리 | Tick 간격 | 설명 |
|------|------|----------|------|
| `Critical` | 0-10m | 매 프레임 | 플레이어, 전투 중 |
| `High` | 10-25m | 0.016s | 근거리 |
| `Medium` | 25-50m | 0.033s | 중거리 |
| `Low` | 50-100m | 0.1s | 원거리 |
| `Minimal` | 100-200m | 0.5s | 최소 업데이트 |
| `Dormant` | 200m+ | 중지 | 화면 밖 |

### 24.3 빠른 시작

```cpp
// 액터 등록
UHarmoniaUpdateRateSubsystem* Subsystem = GetWorld()->GetSubsystem<UHarmoniaUpdateRateSubsystem>();

FHarmoniaUpdateRateConfig Config;
Config.BasePriority = 5;
Config.bUseVisibilityCheck = true;
Config.bUseCombatBoost = true;

Subsystem->RegisterActor(MyActor, Config);

// 전투 상태 설정
Subsystem->SetActorCombatState(MyActor, true);

// 티어 변경 이벤트
Subsystem->OnActorTierChanged.AddDynamic(this, &UMyClass::OnTierChanged);
```

---

## 25. AI 스로틀링 시스템

AI 업데이트 주기를 거리와 중요도에 따라 관리하여 CPU 사용량을 최적화합니다.

### 25.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaAIThrottleComponent` | AI 스로틀 컴포넌트 |
| `FHarmoniaAIThrottleConfig` | 스로틀 설정 |
| `EHarmoniaAIThrottleState` | 스로틀 상태 |

### 25.2 스로틀 상태

| 상태 | 설명 |
|------|------|
| `FullUpdate` | 모든 AI 시스템 활성 |
| `ReducedUpdate` | 인지 간격 증가, 동작 정상 |
| `MinimalUpdate` | 최소 인지, 단순화된 동작 |
| `Suspended` | AI 일시 정지 |

### 25.3 스로틀링 적용 대상

| 시스템 | FullUpdate | ReducedUpdate | MinimalUpdate | Suspended |
|--------|-----------|---------------|---------------|-----------|
| **인지** | 0.1초 | 0.5초 | 2.0초 | 비활성 |
| **행동 트리** | 매 프레임 | 0.1초 | 0.5초 | 일시 정지 |
| **경로 탐색** | 0.2초 | 1.0초 | 1.0초 | 정지 |
| **애니메이션** | 전체 | 렌더 시 | 몽타주만 | 비활성 |

### 25.4 빠른 시작

```cpp
// 컴포넌트 추가 (블루프린트 또는 C++)
UHarmoniaAIThrottleComponent* ThrottleComp = NewObject<UHarmoniaAIThrottleComponent>(AICharacter);
AICharacter->AddOwnedComponent(ThrottleComp);

// 설정
ThrottleComp->Config.bAutoRegisterWithUpdateRate = true;
ThrottleComp->Config.ReducedPerceptionInterval = 0.5f;
ThrottleComp->Config.bPauseBTWhenSuspended = true;

// 전투 상태 설정
ThrottleComp->SetInCombat(true);

// 일시적 부스트
ThrottleComp->BoostUpdateRate(5.0f); // 5초간 FullUpdate

// 상태 변경 이벤트
ThrottleComp->OnThrottleStateChanged.AddDynamic(this, &UMyClass::OnStateChanged);
```

### 25.5 UpdateRateSubsystem 연동

AI Throttle 컴포넌트는 자동으로 UpdateRateSubsystem과 연동됩니다:

```cpp
// 연동 설정
ThrottleComp->Config.bAutoRegisterWithUpdateRate = true;
ThrottleComp->Config.UpdateRateConfig.BasePriority = 3;
ThrottleComp->Config.UpdateRateConfig.bUseVisibilityCheck = true;

// UpdateRateSubsystem의 티어 변경이 자동으로 스로틀 상태에 반영됩니다.
// Critical/High → FullUpdate
// Medium → ReducedUpdate  
// Low/Minimal → MinimalUpdate
// Dormant → Suspended
```

---

## 26. 네트워크 최적화 시스템

거리 기반 네트워크 복제 주파수 관리 및 Dormancy 시스템입니다.

### 26.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaNetworkOptimizationComponent` | 네트워크 최적화 컴포넌트 |
| `FHarmoniaNetOptConfig` | 최적화 설정 |
| `EHarmoniaNetOptLevel` | 최적화 레벨 |

### 26.2 최적화 레벨

| 레벨 | 거리 | NetUpdateFrequency | 설명 |
|------|------|-------------------|------|
| `Critical` | 0-5m | 100Hz | 항상 복제 |
| `High` | 5-15m | 60Hz | 고주파 업데이트 |
| `Medium` | 15-30m | 30Hz | 중주파 업데이트 |
| `Low` | 30-60m | 10Hz | 저주파 업데이트 |
| `Minimal` | 60-100m | 5Hz | 최소 업데이트 |
| `Dormant` | 100m+ | Dormancy | 휴면 상태 |

### 26.3 기능

| 기능 | 설명 |
|------|------|
| **NetUpdateFrequency 조절** | 거리에 따른 자동 조정 |
| **Dormancy 관리** | 원거리 액터 휴면 처리 |
| **전투 상태 인식** | 전투 시 업데이트 우선순위 상승 |
| **Net Relevancy** | 커스텀 관련성 거리 설정 |

### 26.4 빠른 시작

```cpp
// 컴포넌트 추가
UHarmoniaNetworkOptimizationComponent* NetOptComp = 
    NewObject<UHarmoniaNetworkOptimizationComponent>(Actor);
Actor->AddOwnedComponent(NetOptComp);

// 설정
NetOptComp->Config.CriticalDistance = 500.0f;
NetOptComp->Config.HighDistance = 1500.0f;
NetOptComp->Config.bEnableDormancy = true;
NetOptComp->Config.DormancyDelay = 2.0f;

// 전투 상태 설정
NetOptComp->SetInCombat(true);

// 강제 네트워크 업데이트
NetOptComp->ForceNetUpdate();

// Dormancy 플러시
NetOptComp->FlushNetDormancy();

// 레벨 변경 이벤트
NetOptComp->OnLevelChanged.AddDynamic(this, &UMyClass::OnNetOptLevelChanged);
```

### 26.5 Dormancy 설정

```cpp
// Dormancy 설정
NetOptComp->Config.bEnableDormancy = true;
NetOptComp->Config.DormancyMode = DORM_DormantPartial;
NetOptComp->Config.DormancyDelay = 2.0f; // 조건 충족 후 2초 대기

// Dormancy 모드 옵션:
// - DORM_Awake: 항상 깨어 있음
// - DORM_DormantAll: 모든 연결에 대해 휴면
// - DORM_DormantPartial: 일부 연결에 대해 휴면
// - DORM_Initial: 초기 휴면 상태
```

### 26.6 Net Relevancy 최적화

```cpp
// 커스텀 관련성 거리 설정
NetOptComp->Config.bUseCustomRelevancy = true;
NetOptComp->Config.NetCullDistanceSquared = 225000000.0f; // 15000 units squared

// 이는 액터가 해당 거리 이상에서 클라이언트에 복제되지 않음을 의미합니다.
```

---

## 27. 제작 시스템

아이템 제작 및 레시피 관리 시스템입니다.

### 27.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaCraftingSubsystem` | 제작 시스템 서브시스템 |
| `UHarmoniaCraftingConfigDataAsset` | 제작 설정 데이터 에셋 |
| `FHarmoniaCraftingRecipe` | 제작 레시피 정의 |
| `FHarmoniaCraftingIngredient` | 재료 정의 |
| `EHarmoniaCraftingResult` | 제작 결과 열거형 |
| `EHarmoniaCraftingStation` | 제작대 타입 |

### 27.2 제작 결과

| 결과 | 설명 |
|------|------|
| `Success` | 제작 성공 |
| `HighQuality` | 고품질 제작 성공 |
| `Failure` | 제작 실패 |
| `InsufficientMaterials` | 재료 부족 |
| `InsufficientCurrency` | 통화 부족 |
| `InvalidRecipe` | 잘못된 레시피 |
| `StationNotAvailable` | 제작대 없음 |

### 27.3 기본 사용법

```cpp
// 서브시스템 가져오기
UHarmoniaCraftingSubsystem* CraftingSystem = 
    GameInstance->GetSubsystem<UHarmoniaCraftingSubsystem>();

// 설정 적용
UHarmoniaCraftingConfigDataAsset* Config = LoadObject<UHarmoniaCraftingConfigDataAsset>(...);
CraftingSystem->SetConfigDataAsset(Config);

// 레시피 등록
FHarmoniaCraftingRecipe Recipe;
Recipe.RecipeID = FName("IronSword");
Recipe.DisplayName = NSLOCTEXT("Crafting", "IronSword", "철 검");
Recipe.ResultItem = IronSwordClass;
Recipe.BaseSuccessRate = 0.9f;
Recipe.RequiredStation = EHarmoniaCraftingStation::Anvil;
CraftingSystem->RegisterRecipe(Recipe);

// 제작 시도
FHarmoniaCraftingResult Result = CraftingSystem->AttemptCraft(PlayerController, FName("IronSword"));
if (Result.ResultType == EHarmoniaCraftingResult::Success)
{
    // 제작 성공
}
```

### 27.4 제작 요구사항 확인

```cpp
// 제작 가능 여부 확인
FText FailReason;
if (CraftingSystem->CanCraft(PlayerController, RecipeID, FailReason))
{
    // 제작 가능
}

// 특정 제작대에서 가능한 레시피 목록
TArray<FHarmoniaCraftingRecipe> Recipes = 
    CraftingSystem->GetRecipesForStation(EHarmoniaCraftingStation::Anvil);

// 재료 확인
TArray<FHarmoniaCraftingIngredient> Required = 
    CraftingSystem->GetRequiredMaterials(RecipeID);
```

---

## 28. 상점 및 거래 시스템

NPC 상점, 구매/판매, 가격 시스템입니다.

### 28.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaShopSubsystem` | 상점 시스템 서브시스템 |
| `UHarmoniaShopConfigDataAsset` | 상점 설정 데이터 에셋 |
| `FHarmoniaShopDefinition` | 상점 정의 |
| `FHarmoniaShopItem` | 상점 아이템 |
| `EHarmoniaCurrencyType` | 통화 타입 |

### 28.2 통화 타입

| 타입 | 설명 |
|------|------|
| `Gold` | 기본 골드 |
| `Premium` | 프리미엄 재화 |
| `Honor` | 명예 포인트 |
| `Guild` | 길드 포인트 |
| `Event` | 이벤트 재화 |
| `Custom` | 커스텀 재화 |

### 28.3 기본 사용법

```cpp
// 서브시스템 가져오기
UHarmoniaShopSubsystem* ShopSystem = 
    GameInstance->GetSubsystem<UHarmoniaShopSubsystem>();

// 상점 등록
FHarmoniaShopDefinition Shop;
Shop.ShopID = FName("BlacksmithShop");
Shop.DisplayName = NSLOCTEXT("Shop", "Blacksmith", "대장장이");
Shop.BuyPriceModifier = 1.0f;  // 100% 가격
Shop.SellPriceModifier = 0.5f; // 50% 판매가
ShopSystem->RegisterShop(Shop);

// 상점 열기
ShopSystem->OpenShop(PlayerController, FName("BlacksmithShop"));

// 구매
FHarmoniaTransactionResult BuyResult = 
    ShopSystem->BuyItem(PlayerController, FName("BlacksmithShop"), ItemIndex, Quantity);

// 판매
FHarmoniaTransactionResult SellResult = 
    ShopSystem->SellItem(PlayerController, FName("BlacksmithShop"), ItemID, Quantity);
```

### 28.4 가격 시스템

```cpp
// 기본 가격 조회
int64 BasePrice = ShopSystem->GetItemPrice(ShopID, ItemIndex);

// 수정된 구매가 조회 (상점 배율 적용)
int64 BuyPrice = ShopSystem->GetBuyPrice(ShopID, ItemIndex);

// 판매가 조회
int64 SellPrice = ShopSystem->GetSellPrice(ShopID, ItemID);

// 통화 잔액 확인
int64 Gold = ShopSystem->GetCurrencyBalance(PlayerController, EHarmoniaCurrencyType::Gold);
```

---

## 29. 업적 시스템

업적 추적, 완료, 보상 시스템입니다.

### 29.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaAchievementSubsystem` | 업적 시스템 서브시스템 |
| `UHarmoniaAchievementConfigDataAsset` | 업적 설정 데이터 에셋 |
| `FHarmoniaAchievementDefinition` | 업적 정의 |
| `FHarmoniaAchievementProgress` | 업적 진행도 |
| `FHarmoniaAchievementMilestone` | 마일스톤 정의 |

### 29.2 업적 타입

| 타입 | 설명 |
|------|------|
| `Counter` | 카운터 기반 (몬스터 100마리 처치) |
| `Flag` | 플래그 기반 (보스 처치 여부) |
| `Collection` | 수집 기반 (아이템 세트 완성) |
| `Progression` | 진행 기반 (스토리 진행) |

### 29.3 기본 사용법

```cpp
// 서브시스템 가져오기
UHarmoniaAchievementSubsystem* AchievementSystem = 
    GameInstance->GetSubsystem<UHarmoniaAchievementSubsystem>();

// 업적 정의 등록
FHarmoniaAchievementDefinition Achievement;
Achievement.AchievementID = FName("MonsterSlayer100");
Achievement.DisplayName = NSLOCTEXT("Achievement", "MonsterSlayer100", "몬스터 슬레이어");
Achievement.Description = NSLOCTEXT("Achievement", "MonsterSlayer100Desc", "몬스터 100마리 처치");
Achievement.TargetValue = 100;
Achievement.Type = EHarmoniaAchievementType::Counter;
AchievementSystem->RegisterAchievement(Achievement);

// 진행도 업데이트
AchievementSystem->AddProgress(PlayerController, FName("MonsterSlayer100"), 1);

// 진행도 확인
FHarmoniaAchievementProgress Progress = 
    AchievementSystem->GetProgress(PlayerController, FName("MonsterSlayer100"));

// 완료 여부 확인
bool bCompleted = AchievementSystem->IsCompleted(PlayerController, FName("MonsterSlayer100"));
```

### 29.4 이벤트

```cpp
// 업적 완료 이벤트
AchievementSystem->OnAchievementUnlocked.AddDynamic(this, &UMyClass::OnAchievementCompleted);

// 진행도 변경 이벤트
AchievementSystem->OnProgressUpdated.AddDynamic(this, &UMyClass::OnProgressChanged);

void UMyClass::OnAchievementCompleted(APlayerController* Player, FName AchievementID)
{
    // 업적 완료 처리
}
```

---

## 30. 랜덤 아이템 생성 시스템

접두사/접미사, 희귀도, 스탯 롤링 시스템입니다.

### 30.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaRandomItemSubsystem` | 랜덤 아이템 생성 서브시스템 |
| `UHarmoniaRandomItemConfigDataAsset` | 설정 데이터 에셋 |
| `FHarmoniaGeneratedItem` | 생성된 아이템 |
| `FHarmoniaAffixDefinition` | 접사 정의 |
| `EHarmoniaItemRarity` | 아이템 희귀도 |

### 30.2 희귀도

| 희귀도 | 기본 가중치 | 접사 수 | 스탯 배율 |
|--------|------------|---------|----------|
| `Common` | 60 | 0-1 | 1.0x |
| `Uncommon` | 25 | 1-2 | 1.1x |
| `Rare` | 10 | 2-3 | 1.25x |
| `Epic` | 4 | 3-4 | 1.5x |
| `Legendary` | 1 | 4-5 | 2.0x |

### 30.3 기본 사용법

```cpp
// 서브시스템 가져오기
UHarmoniaRandomItemSubsystem* ItemGenSystem = 
    GameInstance->GetSubsystem<UHarmoniaRandomItemSubsystem>();

// 아이템 생성
FHarmoniaItemGenerationParams Params;
Params.ItemLevel = 50;
Params.MinRarity = EHarmoniaItemRarity::Uncommon;
Params.MagicFindBonus = 0.2f; // 20% 희귀 아이템 확률 증가

FHarmoniaGeneratedItem Item = ItemGenSystem->GenerateItem(Params);

// 특정 희귀도로 생성
FHarmoniaGeneratedItem RareItem = 
    ItemGenSystem->GenerateItemWithRarity(Params, EHarmoniaItemRarity::Rare);

// 스탯 재롤링
ItemGenSystem->RerollAffixes(Item);
```

### 30.4 접사 시스템

```cpp
// 접두사 등록
FHarmoniaAffixDefinition Prefix;
Prefix.AffixID = FName("Flaming");
Prefix.DisplayName = NSLOCTEXT("Affix", "Flaming", "불타는");
Prefix.bIsPrefix = true;
Prefix.AttributeBonuses.Add(FGameplayTag::RequestGameplayTag("Attribute.Fire"), 50.0f);
ItemGenSystem->RegisterAffix(Prefix);

// 접미사 등록
FHarmoniaAffixDefinition Suffix;
Suffix.AffixID = FName("OfTheGiant");
Suffix.DisplayName = NSLOCTEXT("Affix", "OfTheGiant", "거인의");
Suffix.bIsPrefix = false;
Suffix.AttributeBonuses.Add(FGameplayTag::RequestGameplayTag("Attribute.Strength"), 30.0f);
ItemGenSystem->RegisterAffix(Suffix);
```

---

## 31. 강화 및 마법부여 시스템

아이템 강화, 마법부여, 소켓 시스템입니다.

### 31.1 주요 클래스

| 클래스 | 설명 |
|--------|------|
| `UHarmoniaEnhancementSubsystem` | 강화 시스템 서브시스템 |
| `UHarmoniaEnhancementConfigDataAsset` | 설정 데이터 에셋 |
| `FHarmoniaEnhancementLevel` | 강화 레벨 정의 |
| `FHarmoniaEnchantmentDefinition` | 마법부여 정의 |
| `FHarmoniaItemEnhancementState` | 아이템 강화 상태 |
| `EHarmoniaEnhancementResult` | 강화 결과 열거형 |

### 31.2 강화 결과

| 결과 | 설명 |
|------|------|
| `Success` | 강화 성공 (+1 레벨) |
| `GreatSuccess` | 대성공 (+2 레벨) |
| `Failure` | 실패 (레벨 유지) |
| `Downgrade` | 하락 (-1 레벨) |
| `Destruction` | 파괴 (아이템 삭제) |
| `Protected` | 보호됨 (보호권 소모) |

### 31.3 강화 레벨별 확률 (기본값)

| 레벨 | 성공률 | 대성공 | 하락 | 파괴 | 스탯 배율 |
|------|--------|--------|------|------|----------|
| +1~+5 | 95%~75% | 10% | 0% | 0% | +3%/레벨 |
| +6~+10 | 70%~50% | 5% | 10%~30% | 0% | +5%/레벨 |
| +11~+15 | 45%~25% | 2% | 30% | 5%~13% | +7%/레벨 |

### 31.4 기본 사용법

```cpp
// 서브시스템 가져오기
UHarmoniaEnhancementSubsystem* EnhanceSystem = 
    GameInstance->GetSubsystem<UHarmoniaEnhancementSubsystem>();

// 아이템 상태 초기화
FGuid ItemID = FGuid::NewGuid();
FHarmoniaItemEnhancementState State = EnhanceSystem->InitializeItemState(2); // 2개 소켓

// 강화 시도
FHarmoniaEnhancementSessionResult Result = 
    EnhanceSystem->AttemptEnhance(PlayerController, ItemID, bUseProtection);

switch (Result.Result)
{
    case EHarmoniaEnhancementResult::Success:
        // 성공: +1 레벨
        break;
    case EHarmoniaEnhancementResult::Destruction:
        // 파괴: 아이템 삭제 처리
        break;
}

// 성공률 확인
float SuccessRate = EnhanceSystem->GetSuccessRate(ItemID);

// 필요 재료 확인
TArray<FHarmoniaEnhancementMaterial> Materials = 
    EnhanceSystem->GetRequiredMaterials(ItemID);
```

### 31.5 마법부여

```cpp
// 마법부여 등록
FHarmoniaEnchantmentDefinition Enchant;
Enchant.EnchantmentID = FName("FireEnchant");
Enchant.DisplayName = NSLOCTEXT("Enchant", "Fire", "화염 부여");
Enchant.Slot = EHarmoniaEnchantSlot::Primary;
Enchant.AttributeBonuses.Add(FGameplayTag::RequestGameplayTag("Damage.Fire"), 25.0f);
EnhanceSystem->RegisterEnchantment(Enchant);

// 마법부여 적용
EnhanceSystem->ApplyEnchantment(PlayerController, ItemID, FName("FireEnchant"), EHarmoniaEnchantSlot::Primary);

// 마법부여 제거
EnhanceSystem->RemoveEnchantment(PlayerController, ItemID, EHarmoniaEnchantSlot::Primary);

// 적용된 마법부여 확인
TMap<EHarmoniaEnchantSlot, FName> Applied = EnhanceSystem->GetAppliedEnchantments(ItemID);
```

### 31.6 소켓 및 보석

```cpp
// 소켓 추가
EnhanceSystem->AddSocket(PlayerController, ItemID);

// 보석 삽입
FGameplayTag GemTag = FGameplayTag::RequestGameplayTag("Gem.Ruby");
EnhanceSystem->InsertGem(PlayerController, ItemID, 0, GemTag); // 소켓 인덱스 0

// 보석 제거
EnhanceSystem->RemoveGem(PlayerController, ItemID, 0);
```

### 31.7 천장(Pity) 시스템

```cpp
// 천장 카운터 확인
int32 PityCounter = EnhanceSystem->GetPityCounter(ItemID);

// 천장 임계값 확인
int32 Threshold = EnhanceSystem->GetPityThreshold(CurrentLevel);

// 천장 활성화 여부
bool bPityActive = EnhanceSystem->IsPityActive(ItemID);
// 천장 활성화 시 100% 성공 보장
```

### 31.8 보호권

```cpp
// 보호권 적용
EnhanceSystem->ApplyProtection(PlayerController, ItemID, EHarmoniaProtectionType::AntiDowngrade);

// 보호권 개수 확인
int32 ProtectionCount = EnhanceSystem->GetProtectionCount(ItemID);

// 강화 시도 (보호권 사용)
EnhanceSystem->AttemptEnhance(PlayerController, ItemID, true); // bUseProtection = true
```

---

## 📝 라이선스

Copyright © 2025 Snow Game Studio.  
All rights reserved.

---

**제작:** Snow Game Studio  
**최종 업데이트:** 2025-01-15
