# HarmoniaKit 치트 시스템

## 📋 개요

HarmoniaKit 치트 시스템은 개발 과정을 효율적으로 만들어주는 강력한 개발 도구입니다.

### 주요 기능

✅ **체력/마나/스태미나 관리** - 즉시 회복, 수치 설정  
✅ **재화 관리** - 골드 및 다양한 통화 지급  
✅ **아이템 관리** - 아이템 생성 및 인벤토리 조작  
✅ **전투 치트** - 무적, 신모드, 원킬 모드, 데미지 배수  
✅ **이동 치트** - 속도 조절, 비행, 노클립, 텔레포트  
✅ **시간/날씨** - 게임 내 시간과 날씨 제어  
✅ **디버그 도구** - 디버그 정보 표시, 적 제거/스폰  
✅ **블루프린트 지원** - UI와 쉽게 연동 가능한 함수 라이브러리  

## 🚀 빠른 시작

### 1. 게임모드 설정

#### 방법 A: 예제 게임모드 사용
```cpp
// 프로젝트의 게임모드를 AHarmoniaGameModeExample에서 상속
UCLASS()
class AMyGameMode : public AHarmoniaGameModeExample
{
    GENERATED_BODY()
};
```

#### 방법 B: 기존 게임모드에 추가
```cpp
// YourGameMode.cpp
#include "System/HarmoniaCheatManager.h"

AYourGameMode::AYourGameMode()
{
    CheatClass = UHarmoniaCheatManager::StaticClass();
}
```

#### 방법 C: 블루프린트에서 설정
1. 게임모드 블루프린트 열기
2. Class Defaults > Cheat Class를 `HarmoniaCheatManager`로 설정

### 2. 치트 사용

게임 실행 후:
1. **`~`** 키를 눌러 콘솔 열기
2. `HarmoniaHelp` 입력하여 명령어 목록 확인
3. 원하는 명령어 실행

### 예시
```
HarmoniaHealFull              - 체력 즉시 회복
HarmoniaGiveGold 10000        - 골드 10000 지급
HarmoniaSetSpeed 2.0          - 속도 2배
HarmoniaToggleFly             - 비행 모드
HarmoniaSetTime 12 0          - 정오로 시간 설정
```

## 📚 문서

- **[전체 가이드](CheatSystem_Guide.md)** - 상세한 사용 방법 및 확장 가이드
- **[빠른 참조](CheatSystem_QuickReference.md)** - 자주 사용하는 명령어 모음

## 🎯 주요 명령어

| 카테고리 | 명령어 | 설명 |
|---------|--------|------|
| 기본 | `HarmoniaHelp` | 모든 명령어 목록 |
| 체력 | `HarmoniaHealFull` | 체력 완전 회복 |
| 재화 | `HarmoniaGiveGold 10000` | 골드 지급 |
| 전투 | `HarmoniaToggleGodMode` | 신 모드 토글 |
| 이동 | `HarmoniaSetSpeed 2.0` | 속도 2배 |
| 시간 | `HarmoniaSetTime 12 0` | 정오 |
| 날씨 | `HarmoniaSetWeather Rain` | 비 |

## 🎮 블루프린트 사용

### 빠른 액세스 함수

`HarmoniaCheatLibrary`에서 제공하는 블루프린트 노드:

- **Quick Heal** - 즉시 회복
- **Quick Toggle Invincible** - 무적 토글
- **Quick Give Gold** - 골드 10000 지급
- **Quick Toggle Fly** - 비행 토글
- **Quick Double Speed** - 속도 2배
- **Quick Reset Cheats** - 치트 리셋

### UI에 연동 예시

```
Button_Heal 클릭 이벤트:
└─ Quick Heal (World Context: Self)

Button_Gold 클릭 이벤트:
└─ Quick Give Gold (World Context: Self, Amount: 10000)
```

## 🔧 프로젝트별 확장

일부 기능은 프로젝트의 시스템에 맞게 추가 구현이 필요합니다:

### 구현 필요 기능
- ⚠️ `HarmoniaGiveItem` - 인벤토리 시스템 연동
- ⚠️ `HarmoniaSetLevel` - 레벨링 시스템 연동
- ⚠️ `HarmoniaCompleteQuest` - 퀘스트 시스템 연동
- ⚠️ `HarmoniaSpawnEnemy` - 스폰 시스템 연동

### 확장 방법

`HarmoniaCheatManager.h`에 새 함수 추가:
```cpp
UFUNCTION(Exec, BlueprintCallable, Category = "Harmonia|Cheat|Custom")
void HarmoniaMyCustomCheat(int32 Value);
```

`HarmoniaCheatManager.cpp`에 구현:
```cpp
void UHarmoniaCheatManager::HarmoniaMyCustomCheat(int32 Value)
{
    // 치트 로직
    LogCheat(FString::Printf(TEXT("Custom: %d"), Value));
}
```

## 🛡️ 보안

- ✅ **Development/Debug 빌드**에서만 활성화
- ❌ **Shipping 빌드**에서 자동 비활성화
- 🔒 멀티플레이어에서 서버 권한 필요

## 📁 파일 구조

```
HarmoniaKit/
├── Source/HarmoniaKit/
│   ├── Public/
│   │   ├── System/
│   │   │   └── HarmoniaCheatManager.h
│   │   ├── Libraries/
│   │   │   └── HarmoniaCheatLibrary.h
│   │   └── HarmoniaGameModeExample.h
│   └── Private/
│       ├── System/
│       │   └── HarmoniaCheatManager.cpp
│       ├── Libraries/
│       │   └── HarmoniaCheatLibrary.cpp
│       └── HarmoniaGameModeExample.cpp
└── Docs/
    ├── CheatSystem_Guide.md
    ├── CheatSystem_QuickReference.md
    └── CheatSystem_README.md (이 파일)
```

## 💡 개발 팁

### 1. 자주 쓰는 조합 만들기
```
// 개발 세팅
HarmoniaSetMaxHealth 10000
HarmoniaHealFull
HarmoniaGiveGold 999999
HarmoniaSetSpeed 2.0
```

### 2. 키 바인딩 추천
- F1: Quick Heal
- F2: Quick Toggle Fly
- F3: Quick Give Gold
- F5: Quick Reset Cheats

### 3. 개발자 메뉴 제작
UMG로 자주 쓰는 치트를 버튼으로 배치하여 편리하게 사용

## ❓ 문제 해결

### 치트가 작동하지 않을 때
1. Development 빌드인지 확인
2. 게임모드에서 CheatClass 설정 확인
3. 콘솔(~)이 열리는지 확인

### 특정 치트 기능이 작동하지 않을 때
1. Output Log 확인
2. 필요한 컴포넌트(ASC, Currency Manager 등) 확인
3. 프로젝트별 구현이 필요한 기능인지 확인

## 📞 지원

문제가 발생하거나 새로운 기능이 필요한 경우:
1. 문서를 먼저 확인
2. 로그 메시지 확인
3. 프로젝트의 시스템과 연동 확인

---

**Made with ❤️ for HarmoniaKit developers**

개발 편의를 위한 도구이므로 최종 배포 버전에서는 자동으로 비활성화됩니다.
