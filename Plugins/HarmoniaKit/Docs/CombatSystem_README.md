# HarmoniaKit Combat System

전투 시스템은 HarmoniaKit의 핵심 기능으로, Sense System의 멀티스레드 감지 기능과 Gameplay Ability System을 통합하여 고성능 전투 메커니즘을 제공합니다.

## 주요 기능

- **센스 기반 히트 감지**: Sense System의 멀티스레드 방식을 활용한 공격 판정
- **다양한 공격 형태**: Box, Sphere, Capsule, Line 등 다양한 트레이스 모양 지원
- **데미지 타입**: Instant, Duration(DoT), Explosion, Percentage 등
- **Attribute Set**: Health, Stamina, Attack Power, Defense 등 캐릭터 속성 (전투/비전투 모두 사용)
- **Gameplay Effect**: 데미지 적용, 버프/디버프 효과
- **Gameplay Cue**: 히트 이펙트, 사운드, 카메라 셰이크 등
- **애니메이션 통합**: Animation Notify/Notify State를 통한 공격 타이밍 제어

## 시스템 구조

```
Combat System
├── Attribute Set (HarmoniaAttributeSet)
│   ├── Health / MaxHealth
│   ├── Stamina / MaxStamina
│   ├── AttackPower
│   ├── Defense
│   ├── CriticalChance / CriticalDamage
│   └── MovementSpeed / AttackSpeed
│
├── Attack Component (HarmoniaSenseAttackComponent)
│   ├── Sense System 통합
│   ├── 히트 박스 관리 (Box/Sphere/Capsule/Line)
│   ├── 타겟 추적 및 중복 히트 방지
│   └── 데미지 적용 및 이펙트 트리거
│
├── Animation Notifies
│   ├── AnimNotify_HarmoniaAttackCheck (단일 시점 체크)
│   └── AnimNotifyState_HarmoniaAttackWindow (지속 체크)
│
├── Gameplay Effects (Blueprint)
│   ├── GE_Damage_Instant (즉시 데미지)
│   ├── GE_Damage_Duration (DoT)
│   └── GE_Damage_Explosion (폭발)
│
└── Gameplay Cues (Blueprint)
    ├── GC_Hit_Physical (물리 타격)
    ├── GC_Hit_Critical (크리티컬)
    └── GC_Hit_Blocked (방어)
```

## 사용 방법

### 1. Attribute Set 설정

캐릭터에 `HarmoniaAttributeSet`을 추가:

```cpp
// YourCharacter.h
UPROPERTY()
TObjectPtr<UHarmoniaAttributeSet> CombatAttributeSet;

// YourCharacter.cpp
void AYourCharacter::InitializeAttributes()
{
    if (AbilitySystemComponent)
    {
        CombatAttributeSet = AbilitySystemComponent->GetSet<UHarmoniaAttributeSet>();
        if (!CombatAttributeSet)
        {
            CombatAttributeSet = AbilitySystemComponent->AddSet<UHarmoniaAttributeSet>();
        }
    }
}
```

### 2. Attack Component 설정

무기 또는 캐릭터에 `HarmoniaSenseAttackComponent` 추가:

**Blueprint 예시:**
1. 무기 Blueprint를 열기
2. "Add Component" → "Harmonia Sense Attack Component"
3. 컴포넌트를 무기의 적절한 위치(칼날 끝 등)에 배치
4. Details 패널에서 Attack Data 설정:

```
Attack Data
├── Trace Config
│   ├── Trace Shape: Sphere (또는 Box, Capsule)
│   ├── Trace Extent: (50, 50, 50) - 히트 박스 크기
│   ├── Socket Name: "WeaponTip" - 소켓 이름
│   ├── Continuous Detection: false
│   ├── Max Targets: 5
│   └── Show Debug Trace: true (디버깅용)
│
├── Damage Config
│   ├── Damage Type: Instant
│   ├── Base Damage: 20.0
│   ├── Damage Multiplier: 1.0
│   ├── Can Critical: true
│   ├── Critical Chance: 0.1
│   └── Critical Multiplier: 2.0
│
└── Hit Reaction Config
    ├── Reaction Type: Medium
    ├── Gameplay Cue Tag: GameplayCue.Hit.Physical
    └── Impact Force: 1000.0
```

