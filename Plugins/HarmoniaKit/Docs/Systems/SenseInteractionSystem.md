# HarmoniaKit Sense-Based Interaction System

## 개요

HarmoniaKit의 센스 기반 인터랙션 시스템은 SenseSystem 플러그인을 활용하여 다양한 방식의 인터랙션을 구현합니다. 이 시스템은 근접(Proximity), 시야(Vision), 청각(Hearing) 등 여러 센서를 통해 인터랙션을 감지하고 처리합니다.

## 주요 기능

### 1. 다양한 인터랙션 트리거 타입

- **OnProximityEnter**: 범위 안에 들어올 때 트리거
- **OnProximityStay**: 범위 안에 있는 동안 지속적으로 트리거
- **OnProximityExit**: 범위를 벗어날 때 트리거
- **OnSeen**: 시야에 처음 들어올 때 트리거
- **WhileSeen**: 시야에 있는 동안 지속적으로 트리거
- **OnLostSight**: 시야에서 벗어날 때 트리거
- **OnHeard**: 소리를 감지했을 때 트리거
- **Manual**: 입력이 필요한 수동 트리거
- **Automatic**: 감지 시 자동 트리거

### 2. 인터랙션 모드

- **Single**: 트리거당 한 번만 실행
- **Continuous**: 조건이 충족되는 동안 지속적으로 실행
- **Repeatable**: 쿨다운 후 반복 가능
- **OneTime**: 한 번만 실행 가능

### 3. 주요 컴포넌트

#### UHarmoniaSenseInteractableComponent
- 인터랙션 가능한 오브젝트에 부착
- `USenseStimulusComponent`를 상속하여 센스 자극 발생
- 여러 인터랙션 설정 동시 지원
- 쿨다운, 일회성 인터랙션 등 제어 가능

#### UHarmoniaSenseInteractionComponent
- 플레이어나 AI에 부착
- `USenseReceiverComponent`를 사용하여 주변 감지
- 우선순위 기반 타겟 선택
- 자동/수동 인터랙션 지원

#### AHarmoniaSenseInteractableActor
- 완전한 인터랙션 가능 액터
- 비주얼 피드백 (하이라이트, 활성화 등)
- UI 위젯 표시
- 사운드 이펙트 지원
- 리스폰/제거 옵션

## 사용 방법

### 1. 근접 기반 인터랙션 (Proximity Interaction)

플레이어가 특정 범위에 들어오면 자동으로 인터랙션이 발생합니다.

```cpp
// C++ 예제
FSenseInteractionConfig ProximityConfig;
ProximityConfig.InteractionType = EHarmoniaInteractionType::Pickup;
ProximityConfig.TriggerType = ESenseInteractionTriggerType::OnProximityEnter;
ProximityConfig.InteractionMode = ESenseInteractionMode::Single;
ProximityConfig.SensorTag = FName("Proximity");
ProximityConfig.InteractionRange = 200.0f;
ProximityConfig.MinimumSenseScore = 0.5f;
ProximityConfig.bEnabled = true;

InteractableComponent->AddInteractionConfig(ProximityConfig);
```

**Blueprint 설정:**
1. 액터에 `HarmoniaSenseInteractableComponent` 추가
2. `Interaction Configs` 배열에 새 요소 추가
3. `Trigger Type`을 `On Proximity Enter`로 설정
4. `Interaction Range` 설정 (예: 200)
5. `Sensor Tag`를 "Proximity"로 설정

### 2. 시야 기반 인터랙션 (Vision-Based Interaction)

플레이어가 오브젝트를 바라볼 때 하이라이트되고, 버튼을 눌러 인터랙션합니다.

```cpp
// C++ 예제
FSenseInteractionConfig VisionConfig;
VisionConfig.InteractionType = EHarmoniaInteractionType::Custom;
VisionConfig.TriggerType = ESenseInteractionTriggerType::Manual;
VisionConfig.InteractionMode = ESenseInteractionMode::Repeatable;
VisionConfig.SensorTag = FName("Vision");
VisionConfig.MinimumSenseScore = 0.7f;
VisionConfig.bRequiresLineOfSight = true;
VisionConfig.CooldownTime = 2.0f;
VisionConfig.InteractionPrompt = FText::FromString("Press E to Examine");
VisionConfig.bEnabled = true;

InteractableComponent->AddInteractionConfig(VisionConfig);
```

**Blueprint 설정:**
1. `Trigger Type`을 `Manual`로 설정
2. `Sensor Tag`를 "Vision"으로 설정
3. `Requires Line Of Sight`를 true로 설정
4. `Interaction Prompt` 텍스트 설정
5. `Interaction Mode`를 `Repeatable`로 설정

### 3. 청각 기반 인터랙션 (Sound-Based Interaction)

소리를 감지하여 반응하는 인터랙션입니다.

