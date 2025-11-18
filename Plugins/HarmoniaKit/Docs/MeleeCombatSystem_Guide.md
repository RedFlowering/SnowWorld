# Harmonia Kit - Melee Combat System Guide

## 개요

하모니아 키트의 근접 전투 시스템은 Dark Souls/Elden Ring 스타일의 소울라이크 전투를 제공합니다.

## 핵심 기능

### 1. 무기 시스템 (11종)
- **Sword** (칼): 밸런스 잡힌 무기
- **GreatSword** (대검): 느리지만 강력한 공격
- **Dagger** (단검): 빠르지만 약한 공격
- **Axe** (도끼): 높은 데미지, 느린 속도
- **Spear** (창): 긴 리치
- **Hammer** (망치): 높은 포이즈 데미지
- **Katana** (카타나): 빠른 연속 공격
- **Shield** (방패): 방어 중심
- **Fist** (주먹): 비무장 상태
- **Whip** (채찍): 독특한 사거리
- **Scythe** (낫): 넓은 휩쓸기 공격

### 2. 전투 시스템
- **라이트 공격**: 빠른 연속 공격, 콤보 체인
- **헤비 공격**: 느리지만 강력한 일격
- **블로킹**: 데미지 감소, 스태미나 소모
- **패리**: 정확한 타이밍으로 반격 기회 창출
- **회피**: I-프레임을 이용한 무적 상태

### 3. 스태미나 시스템
- 모든 행동이 스태미나 소모
- 스태미나 부족 시 가드 브레이크
- 자동 재생 시스템

## 설정 가이드

### 1단계: 데이터 테이블 생성

#### 무기 데이터 테이블

1. **Content Browser**에서 우클릭 → **Miscellaneous** → **Data Table**
2. Row Structure로 `HarmoniaMeleeWeaponData` 선택
3. 이름: `DT_MeleeWeapons`
4. CSV 임포트:
   - `Plugins/HarmoniaKit/Content/DataTables/DT_MeleeWeapons_Example.csv` 사용
   - 또는 수동으로 각 무기 데이터 입력

**무기 데이터 속성:**
```
- WeaponType: 무기 타입 (Sword, GreatSword 등)
- DisplayName: 표시 이름
- BaseDamageMultiplier: 기본 데미지 배수
- AttackSpeedMultiplier: 공격 속도 배수
- LightAttackStaminaCost: 라이트 공격 스태미나 소모
- HeavyAttackStaminaCost: 헤비 공격 스태미나 소모
- PoiseDamage: 포이즈 데미지 (경직력)
- bCanParry: 패리 가능 여부
- bCanBlock: 블로킹 가능 여부
- BlockDamageReduction: 블로킹 데미지 감소율 (0-1)
- BlockStaminaCost: 블로킹 시 스태미나 소모
- MaxComboChain: 최대 콤보 체인 수
- ComboWindowDuration: 콤보 입력 윈도우 시간
```

#### 콤보 시퀀스 데이터 테이블

1. **Content Browser**에서 우클릭 → **Miscellaneous** → **Data Table**
2. Row Structure로 `HarmoniaComboAttackSequence` 선택
3. 이름: `DT_ComboSequences`

**예시 콤보 데이터:**
```
Row Name: Sword_Light
- ComboName: "Sword Light Combo"
- WeaponType: Sword
- bIsHeavyCombo: false
- ComboSteps:
  [0]:
    - AttackMontage: AM_Sword_Light_1
    - DamageMultiplier: 1.0
    - StaminaCostMultiplier: 1.0
    - bCanBeCanceled: true
    - MinimumCancelTime: 0.2
  [1]:
    - AttackMontage: AM_Sword_Light_2
    - DamageMultiplier: 1.1
    - StaminaCostMultiplier: 1.0
    - bCanBeCanceled: true
    - MinimumCancelTime: 0.2
  [2]:
    - AttackMontage: AM_Sword_Light_3
    - DamageMultiplier: 1.3
    - StaminaCostMultiplier: 1.2
    - bCanBeCanceled: false
    - MinimumCancelTime: 0.3
```

### 2단계: 컴포넌트 설정

#### 캐릭터에 컴포넌트 추가

1. **HarmoniaMeleeCombatComponent** 추가
   - WeaponDataTable: `DT_MeleeWeapons` 설정
   - ComboSequencesDataTable: `DT_ComboSequences` 설정

2. **HarmoniaSenseAttackComponent** 추가
   - 히트박스 설정:
     - TraceShape: Box, Sphere, Capsule 중 선택
     - TraceExtent: 히트박스 크기
     - SocketName: 무기 소켓 이름 (예: `weapon_r`)

3. **HarmoniaEquipmentComponent** 추가 (선택사항)
   - 장비 시스템과 통합

### 3단계: 애니메이션 설정

#### 애니메이션 몽타주 생성

1. 각 공격별 애니메이션 몽타주 생성
2. 섹션 이름 설정: `Attack_1`, `Attack_2`, `Attack_3` 등

#### 애니메이션 노티파이 배치

##### 1회성 히트 체크 (빠른 공격)
```
- AnimNotify: Melee Attack Hit
- 위치: 무기가 타겟을 맞는 정확한 프레임
- 설정:
  - DamageMultiplier: 1.0 (기본값)
  - bIsCriticalHitPoint: false (스윗 스팟이면 true)
```

##### 지속 히트 체크 (휩쓸기 공격)
```
- AnimNotifyState: Harmonia Attack Window
- 시작: 무기 스윙 시작
- 끝: 무기 스윙 종료
- 설정:
  - bUseCustomAttackData: false (무기 기본값 사용)
  - bClearHitTargetsOnStart: true (중복 히트 방지)
```