### 3. Animation Notify 설정

공격 애니메이션 몽타주에 Notify 추가:

**단일 시점 체크 (빠른 공격):**
1. Animation Montage 열기
2. 공격이 타격하는 프레임에 우클릭
3. "Add Notify" → "Harmonia Attack Check"
4. Notify 설정:
   - Attack Component Name: 비워두면 자동 검색
   - Use Custom Attack Data: false (컴포넌트 기본값 사용)

**지속 윈도우 (휘두르기 공격):**
1. Animation Montage 열기
2. 공격 시작 프레임에 우클릭
3. "Add Notify State" → "Harmonia Attack Window"
4. 공격 종료 프레임까지 드래그하여 윈도우 길이 설정
5. Notify State 설정:
   - Clear Hit Targets On Start: true
   - Use Custom Attack Data: false

### 4. Gameplay Effect 생성 (Blueprint)

**즉시 데미지 Effect:**
1. Content Browser에서 우클릭
2. "Gameplay" → "Gameplay Effect"
3. 이름: `GE_Damage_Instant`
4. Effect 설정:
   ```
   Duration Policy: Instant

   Modifiers:
   ├── Attribute: HarmoniaAttributeSet.Damage
   ├── Modifier Op: Additive
   ├── Modifier Magnitude
   │   └── Set By Caller Magnitude
   │       └── Data Tag: Data.Damage
   ```

**참고**: HarmoniaAttributeSet은 전투와 비전투 상황 모두에서 사용됩니다.
식사로 체력 회복, 휴식으로 스태미나 회복 등에도 동일한 Attribute Set을 사용합니다.

**DoT Effect:**
1. 새 Gameplay Effect 생성: `GE_Damage_Duration`
2. Effect 설정:
   ```
   Duration Policy: Has Duration
   Duration Magnitude: 5.0 seconds

   Period: 1.0 seconds (틱 간격)
   Execute Periodic Effect on Application: true

   Modifiers:
   ├── Attribute: HarmoniaAttributeSet.Damage
   ├── Modifier Op: Additive
   └── Modifier Magnitude
       └── Set By Caller Magnitude
           └── Data Tag: Data.Damage
   ```

### 5. Gameplay Cue 생성 (Blueprint)

1. Content Browser에서 우클릭
2. "Gameplay" → "Gameplay Cue Notify Actor" (또는 Static)
3. 이름: `GC_Hit_Physical`
4. Gameplay Cue Tag: `GameplayCue.Hit.Physical`
5. On Execute 이벤트에서 파티클, 사운드 재생:

```blueprint
On Execute:
├── Spawn Emitter at Location (Parameters.Location)
│   └── Template: HitSpark_VFX
├── Play Sound at Location (Parameters.Location)
│   └── Sound: HitImpact_SFX
└── Apply Camera Shake (Parameters.Instigator)
    └── Shake Class: CameraShake_Hit
```

### 6. Combo Attack Ability 설정

1. `HarmoniaGameplayAbility_ComboAttack` Blueprint 생성
2. Combo Data Table 생성 (Row: `FComboAttackData`)
3. Ability 설정:
   ```
   Combo Data Table: DT_MeleeCombo
   Combo Sequence: [Combo1, Combo2, Combo3, Combo4]
   Attack Component Name: "WeaponAttack"
   Trigger Attack Component: true
   ```

4. Data Table 예시 (`DT_MeleeCombo`):
   ```
   Row Name: Combo1
   ├── Display Name: "Quick Slash"
   ├── Attack Montage: AM_Slash_01
   ├── Damage Multiplier: 1.0
   ├── Combo Window Duration: 0.8
   └── Combo Tag: Ability.Attack.Melee.Combo1

   Row Name: Combo2
   ├── Display Name: "Power Strike"
   ├── Attack Montage: AM_Slash_02
   ├── Damage Multiplier: 1.5
   ├── Combo Window Duration: 0.8
   └── Combo Tag: Ability.Attack.Melee.Combo2
   ```

## 데미지 계산 흐름

