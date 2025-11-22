# HarmoniaKit 치트 시스템 빌드 설정

## 📋 개요

HarmoniaKit의 치트 시스템은 **에디터와 개발(Development) 빌드에서만 동작**하며, **배포(Shipping) 빌드에서는 자동으로 비활성화**됩니다.

이는 게임 배포 시 치트 기능이 최종 사용자에게 노출되는 것을 방지하기 위한 안전 장치입니다.

## 🔧 기술적 구현

### 전처리기 지시문 (Preprocessor Directives)

치트 시스템의 모든 코드는 `#if !UE_BUILD_SHIPPING` 전처리기 지시문으로 감싸져 있습니다:

```cpp
// 치트 시스템은 에디터와 개발 빌드에서만 동작하며, 배포(Shipping) 빌드에서는 비활성화됩니다.
#if !UE_BUILD_SHIPPING

// 치트 시스템 코드...

#endif // !UE_BUILD_SHIPPING
```

### 적용된 파일

다음 파일들이 빌드 설정에 따라 조건부 컴파일됩니다:

1. **HarmoniaCheatManager.h** - 치트 매니저 헤더
2. **HarmoniaCheatManager.cpp** - 치트 매니저 구현
3. **HarmoniaCheatLibrary.h** - 치트 라이브러리 헤더
4. **HarmoniaCheatLibrary.cpp** - 치트 라이브러리 구현

## 📊 빌드 타입별 동작

| 빌드 타입 | 치트 시스템 | UE_BUILD_SHIPPING | 설명 |
|----------|-----------|------------------|------|
| **Editor** | ✅ 활성화 | 정의되지 않음 | 에디터에서 모든 치트 사용 가능 |
| **Development** | ✅ 활성화 | 정의되지 않음 | 개발 빌드에서 모든 치트 사용 가능 |
| **Debug** | ✅ 활성화 | 정의되지 않음 | 디버그 빌드에서 모든 치트 사용 가능 |
| **DebugGame** | ✅ 활성화 | 정의되지 않음 | 디버그게임 빌드에서 모든 치트 사용 가능 |
| **Shipping** | ❌ 비활성화 | 정의됨 (1) | **치트 코드가 컴파일되지 않음** |
| **Test** | ❌ 비활성화 | 정의됨 (1) | 치트 코드가 컴파일되지 않음 |

## 🎯 주요 특징

### 1. 컴파일 타임 제거
- Shipping 빌드에서는 치트 관련 코드가 **완전히 컴파일되지 않습니다**
- 런타임 검사가 아닌 컴파일 타임 제거로 성능 오버헤드가 전혀 없습니다
- 최종 바이너리 크기가 줄어듭니다

### 2. 보안
- 배포된 게임에서 치트 코드가 물리적으로 존재하지 않습니다
- 리버스 엔지니어링으로도 치트 기능을 찾을 수 없습니다

### 3. 자동화
- 개발자가 별도의 설정 없이 자동으로 적용됩니다
- 빌드 파이프라인에서 추가 작업이 필요하지 않습니다

## 💡 사용 예시

### Editor/Development 빌드
```cpp
// ✅ 정상 동작
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // 치트 매니저가 정상적으로 작동
    if (UHarmoniaCheatLibrary::IsCheatsEnabled(this))
    {
        UE_LOG(LogTemp, Log, TEXT("치트 시스템 활성화됨"));
    }
}
```

### Shipping 빌드
```cpp
// ❌ UHarmoniaCheatManager 클래스가 존재하지 않음
// ❌ UHarmoniaCheatLibrary 클래스가 존재하지 않음
// 컴파일 에러가 발생하므로 Shipping 빌드를 위한 코드에서는
// 치트 관련 코드를 조건부로 사용해야 함
```

## 🔒 블루프린트에서의 사용

### Development/Editor 빌드
- 모든 치트 관련 블루프린트 노드가 정상 작동합니다
- `Get Harmonia Cheat Manager`
- `Is Cheat Manager Available`
- `Quick Heal`, `Quick Toggle Invincible` 등 모든 빠른 치트 함수

### Shipping 빌드
- 치트 관련 블루프린트 노드들이 아예 표시되지 않습니다
- 이미 사용 중인 노드는 무시되며 에러를 발생시키지 않습니다

## ⚠️ 주의사항

### 1. 조건부 컴파일 필요
프로젝트 코드에서 치트 시스템을 직접 참조하는 경우, 같은 전처리기 지시문으로 감싸야 합니다:

```cpp
// 올바른 사용법
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

### 2. 블루프린트 의존성
블루프린트에서 치트 시스템을 사용하는 경우:
- Development 빌드에서 테스트 완료 후
- Shipping 빌드 전에 해당 블루프린트 로직을 비활성화하거나
- 조건문으로 감싸는 것을 권장합니다

### 3. Header 포함 순서
```cpp
// ✅ 올바른 방법
#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
    #include "System/HarmoniaCheatManager.h"
#endif

// ❌ 잘못된 방법 (Shipping 빌드 시 컴파일 에러)
#include "System/HarmoniaCheatManager.h"  // Shipping에서 파일이 존재하지 않음
```

## 🧪 빌드 테스트

### Development 빌드 확인
```
1. Visual Studio에서 빌드 구성을 "Development Editor" 또는 "Development"로 설정
2. 컴파일 및 실행
3. 콘솔(~)을 열고 "HarmoniaHelp" 입력
4. ✅ 치트 명령어 목록이 표시되어야 함
```

### Shipping 빌드 확인
```
1. Visual Studio에서 빌드 구성을 "Shipping"로 설정
2. 컴파일
3. ✅ 치트 관련 코드가 컴파일 대상에서 제외됨
4. 실행 파일 크기가 Development 빌드보다 작아야 함
```

## 📚 관련 문서

- [CheatSystem_README.md](CheatSystem_README.md) - 치트 시스템 전체 개요
- [CheatSystem_Guide.md](CheatSystem_Guide.md) - 사용 가이드
- [CheatSystem_QuickReference.md](CheatSystem_QuickReference.md) - 빠른 참조

## 🔄 업데이트 이력

- **2025-11-22**: 초기 문서 작성 및 빌드 설정 구현
  - `#if !UE_BUILD_SHIPPING` 전처리기 지시문 적용
  - 모든 치트 시스템 파일에 조건부 컴파일 추가
  - Shipping 빌드에서 완전히 제거되도록 설정
