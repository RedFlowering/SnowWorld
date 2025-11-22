# Melee Combat System - Quick Start Guide

## 5분 안에 시작하기

### 1. Blueprint 캐릭터 생성 (1분)

1. Content Browser에서 우클릭
2. **Blueprint Class** → **HarmoniaMeleeCharacterExample** 선택
3. 이름: `BP_MyMeleeCharacter`

### 2. 데이터 테이블 임포트 (1분)

#### 무기 데이터 테이블

1. Content Browser에서 우클릭 → **Miscellaneous** → **Data Table**
2. Row Structure: `HarmoniaMeleeWeaponData`
3. 이름: `DT_MeleeWeapons`
4. 더블클릭하여 열기
5. 상단 메뉴 → **Import** → CSV 선택
6. 파일: `Plugins/HarmoniaKit/Content/DataTables/DT_MeleeWeapons_Example.csv`

### 3. 컴포넌트 설정 (2분)

`BP_MyMeleeCharacter` 열기:

#### 3.1 Melee Combat Component
- Details 패널에서 찾기
- **Weapon Data Table**: `DT_MeleeWeapons` 설정

#### 3.2 Main Hand Attack Component
- Details 패널에서 찾기
- **Attack Data** → **Trace Config** 설정:
  ```
  Trace Shape: Box
  Trace Extent: X=100, Y=50, Z=50
  Socket Name: weapon_r
  bHit Once Per Target: true
  Max Targets: 5
  ```

### 4. Input 설정 (1분)

#### Input Actions 생성

Content Browser → `Input` 폴더 생성

1. **IA_LightAttack** (Input Action)
2. **IA_HeavyAttack** (Input Action)
3. **IA_Block** (Input Action)
4. **IA_Dodge** (Input Action)

#### Input Mapping Context 생성

1. `IMC_MeleeCombat` (Input Mapping Context)
2. 더블클릭하여 열기
3. Mappings 추가:
   ```
   IA_LightAttack → Left Mouse Button
   IA_HeavyAttack → E (keyboard)
   IA_Block → Right Mouse Button
   IA_Dodge → Space
   ```

#### 캐릭터에 할당

`BP_MyMeleeCharacter`에서:
- **Melee Combat Mapping Context**: `IMC_MeleeCombat`
- **Light Attack Action**: `IA_LightAttack`
- **Heavy Attack Action**: `IA_HeavyAttack`
- **Block Action**: `IA_Block`
- **Dodge Action**: `IA_Dodge`

### 5. 완료! 테스트

1. 레벨에 `BP_MyMeleeCharacter` 배치
2. **Auto Possess Player**: Player 0 설정
3. **Play (PIE)**

**조작법:**
- 마우스 좌클릭: 라이트 공격
- E: 헤비 공격
- 마우스 우클릭: 블로킹
- Space: 회피

---

## 애니메이션 추가하기 (선택사항)

### 공격 애니메이션 설정

1. **Animation Montage** 생성
   - 이름: `AM_Sword_Light_1`

2. **Animation Notify** 추가
   - 무기가 타겟을 맞는 프레임에 우클릭
   - **Add Notify** → **Melee Attack Hit**
   - Settings:
     ```
     Damage Multiplier: 1.0
     bIs Critical Hit Point: false
     ```

3. **Combo Window Notify State** 추가
   - 공격 후반부에 우클릭
   - **Add Notify State** → **Melee Combo Window**
   - 길이: 약 0.5초

### 콤보 시퀀스 설정

1. **Data Table** 생성
   - Row Structure: `HarmoniaComboAttackSequence`
   - 이름: `DT_ComboSequences`

2. 행 추가: `Sword_Light`
   ```
   Combo Name: "Sword Light Combo"
   Weapon Type: Sword
   bIs Heavy Combo: false
   Combo Steps:
     [0]:
       Attack Montage: AM_Sword_Light_1
       Damage Multiplier: 1.0
       Stamina Cost Multiplier: 1.0
       bCan Be Canceled: true
   ```

3. `BP_MyMeleeCharacter`에서:
   - **Combo Sequences Data Table**: `DT_ComboSequences` 설정

---

## 히트박스 디버깅

### 시각화 켜기

1. `BP_MyMeleeCharacter` → **Main Hand Attack Component**
2. **Attack Data** → **Trace Config**
3. **bShow Debug Trace**: ✓ 체크

### PIE로 실행
- 공격 시 빨간색 박스가 표시됨
- 박스 크기와 위치 조정 가능

---

## 무기 전환

### Blueprint에서

Event Graph에서:
```
Event BeginPlay
└─> Switch Weapon (Katana)
```

또는 런타임에:
```
키 입력 (1)
└─> Switch Weapon (Sword)

키 입력 (2)
└─> Switch Weapon (Dagger)

키 입력 (3)
└─> Switch Weapon (GreatSword)
```

### C++에서

```cpp
AHarmoniaMeleeCharacterExample* Character = ...;
Character->SwitchWeapon(EHarmoniaMeleeWeaponType::Katana);
```

---

## 자주 발생하는 문제

### "공격이 안 됩니다"

**체크리스트:**
1. ✓ Input Mapping Context가 캐릭터에 할당되었나?
2. ✓ Ability Classes가 설정되었나?
3. ✓ AbilitySystemComponent가 있나?
4. ✓ HarmoniaSenseAttackComponent가 있나?

### "히트 감지가 안 됩니다"

**체크리스트:**
1. ✓ 애니메이션 노티파이가 올바른 프레임에 있나?
2. ✓ bShow Debug Trace로 히트박스 확인
3. ✓ 타겟에 콜리전이 있나?
4. ✓ Trace Channel 설정이 맞나?

### "스태미나가 소모되지 않습니다"

**체크리스트:**
1. ✓ HarmoniaAttributeSet이 ASC에 등록되었나?
2. ✓ 무기 데이터에 스태미나 코스트가 설정되었나?

---

## 다음 단계

- **애니메이션 추가**: 각 무기별 고유 모션
- **VFX/SFX 추가**: 히트 이펙트, 사운드
- **밸런싱**: 무기별 데미지, 스태미나 조정
- **적 AI**: 몬스터에게 동일 시스템 적용

## 도움말

전체 가이드: `Docs/MeleeCombatSystem_Guide.md`

이슈 리포팅: [GitHub Issues](https://github.com/your-repo/issues)