```
1. Animation Notify 트리거
   ↓
2. HarmoniaSenseAttackComponent.StartAttack()
   ↓
3. Sense System이 멀티스레드로 주변 타겟 감지
   ↓
4. OnSenseDetected() 콜백 → ProcessHitTarget()
   ↓
5. 데미지 계산:
   - BaseDamage = AttackData.BaseDamage
   - FinalDamage = BaseDamage × DamageMultiplier × (AttackPower / 10)
   - Critical? FinalDamage × CriticalMultiplier
   - Defense 적용: FinalDamage - Defense
   ↓
6. Gameplay Effect 적용 (Target ASC)
   ↓
7. Attribute Set PostGameplayEffectExecute()
   - Damage → Health 변환
   - Delegate 브로드캐스트
   ↓
8. Gameplay Cue 실행
   - 파티클, 사운드, 카메라 셰이크
   ↓
9. Hit Reaction 적용
   - Physics Impulse
   - Hit Pause (선택)
```

## 데미지 타입 상세

### Instant Damage (즉시 데미지)
- 기본 근접/원거리 공격
- 단일 히트로 즉시 적용
- Defense로 감소 가능

### Duration Damage (DoT)
- 독, 화상, 출혈 등
- `DurationSeconds` 동안 지속
- `TickInterval` 마다 데미지 적용
- 총 데미지를 틱 수로 나누어 적용

### Explosion Damage (폭발)
- 범위 공격
- `ExplosionRadius` 내 모든 타겟
- 거리에 따른 Falloff 적용
- `FalloffExponent`로 감쇠 곡선 조절

### Percentage Damage (퍼센트)
- 최대 체력 기준 비율 데미지
- Defense 무시
- 보스전 메커니즘 등에 유용

## Gameplay Tags

### 데미지 타입
- `Damage.Type.Physical`
- `Damage.Type.Fire`
- `Damage.Type.Ice`
- `Damage.Type.Lightning`
- `Damage.Type.Poison`

### 데미지 수정자
- `Damage.Critical` - 크리티컬 히트
- `Damage.Blocked` - 방어됨

### 히트 리액션
- `HitReaction.Light` - 약한 타격
- `HitReaction.Medium` - 중간 타격
- `HitReaction.Heavy` - 강한 타격
- `HitReaction.Knockback` - 넉백
- `HitReaction.Stun` - 스턴

### Gameplay Cues
- `GameplayCue.Hit.Physical` - 물리 타격 이펙트
- `GameplayCue.Hit.Critical` - 크리티컬 이펙트
- `GameplayCue.Hit.Blocked` - 방어 이펙트

### Data Tags (SetByCaller)
- `Data.Damage` - 데미지 값 전달
- `Data.Healing` - 힐링 값 전달

## 고급 사용법

### Custom Damage Type 구현

```cpp
// Custom 데미지 타입 처리
if (DamageConfig.DamageType == EHarmoniaDamageType::Custom)
{
    // 사용자 정의 로직
    // 예: 현재 체력 비율에 따른 추가 데미지
    float HealthPercent = TargetCombatSet->GetHealth() / TargetCombatSet->GetMaxHealth();
    if (HealthPercent < 0.3f)
    {
        FinalDamage *= 1.5f; // Execute threshold
    }
}
```

### Multiple Attack Components

하나의 무기에 여러 히트 박스:

```cpp
// Blueprint
Components:
├── WeaponAttack_Tip (끝부분)
│   └── Attack Data: High damage, small radius
├── WeaponAttack_Blade (날 부분)
│   └── Attack Data: Medium damage, medium radius
└── WeaponAttack_Handle (손잡이)
    └── Attack Data: Low damage, large radius

// Animation Notify에서 각각 트리거
Notify "Attack Tip":
    Attack Component Name: "WeaponAttack_Tip"

Notify "Attack Blade":
    Attack Component Name: "WeaponAttack_Blade"
```

### Combo-Specific Damage Modifiers

```cpp
// FComboAttackData 확장
USTRUCT(BlueprintType)
struct FComboAttackData_Extended : public FComboAttackData
{
    GENERATED_BODY()

    // 이 콤보 전용 공격 데이터 재정의
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FHarmoniaAttackData AttackOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseAttackOverride = false;
};
```

### Hit Reaction Animations

타겟의 Gameplay Ability로 히트 리액션 애니메이션 재생:

```cpp
// Blueprint Ability: GA_HitReaction
On Gameplay Event (HitReaction.Medium):
├── Play Montage: AM_HitReaction_Medium
├── Add Gameplay Tag: State.HitStunned
└── Wait 0.5s → Remove Tag
```

