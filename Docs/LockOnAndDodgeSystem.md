# Lock-On Targeting and Dodge Roll System

소울 라이크 전투 시스템의 핵심 기능인 락온 타게팅과 회피 구르기 시스템이 구현되었습니다.

## 구현된 기능

### 1. 락온 타게팅 시스템 (Lock-On Targeting)

적에게 시선을 고정하는 락온 시스템이 구현되었습니다.

#### 주요 기능:
- **자동 타겟 찾기**: 카메라 중심에서 가장 가까운 적을 자동으로 찾아 락온
- **타겟 전환**: 좌우로 타겟을 전환할 수 있는 기능
- **자동 카메라 회전**: 락온된 적을 향해 카메라가 자동으로 회전
- **거리 기반 해제**: 적이 너무 멀어지면 자동으로 락온 해제
- **시야 확인**: 장애물에 가려진 적은 타겟팅 불가

#### 구현 파일:
- `Source/LyraGame/Character/LockOnTargetingComponent.h`
- `Source/LyraGame/Character/LockOnTargetingComponent.cpp`

#### 설정 가능한 파라미터:
- `MaxLockOnDistance`: 락온 가능한 최대 거리 (기본값: 1500.0)
- `MaxLockOnAngle`: 초기 락온 가능한 최대 각도 (기본값: 60.0)
- `LockOnBreakDistance`: 락온 자동 해제 거리 (기본값: 2000.0)
- `CameraRotationSpeed`: 카메라 회전 속도 (기본값: 10.0)
- `TargetHeightOffset`: 타겟 높이 오프셋 (기본값: 100.0)
- `TargetableTag`: 타겟팅 가능한 액터의 태그 (기본값: "Enemy")

### 2. 회피 구르기 시스템 (Dodge Roll)

타이밍 기반 회피 메커니즘과 무적 프레임이 구현되었습니다.

#### 주요 기능:
- **방향 기반 회피**: 입력 방향으로 구르기 (입력 없으면 전방)
- **무적 프레임 (i-frames)**: 회피 중 일정 시간 동안 무적 상태
- **애니메이션 지원**: 회피 몽타주 재생 지원
- **스태미나 소모**: 회피 시 스태미나 소모 (설정 가능)
- **네트워크 지원**: 멀티플레이어 환경에서 작동

#### 구현 파일:
- `Source/LyraGame/AbilitySystem/Abilities/LyraGameplayAbility_Dodge.h`
- `Source/LyraGame/AbilitySystem/Abilities/LyraGameplayAbility_Dodge.cpp`

#### 설정 가능한 파라미터:
- `DodgeMontage`: 회피 애니메이션 몽타주
- `DodgeDistance`: 회피 거리 (기본값: 400.0)
- `DodgeDuration`: 회피 지속 시간 (기본값: 0.5)
- `InvincibilityDuration`: 무적 프레임 지속 시간 (기본값: 0.3)
- `InvincibilityEffect`: 무적 상태 GameplayEffect
- `StaminaCost`: 스태미나 소모량 (기본값: 20.0)

### 3. GameplayTag 정의

전투 시스템에 필요한 GameplayTag들이 정의되었습니다.

#### 구현 파일:
- `Source/LyraGame/Character/CombatGameplayTags.h`
- `Source/LyraGame/Character/CombatGameplayTags.cpp`

#### 정의된 태그:
- `Ability.LockOn`: 락온 어빌리티
- `Status.LockedOn`: 락온 상태
- `Ability.Dodge`: 회피 어빌리티
- `Status.Dodging`: 회피 중 상태
- `Status.Invincible`: 무적 상태
- `InputTag.LockOn`: 락온 입력
- `InputTag.Dodge`: 회피 입력
- `InputTag.SwitchTargetLeft`: 왼쪽 타겟 전환
- `InputTag.SwitchTargetRight`: 오른쪽 타겟 전환

## 설정 방법

### 1. 적 캐릭터 설정

락온 가능한 적에게 "Enemy" 태그를 추가해야 합니다:

1. 적 캐릭터 블루프린트를 엽니다
2. Details 패널에서 Tags 섹션을 찾습니다
3. "Enemy" 태그를 추가합니다

### 2. 입력 설정

Enhanced Input 시스템을 사용하여 입력을 설정해야 합니다:

