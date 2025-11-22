# HarmoniaKit 치트 시스템 가이드

## 개요

HarmoniaKit 치트 시스템은 개발 과정을 편리하게 만들기 위한 강력한 도구입니다. 콘솔 명령어를 통해 게임의 다양한 요소를 즉시 조작할 수 있습니다.

## 설정 방법

### 1. 게임모드에 치트 매니저 등록

프로젝트의 GameMode 클래스에서 치트 매니저를 등록해야 합니다.

#### C++에서 등록:

```cpp
// YourGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "YourGameMode.generated.h"

UCLASS()
class YOURPROJECT_API AYourGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AYourGameMode();
};

// YourGameMode.cpp
#include "YourGameMode.h"
#include "System/HarmoniaCheatManager.h"

AYourGameMode::AYourGameMode()
{
    // 치트 매니저 클래스 설정
    CheatClass = UHarmoniaCheatManager::StaticClass();
}
```

#### 블루프린트에서 등록:

1. 게임모드 블루프린트를 엽니다
2. **Class Defaults**를 선택합니다
3. **Cheat Class**를 `HarmoniaCheatManager`로 설정합니다

### 2. 치트 활성화

치트는 다음 조건에서 자동으로 활성화됩니다:

- **Development** 또는 **Debug** 빌드
- 또는 게임 실행 시 `-EnableCheats` 커맨드라인 인자 추가

**Shipping** 빌드에서는 치트가 비활성화됩니다.

## 사용 방법

### 콘솔 명령어 사용

1. 게임 실행 중 **`~`** (물결) 키를 눌러 콘솔을 엽니다
2. 원하는 치트 명령어를 입력합니다
3. **Enter** 키를 눌러 실행합니다

### 도움말 명령어

모든 치트 명령어 목록을 보려면:
```
HarmoniaHelp
```

## 주요 치트 명령어

### 체력/마나/스태미나

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `HarmoniaSetHealth <값>` | 현재 체력 설정 | `HarmoniaSetHealth 1000` |
| `HarmoniaSetMaxHealth <값>` | 최대 체력 설정 | `HarmoniaSetMaxHealth 2000` |
| `HarmoniaHealFull` | 체력 완전 회복 | `HarmoniaHealFull` |
| `HarmoniaSetMana <값>` | 마나 설정 | `HarmoniaSetMana 500` |
| `HarmoniaSetStamina <값>` | 스태미나 설정 | `HarmoniaSetStamina 300` |

### 재화

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `HarmoniaGiveGold <양>` | 골드 지급 (음수로 제거 가능) | `HarmoniaGiveGold 10000` |
| `HarmoniaSetGold <양>` | 골드를 특정 값으로 설정 | `HarmoniaSetGold 999999` |

### 전투

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `HarmoniaToggleInvincible` | 무적 모드 토글 | `HarmoniaToggleInvincible` |
| `HarmoniaToggleGodMode` | 신 모드 토글 (무적 + 무한 자원) | `HarmoniaToggleGodMode` |
| `HarmoniaToggleOneHitKill` | 원샷 원킬 모드 토글 | `HarmoniaToggleOneHitKill` |
| `HarmoniaSetDamageMultiplier <배수>` | 데미지 배수 설정 | `HarmoniaSetDamageMultiplier 5.0` |

### 이동

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `HarmoniaSetSpeed <배수>` | 이동 속도 배수 설정 | `HarmoniaSetSpeed 2.0` |
| `HarmoniaToggleFly` | 비행 모드 토글 | `HarmoniaToggleFly` |
| `HarmoniaToggleNoClip` | 노클립 모드 토글 (벽 통과) | `HarmoniaToggleNoClip` |
| `HarmoniaTeleport <X> <Y> <Z>` | 좌표로 텔레포트 | `HarmoniaTeleport 0 0 1000` |

### 시간/날씨

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `HarmoniaSetTime <시> <분>` | 게임 내 시간 설정 | `HarmoniaSetTime 18 30` |
| `HarmoniaSetTimeScale <배수>` | 시간 흐름 속도 설정 | `HarmoniaSetTimeScale 10.0` |
| `HarmoniaSetWeather <타입>` | 날씨 변경 | `HarmoniaSetWeather Rain` |

**날씨 타입**: Clear, Cloudy, Rain, Storm, Fog, Snow, Blizzard

### 기타

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `HarmoniaToggleDebugInfo` | 디버그 정보 표시 토글 | `HarmoniaToggleDebugInfo` |
| `HarmoniaResetCheats` | 모든 치트 리셋 | `HarmoniaResetCheats` |

## 블루프린트에서 사용