## 성능 최적화

### Sense System 설정
- **Sensor Update Frequency**: 공격 중에만 높은 빈도 (60Hz)
- **Non-Combat**: 낮은 빈도 (10Hz) 또는 비활성화
- **Max Tracked Targets**: 필요한 최소값으로 제한 (5-10)

### Hit-Once-Per-Target
- `bHitOncePerTarget = true`로 중복 데미지 방지
- 대부분의 근접 공격에 권장

### Continuous vs Single-Shot
- **빠른 공격**: `bContinuousDetection = false`
- **휘두르기**: `bContinuousDetection = true`

## 디버깅

### Debug Visualization
```cpp
// Attack Component에서
bShowDebugTrace = true

// 콘솔 명령
ShowDebug AbilitySystem
ShowDebug Attributes
```

### Common Issues

**1. 공격이 히트하지 않음**
- Sense Stimulus가 타겟에 있는지 확인
- Sensor Tag가 일치하는지 확인
- Trace 범위가 충분한지 확인 (Debug Trace 활성화)
- Target이 Ability System을 가지고 있는지 확인

**2. 데미지가 적용되지 않음**
- Gameplay Effect Class가 설정되었는지 확인
- Attribute Set이 Target에 있는지 확인
- SetByCaller Tag가 올바른지 확인 (`Data.Damage`)

**3. Gameplay Cue가 재생되지 않음**
- Gameplay Cue Tag가 등록되었는지 확인
- Gameplay Cue Manager에 경로가 추가되었는지 확인
- Tag가 정확히 일치하는지 확인

## 예제 프로젝트 구조

```
Content/
├── Abilities/
│   ├── GA_MeleeAttack
│   └── GA_RangedAttack
├── Effects/
│   ├── GE_Damage_Instant
│   ├── GE_Damage_Fire
│   └── GE_Healing_Instant
├── Cues/
│   ├── GC_Hit_Physical
│   ├── GC_Hit_Critical
│   └── GC_Hit_Fire
├── DataTables/
│   └── DT_MeleeCombo
└── Weapons/
    ├── BP_Sword
    │   └── Component: SenseAttackComponent
    └── BP_Axe
        └── Component: SenseAttackComponent
```

## 참고 문서