```cpp
// C++ 예제
FSenseInteractionConfig HearingConfig;
HearingConfig.InteractionType = EHarmoniaInteractionType::Custom;
HearingConfig.TriggerType = ESenseInteractionTriggerType::OnHeard;
HearingConfig.InteractionMode = ESenseInteractionMode::Continuous;
HearingConfig.SensorTag = FName("Hearing");
HearingConfig.MinimumSenseScore = 0.3f;
HearingConfig.bEnabled = true;

InteractableComponent->AddInteractionConfig(HearingConfig);
```

### 4. 자동 인터랙션 (Automatic Interaction)

감지되는 즉시 자동으로 실행되는 인터랙션입니다 (예: 아이템 자동 수집).

```cpp
// C++ 예제
FSenseInteractionConfig AutoConfig;
AutoConfig.InteractionType = EHarmoniaInteractionType::Pickup;
AutoConfig.TriggerType = ESenseInteractionTriggerType::Automatic;
AutoConfig.InteractionMode = ESenseInteractionMode::OneTime;
AutoConfig.SensorTag = FName("Proximity");
AutoConfig.InteractionRange = 100.0f;
AutoConfig.MinimumSenseScore = 0.8f;
AutoConfig.bEnabled = true;

InteractableComponent->AddInteractionConfig(AutoConfig);
```

### 5. 플레이어 캐릭터 설정

플레이어 캐릭터에 `UHarmoniaSenseInteractionComponent`를 추가합니다:

```cpp
// C++ 예제
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
UHarmoniaSenseInteractionComponent* SenseInteractionComponent;

// Constructor
SenseInteractionComponent = CreateDefaultSubobject<UHarmoniaSenseInteractionComponent>(
    TEXT("SenseInteractionComponent"));

// 모니터링할 센서 태그 설정
SenseInteractionComponent->MonitoredSensorTags.Empty();
SenseInteractionComponent->MonitoredSensorTags.Add(FName("Proximity"));
SenseInteractionComponent->MonitoredSensorTags.Add(FName("Vision"));
SenseInteractionComponent->MonitoredSensorTags.Add(FName("Hearing"));

// 입력 설정 (Enhanced Input)
SenseInteractionComponent->InteractAction = InteractInputAction;
SenseInteractionComponent->InputMapping = InteractInputMapping;
```

**Blueprint 설정:**
1. 캐릭터 블루프린트를 열기
2. `Add Component` → `HarmoniaSenseInteractionComponent`
3. `Monitored Sensor Tags`에 사용할 센서 추가
4. `Interact Action`과 `Input Mapping` 설정

### 6. 센서 설정

`USenseReceiverComponent`에 센서를 추가합니다:

**근접 센서 (Proximity):**
```cpp
// Proximity 센서는 거리 기반 감지
// SensorDistanceTest 또는 SensorTouch 사용
```

**시야 센서 (Vision):**
```cpp
// Vision 센서는 시야각과 거리 기반 감지
// SensorSight 사용
```

**청각 센서 (Hearing):**
```cpp
// Hearing 센서는 소리 기반 감지
// SensorHearing 사용
```

## 이벤트 처리

### Blueprint 이벤트

**Interactable Component 이벤트:**
- `OnSenseInteractionTriggered`: 인터랙션이 트리거될 때
- `OnSensed`: 감지되었을 때
- `OnLostSense`: 감지에서 벗어났을 때

**Interaction Component 이벤트:**
- `OnInteractableSensed`: 새로운 인터랙터블 감지
- `OnInteractableLost`: 인터랙터블 감지 해제
- `OnBestTargetChanged`: 최적 타겟 변경
- `OnInteractionCompleted`: 인터랙션 완료

### C++ 이벤트 바인딩

```cpp
// Interactable에서
InteractableComponent->OnSenseInteractionTriggered.AddDynamic(
    this, &AMyActor::OnInteracted);

// Interactor에서
SenseInteractionComponent->OnInteractableSensed.AddDynamic(
    this, &AMyCharacter::OnTargetSensed);

SenseInteractionComponent->OnBestTargetChanged.AddDynamic(
    this, &AMyCharacter::OnBestTargetChanged);
```

## 예제 액터 사용

### AHarmoniaSenseInteractableActor 사용

1. **레벨에 배치:**
   - Content Browser에서 `AHarmoniaSenseInteractableActor` 기반 Blueprint 생성
   - 레벨에 배치
   - Details 패널에서 설정 조정

2. **설정 옵션:**
   - `Mesh Component`: 비주얼 메시 설정
   - `Interaction Configs`: 인터랙션 설정 배열
   - `Idle/Highlight/Active/Disabled Material`: 상태별 머티리얼
   - `Respawn Time`: 리스폰 시간 (0 = 리스폰 안 함)
   - `Destroy After Interaction`: 인터랙션 후 제거
   - `Disable After Interaction`: 인터랙션 후 비활성화

