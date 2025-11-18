# Recovery Item System - 빠른 시작 가이드

## 5분 안에 시작하기

### 1단계: 컴포넌트 추가 (1분)

**플레이어 Character 블루프린트:**
1. `Content/Characters/YourCharacter` 블루프린트 열기
2. Components 패널에서 "Add Component" 클릭
3. "Harmonia Rechargeable Item Component" 검색 후 추가

### 2단계: 공명 파편 등록 (2분)

**Event Graph에서:**

```
Event BeginPlay
  └─ Register Resonance Shard (Azure)
       ├─ Rechargeable Item Component: Self
       ├─ Frequency: Azure
       └─ Variant Config:
            ├─ Shard Name: "푸른 공명 파편"
            ├─ Health Recovery Percent: 0.5 (50%)
            └─ Shard Color: (0, 0.5, 1, 1) // 푸른색
```

### 3단계: Gameplay Ability 생성 (1분)

1. Content Browser에서 우클릭
2. Blueprint Class → `HarmoniaGameplayAbility_UseRecoveryItem`
3. 이름: `GA_UseAzureShard`
4. 열기 후 Details 패널:
   - Item Type: `Resonance Shard`
   - Shard Frequency: `Azure`

### 4단계: 입력 바인딩 (1분)

**Input Mapping Context에서:**
1. 새 Input Action 생성: `IA_UseRecoveryItem`
2. 키 바인딩: `Q` 키
3. Character 블루프린트에서:

```
IA_UseRecoveryItem (Triggered)
  └─ Try Activate Ability By Class
       └─ Ability Class: GA_UseAzureShard
```

### 완료!

이제 게임을 실행하고 `Q` 키를 눌러 공명 파편을 사용할 수 있습니다!

---

## 다음 단계

### 체크포인트에서 자동 충전 설정

체크포인트(크리스탈 공명기)와 자동으로 연동됩니다. 별도 설정 불필요!

### UI에 충전 횟수 표시

**Widget 블루프린트:**
```
Event Construct
  └─ Bind Event to OnChargesChanged
       └─ Update Text
            └─ Get Remaining Charges
```

### 다른 회복 아이템 추가

**얼어붙은 시간의 눈송이 등록:**
```
Event BeginPlay
  └─ Register Recovery Item
       ├─ Item Type: Frozen Time Snowflake
       └─ Config:
            ├─ Max Charges: 3
            ├─ Usage Duration: 1.0
            └─ Effect Type: Time Reversal
```

---

## 전체 예제 블루프린트

### Character BeginPlay

```
Event BeginPlay
  ├─ Register Resonance Shard (Azure)
  │    └─ ... 설정
  ├─ Register Resonance Shard (Crimson)
  │    └─ ... 설정
  ├─ Register Resonance Shard (Verdant)
  │    └─ ... 설정
  └─ Register Recovery Item (Frozen Time Snowflake)
       └─ ... 설정
```

### 입력 처리

```
IA_UseRecoveryItem
  └─ Try Activate Ability By Class
       └─ GA_UseAzureShard

IA_UseSnowflake
  └─ Try Activate Ability By Class
       └─ GA_UseFrozenTimeSnowflake
```

---

## C++ 전체 예제

```cpp
// MyCharacter.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recovery Items")
TObjectPtr<UHarmoniaRechargeableItemComponent> RechargeableItemComponent;

// MyCharacter.cpp
AMyCharacter::AMyCharacter()
{
    RechargeableItemComponent = CreateDefaultSubobject<UHarmoniaRechargeableItemComponent>(TEXT("RechargeableItemComponent"));
}

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 공명 파편 등록
    RegisterResonanceShards();

    // 기타 회복 아이템 등록
    RegisterOtherRecoveryItems();
}

void AMyCharacter::RegisterResonanceShards()
{
    // Azure 파편
    FHarmoniaResonanceShardVariant AzureShard;
    AzureShard.Frequency = EHarmoniaResonanceFrequency::Azure;
    AzureShard.ShardName = FText::FromString(TEXT("푸른 공명 파편"));
    AzureShard.HealthRecoveryPercent = 0.5f;
    AzureShard.ShardColor = FLinearColor(0.0f, 0.5f, 1.0f);
    RechargeableItemComponent->RegisterResonanceShard(EHarmoniaResonanceFrequency::Azure, AzureShard);

    // Crimson 파편
    FHarmoniaResonanceShardVariant CrimsonShard;
    CrimsonShard.Frequency = EHarmoniaResonanceFrequency::Crimson;
    CrimsonShard.ShardName = FText::FromString(TEXT("붉은 공명 파편"));
    CrimsonShard.HealthRecoveryPercent = 0.5f;
    CrimsonShard.ShardColor = FLinearColor(1.0f, 0.0f, 0.0f);
    RechargeableItemComponent->RegisterResonanceShard(EHarmoniaResonanceFrequency::Crimson, CrimsonShard);

    // ... 나머지 파편들
}

void AMyCharacter::RegisterOtherRecoveryItems()
{
    // 얼어붙은 시간의 눈송이
    FHarmoniaRecoveryItemConfig SnowflakeConfig;
    SnowflakeConfig.ItemType = EHarmoniaRecoveryItemType::FrozenTimeSnowflake;
    SnowflakeConfig.ItemName = FText::FromString(TEXT("얼어붙은 시간의 눈송이"));
    SnowflakeConfig.MaxCharges = 3;
    SnowflakeConfig.InitialCharges = 3;
    SnowflakeConfig.UsageDuration = 1.0f;
    SnowflakeConfig.EffectType = EHarmoniaRecoveryEffectType::TimeReversal;
    RechargeableItemComponent->RegisterRecoveryItem(EHarmoniaRecoveryItemType::FrozenTimeSnowflake, SnowflakeConfig);

    // 온천 보온병
    FHarmoniaRecoveryItemConfig FlaskConfig;
    FlaskConfig.ItemType = EHarmoniaRecoveryItemType::ThermalSpringFlask;
    FlaskConfig.ItemName = FText::FromString(TEXT("온천 보온병"));
    FlaskConfig.MaxCharges = 4;
    FlaskConfig.InitialCharges = 4;
    FlaskConfig.UsageDuration = 2.0f;
    FlaskConfig.EffectType = EHarmoniaRecoveryEffectType::OverTime;
    FlaskConfig.RecoveryDuration = 3.0f;
    RechargeableItemComponent->RegisterRecoveryItem(EHarmoniaRecoveryItemType::ThermalSpringFlask, FlaskConfig);
}
```

---

## 테스트 체크리스트

- [ ] 컴포넌트가 Character에 추가되었는지 확인
- [ ] 아이템이 등록되었는지 확인 (BeginPlay 로그)
- [ ] Gameplay Ability가 부여되었는지 확인
- [ ] 입력 키가 올바르게 바인딩되었는지 확인
- [ ] Q 키를 눌렀을 때 아이템이 사용되는지 확인
- [ ] 충전 횟수가 감소하는지 확인
- [ ] 체크포인트에서 충전되는지 확인
- [ ] UI에 충전 횟수가 표시되는지 확인

---

## 문제 해결

### "Component not found" 에러
→ Character에 컴포넌트가 추가되었는지 확인

### "Cannot use item" 경고
→ 아이템이 등록되었는지, 충전 횟수가 남아있는지 확인

### 아이템이 충전되지 않음
→ 체크포인트 서브시스템이 활성화되었는지 확인

### VFX가 재생되지 않음
→ Niagara System 애셋이 할당되었는지 확인

---

**더 자세한 정보는 `RecoveryItemSystem_README.md`를 참고하세요!**