##### 콤보 윈도우
```
- AnimNotifyState: Melee Combo Window
- 위치: 공격 애니메이션 후반부 (다음 공격 입력 받을 시점)
- 설정:
  - bAutoAdvanceCombo: true (자동으로 다음 콤보 진행)
```

### 4단계: Gameplay Abilities 설정

#### 블루프린트에서 어빌리티 부여

1. **Character Blueprint** 열기
2. **Ability System Component**에서:
   - `HarmoniaGameplayAbility_MeleeAttack` 추가 (라이트 공격용)
     - bIsHeavyAttack: false
   - `HarmoniaGameplayAbility_MeleeAttack` 추가 (헤비 공격용)
     - bIsHeavyAttack: true
   - `HarmoniaGameplayAbility_Block` 추가
   - `HarmoniaGameplayAbility_Parry` 추가
   - `HarmoniaGameplayAbility_Dodge` 추가

### 5단계: Input Mapping

#### Enhanced Input Actions 생성

1. **IA_LightAttack** (Input Action)
   - Value Type: Digital (bool)

2. **IA_HeavyAttack** (Input Action)
   - Value Type: Digital (bool)

3. **IA_Block** (Input Action)
   - Value Type: Digital (bool)

4. **IA_Parry** (Input Action)
   - Value Type: Digital (bool)

5. **IA_Dodge** (Input Action)
   - Value Type: Digital (bool)

#### Input Mapping Context

**IMC_MeleeCombat** 생성:
```
- IA_LightAttack: Mouse Left Button
- IA_HeavyAttack: Mouse Left Button (Hold > 0.3s)
- IA_Block: Mouse Right Button
- IA_Parry: Q
- IA_Dodge: Space
```

#### Character Blueprint에서 바인딩

```cpp
// Event Graph
Event IA_LightAttack -> Try Activate Ability by Class (HarmoniaGameplayAbility_MeleeAttack [Light])
Event IA_HeavyAttack -> Try Activate Ability by Class (HarmoniaGameplayAbility_MeleeAttack [Heavy])
Event IA_Block -> Try Activate Ability by Class (HarmoniaGameplayAbility_Block)
Event IA_Parry -> Try Activate Ability by Class (HarmoniaGameplayAbility_Parry)
Event IA_Dodge -> Try Activate Ability by Class (HarmoniaGameplayAbility_Dodge)
```

## 사용 예시

### C++에서 무기 변경
```cpp
UHarmoniaMeleeCombatComponent* MeleeComp = Character->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
if (MeleeComp)
{
    MeleeComp->SetCurrentWeaponType(EHarmoniaMeleeWeaponType::Katana);
}
```

### Blueprint에서 콤보 확인
```
Get Melee Combat Component
└─> Get Current Combo Index (returns int32)
└─> Get Max Combo Count (returns int32)
└─> Is In Combo Window (returns bool)
```

### 스태미나 체크
```
Get Melee Combat Component
└─> Get Current Stamina (returns float)
└─> Has Enough Stamina (Stamina Cost) (returns bool)
```

## 고급 기능

### 커스텀 히트 이펙트

1. **Gameplay Cue** 생성
2. **Melee Attack Ability**에서:
   - HitGameplayCueTag 설정
   - HitCameraShakeClass 설정

### 무기별 히트박스 커스터마이징

1. **AnimNotify_MeleeAttackHit**에서:
   - `bUseCustomAttackData = true` 설정
   - `CustomAttackData` 직접 설정

### 피격 반응 커스터마이징

1. **Hit Reaction Data Table** 생성
2. 피격 방향별 애니메이션 설정
3. **HitReactionConfig**에 반영

## 디버깅

### 히트박스 시각화

1. **HarmoniaSenseAttackComponent** 선택
2. **AttackData** → **TraceConfig** → **bShowDebugTrace = true**
3. PIE로 실행하면 히트박스가 화면에 표시됨

### 콘솔 커맨드

```
# 스태미나 무한
God

# 공격 디버그 로그
log LogTemp Verbose

# 애니메이션 디버그
showdebug animation
```

## 성능 최적화

### Sense System 최적화

- **MaxTargets**: 동시에 맞을 수 있는 최대 타겟 수 제한
- **bHitOncePerTarget**: 타겟당 1회만 히트 (중복 히트 방지)
- **MinimumSenseScore**: 히트 감지 임계값 (0-1)

### 애니메이션 최적화

- 불필요한 본 제거
- LOD 설정
- 애니메이션 압축

## 자주 묻는 질문 (FAQ)

**Q: 콤보가 연결되지 않습니다.**
A: AnimNotifyState_MeleeComboWindow가 올바르게 배치되었는지 확인하세요. 콤보 윈도우 시간이 너무 짧을 수 있습니다.

**Q: 히트 감지가 안 됩니다.**
A:
1. HarmoniaSenseAttackComponent가 추가되었는지 확인
2. 애니메이션 노티파이가 올바른 프레임에 있는지 확인
3. bShowDebugTrace로 히트박스 확인

**Q: 스태미나가 소모되지 않습니다.**
A: HarmoniaAttributeSet이 Ability System Component에 등록되었는지 확인하세요.

**Q: 패리가 작동하지 않습니다.**
A: 무기 데이터에서 bCanParry = true로 설정되었는지 확인하세요.

## 추가 리소스

- [Gameplay Ability System 문서](https://docs.unrealengine.com/en-US/gameplay-ability-system-for-unreal-engine/)
- [Enhanced Input 문서](https://docs.unrealengine.com/en-US/enhanced-input-in-unreal-engine/)
- [Animation Notify 문서](https://docs.unrealengine.com/en-US/animation-notifications-in-unreal-engine/)

## 라이선스

Copyright 2025 Snow Game Studio
