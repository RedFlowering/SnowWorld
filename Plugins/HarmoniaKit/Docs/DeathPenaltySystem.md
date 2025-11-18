# Harmonia Death Penalty & Currency System

## 개요

Harmonia Kit의 사망 페널티 및 화폐 시스템은 Dark Souls/Elden Ring 스타일의 사망 메커니즘을 제공하되, **독창적인 "기억의 메아리(Memory Echo)"** 컨셉을 구현합니다.

## 핵심 기능

### 1. 다층 화폐 시스템 (Multi-layered Currency)

4가지 독특한 화폐 타입:

- **Memory Essence (기억의 정수)**: 경험치/레벨업 화폐
- **Soul Crystals (영혼 결정)**: 장비 강화 및 업그레이드
- **Forgotten Knowledge (잊혀진 지식)**: 스킬 포인트 획득
- **Time Fragments (시간 파편)**: 특수 아이템 구매

### 2. 몽환 상태 (Ethereal State)

사망 시 플레이어는 "몽환 상태"로 부활:
- 능력치 -30% (Health, Damage, Stamina Regen)
- 이동 속도 -15%
- 기억을 회수하면 정상 상태 복구

### 3. 시간 부패 (Time Decay)

떨어진 화폐는 시간이 지나면 서서히 감소:
- 5분마다 10% 감소
- 빨리 회수할수록 보너스 (30초 이내: +20%)
- 시각적 피드백: 메아리가 점점 희미해짐

### 4. 이중 사망 메커니즘

몽환 상태에서 다시 죽으면:
- 이전 기억은 화폐 타입별 설정에 따라 영구 손실
- 새로운 기억은 새 위치에 생성
- 추가 페널티: 최대 체력 -10% (안전지대에서 복구)

### 5. 기억 공명 (Memory Resonance) 🌟 독창적!

떨어진 기억 주변 20m 내 적들이 강화:
- 데미지 +10%
- 체력 +10%
- 플레이어의 전투 패턴을 "학습"한 상태
- 긴장감과 전략적 플레이 유도

## 구성 요소

### Components

#### UHarmoniaCurrencyManagerComponent
플레이어의 화폐 인벤토리 관리

```cpp
// 플레이어 캐릭터에 추가
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Currency")
UHarmoniaCurrencyManagerComponent* CurrencyManager;
```

**주요 기능:**
- `AddCurrency()` - 화폐 추가
- `RemoveCurrency()` - 화폐 제거
- `GetCurrencyAmount()` - 현재 보유량 조회
- `HasCurrency()` - 충분한 화폐 보유 확인
- `ConvertCurrency()` - 화폐 변환

#### UHarmoniaDeathPenaltyComponent
사망 페널티 및 회수 메커니즘 관리

```cpp
// 플레이어 캐릭터에 추가
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Death Penalty")
UHarmoniaDeathPenaltyComponent* DeathPenalty;
```

**주요 기능:**
- `OnPlayerDeath()` - 사망 처리
- `RecoverCurrenciesFromMemoryEcho()` - 화폐 회수
- `IsInEtherealState()` - 몽환 상태 확인
- `GetDistanceToMemoryEcho()` - 기억까지 거리
- `ResetAllPenalties()` - 페널티 초기화

### Actors

#### AHarmoniaMemoryEchoActor
플레이어가 사망 시 떨어뜨린 화폐를 나타내는 액터

**특징:**
- 시간 부패 시스템 자동 적용
- 주변 적 강화 (Memory Resonance)
- 네트워크 리플리케이션 지원
- 시각/오디오 효과 통합

### Data Assets

#### UHarmoniaCurrencyDataAsset
개별 화폐 타입 정의 (데이터 드리븐)

**설정 항목:**
- 표시 이름 및 아이콘
- 최대 소지량
- 사망 시 드롭 여부 및 비율
- 교환 비율
- VFX/SFX

#### UHarmoniaDeathPenaltyConfigAsset
사망 페널티 규칙 설정 (데이터 드리븐)

**설정 항목:**
- 화폐별 드롭 비율
- 능력치 페널티 배율
- 시간 부패 설정
- 기억 공명 설정
- 난이도 배율

### Blueprint Function Library

#### UHarmoniaDeathPenaltyLibrary
Blueprint에서 쉽게 사용할 수 있는 헬퍼 함수

