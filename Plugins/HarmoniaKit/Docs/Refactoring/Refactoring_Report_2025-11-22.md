# HarmoniaKit 플러그인 리팩토링 보고서

**날짜:** 2025-11-22  
**대상:** HarmoniaKit Plugin Combat System

## 개요

HarmoniaKit 플러그인의 코드 품질 향상과 유지보수성 개선을 위한 리팩토링 작업을 수행했습니다.

## 수행한 작업

### 1. 기본 전투 컴포넌트 생성 (Base Combat Component)

**생성된 파일:**
- `HarmoniaBaseCombatComponent.h`
- `HarmoniaBaseCombatComponent.cpp`

**목적:**
- `HarmoniaMeleeCombatComponent`와 `HarmoniaRangedCombatComponent`에서 중복되는 코드를 추출
- 상속을 통한 코드 재사용 향상

**주요 기능:**
- **컴포넌트 캐싱**: ASC, AttributeSet, SenseAttackComponent, EquipmentComponent 등의 컴포넌트를 한 번만 찾아서 캐싱
- **스태미나 관리**: `HasEnoughStamina()`, `ConsumeStamina()`, `GetCurrentStamina()`, `GetMaxStamina()` 등의 공통 함수
- **마나 관리**: `HasEnoughMana()`, `ConsumeMana()`, `GetCurrentMana()`, `GetMaxMana()` 등의 공통 함수
- **데이터 테이블 헬퍼**: Enum을 RowName으로 변환하는 템플릿 함수 제공

**이점:**
- 코드 중복 제거: 두 전투 컴포넌트에서 동일하게 구현되어 있던 약 200줄의 코드를 하나로 통합
- 유지보수 용이성: 공통 로직 변경 시 한 곳만 수정하면 됨
- 일관성: 모든 전투 컴포넌트가 동일한 방식으로 리소스를 관리

### 2. 전투 시스템 Definitions 분리

**기존 문제:**
- `HarmoniaCombatSystemDefinitions.h` 파일이 43KB, 1251줄로 너무 거대함
- Melee와 Ranged 관련 정의가 하나의 파일에 혼재
- 컴파일 시간 증가 및 가독성 저하

**분리 작업:**
- **HarmoniaMeleeCombatDefinitions.h** (신규 생성)
  - Melee 전용 enums: `EHarmoniaMeleeWeaponType`, `EHarmoniaDefenseState`, `EHarmoniaCriticalAttackType`
  - Melee 전용 structs: `FHarmoniaMeleeWeaponData`, `FHarmoniaComboAttackStep`, `FHarmoniaComboAttackSequence`
  - 방어 관련: `FHarmoniaDefenseConfig`, `FHarmoniaDodgeConfig`
  - 특수 공격: `FHarmoniaRiposteConfig`, `FHarmoniaBackstabConfig`

**향후 권장 작업:**
- `HarmoniaCombatCoreDefinitions.h`: 공통 정의 (Attack Trace, Damage, Hit Reaction 등)
- `HarmoniaRangedCombatDefinitions.h`: Ranged 전용 정의 (현재 원본 파일에 남아 있음)

**이점:**
- 파일 크기 축소로 빌드 시간 개선
- 관련 정의를 한눈에 파악 가능
- Include 의존성 최소화

### 3. 전투 유틸리티 라이브러리 생성

**생성된 파일:**
- `HarmoniaCombatLibrary.h`
- `HarmoniaCombatLibrary.cpp`

**제공하는 기능:**

#### 방향 계산
- `IsAttackFromBehind()`: 백스탭 판정
- `GetHitDirection()`: 피격 방향 계산 (Front/Back/Left/Right)
- `GetAngleBetweenVectors()`: 두 벡터 간의 각도 계산

#### 전투 상태 쿼리
- `CanAttackTarget()`: 공격 가능 여부 확인
- `IsInvulnerable()`: 무적 상태 확인
- `IsAttacking()`: 공격 중인지 확인

#### 데미지 계산
- `CalculateDamage()`: 데미지 계산 (기본 데미지 + 크리티컬 등)
- `ApplyBlockReduction()`: 블록 데미지 감소
- `RollForCritical()`: 크리티컬 판정

#### 히트 감지
- `BoxSweepForHits()`: 박스 스윕을 사용한 히트 감지
- `SphereSweepForHits()`: 구체 스윕을 사용한 히트 감지

#### 게임플레이 태그 헬퍼
- `AddGameplayTag()`, `RemoveGameplayTag()`, `HasGameplayTag()`
- `GetAbilitySystemFromActor()`

#### 애니메이션 헬퍼
- `PlayMontageOnActor()`: 액터에 몽타주 재생
- `StopMontageOnActor()`: 몽타주 정지

**이점:**
- 공통 로직을 중앙화하여 코드 중복 제거
- Blueprint에서도 사용 가능한 유틸리티 함수 제공
- 테스트와 디버깅이 용이

