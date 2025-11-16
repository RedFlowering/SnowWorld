# HarmoniaKit 콤보 공격 시스템 설정 가이드

## 개요
HarmoniaKit의 콤보 공격 시스템은 Gameplay Ability System(GAS)을 사용하여 순차적인 콤보 공격을 구현합니다. 플레이어가 공격 버튼을 연속으로 누르면 콤보가 이어지고, 일정 시간 동안 입력이 없으면 콤보가 초기화됩니다.

## 시스템 구성 요소

### 1. C++ 클래스
- **HarmoniaGameplayAbility_ComboAttack**: 콤보 공격 로직을 담당하는 GameplayAbility
- **HarmoniaGameplayTagsBFL**: DataTable 접근을 위한 Blueprint Function Library
- **FHarmoniaComboAttackData**: 콤보 공격 설정 데이터 구조체
- **FHarmoniaGameplayTagData**: Gameplay Tag 설정 데이터 구조체

### 2. 파일 위치
```
HarmoniaKit/
├── Source/HarmoniaKIt/
│   ├── Public/AbilitySystem/
│   │   ├── Abilities/
│   │   │   └── HarmoniaGameplayAbility_ComboAttack.h
│   │   ├── Definitions/
│   │   │   └── HarmoniaGameplayTagsDefinitions.h
│   │   └── HarmoniaGameplayTagsBFL.h
│   └── Private/AbilitySystem/
│       ├── Abilities/
│       │   └── HarmoniaGameplayAbility_ComboAttack.cpp
│       └── HarmoniaGameplayTagsBFL.cpp
```

## 설정 단계

### 1단계: GameplayTags 설정

먼저 프로젝트에 필요한 GameplayTag를 추가해야 합니다.

1. **프로젝트 설정에서 GameplayTag 추가**:
   - `프로젝트 설정 > GameplayTags > Gameplay Tag List`로 이동
   - 다음 태그들을 추가:
     ```
     Harmonia.Ability.Attack.Combo.0
     Harmonia.Ability.Attack.Combo.1
     Harmonia.Ability.Attack.Combo.2
     Harmonia.State.Attacking
     Harmonia.State.ComboWindow
     Harmonia.Input.Attack
     Data.Damage
     ```

### 2단계: DataTable 생성

#### A. GameplayTags DataTable
1. 콘텐츠 브라우저에서 우클릭 → `Miscellaneous > Data Table` 선택
2. Row Structure로 `HarmoniaGameplayTagData` 선택
3. 이름을 `DT_HarmoniaGameplayTags`로 지정
4. 경로: `/HarmoniaKit/DataTables/`에 저장

**예시 데이터**:
| Row Name | TagId | Tag | DisplayName | Description | Category |
|----------|-------|-----|-------------|-------------|----------|
| Attack | Attack | Harmonia.Input.Attack | 공격 | 기본 공격 입력 | Input |
| Attacking | Attacking | Harmonia.State.Attacking | 공격 중 | 공격 실행 상태 | State |
| ComboWindow | ComboWindow | Harmonia.State.ComboWindow | 콤보 윈도우 | 콤보 입력 대기 상태 | State |

#### B. Combo Attack DataTable
1. 콘텐츠 브라우저에서 우클릭 → `Miscellaneous > Data Table` 선택
2. Row Structure로 `HarmoniaComboAttackData` 선택
3. 이름을 `DT_ComboAttackData`로 지정
4. 경로: `/HarmoniaKit/DataTables/`에 저장

**예시 데이터** (3단계 콤보):
| Row Name | ComboIndex | ComboTag | AttackMontage | DamageMultiplier | ComboWindowDuration | ComboInputStartTime | ComboInputEndTime |
|----------|------------|----------|---------------|------------------|---------------------|---------------------|-------------------|
| Combo_0 | 0 | Harmonia.Ability.Attack.Combo.0 | AM_Attack_1 | 1.0 | 1.5 | 0.5 | 0.9 |
| Combo_1 | 1 | Harmonia.Ability.Attack.Combo.1 | AM_Attack_2 | 1.2 | 1.5 | 0.5 | 0.9 |
| Combo_2 | 2 | Harmonia.Ability.Attack.Combo.2 | AM_Attack_3 | 1.5 | 1.5 | 0.5 | 0.9 |

**필드 설명**:
- **ComboIndex**: 콤보 순서 (0부터 시작)
- **ComboTag**: 이 콤보 단계를 식별하는 태그
- **AttackMontage**: 재생할 공격 애니메이션 몽타주
- **DamageMultiplier**: 데미지 배율 (콤보가 진행될수록 증가)
- **ComboWindowDuration**: 콤보 입력 대기 시간 (초)
- **ComboInputStartTime**: 애니메이션에서 다음 콤보 입력을 받기 시작하는 시점 (0-1 정규화)
- **ComboInputEndTime**: 애니메이션에서 콤보 입력 윈도우가 닫히는 시점 (0-1 정규화)

#### C. DataTable 경로 설정
`HarmoniaGameplayTagsBFL.cpp` 파일에서 DataTable 경로를 업데이트하세요:
```cpp
// 28번째 줄 부근
const FString DataTablePath = TEXT("/HarmoniaKit/DataTables/DT_HarmoniaGameplayTags.DT_HarmoniaGameplayTags");

// 35번째 줄 부근
const FString DataTablePath = TEXT("/HarmoniaKit/DataTables/DT_ComboAttackData.DT_ComboAttackData");
```

### 3단계: Animation Montage 준비

각 콤보 단계마다 Animation Montage를 생성해야 합니다.

