# HarmoniaKit Combat System

전투 시스템은 HarmoniaKit의 핵심 기능으로, Sense System의 멀티스레드 감지 기능과 Gameplay Ability System을 통합하여 고성능 전투 메커니즘을 제공합니다.

## 주요 기능

- **센스 기반 히트 감지**: Sense System의 멀티스레드 방식을 활용한 공격 판정
- **다양한 공격 형태**: Box, Sphere, Capsule, Line 등 다양한 트레이스 모양 지원
- **데미지 타입**: Instant, Duration(DoT), Explosion, Percentage 등
- **Attribute Set**: Health, Stamina, Attack Power, Defense 등 전투 속성
- **Gameplay Effect**: 데미지 적용, 버프/디버프 효과
- **Gameplay Cue**: 히트 이펙트, 사운드, 카메라 셰이크 등
- **애니메이션 통합**: Animation Notify/Notify State를 통한 공격 타이밍 제어

## 시스템 구조

```
Combat System
├── Attribute Set (HarmoniaCombatAttributeSet)
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

캐릭터에 `HarmoniaCombatAttributeSet`을 추가:

```cpp
// YourCharacter.h
UPROPERTY()
TObjectPtr<UHarmoniaCombatAttributeSet> CombatAttributeSet;

// YourCharacter.cpp
void AYourCharacter::InitializeAttributes()
{
    if (AbilitySystemComponent)
    {
        CombatAttributeSet = AbilitySystemComponent->GetSet<UHarmoniaCombatAttributeSet>();
        if (!CombatAttributeSet)
        {
            CombatAttributeSet = AbilitySystemComponent->AddSet<UHarmoniaCombatAttributeSet>();
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
   ├── Attribute: HarmoniaCombatAttributeSet.Damage
   ├── Modifier Op: Additive
   ├── Modifier Magnitude
   │   └── Set By Caller Magnitude
   │       └── Data Tag: Data.Damage
   ```

**DoT Effect:**
1. 새 Gameplay Effect 생성: `GE_Damage_Duration`
2. Effect 설정:
   ```
   Duration Policy: Has Duration
   Duration Magnitude: 5.0 seconds

   Period: 1.0 seconds (틱 간격)
   Execute Periodic Effect on Application: true

   Modifiers:
   ├── Attribute: HarmoniaCombatAttributeSet.Damage
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