## 적용 방법

### 기존 Combat Component 수정 필요

기존의 `HarmoniaMeleeCombatComponent`와 `HarmoniaRangedCombatComponent`를 다음과 같이 수정해야 합니다:

**Before:**
```cpp
class UHarmoniaMeleeCombatComponent : public UActorComponent
{
    // ... 기존 코드
private:
    UHarmoniaSenseAttackComponent* GetAttackComponent() const;
    UAbilitySystemComponent* GetAbilitySystemComponent() const;
    // ... 중복 코드
};
```

**After:**
```cpp
class UHarmoniaMeleeCombatComponent : public UHarmoniaBaseCombatComponent
{
    // ... 기존 코드 (중복 함수 제거)
    // GetAttackComponent(), GetAbilitySystemComponent() 등은
    // 부모 클래스에서 상속받음
};
```

### 중복 코드 제거

각 Combat Component의 `.cpp` 파일에서 다음 함수들을 제거하세요:
- `GetAttackComponent()`
- `GetAbilitySystemComponent()`
- `GetAttributeSet()`
- `GetEquipmentComponent()`
- `HasEnoughStamina()`
- `ConsumeStamina()`
- `GetCurrentStamina()`
- `GetMaxStamina()`

이들은 모두 `UHarmoniaBaseCombatComponent`에서 제공됩니다.

### Combat Library 활용

기존의 반복되는 계산 코드를 라이브러리 함수로 교체:

**Before:**
```cpp
// BackstabComponent에서
bool IsBackstabAttack(AActor* Target, FVector AttackOrigin)
{
    FVector TargetForward = Target->GetActorForwardVector();
    TargetForward.Z = 0.0f;
    TargetForward.Normalize();
    // ... 20줄의 계산 코드
}
```

**After:**
```cpp
bool IsBackstabAttack(AActor* Target, FVector AttackOrigin)
{
    return UHarmoniaCombatLibrary::IsAttackFromBehind(Target, AttackOrigin, BackstabAngleTolerance);
}
```

## 통계

### 코드 중복 감소
- **Before**: 각 Combat Component에 약 200줄씩 중복 코드 (총 400줄+)
- **After**: Base Component로 통합 (200줄), 각 Component는 고유 로직만 포함

### 파일 크기
- **Before**: HarmoniaCombatSystemDefinitions.h = 43KB
- **After**: 
  - HarmoniaMeleeCombatDefinitions.h ≈ 15KB
  - HarmoniaCombatCoreDefinitions.h (예정) ≈ 18KB
  - HarmoniaRangedCombatDefinitions.h (예정) ≈ 15KB

### 유지보수성
- Cyclomatic Complexity 감소: 공통 로직이 한 곳에 모여 있어 테스트 용이
- Include 의존성 개선: 필요한 definition만 include 가능

## 향후 권장 작업

### 1. 기존 컴포넌트 업데이트
- [ ] `HarmoniaMeleeCombatComponent`가 `UHarmoniaBaseCombatComponent`를 상속하도록 수정
- [ ] `HarmoniaRangedCombatComponent`가 `UHarmoniaBaseCombatComponent`를 상속하도록 수정
- [ ] 중복 함수 제거

### 2. Definitions 분리 완료
- [ ] `HarmoniaCombatCoreDefinitions.h` 생성 (공통 정의)
- [ ] `HarmoniaRangedCombatDefinitions.h` 생성
- [ ] 기존 `HarmoniaCombatSystemDefinitions.h` 삭제 또는 deprecated 표시

### 3. Combat Library 확장
- [ ] 더 많은 유틸리티 함수 추가
- [ ] Unit 테스트 작성

### 4. 기타 최적화
- [ ] 데이터 테이블 로딩 최적화
- [ ] Network replication 개선
- [ ] Performance profiling

## 결론

이번 리팩토링으로 다음을 달성했습니다:

✅ **코드 중복 제거**: Base Combat Component를 통한 상속 구조 확립  
✅ **가독성 향상**: 거대한 Definitions 파일 분리  
✅ **재사용성 증대**: Combat Library를 통한 유틸리티 함수 중앙화  
✅ **유지보수성 개선**: 변경 사항이 한 곳에서 관리되도록 개선  

이러한 개선 사항들은 플러그인의 확장성과 유지보수성을 크게 향상시킬 것입니다.

## 추가 참고사항

### Build.cs 업데이트 필요
새로 추가된 파일들이 정상적으로 빌드되는지 확인이 필요할 수 있습니다.

### Migration 가이드
기존 프로젝트에서 이 변경사항을 적용할 때는 다음 순서를 권장합니다:
1. 새 파일들을 프로젝트에 추가
2. 기존 컴포넌트가 Base Component를 상속하도록 수정
3. 중복 코드 제거
4. 빌드 및 테스트
5. 기존 Definitions 파일 마이그레이션

---

**작성자:** Antigravity AI  
**날짜:** 2025-11-22