- [Sense System Documentation](./SenseInteractionSystem.md)
- [Combo Attack System](./ComboAttackSystem_README.md)
- [Lyra Gameplay Ability System](https://docs.unrealengine.com/5.3/gameplay-ability-system-for-lyra-sample-game-in-unreal-engine/)

## License

Copyright 2025 Snow Game Studio.

## 히트 리액션 시스템

### 개요

히트 리액션 시스템은 캐릭터가 데미지를 받았을 때 자동으로 반응하는 시스템입니다.

### 주요 기능

- **방향별 애니메이션**: 피격 방향에 따라 다른 애니메이션 재생 (앞/뒤/왼쪽/오른쪽)
- **히트 스턴**: 일정 시간 동안 행동 불가 상태
- **이동 속도 감소**: 히트 리액션 중 이동 속도 조절
- **무적 프레임**: 연속 피격 방지를 위한 무적 시간
- **자동 트리거**: 데미지를 받으면 자동으로 활성화
- **인터럽트 지원**: 설정에 따라 현재 히트 리액션 중단 가능

### 사용 방법

#### 1. Hit Reaction Data Table 생성

```
Content Browser 우클릭
└── Miscellaneous → Data Table
    └── Row Structure: FHitReactionData
    └── 이름: DT_HitReactions
```

#### 2. Data Table 설정 예시

**Light (약한 피격)**
```
Row Name: Light
├── Display Name: "Light Hit"
├── Reaction Type: Light
├── Hit Montage Front: AM_HitReact_Light_Front
├── Hit Montage Back: AM_HitReact_Light_Back
├── Hit Montage Left: AM_HitReact_Light_Left
├── Hit Montage Right: AM_HitReact_Light_Right
├── Stun Duration: 0.2
├── Can Be Interrupted: true
├── Movement Speed Multiplier: 0.8
├── Disable Input: false
└── Applied Tags: [State.HitReaction, State.HitStunned]
```

**Medium (중간 피격)**
```
Row Name: Medium
├── Display Name: "Medium Hit"
├── Reaction Type: Medium
├── Hit Montage Front: AM_HitReact_Medium_Front
├── Hit Montage Back: AM_HitReact_Medium_Back
├── Hit Montage Left: AM_HitReact_Medium_Left
├── Hit Montage Right: AM_HitReact_Medium_Right
├── Stun Duration: 0.4
├── Can Be Interrupted: true
├── Movement Speed Multiplier: 0.5
└── Disable Input: true
```

**Heavy (강한 피격)**
```
Row Name: Heavy
├── Display Name: "Heavy Hit"
├── Reaction Type: Heavy
├── Hit Montage Front: AM_HitReact_Heavy_Front
├── Hit Montage Back: AM_HitReact_Heavy_Back
├── Hit Montage Left: AM_HitReact_Heavy_Left
├── Hit Montage Right: AM_HitReact_Heavy_Right
├── Stun Duration: 0.8
├── Can Be Interrupted: false
├── Movement Speed Multiplier: 0.3
└── Disable Input: true
```

#### 3. Gameplay Ability 생성

```cpp
// Blueprint에서:
1. Content Browser 우클릭
2. Blueprint Class → HarmoniaGameplayAbility_HitReaction
3. 이름: GA_HitReaction

// 설정:
Hit Reaction Data Table: DT_HitReactions
Default Reaction Type: Light
Auto Detect Hit Direction: true
Direction Angle Threshold: 45.0
Allow Interruption: true
Minimum Interrupt Time: 0.1
Apply Invincibility Frames: true
Invincibility Duration: 0.5
```

#### 4. Ability System Component에 추가

```cpp
// C++ 코드
void AYourCharacter::GiveAbilities()
{
    if (AbilitySystemComponent)
    {
        // 히트 리액션 어빌리티 추가
        FGameplayAbilitySpec HitReactionSpec(
            UHarmoniaGameplayAbility_HitReaction::StaticClass(),
            1  // Level
        );
        AbilitySystemComponent->GiveAbility(HitReactionSpec);
    }
}
```

**또는 Blueprint:**
```
Event BeginPlay
└── Get Ability System Component
    └── Give Ability
        └── Ability: GA_HitReaction
        └── Level: 1
```

#### 5. 자동 트리거 설정

히트 리액션은 **자동으로 트리거됩니다**:
- 캐릭터가 데미지를 받으면 `HarmoniaAttributeSet`에서 자동으로 `GameplayEvent.HitReaction` 이벤트 발생
- 이벤트를 받은 어빌리티가 자동 활성화

**수동 트리거 (선택사항):**
```cpp
// C++에서 수동 트리거
FGameplayEventData EventData;
EventData.Instigator = AttackerActor;
EventData.EventMagnitude = static_cast<float>(EHarmoniaHitReactionType::Medium);

AbilitySystemComponent->HandleGameplayEvent(
    HarmoniaGameplayTags::GameplayEvent_HitReaction,
    &EventData
);
```

### 히트 방향 계산

시스템은 공격자 위치를 기반으로 자동으로 히트 방향을 계산합니다:

```
공격자 위치와 피격자의 Forward Vector 비교:

           Front (0-45°)
               ↑
               |
    Left ←----@----→ Right
    (45-135°)  |  (45-135°)
               |
               ↓
          Back (135-180°)
```

### 히트 리액션 타입별 사용 예시

#### Light - 약한 공격
- 빠른 근접 공격
- 화살
- 총알

```cpp
// Attack Component에서
DamageConfig.BaseDamage = 10.0f;
HitReactionConfig.ReactionType = EHarmoniaHitReactionType::Light;
```

#### Medium - 중간 공격
- 일반 근접 무기
- 마법 공격
- 폭발

```cpp
DamageConfig.BaseDamage = 25.0f;
HitReactionConfig.ReactionType = EHarmoniaHitReactionType::Medium;
```

#### Heavy - 강한 공격
- 대형 무기 (도끼, 해머)
- 보스 공격
- 크리티컬 히트

```cpp
DamageConfig.BaseDamage = 50.0f;
HitReactionConfig.ReactionType = EHarmoniaHitReactionType::Heavy;
```

#### Knockback - 넉백
- 밀어내는 공격
- 폭발
- 충격파

```cpp
DamageConfig.BaseDamage = 30.0f;
HitReactionConfig.ReactionType = EHarmoniaHitReactionType::Knockback;
HitReactionConfig.ImpactForce = 2000.0f;
```

#### Stun - 스턴
- 기절 공격
- 전기 공격
- 특수 스킬

```cpp
DamageConfig.BaseDamage = 15.0f;
HitReactionConfig.ReactionType = EHarmoniaHitReactionType::Stun;
// Stun Duration은 Data Table에서 설정
```

### 고급 설정

#### 무적 프레임 활용

연속 피격을 방지하기 위한 무적 시간:

```cpp
// GA_HitReaction Blueprint
Apply Invincibility Frames: true
Invincibility Duration: 0.5  // 0.5초 무적

// 이 시간 동안 State.Invincible 태그 적용
// 모든 공격이 이 태그를 체크하도록 설정 가능
```

#### 히트 리액션 인터럽트

```cpp
// 약한 히트는 강한 히트로 대체 가능
Allow Interruption: true
Minimum Interrupt Time: 0.1

// 예시: Light 히트 중 Heavy 히트를 받으면
// 0.1초 후부터 Heavy 히트로 전환됨
```

#### 입력 비활성화

```cpp
// Data Table에서
Disable Input: true

// 히트 리액션 중 플레이어 입력 무시
// 주로 Medium, Heavy에서 사용
```

### Gameplay Tags

히트 리액션 시스템은 다음 태그를 사용합니다:

**State Tags:**
- `State.HitReaction` - 히트 리액션 중
- `State.HitStunned` - 스턴 상태
- `State.Invincible` - 무적 상태

**Event Tags:**
- `GameplayEvent.HitReaction` - 히트 리액션 트리거

**사용 예시:**
```cpp
// 공격 어빌리티에서 히트 중인 적은 공격 불가
BlockedTags.AddTag(State_HitReaction);

// 무적 중에는 데미지 무시
if (TargetASC->HasMatchingGameplayTag(State_Invincible))
{
    return; // Skip damage
}
```

### 디버깅

**로그 활성화:**
```cpp
// HitReaction에서 로그 출력
UE_LOG(LogTemp, Log, TEXT("HitReaction: Performing %s reaction in direction %d"),
    *ReactionData.DisplayName.ToString(),
    static_cast<int32>(Direction));
```

**일반적인 문제:**

1. **히트 리액션이 재생되지 않음**
   - Ability가 ASC에 추가되었는지 확인
   - Data Table이 설정되었는지 확인
   - 해당 방향의 Montage가 있는지 확인

2. **방향이 잘못 계산됨**
   - `Direction Angle Threshold` 조정 (기본 45도)
   - Instigator 위치가 올바른지 확인

3. **무적이 작동하지 않음**
   - `Apply Invincibility Frames: true` 확인
   - 공격 시스템에서 `State.Invincible` 체크 구현

4. **연속 히트가 재생되지 않음**
   - `Allow Interruption: true` 확인
   - `Minimum Interrupt Time` 조정

### 예제 시나리오

#### 시나리오 1: 기본 근접 전투

```
1. 플레이어가 적을 공격
2. 데미지 적용 (25 damage)
3. HarmoniaAttributeSet에서 GameplayEvent.HitReaction 트리거
4. GA_HitReaction 자동 활성화
5. 피격 방향 계산 (Back)
6. AM_HitReact_Medium_Back 재생
7. 0.4초 스턴 적용
8. 애니메이션 종료 후 어빌리티 종료
```

#### 시나리오 2: 보스 강공격

```
1. 보스가 플레이어에게 강공격 (50 damage, Heavy)
2. HitReaction 트리거 (EventMagnitude = Heavy)
3. AM_HitReact_Heavy_Front 재생
4. 0.8초 스턴 + 입력 비활성화
5. Movement Speed 30%로 감소
6. 애니메이션 종료까지 인터럽트 불가
7. 종료 후 모든 효과 제거
```

### 성능 최적화

- **몽타주 미리 로드**: 자주 사용되는 히트 몽타주는 미리 로드
- **무적 프레임 최소화**: 필요한 만큼만 사용 (0.3-0.5초)
- **방향 계산 캐싱**: 동일 프레임에서 중복 계산 방지