1. Input Action 생성:
   - `IA_LockOn`: 락온 토글
   - `IA_Dodge`: 회피 구르기
   - `IA_SwitchTargetLeft`: 왼쪽 타겟 전환
   - `IA_SwitchTargetRight`: 오른쪽 타겟 전환

2. Input Mapping Context에 액션 추가:
   - 락온: 마우스 우클릭 또는 컨트롤러 L1
   - 회피: Space 또는 컨트롤러 Circle/B
   - 타겟 전환: 마우스 휠 또는 컨트롤러 스틱

3. InputConfig 데이터 애셋 설정:
   - 각 Input Action을 해당 InputTag와 연결

### 3. GameplayAbility 블루프린트 생성

#### Dodge Ability 블루프린트:

1. `LyraGameplayAbility_Dodge`를 부모로 하는 블루프린트 생성
2. 다음 설정:
   - Dodge Montage: 회피 애니메이션 몽타주 할당
   - Dodge Distance: 원하는 회피 거리 설정
   - Invincibility Duration: 무적 프레임 시간 설정
   - Invincibility Effect: 무적 GameplayEffect 할당

### 4. 무적 GameplayEffect 생성

1. GameplayEffect 블루프린트 생성 (이름: GE_DodgeInvincibility)
2. Duration Policy: Has Duration으로 설정
3. Duration Magnitude: 0.3초 (InvincibilityDuration과 동일하게)
4. Granted Tags에 `Status.Invincible` 추가
5. 이 태그를 가진 캐릭터는 데미지를 받지 않도록 데미지 시스템 수정 필요

### 5. 캐릭터에 Ability 부여

Experience 또는 AbilitySet에서 Dodge Ability를 캐릭터에 부여:

1. 해당 Experience/AbilitySet 열기
2. Abilities 배열에 GA_Dodge 블루프린트 추가
3. Input Tag를 `InputTag.Dodge`로 설정

## 사용 방법

### 게임 플레이:

1. **락온 활성화**:
   - 락온 키를 누르면 가장 가까운 적에게 락온
   - 카메라가 자동으로 적을 향함

2. **타겟 전환**:
   - 락온 중 타겟 전환 키로 다른 적으로 전환

3. **락온 해제**:
   - 락온 키를 다시 누르거나
   - 적이 너무 멀어지면 자동 해제

4. **회피 구르기**:
   - 회피 키를 누르면 현재 이동 방향으로 구르기
   - 회피 중 약 0.3초 동안 무적

### 블루프린트에서 사용:

```cpp
// 락온 컴포넌트 접근
ULockOnTargetingComponent* LockOn = Character->GetLockOnComponent();

// 락온 상태 확인
bool bIsLocked = LockOn->IsLockedOn();

// 현재 타겟 가져오기
AActor* Target = LockOn->GetCurrentTarget();

// 락온 토글
LockOn->ToggleLockOn();
```

## 추가 개선 사항

앞으로 추가하면 좋을 기능들:

1. **락온 UI**:
   - 락온된 적에게 표시되는 UI 마커
   - 타겟 전환 시 피드백

2. **회피 애니메이션**:
   - 4방향 회피 애니메이션
   - 카메라 효과

3. **스태미나 시스템**:
   - 회피 시 스태미나 소모
   - 스태미나 부족 시 회피 불가

4. **락온 카메라 모드**:
   - 전용 카메라 모드 추가
   - 락온 시 카메라 거리/각도 조정

5. **적 그룹 관리**:
   - 여러 적 그룹 관리
   - 보스 전용 락온 우선순위

## 문제 해결

### 락온이 작동하지 않는 경우:

1. 적에게 "Enemy" 태그가 있는지 확인
2. LockOnComponent가 BaseCharacter에 추가되었는지 확인
3. 적과의 거리가 MaxLockOnDistance 이내인지 확인

### 회피가 작동하지 않는 경우:

1. Dodge Ability가 캐릭터에 부여되었는지 확인
2. Input Tag가 올바르게 설정되었는지 확인
3. 캐릭터가 지상에 있는지 확인 (공중에서는 회피 불가)

### 무적 프레임이 작동하지 않는 경우:

1. InvincibilityEffect가 할당되었는지 확인
2. 데미지 시스템이 `Status.Invincible` 태그를 체크하도록 수정 필요
3. GameplayEffect의 Duration이 올바르게 설정되었는지 확인