3. **커스텀 로직 추가:**
   ```cpp
   // Blueprint 또는 C++에서 이벤트 오버라이드
   void AMyInteractableActor::OnInteractionTriggered_Implementation(
       AActor* Interactor, FName SensorTag)
   {
       Super::OnInteractionTriggered_Implementation(Interactor, SensorTag);

       // 커스텀 로직 추가
       // 예: 아이템 지급, 퀘스트 업데이트, 이펙트 재생 등
   }
   ```

## 다양한 인터랙션 예제

### 1. 아이템 픽업 (자동 수집)

```cpp
// 플레이어가 가까이 가면 자동으로 수집되는 아이템
FSenseInteractionConfig Config;
Config.InteractionType = EHarmoniaInteractionType::Pickup;
Config.TriggerType = ESenseInteractionTriggerType::Automatic;
Config.InteractionMode = ESenseInteractionMode::OneTime;
Config.SensorTag = FName("Proximity");
Config.InteractionRange = 150.0f;
Config.bEnabled = true;
```

### 2. 문 열기 (수동, 근접)

```cpp
// 가까이 가면 프롬프트가 표시되고, 버튼을 눌러 열기
FSenseInteractionConfig Config;
Config.InteractionType = EHarmoniaInteractionType::Open;
Config.TriggerType = ESenseInteractionTriggerType::Manual;
Config.InteractionMode = ESenseInteractionMode::Repeatable;
Config.SensorTag = FName("Proximity");
Config.InteractionRange = 200.0f;
Config.CooldownTime = 1.0f;
Config.InteractionPrompt = FText::FromString("Press E to Open Door");
Config.bEnabled = true;
```

### 3. NPC 대화 (시야 기반)

```cpp
// NPC를 바라보면 대화 프롬프트 표시
FSenseInteractionConfig Config;
Config.InteractionType = EHarmoniaInteractionType::Custom;
Config.TriggerType = ESenseInteractionTriggerType::Manual;
Config.InteractionMode = ESenseInteractionMode::Repeatable;
Config.SensorTag = FName("Vision");
Config.MinimumSenseScore = 0.8f;
Config.bRequiresLineOfSight = true;
Config.InteractionPrompt = FText::FromString("Press E to Talk");
Config.bEnabled = true;
```

### 4. 위험 지역 (자동 경고)

```cpp
// 위험 지역에 들어가면 자동으로 경고
FSenseInteractionConfig Config;
Config.InteractionType = EHarmoniaInteractionType::Custom;
Config.TriggerType = ESenseInteractionTriggerType::OnProximityEnter;
Config.InteractionMode = ESenseInteractionMode::Continuous;
Config.SensorTag = FName("Proximity");
Config.InteractionRange = 500.0f;
Config.bEnabled = true;
```

### 5. 자원 채집 (반복 가능)

```cpp
// 나무나 광석을 반복해서 채집
FSenseInteractionConfig Config;
Config.InteractionType = EHarmoniaInteractionType::Gather;
Config.TriggerType = ESenseInteractionTriggerType::Manual;
Config.InteractionMode = ESenseInteractionMode::Repeatable;
Config.SensorTag = FName("Proximity");
Config.InteractionRange = 250.0f;
Config.CooldownTime = 2.0f;
Config.InteractionPrompt = FText::FromString("Press E to Gather");
Config.bEnabled = true;
```

## 디버깅

### 디버그 표시 활성화

```cpp
// Interactable Component
InteractableComponent->bShowDebugInfo = true;

// Interaction Component
SenseInteractionComponent->bShowDebugInfo = true;
```

이렇게 하면 감지된 타겟과의 연결선이 표시됩니다.

### 콘솔 명령어

SenseSystem 플러그인의 디버그 명령어를 사용할 수 있습니다:
- `SenseSys.Debug.DrawSensors 1`: 센서 시각화
- `SenseSys.Debug.DrawStimulus 1`: 자극 시각화

## 네트워크 복제

모든 인터랙션은 자동으로 서버-클라이언트 복제를 지원합니다:

- 클라이언트에서 인터랙션 시도
- 서버에서 검증 및 실행
- 결과를 클라이언트에 전달
- 모든 클라이언트에 상태 동기화

## 성능 최적화

1. **센서 업데이트 빈도 조절**: `UpdateTimeRate` 설정
2. **최대 추적 타겟 제한**: `MaxTrackedTargets` 설정
3. **센서 범위 최적화**: 필요한 만큼만 설정
4. **우선순위 기반 처리**: 중요한 타겟 우선 처리

## 요약

HarmoniaKit의 센스 기반 인터랙션 시스템은:
- ✅ 다양한 감지 방식 지원 (근접, 시야, 청각)
- ✅ 유연한 트리거 조건
- ✅ 자동/수동 인터랙션
- ✅ 네트워크 복제 지원
- ✅ 비주얼 피드백 및 UI
- ✅ Blueprint 친화적
- ✅ 확장 가능한 구조

이 시스템을 활용하여 플레이어 경험을 향상시키는 다양한 인터랙션을 구현할 수 있습니다!