```cpp
// 사망 처리
UHarmoniaDeathPenaltyLibrary::HandlePlayerDeath(PlayerCharacter, DeathLocation);

// 화폐 회수
bool bSuccess = UHarmoniaDeathPenaltyLibrary::RecoverCurrencies(PlayerCharacter);

// 화폐 추가
UHarmoniaDeathPenaltyLibrary::AddCurrency(PlayerCharacter, EHarmoniaCurrencyType::MemoryEssence, 100);
```

## 설정 방법

### 1. 플레이어 캐릭터 설정

플레이어 캐릭터 Blueprint에 다음 컴포넌트 추가:

```cpp
// C++
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Currency")
UHarmoniaCurrencyManagerComponent* CurrencyManager;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Death Penalty")
UHarmoniaDeathPenaltyComponent* DeathPenalty;
```

또는 Blueprint 에디터에서:
1. Components 패널에서 "Add Component"
2. "Harmonia Currency Manager Component" 추가
3. "Harmonia Death Penalty Component" 추가

### 2. Data Asset 생성

#### Currency Data Assets

각 화폐 타입별로 Data Asset 생성:

1. Content Browser에서 우클릭
2. Miscellaneous → Data Asset
3. `HarmoniaCurrencyDataAsset` 선택
4. 속성 설정:
   - Currency Type: MemoryEssence, SoulCrystals, etc.
   - Display Name: "기억의 정수"
   - Max Carry Amount: 999999 (또는 0 = 무제한)
   - Drop Percentage: 1.0 (100%)

#### Death Penalty Config Asset

1. Data Asset 생성: `HarmoniaDeathPenaltyConfigAsset`
2. 기본 설정 사용 또는 커스터마이징:
   - Difficulty Multiplier: 1.0 = 일반, 2.0 = 하드
   - Time Decay: 활성화/비활성화
   - Memory Resonance: 활성화/비활성화

### 3. Component 설정

Death Penalty Component 설정:
- Death Penalty Config: 생성한 Config Asset 할당
- Currency Data Assets: 화폐 타입별 Data Asset 맵핑
- Memory Echo Actor Class: BP_MemoryEchoActor (Blueprint 생성 필요)

Currency Manager Component 설정:
- Currency Data Assets: 화폐 타입별 Data Asset 맵핑

### 4. 사망 이벤트 연결

플레이어 캐릭터의 사망 이벤트에서:

```cpp
// C++
void AMyPlayerCharacter::OnDeath()
{
    if (DeathPenalty)
    {
        DeathPenalty->OnPlayerDeath(GetActorLocation());
    }
}
```

또는 Blueprint:
1. Health가 0이 되는 이벤트
2. "Handle Player Death" 노드 호출 (Library 함수)
3. Death Location = Actor Location

### 5. 화폐 회수 트리거

상호작용 시스템에서:

```cpp
// C++
void AMyPlayerCharacter::Interact()
{
    if (DeathPenalty && DeathPenalty->HasActiveMemoryEcho())
    {
        // 거리 체크
        if (DeathPenalty->GetDistanceToMemoryEcho() < 200.0f)
        {
            DeathPenalty->RecoverCurrenciesFromMemoryEcho();
        }
    }
}
```

## Blueprint 사용 예시

### 사망 처리

```
Event OnHealthZero
└─ Handle Player Death
   ├─ Player Character: Self
   └─ Death Location: GetActorLocation
```

### 화폐 획득

```
Event OnKillEnemy
└─ Add Currency
   ├─ Player Character: Self
   ├─ Currency Type: Memory Essence
   └─ Amount: 50
```

### 화폐 사용

```
Event OnPurchaseItem
├─ Has Currency?
│  ├─ Currency Type: Soul Crystals
│  └─ Amount: 100
└─ Branch
   ├─ True: Remove Currency
   │  ├─ Currency Type: Soul Crystals
   │  └─ Amount: 100
   └─ False: Show "Not Enough" Message
```

### 몽환 상태 UI

```
Event Tick
└─ Is Player In Ethereal State?
   └─ Branch
      ├─ True: Show Ethereal Effect
      └─ False: Hide Effect
```

## 데이터 드리븐 설정

### 화폐별 드롭 규칙 커스터마이징

Death Penalty Config에서 각 화폐별로:

