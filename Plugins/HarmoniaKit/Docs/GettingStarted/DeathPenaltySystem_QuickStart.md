# Death Penalty System - Quick Start Guide

## 5분 안에 시작하기

### Step 1: 플레이어 캐릭터에 컴포넌트 추가

플레이어 캐릭터 Blueprint를 열고:

1. **Components 패널**에서 "Add Component" 클릭
2. **"Harmonia Currency Manager Component"** 검색 후 추가
3. **"Harmonia Death Penalty Component"** 검색 후 추가

### Step 2: Data Assets 생성

#### Currency Data Assets (4개 생성)

Content Browser에서:
1. 우클릭 → Miscellaneous → Data Asset
2. **UHarmoniaCurrencyDataAsset** 선택
3. 이름: `DA_MemoryEssence`

각 화폐별로 반복:
- `DA_MemoryEssence`
- `DA_SoulCrystals`
- `DA_ForgottenKnowledge`
- `DA_TimeFragments`

**각 Data Asset 설정:**

```
DA_MemoryEssence:
  Currency Type: Memory Essence
  Display Name: "기억의 정수"
  Description: "경험치와 레벨업에 사용됩니다"
  Currency Color: (0.5, 0.8, 1.0, 1.0) - 파란색
  Max Carry Amount: 0 (무제한)
  Can Drop On Death: true
  Drop Percentage: 1.0 (100%)

DA_SoulCrystals:
  Currency Type: Soul Crystals
  Display Name: "영혼 결정"
  Description: "장비 강화에 사용됩니다"
  Currency Color: (1.0, 0.3, 0.8, 1.0) - 분홍색
  Max Carry Amount: 0
  Can Drop On Death: true
  Drop Percentage: 0.75 (75%)

DA_ForgottenKnowledge:
  Currency Type: Forgotten Knowledge
  Display Name: "잊혀진 지식"
  Description: "스킬 포인트 획득에 사용됩니다"
  Currency Color: (1.0, 0.9, 0.3, 1.0) - 노란색
  Max Carry Amount: 999
  Can Drop On Death: true
  Drop Percentage: 0.5 (50%)

DA_TimeFragments:
  Currency Type: Time Fragments
  Display Name: "시간 파편"
  Description: "특수 아이템 구매에 사용됩니다"
  Currency Color: (0.8, 1.0, 0.8, 1.0) - 연두색
  Max Carry Amount: 100
  Can Drop On Death: true
  Drop Percentage: 1.0 (100%)
```

#### Death Penalty Config Asset

1. 우클릭 → Miscellaneous → Data Asset
2. **UHarmoniaDeathPenaltyConfigAsset** 선택
3. 이름: `DA_DeathPenaltyConfig_Default`

**기본 설정 사용 (이미 좋은 기본값이 설정되어 있음)**

### Step 3: Component 연결

플레이어 캐릭터 Blueprint에서:

#### Harmonia Death Penalty Component 설정:

1. Death Penalty Component 선택
2. Details 패널에서:
   - **Death Penalty Config**: `DA_DeathPenaltyConfig_Default` 할당
   - **Currency Data Assets**: Map에 4개 추가
     - Key: Memory Essence → Value: `DA_MemoryEssence`
     - Key: Soul Crystals → Value: `DA_SoulCrystals`
     - Key: Forgotten Knowledge → Value: `DA_ForgottenKnowledge`
     - Key: Time Fragments → Value: `DA_TimeFragments`
   - **Memory Echo Actor Class**: `BP_MemoryEchoActor` (아래 참조)

#### Harmonia Currency Manager Component 설정:

1. Currency Manager Component 선택
2. Details 패널에서:
   - **Currency Data Assets**: 위와 동일하게 4개 맵핑

### Step 4: Memory Echo Actor Blueprint 생성

1. Content Browser에서 우클릭 → Blueprint Class
2. **Actor** 선택
3. 이름: `BP_MemoryEchoActor`
4. 부모 클래스 변경:
   - Class Settings → Parent Class → **AHarmoniaMemoryEchoActor**

**BP_MemoryEchoActor 설정:**

Components:
- Mesh Component: 원하는 Static Mesh 설정 (예: Sphere)
- Effect Component: 파티클 시스템 할당 (선택사항)
- Ambient Sound: 사운드 할당 (선택사항)

### Step 5: 사망 이벤트 연결

플레이어 캐릭터 Blueprint Event Graph:

#### 체력 0 이벤트 처리

```
Event Graph:

[Event: Health Reaches Zero]
    |
    ├─ Get Actor Location
    |     |
    |     └─ [Variable] Death Location
    |
    └─ Handle Player Death (Library Function)
          ├─ Player Character: Self
          └─ Death Location: [Death Location]
```

또는 직접:

```
[Custom Event: OnDeath]
    |
    ├─ Get Death Penalty Component
    |     |
    |     └─ On Player Death
    |           ├─ Death Location: Get Actor Location
```

### Step 6: 화폐 회수 트리거

플레이어 Interaction 시스템에:

```
[Event: Player Press Interact Key]
    |
    ├─ Get Death Penalty Component
    |     |
    |     ├─ Has Active Memory Echo?
    |     |
    |     └─ Branch
    |           ├─ True:
    |           |    ├─ Get Distance To Memory Echo
    |           |    |
    |           |    └─ [< 200.0?] Branch
    |           |          ├─ True: Recover Currencies From Memory Echo
    |           |          |         |
    |           |          |         └─ [Success?] Print "Currencies Recovered!"
    |           |          |
    |           |          └─ False: Print "Too far from Memory Echo"
    |           |
    |           └─ False: (No memory echo)
```