### 블루프린트 함수 라이브러리

`HarmoniaCheatLibrary`를 통해 블루프린트에서도 치트 기능을 사용할 수 있습니다.

#### 빠른 액세스 함수

```
Quick Heal - 즉시 체력 회복
Quick Toggle Invincible - 무적 토글
Quick Give Gold - 골드 10000 지급
Quick Toggle Fly - 비행 모드 토글
Quick Double Speed - 속도 2배
Quick Reset Cheats - 치트 리셋
```

#### 예시: UI 버튼에 연결

1. UI 위젯에서 버튼을 만듭니다
2. 버튼의 OnClicked 이벤트에서 `Quick Heal` 노드를 추가합니다
3. World Context Object에 Self를 연결합니다

### 치트 매니저 직접 사용

더 세밀한 제어가 필요한 경우:

```
1. Get Harmonia Cheat Manager 노드로 치트 매니저 가져오기
2. 원하는 치트 함수 호출
```

## 개발 팁

### 1. 자주 사용하는 치트 조합 만들기

블루프린트 함수로 자주 사용하는 치트 조합을 만들 수 있습니다:

```
함수 이름: DevSetup
- HarmoniaSetMaxHealth 10000
- HarmoniaHealFull
- HarmoniaGiveGold 999999
- HarmoniaSetSpeed 2.0
```

### 2. 키 바인딩 설정

Project Settings > Input에서 개발 단축키를 설정할 수 있습니다:

- F1: Quick Heal
- F2: Quick Toggle Fly
- F3: Quick Give Gold
- F5: Quick Reset Cheats

### 3. 개발자 메뉴 만들기

UMG를 사용하여 개발자 전용 치트 메뉴를 만들 수 있습니다:

```
- 메뉴를 열 키 설정 (예: F12)
- 자주 사용하는 치트를 버튼으로 배치
- 입력 필드로 커스텀 값 설정 가능
```

## 확장 가이드

### 새로운 치트 명령어 추가

1. `HarmoniaCheatManager.h`에 함수 선언 추가:

```cpp
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Custom")
void HarmoniaMyCustomCheat(int32 Value);
```

2. `HarmoniaCheatManager.cpp`에 구현 추가:

```cpp
void UHarmoniaCheatManager::HarmoniaMyCustomCheat(int32 Value)
{
    // 치트 로직 구현
    LogCheat(FString::Printf(TEXT("My custom cheat: %d"), Value));
}
```

3. (선택) `HarmoniaHelp()` 함수에 도움말 추가

### 프로젝트별 치트 구현

일부 치트 기능은 프로젝트의 시스템에 맞게 구현이 필요합니다:

- 아이템 지급 (인벤토리 시스템 연동)
- 퀘스트 완료 (퀘스트 시스템 연동)
- 적 스폰 (스폰 시스템 연동)
- 레벨/경험치 (레벨링 시스템 연동)

이러한 기능들은 현재 로그 메시지만 출력하도록 구현되어 있으며, 
실제 프로젝트의 시스템과 연동하여 구현해야 합니다.

## 주의사항

1. **빌드 타입 확인**: Shipping 빌드에서는 치트가 비활성화됩니다
2. **멀티플레이어**: 치트는 로컬 클라이언트에서만 작동합니다
3. **세이브 데이터**: 치트 사용 후 세이브하면 치트된 상태가 저장됩니다
4. **디버깅**: 치트 사용 시 화면과 로그에 메시지가 출력됩니다

## 문제 해결

### 치트가 작동하지 않을 때

1. **개발 빌드인지 확인**: Shipping 빌드는 치트 불가
2. **CheatClass 설정 확인**: 게임모드에서 HarmoniaCheatManager가 설정되었는지 확인
3. **콘솔 활성화 확인**: ~ 키로 콘솔이 열리는지 확인
4. **명령어 철자 확인**: 대소문자를 정확히 입력해야 함

### 특정 치트가 작동하지 않을 때

1. **로그 확인**: Output Log에서 에러 메시지 확인
2. **컴포넌트 확인**: 필요한 컴포넌트(ASC, Currency Manager 등)가 있는지 확인
3. **시스템 구현 확인**: 일부 치트는 추가 구현이 필요함

## 추가 리소스

- `HarmoniaCheatManager.h` - 모든 치트 함수 정의
- `HarmoniaCheatManager.cpp` - 치트 구현 로직
- `HarmoniaCheatLibrary.h` - 블루프린트 래퍼 함수

---

**참고**: 이 치트 시스템은 개발 목적으로만 사용해야 하며, 
최종 배포 버전(Shipping)에서는 자동으로 비활성화됩니다.