```
Currency Drop Configs:
[0] Memory Essence
    - Drop Percentage: 1.0 (100% 드롭)
    - Permanent Loss: 1.0 (100% 영구 손실)

[1] Soul Crystals
    - Drop Percentage: 0.75 (75% 드롭)
    - Permanent Loss: 0.5 (50% 영구 손실)

[2] Forgotten Knowledge
    - Drop Percentage: 0.5 (50% 드롭)
    - Permanent Loss: 0.25 (25% 영구 손실)

[3] Time Fragments
    - Drop Percentage: 1.0 (100% 드롭)
    - Permanent Loss: 0.0 (절대 손실 안 됨!)
```

### 능력치 페널티 조정

```
Attribute Penalties:
- Health Multiplier: 0.7 (70% 체력)
- Damage Multiplier: 0.7 (70% 데미지)
- Stamina Regen Multiplier: 0.7 (70% 스태미나 회복)
- Movement Speed Multiplier: 0.85 (85% 이동 속도)
- Max Health Penalty Per Death: 0.1 (사망당 -10% 최대 체력)
- Max Health Penalty Stacks: 3 (최대 3회 누적)
```

### 시간 부패 설정

```
Time Decay Config:
- Enable: true
- Decay Start Time: 60.0 (1분 후 시작)
- Decay Interval: 300.0 (5분마다)
- Decay Percentage: 0.1 (10% 감소)
- Fast Recovery Window: 30.0 (30초 이내)
- Fast Recovery Bonus: 0.2 (+20% 보너스)
```

### 기억 공명 설정

```
Memory Resonance Config:
- Enable: true
- Resonance Radius: 2000.0 (20m)
- Enemy Damage Buff: 0.1 (+10% 데미지)
- Enemy Health Buff: 0.1 (+10% 체력)
- Effect Intensity: 0.5 (시각 효과 강도)
```

## 네트워크 멀티플레이어 지원

모든 컴포넌트와 액터는 네트워크 리플리케이션을 지원합니다:

- 화폐 변경사항 자동 동기화
- 기억 메아리 위치 공유
- 서버 검증된 트랜잭션

### 다른 플레이어 기억 훔치기 (옵션)

Config에서 활성화:
```
Allow Other Player Recovery: true
Other Player Recovery Percentage: 0.1 (10% 훔치기)
```

## 고급 기능

### GAS (Gameplay Ability System) 통합

Ethereal State 페널티는 Gameplay Effect로 적용 가능:
- Health, Damage, Stamina Regen 수정
- Movement Speed 버프/디버프
- 커스텀 Visual Effect

### 저장/로드 시스템

```cpp
// 저장
TMap<EHarmoniaCurrencyType, int32> SavedData = CurrencyManager->ExportCurrencyData();

// 로드
CurrencyManager->ImportCurrencyData(SavedData);
```

### 이벤트 바인딩

```cpp
// 화폐 변경 이벤트
CurrencyManager->OnCurrencyChanged.AddDynamic(this, &AMyClass::OnCurrencyChanged);

// 사망 상태 변경
DeathPenalty->OnDeathStateChanged.AddDynamic(this, &AMyClass::OnDeathStateChanged);

// 화폐 회수
DeathPenalty->OnCurrenciesRecovered.AddDynamic(this, &AMyClass::OnCurrenciesRecovered);
```

## 디버깅

로그 카테고리: `LogTemp` (추후 `LogHarmoniaDeathPenalty`로 분리 예정)

유용한 명령어:
```
// 화폐 추가 (치트)
// C++에서 직접 호출 또는 Console Command 구현

// 페널티 초기화
DeathPenalty->ResetAllPenalties();

// 기억 메아리 생성 (테스트)
// Memory Echo Actor를 월드에 직접 배치
```

## 성능 고려사항

- Memory Echo는 최대 수명 후 자동 소멸
- Resonance 업데이트는 1초 간격
- Time Decay는 설정된 간격(기본 5분)마다만 적용
- 적 버프는 범위 진입/이탈 시에만 갱신

## 향후 확장 가능성

- 추가 화폐 타입 (Custom enum)
- 화폐별 독립적인 드롭 위치
- 공간적 오디오 및 방향 UI
- 기억 메아리 상호작용 애니메이션
- 화폐 교환 NPC 시스템
- 퀘스트 보상 통합

## 참고사항

이 시스템은 **완전히 데이터 드리븐**으로 설계되어:
- 코드 수정 없이 디자이너가 밸런싱 가능
- 새로운 화폐 타입 쉽게 추가
- 난이도별 프리셋 생성 가능
- 게임 모드별 다른 설정 적용 가능

## 라이센스

Copyright Epic Games, Inc. All Rights Reserved.