### Step 7: 화폐 획득 테스트

적 처치 시:

```
[Event: On Enemy Killed]
    |
    └─ Add Currency (Library Function)
          ├─ Player Character: Self
          ├─ Currency Type: Memory Essence
          └─ Amount: 50
```

### Step 8: UI 연동 (선택사항)

#### 화폐 표시 Widget

```
[Event Construct]
    |
    └─ Get Currency Manager Component
          |
          └─ Bind Event to [On Currency Changed]
                |
                └─ [Event: Currency Changed]
                      |
                      └─ Update UI Text
                            ├─ Currency Type
                            ├─ New Amount
                            └─ Delta
```

#### 몽환 상태 표시

```
[Event Tick]
    |
    └─ Is Player In Ethereal State? (Library Function)
          |
          └─ Branch
                ├─ True: Show Ethereal Overlay Effect
                └─ False: Hide Overlay
```

#### 기억까지 거리 표시

```
[Event Tick]
    |
    └─ Get Distance To Memory Echo (Library Function)
          |
          └─ [> 0?] Branch
                ├─ True: Update Distance Text
                |         └─ Format: "Memory Echo: {0}m"
                |
                └─ False: Hide Distance Widget
```

## 테스트 시나리오

### 시나리오 1: 기본 사망 및 회수

1. **화폐 추가** (테스트용):
   ```
   Add Currency → Memory Essence → 1000
   ```

2. **플레이어 사망**:
   - Health를 0으로 설정
   - Memory Echo가 사망 위치에 생성됨
   - 플레이어는 몽환 상태로 부활

3. **화폐 회수**:
   - Memory Echo에 접근 (200 units 이내)
   - Interact 키 누름
   - 화폐 회수 성공!

### 시나리오 2: 이중 사망 (페널티)

1. **첫 번째 사망**:
   - Memory Essence: 1000 드롭
   - 몽환 상태 진입

2. **회수하지 않고 다시 사망**:
   - 이전 Memory Echo 소멸 (화폐 영구 손실)
   - 새로운 Memory Echo 생성
   - Max Health -10% 페널티

3. **안전지대에서 휴식**:
   - Reset All Penalties 호출
   - 모든 페널티 제거

### 시나리오 3: 빠른 회수 보너스

1. **사망**
2. **30초 이내에 회수**:
   - 원래 1000 → 회수 시 1200 (20% 보너스!)

### 시나리오 4: 시간 부패

1. **사망**
2. **5분 대기**:
   - Memory Essence: 1000 → 900 (10% 감소)
3. **10분 대기**:
   - 900 → 810 (추가 10% 감소)

## 디버그 팁

### 콘솔 명령어 (C++로 추가 구현 필요)

```cpp
// 화폐 추가 치트
UFUNCTION(Exec)
void AddCurrencyCheat(int32 Amount)
{
    UHarmoniaDeathPenaltyLibrary::AddCurrency(
        GetPawn(),
        EHarmoniaCurrencyType::MemoryEssence,
        Amount
    );
}

// 페널티 리셋
UFUNCTION(Exec)
void ResetPenalties()
{
    UHarmoniaDeathPenaltyLibrary::ResetDeathPenalties(GetPawn());
}
```

### Blueprint 디버그

1. **Print String 노드 사용**:
   - Currency Changed 이벤트에 Print
   - Death State Changed에 Print

2. **Details 패널 확인**:
   - Death Penalty Component → Current Death State
   - Currency Manager → Currency Amounts

## 다음 단계

1. **Memory Echo 비주얼 개선**:
   - 커스텀 파티클 이펙트
   - 맥동하는 애니메이션
   - 시간 부패에 따른 색상 변화

2. **UI 폴리싱**:
   - 화폐 아이콘 표시
   - 획득/손실 애니메이션
   - Mini-map에 Memory Echo 마커

3. **오디오**:
   - 화폐 획득 사운드
   - Memory Echo 주변 앰비언트 사운드
   - 몽환 상태 음악 변화

4. **밸런싱**:
   - 각 화폐 드롭률 조정
   - 시간 부패 속도 조정
   - 능력치 페널티 강도 조정

5. **확장 기능**:
   - 화폐 교환 NPC
   - 퀘스트 보상 통합
   - 멀티플레이어 기억 훔치기

## 문제 해결

**Q: Memory Echo가 생성되지 않습니다**
- Death Penalty Config가 할당되었는지 확인
- Memory Echo Actor Class가 설정되었는지 확인
- OnPlayerDeath()가 호출되는지 확인

**Q: 화폐가 회수되지 않습니다**
- 거리가 200 units 이내인지 확인
- BelongsToPlayer() 체크 확인
- Currency Manager Component가 있는지 확인

**Q: 몽환 상태 효과가 적용되지 않습니다**
- Ability System Component가 있는지 확인
- Gameplay Effect 구현 필요 (현재 플레이스홀더)

**Q: 네트워크에서 작동하지 않습니다**
- Server Authority 확인
- Replication 설정 확인
- RPCs가 제대로 호출되는지 확인

## 완성!

이제 독창적인 "기억의 메아리" 사망 페널티 시스템이 작동합니다! 🎉

더 자세한 내용은 `DeathPenaltySystem.md`를 참조하세요.