1. **콘텐츠 브라우저에서 공격 애니메이션 선택**
2. 우클릭 → `Create > Create AnimMontage` 선택
3. 이름 예시: `AM_Attack_1`, `AM_Attack_2`, `AM_Attack_3`
4. 각 몽타주를 열고:
   - 애니메이션 타이밍 조정
   - 필요시 Notify 추가 (타격 판정, 이펙트 등)

### 4단계: GameplayAbility Blueprint 생성

1. 콘텐츠 브라우저에서 우클릭 → `Blueprint Class` 선택
2. 부모 클래스로 `HarmoniaGameplayAbility_ComboAttack` 선택
3. 이름을 `GA_ComboAttack`로 지정
4. Blueprint를 열고 다음 설정:
   - **Activation Policy**: `On Input Triggered`
   - **Activation Group**: `Independent` 또는 원하는 그룹
   - **Ability Tags**: `Harmonia.Ability.Attack.Combo`
   - **Attacking Tag**: `Harmonia.State.Attacking`
   - **Combo Window Tag**: `Harmonia.State.ComboWindow`
   - **(선택사항) Attack Damage Effect**: 데미지를 적용할 GameplayEffect 클래스

### 5단계: Input Action 설정

Enhanced Input System을 사용하여 입력을 설정합니다.

#### A. Input Action 생성
1. 콘텐츠 브라우저에서 우클릭 → `Input > Input Action` 선택
2. 이름을 `IA_Attack`로 지정
3. Value Type을 `Digital (bool)`로 설정

#### B. Input Mapping Context에 추가
1. 기존 Input Mapping Context를 열거나 새로 생성
2. `IA_Attack` 액션을 추가하고 원하는 키를 매핑 (예: 마우스 왼쪽 버튼)

#### C. Input Config Data Asset 생성/수정
1. `LyraInputConfig` 타입의 Data Asset을 생성하거나 기존 것을 수정
2. **Ability Input Actions** 배열에 추가:
   - **Input Action**: `IA_Attack`
   - **Input Tag**: `Harmonia.Input.Attack`

### 6단계: 캐릭터에 Ability 부여

#### A. LyraPawnData에서 설정
가장 권장되는 방법은 LyraPawnData를 통해 어빌리티를 부여하는 것입니다.

1. 캐릭터가 사용하는 `LyraPawnData` 에셋을 엽니다
2. **Ability Sets** 배열에 어빌리티 추가:
   - Ability: `GA_ComboAttack`
   - Input Tag: `Harmonia.Input.Attack`

#### B. Blueprint에서 직접 부여 (대안)
캐릭터 Blueprint의 BeginPlay에서:
```
Event BeginPlay
  → Get Ability System Component
  → Give Ability (GA_ComboAttack)
     - Input Tag: Harmonia.Input.Attack
```

### 7단계: 테스트

1. 에디터에서 플레이 (PIE)
2. 공격 키를 눌러 첫 번째 공격 확인
3. 애니메이션이 진행되는 동안 다시 공격 키를 눌러 콤보 확인
4. 콤보 윈도우가 끝날 때까지 입력하지 않으면 콤보가 초기화되는지 확인

## 고급 설정

### 데미지 적용

콤보 공격에서 실제 데미지를 적용하려면:

1. **GameplayEffect 생성**:
   - 콘텐츠 브라우저에서 우클릭 → `Gameplay > Gameplay Effect` 선택
   - 이름을 `GE_ComboAttackDamage`로 지정

2. **GameplayEffect 설정**:
   - Duration Policy: `Instant`
   - Modifiers 추가:
     - Attribute: `LyraHealthSet.Health` (또는 사용하는 Health Attribute)
     - Modifier Op: `Add`
     - Modifier Magnitude: `Set By Caller`
       - Data Tag: `Data.Damage`

3. **GA_ComboAttack Blueprint에서**:
   - **Attack Damage Effect** 변수를 `GE_ComboAttackDamage`로 설정

### 콤보 확장

더 많은 콤보 단계를 추가하려면:
1. `DT_ComboAttackData`에 새 행 추가
2. ComboIndex를 순차적으로 증가 (3, 4, 5...)
3. 해당하는 Animation Montage 설정
4. GameplayTag 추가 (Harmonia.Ability.Attack.Combo.3, ...)

### 애니메이션 노티파이 활용

공격 타이밍을 정확하게 제어하려면:
1. Animation Montage에 `AnimNotify` 추가
2. 노티파이 이벤트를 처리하여:
   - 데미지 적용 타이밍 제어
   - 이펙트/사운드 재생
   - 히트 체크 실행

## 트러블슈팅

### 콤보가 연결되지 않음
- DataTable의 ComboWindowDuration이 충분히 긴지 확인
- ComboInputStartTime/EndTime이 적절한지 확인
- Input Tag가 올바르게 설정되었는지 확인

### 애니메이션이 재생되지 않음
- DataTable에 Animation Montage가 올바르게 설정되었는지 확인
- 캐릭터의 Skeletal Mesh와 Montage의 Skeleton이 일치하는지 확인

### 어빌리티가 활성화되지 않음
- LyraPawnData 또는 캐릭터에 어빌리티가 부여되었는지 확인
- Input Config에 Input Tag가 올바르게 설정되었는지 확인
- AbilitySystemComponent가 초기화되었는지 확인

### DataTable 경로 오류
- `HarmoniaGameplayTagsBFL.cpp`의 경로가 실제 에셋 경로와 일치하는지 확인
- 에셋 경로는 `/Game/` 대신 `/HarmoniaKit/`로 시작할 수 있습니다

## 참고 자료

- Unreal Engine Gameplay Ability System 문서
- Lyra 샘플 프로젝트
- HarmoniaKit 소스 코드:
  - `HarmoniaGameplayAbility_ComboAttack.h/cpp`
  - `HarmoniaGameplayTagsDefinitions.h`
  - `HarmoniaGameplayTagsBFL.h/cpp`
