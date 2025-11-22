# HarmoniaKit Cheat System

**Version:** 1.0  
**Last Updated:** 2025-11-22

## 📋 개요

HarmoniaKit 치트 시스템은 개발 과정을 편리하게 만들기 위한 강력한 도구입니다. 콘솔 명령어를 통해 게임의 다양한 요소를 즉시 조작할 수 있습니다.

> ⚠️ **중요**: 치트 시스템은 **에디터와 개발(Development) 빌드에서만 동작**하며, **배포(Shipping) 빌드에서는 자동으로 비활성화**됩니다.

---

## 🔧 빌드 설정 및 제한

치트 시스템의 모든 코드는 `#if !UE_BUILD_SHIPPING` 전처리기 지시문으로 감싸져 있어, Shipping 빌드 시 코드 자체가 컴파일에서 제외됩니다.

### 빌드 타입별 동작

| 빌드 타입 | 치트 시스템 | 설명 |
|----------|-----------|------|
| **Editor** | ✅ 활성화 | 에디터에서 모든 치트 사용 가능 |
| **Development** | ✅ 활성화 | 개발 빌드에서 모든 치트 사용 가능 |
| **Debug** | ✅ 활성화 | 디버그 빌드에서 모든 치트 사용 가능 |
| **Shipping** | ❌ 비활성화 | **치트 코드가 컴파일되지 않음** |

### 주의사항

프로젝트 코드에서 치트 시스템을 직접 참조하는 경우, 반드시 조건부 컴파일을 적용해야 합니다:

```cpp
// ✅ 올바른 사용법
#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
    #include "System/HarmoniaCheatManager.h"
#endif

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
#if !UE_BUILD_SHIPPING
    // 치트 매니저 초기화
    if (!CheatManager)
    {
        CheatManager = NewObject<UHarmoniaCheatManager>(this);
    }
#endif
}
```

---

## ⚙️ 설정 방법

### 1. 게임모드에 치트 매니저 등록

프로젝트의 GameMode 클래스에서 치트 매니저를 등록해야 합니다.

**C++에서 등록:**
```cpp
// YourGameMode.cpp
#include "YourGameMode.h"
#if !UE_BUILD_SHIPPING
#include "System/HarmoniaCheatManager.h"
#endif

AYourGameMode::AYourGameMode()
{
#if !UE_BUILD_SHIPPING
    // 치트 매니저 클래스 설정
    CheatClass = UHarmoniaCheatManager::StaticClass();
#endif
}
```

**블루프린트에서 등록:**
1. 게임모드 블루프린트를 엽니다.
2. **Class Defaults**를 선택합니다.
3. **Cheat Class**를 `HarmoniaCheatManager`로 설정합니다.

### 2. 치트 활성화

치트는 다음 조건에서 자동으로 활성화됩니다:
- **Development** 또는 **Debug** 빌드
- 또는 게임 실행 시 `-EnableCheats` 커맨드라인 인자 추가

---

## 🎮 사용 방법

### 콘솔 명령어 사용

1. 게임 실행 중 **`~`** (물결) 키를 눌러 콘솔을 엽니다.
2. 원하는 치트 명령어를 입력합니다.
3. **Enter** 키를 눌러 실행합니다.

**도움말 명령어:**
```
HarmoniaHelp
```

---

## 📜 주요 치트 명령어

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

---

## 🔌 블루프린트에서 사용

`HarmoniaCheatLibrary`를 통해 블루프린트에서도 치트 기능을 사용할 수 있습니다.

### 빠른 액세스 함수
- `Quick Heal` - 즉시 체력 회복
- `Quick Toggle Invincible` - 무적 토글
- `Quick Give Gold` - 골드 10000 지급
- `Quick Toggle Fly` - 비행 모드 토글
- `Quick Double Speed` - 속도 2배
- `Quick Reset Cheats` - 치트 리셋

### 예시: UI 버튼에 연결
1. UI 위젯에서 버튼을 만듭니다.
2. 버튼의 OnClicked 이벤트에서 `Quick Heal` 노드를 추가합니다.
3. World Context Object에 Self를 연결합니다.

---

## 🚀 확장 가이드

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

### 프로젝트별 치트 구현
일부 치트 기능(아이템 지급, 퀘스트 완료 등)은 프로젝트의 시스템에 맞게 구현이 필요합니다. `HarmoniaCheatManager.cpp`의 해당 함수들을 오버라이드하거나 수정하여 프로젝트의 인벤토리/퀘스트 시스템과 연동하세요.

---

## ❓ 문제 해결

**치트가 작동하지 않을 때:**
1. **빌드 확인**: Shipping 빌드가 아닌지 확인하세요.
2. **CheatClass 확인**: 게임모드에 `HarmoniaCheatManager`가 설정되었는지 확인하세요.
3. **콘솔 확인**: `~` 키로 콘솔이 열리는지 확인하세요.

**특정 치트가 작동하지 않을 때:**
1. **로그 확인**: Output Log에서 에러 메시지를 확인하세요.
2. **컴포넌트 확인**: 필요한 컴포넌트(ASC, Currency Manager 등)가 캐릭터에 있는지 확인하세요.
