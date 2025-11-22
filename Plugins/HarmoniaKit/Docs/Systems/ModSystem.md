# HarmoniaModSystem (하모니아 모딩 시스템)

## 개요 (Overview)

HarmoniaModSystem은 게임에 커뮤니티 제작 콘텐츠를 추가할 수 있는 강력하고 유연한 모딩 프레임워크입니다.

**주요 기능:**
- ✅ **모드 로더** - 우선순위 및 의존성 체크가 포함된 자동 모드 로딩
- ✅ **핫리로드** - 게임 재시작 없이 런타임에 모드 적용/해제
- ✅ **에셋 오버라이드** - 모델, 텍스처, 사운드, 애니메이션 교체
- ✅ **블루프린트 확장** - 커스텀 노드 및 기능 추가 지원
- ✅ **데이터 테이블 패치** - 아이템 스탯, 몬스터 능력치 수정
- ✅ **커스텀 난이도/룰셋** - Nuzlocke 같은 특수 규칙 시스템
- ✅ **Steam Workshop 연동** - 자동 다운로드/업데이트 (설정 필요)
- ✅ **충돌 감지** - 모드 간 비호환성 자동 탐지 및 경고
- ✅ **샌드박스 모드** - 위험한 모드 격리 실행 지원

---

## 빠른 시작 (Quick Start)

### 1. 시스템 활성화

**DefaultGame.ini** 에서 모드 시스템 설정:

```ini
[/Script/HarmoniaModSystem.HarmoniaModSubsystem]
+ModSearchPaths=/Game/Mods/
+ModSearchPaths=../../../SnowWorld/Saved/Mods/
bAutoLoadModsOnStartup=true
bEnableHotReload=true
bEnableConflictDetection=true
MaxModCount=256
```

### 2. 모드 제작

모드 디렉토리 구조:

```
Mods/
└── MyAwesomeMod/
    ├── ModInfo.json         # 모드 메타데이터
    ├── Content/             # 커스텀 에셋
    │   ├── Meshes/
    │   ├── Textures/
    │   └── Sounds/
    └── Data/                # 데이터 테이블
        └── ItemStats.json
```

**ModInfo.json** 예시:

```json
{
  "ModId": "MyAwesomeMod",
  "DisplayName": "My Awesome Mod",
  "Author": "ModAuthor",
  "Version": "1.0.0",
  "Description": "Adds awesome features to the game!",
  "LoadPriority": 2,
  "Homepage": "https://github.com/user/mymod",

  "Dependencies": [
    {
      "ModId": "CoreFrameworkMod",
      "MinVersion": "2.0.0",
      "Optional": false
    }
  ],

  "Incompatibilities": [
    {
      "ModId": "ConflictingMod",
      "Reason": "Both mods override the same game systems"
    }
  ],

  "AssetOverrides": [
    {
      "OriginalAssetPath": "/Game/Characters/Player/PlayerMesh",
      "OverrideAssetPath": "/MyAwesomeMod/Characters/CustomPlayerMesh",
      "OverrideType": 1
    }
  ],

  "DataTablePatches": [
    {
      "TargetDataTable": "/Game/Data/ItemStats",
      "RowName": "IronSword",
      "PropertyName": "Damage",
      "NewValue": "150",
      "Operation": "Set"
    }
  ],

  "CustomRulesets": [
    {
      "RulesetId": "NuzlockeMode",
      "DisplayName": "Nuzlocke Challenge",
      "Description": "Permadeath and capture restrictions",
      "RuleTags": "Rule.Nuzlocke.Permadeath,Rule.Nuzlocke.OneCatchPerArea",
      "ConfigValues": {
        "DifficultyMultiplier": "1.5",
        "AllowRevive": "false"
      },
      "IsDifficultyModifier": true
    }
  ],

  "SandboxMode": false,
  "HotReloadEnabled": true,
  "WorkshopItemId": 0
}
```

### 3. 코드에서 사용

#### C++ 예시

```cpp
#include "System/HarmoniaModSubsystem.h"
#include "System/HarmoniaAssetOverrideSubsystem.h"
#include "System/HarmoniaDataTablePatcher.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Get mod subsystem
    UHarmoniaModSubsystem* ModSystem = GetGameInstance()->GetSubsystem<UHarmoniaModSubsystem>();

    if (ModSystem)
    {
        // Discover and load mods
        int32 DiscoveredCount = ModSystem->DiscoverMods();
        UE_LOG(LogTemp, Log, TEXT("Discovered %d mods"), DiscoveredCount);

        TArray<FName> FailedMods;
        int32 LoadedCount = ModSystem->LoadAllMods(FailedMods);
        UE_LOG(LogTemp, Log, TEXT("Loaded %d mods"), LoadedCount);

        // Check for conflicts
        TArray<FHarmoniaModConflict> Conflicts = ModSystem->DetectConflicts();
        if (Conflicts.Num() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Detected %d mod conflicts"), Conflicts.Num());
        }
    }
}

// Load asset with override support
UStaticMesh* LoadMeshWithOverride(const FSoftObjectPath& MeshPath)
{
    UHarmoniaAssetOverrideSubsystem* AssetOverride =
        GetGameInstance()->GetSubsystem<UHarmoniaAssetOverrideSubsystem>();

    if (AssetOverride)
    {
        UObject* Asset = AssetOverride->LoadAssetWithOverride(MeshPath);
        return Cast<UStaticMesh>(Asset);
    }

    return nullptr;
}

// Check active rulesets
bool IsNuzlockeModeActive()
{
    UHarmoniaModRulesetSubsystem* RulesetSystem =
        GetGameInstance()->GetSubsystem<UHarmoniaModRulesetSubsystem>();

    if (RulesetSystem)
    {
        return RulesetSystem->IsRulesetActive(FName("NuzlockeMode"));
    }

    return false;
}
```

#### Blueprint 사용

블루프린트에서 모든 기능에 접근 가능합니다:

1. **Get Game Instance** → **Get Subsystem** → **Harmonia Mod Subsystem**
2. **Discover Mods** 호출
3. **Load All Mods** 호출
4. **Get Loaded Mods** 로 로드된 모드 목록 확인

---

## 핵심 시스템 (Core Systems)

### 1. 모드 로더 (Mod Loader)

**HarmoniaModSubsystem** - 메인 모드 관리 시스템

#### 주요 기능:
- 자동 모드 검색 및 발견
- 의존성 해결 및 우선순위 기반 로딩
- 런타임 모드 로드/언로드
- 충돌 감지 및 호환성 검증

#### 로드 우선순위:
```cpp
enum class EHarmoniaModLoadPriority
{
    Critical = 0,    // 핵심 프레임워크 모드 (먼저 로드)
    High = 1,        // 높은 우선순위
    Normal = 2,      // 일반 (기본값)
    Low = 3,         // 낮은 우선순위
    Cosmetic = 4     // 시각/UI 모드 (마지막에 로드)
};
```

#### 이벤트:
- `OnModLoaded` - 모드 로드 성공 시
- `OnModLoadFailed` - 모드 로드 실패 시
- `OnModsReloaded` - 핫리로드 완료 시
- `OnModConflictDetected` - 충돌 감지 시

---

### 2. 핫리로드 (Hot-Reload)

게임 재시작 없이 런타임에 모드를 적용/해제할 수 있습니다.

```cpp
// C++ 예시
ModSystem->ReloadAllMods();  // 모든 모드 리로드
ModSystem->ReloadMod(FName("MyMod"));  // 특정 모드만 리로드

// 설정에서 활성화
bEnableHotReload = true;
```

**주의사항:**
- 핫리로드는 에셋 변경에 적합합니다
- Blueprint 클래스 변경은 재시작이 필요할 수 있습니다
- 중요한 시스템 변경 시 주의가 필요합니다

---

### 3. 에셋 오버라이드 (Asset Override)

**HarmoniaAssetOverrideSubsystem** - 런타임 에셋 교체

#### 지원하는 에셋 타입:
- Static Mesh (모델)
- Skeletal Mesh (캐릭터 메시)
- Texture (텍스처)
- Material (머티리얼)
- Sound (사운드)
- Animation (애니메이션)
- Niagara Effect (이펙트)
- Data Table (데이터 테이블)
- Blueprint Class (블루프린트)

#### 사용 예시:

```cpp
// 에셋 오버라이드 등록
FHarmoniaAssetOverride Override;
Override.OriginalAssetPath = FSoftObjectPath("/Game/Weapons/Sword");
Override.OverrideAssetPath = FSoftObjectPath("/MyMod/Weapons/CoolSword");
Override.OverrideType = EHarmoniaAssetOverrideType::StaticMesh;

AssetOverrideSystem->RegisterAssetOverride(Override, FName("MyMod"));

// 오버라이드 적용하여 로드
UObject* Asset = AssetOverrideSystem->LoadAssetWithOverride(
    FSoftObjectPath("/Game/Weapons/Sword")
);
// → /MyMod/Weapons/CoolSword 가 로드됨
```

---

### 4. 데이터 테이블 패치 (Data Table Patching)

**HarmoniaDataTablePatcher** - 런타임 데이터 수정

#### 지원하는 연산:
- **Set** - 값 직접 설정
- **Add** - 숫자 값에 더하기
- **Multiply** - 숫자 값 곱하기
- **Append** - 문자열 추가

#### 사용 예시:

```cpp
// 아이템 데미지 2배로 증가
FHarmoniaDataTablePatch Patch;
Patch.TargetDataTable = FSoftObjectPath("/Game/Data/ItemStats");
Patch.RowName = FName("IronSword");
Patch.PropertyName = FName("Damage");
Patch.NewValue = "2.0";
Patch.Operation = "Multiply";

FHarmoniaPatchResult Result;
DataTablePatcher->ApplyPatch(Patch, FName("MyBalanceMod"), Result);

if (Result.bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Patched: %s → %s"), *Result.OldValue, *Result.NewValue);
}
```

**JSON 형식:**
```json
{
  "DataTablePatches": [
    {
      "TargetDataTable": "/Game/Data/MonsterStats",
      "RowName": "Dragon",
      "PropertyName": "Health",
      "NewValue": "5000",
      "Operation": "Set"
    }
  ]
}
```

---

### 5. 커스텀 난이도/룰셋 (Custom Rulesets)

**HarmoniaModRulesetSubsystem** - 게임 규칙 커스터마이징

#### 사용 사례:
- Nuzlocke 모드 (포켓몬 스타일)
- Ironman 모드 (세이브 제한)
- Randomizer 모드
- 커스텀 난이도 설정

#### 예시:

```cpp
// Nuzlocke 모드 활성화
RulesetSystem->ActivateRuleset(FName("NuzlockeMode"));

// 규칙 태그 확인
if (RulesetSystem->IsRuleTagActive(FGameplayTag::RequestGameplayTag("Rule.Nuzlocke.Permadeath")))
{
    // 영구 사망 처리
    HandlePermadeath();
}

// 난이도 배율 가져오기
float DifficultyMult = RulesetSystem->GetDifficultyMultiplier();
int32 AdjustedDamage = BaseDamage * DifficultyMult;
```

**ModInfo.json 정의:**
```json
{
  "CustomRulesets": [
    {
      "RulesetId": "Ironman",
      "DisplayName": "Ironman Mode",
      "Description": "Single save file, permanent death",
      "RuleTags": "Rule.Ironman.SingleSave,Rule.Ironman.Permadeath",
      "ConfigValues": {
        "MaxSaveSlots": "1",
        "AllowManualSave": "false",
        "DifficultyMultiplier": "1.3"
      },
      "IsDifficultyModifier": true
    }
  ]
}
```

---

### 6. Steam Workshop 연동 (Steam Workshop Integration)

**HarmoniaWorkshopIntegration** - Steam Workshop 자동화

#### 기능:
- 자동 모드 다운로드
- 업데이트 감지 및 적용
- 구독 관리
- 설치 및 활성화

#### 설정 (Steam 앱 설정 필요):

1. **DefaultEngine.ini**:
```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=YOUR_APP_ID
```

2. **사용 예시**:
```cpp
// Workshop 아이템 구독
WorkshopIntegration->SubscribeToItem(1234567890);

// 업데이트 확인 및 다운로드
int32 UpdateCount = WorkshopIntegration->CheckForUpdates();
WorkshopIntegration->DownloadAllUpdates();

// 이벤트 바인딩
WorkshopIntegration->OnWorkshopItemDownloaded.AddDynamic(
    this, &AMyClass::OnModDownloaded
);
```

**주의:** Steam API 설정이 필요하며, 현재 stub 구현입니다.

---

### 7. 충돌 감지 (Conflict Detection)

자동으로 모드 간 충돌을 감지합니다:

#### 감지되는 충돌:
- 명시적 비호환성 선언
- 동일 에셋 오버라이드
- 동일 데이터 테이블 row 수정
- 의존성 버전 불일치

#### 심각도 레벨:
```cpp
enum class EHarmoniaModConflictSeverity
{
    None,      // 충돌 없음
    Warning,   // 경고 - 작동할 수 있음
    Error,     // 에러 - 문제 발생 가능
    Critical   // 치명적 - 로드 불가
};
```

#### 사용:
```cpp
TArray<FHarmoniaModConflict> Conflicts = ModSystem->DetectConflicts();

for (const FHarmoniaModConflict& Conflict : Conflicts)
{
    UE_LOG(LogTemp, Warning, TEXT("Conflict: %s <-> %s: %s"),
        *Conflict.ModA.ToString(),
        *Conflict.ModB.ToString(),
        *Conflict.Description);
}
```

---

### 8. 샌드박스 모드 (Sandbox Mode)

신뢰할 수 없는 모드를 격리하여 실행합니다.

**ModInfo.json**:
```json
{
  "SandboxMode": true
}
```

샌드박스 모드에서는:
- 제한된 API 접근
- 파일 시스템 접근 제한
- 네트워크 접근 제한
- 안전한 환경에서 실행

---

## API 레퍼런스 (API Reference)

### UHarmoniaModSubsystem

주요 함수들:

```cpp
// 모드 검색
int32 DiscoverMods();

// 모드 로드
bool LoadMod(FName ModId, FString& OutErrorMessage);
int32 LoadAllMods(TArray<FName>& OutFailedMods);

// 모드 언로드
bool UnloadMod(FName ModId);
void UnloadAllMods();

// 핫리로드
int32 ReloadAllMods();
bool ReloadMod(FName ModId);

// 쿼리
bool GetModInfo(FName ModId, FHarmoniaModInfo& OutModInfo) const;
TArray<FHarmoniaModInfo> GetAllMods() const;
TArray<FHarmoniaModInfo> GetLoadedMods() const;
bool IsModLoaded(FName ModId) const;
TArray<FHarmoniaModInfo> GetModsByTag(FGameplayTag Tag) const;

// 의존성 및 충돌
bool CheckDependencies(FName ModId, TArray<FHarmoniaModDependency>& OutMissingDependencies) const;
TArray<FHarmoniaModConflict> DetectConflicts() const;
bool AreModsIncompatible(FName ModA, FName ModB) const;
TArray<FName> GetLoadOrder() const;
```

### UHarmoniaAssetOverrideSubsystem

```cpp
bool RegisterAssetOverride(const FHarmoniaAssetOverride& Override, FName ModId);
bool UnregisterAssetOverride(const FSoftObjectPath& OriginalAssetPath, FName ModId);
void UnregisterAllModOverrides(FName ModId);

bool GetOverriddenAssetPath(const FSoftObjectPath& OriginalAssetPath, FSoftObjectPath& OutOverridePath) const;
UObject* LoadAssetWithOverride(const FSoftObjectPath& AssetPath);
bool HasOverride(const FSoftObjectPath& AssetPath) const;

int32 ApplyModOverrides(FName ModId, const TArray<FHarmoniaAssetOverride>& Overrides);
void RevertModOverrides(FName ModId);
```

### UHarmoniaDataTablePatcher

```cpp
bool ApplyPatch(const FHarmoniaDataTablePatch& Patch, FName ModId, FHarmoniaPatchResult& OutResult);
int32 ApplyPatches(const TArray<FHarmoniaDataTablePatch>& Patches, FName ModId);

bool RevertPatch(const FHarmoniaDataTablePatch& Patch, FName ModId);
int32 RevertAllModPatches(FName ModId);
void RevertAllPatches();

bool GetOriginalValue(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName, FString& OutOriginalValue) const;
bool IsValuePatched(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName) const;

TArray<FHarmoniaDataTablePatch> GetModPatches(FName ModId) const;
```

### UHarmoniaModRulesetSubsystem

```cpp
bool RegisterRuleset(const FHarmoniaCustomRuleset& Ruleset, FName ModId);
bool UnregisterRuleset(FName RulesetId);

bool ActivateRuleset(FName RulesetId);
bool DeactivateRuleset(FName RulesetId);
void DeactivateAllRulesets();

TArray<FHarmoniaCustomRuleset> GetAllRulesets() const;
TArray<FHarmoniaCustomRuleset> GetActiveRulesets() const;
bool IsRulesetActive(FName RulesetId) const;
bool IsRuleTagActive(FGameplayTag Tag) const;

bool GetRuleConfigValue(FName ConfigKey, FString& OutValue) const;
FGameplayTagContainer GetActiveRuleTags() const;
float GetDifficultyMultiplier() const;
```

---

## 모범 사례 (Best Practices)

### 1. 모드 ID 명명 규칙
- 고유하고 설명적인 이름 사용
- 작성자 접두사 권장: `AuthorName_ModName`
- 특수문자 피하기

### 2. 버전 관리
- 시맨틱 버저닝 사용: `Major.Minor.Patch`
- 큰 변경 시 Major 증가
- 기능 추가 시 Minor 증가
- 버그 수정 시 Patch 증가

### 3. 의존성 선언
- 필수 의존성 명시
- 최소 버전 지정
- 선택적 의존성은 `Optional: true`

### 4. 충돌 방지
- 다른 모드와 겹칠 수 있는 리소스 명시
- 네임스페이스 사용 (예: `/MyMod/...`)
- 호환성 정보 제공

### 5. 성능 고려
- 대용량 에셋은 지연 로딩 사용
- 불필요한 오버라이드 피하기
- 데이터 테이블 패치 최소화

### 6. 테스트
- 다른 인기 모드와 호환성 테스트
- 로드/언로드 반복 테스트
- 핫리로드 동작 검증

---

## 문제 해결 (Troubleshooting)

### 모드가 로드되지 않음

1. **ModInfo.json 검증**
   - JSON 문법 오류 확인
   - 필수 필드 존재 확인

2. **로그 확인**
   ```
   LogHarmoniaModSystem: Error: ...
   ```

3. **의존성 확인**
   - 필수 의존성이 설치되었는지 확인
   - 버전 호환성 검증

### 충돌 오류

1. **충돌 감지 로그 확인**
   ```cpp
   TArray<FHarmoniaModConflict> Conflicts = ModSystem->DetectConflicts();
   ```

2. **로드 순서 조정**
   - LoadPriority 변경
   - 의존성 추가

### 에셋 오버라이드 작동 안 함

1. **경로 확인**
   - 원본 에셋 경로가 정확한지 확인
   - 오버라이드 에셋이 존재하는지 확인

2. **에셋 타입 일치**
   - OverrideType이 올바른지 확인

3. **로드 방법**
   - `LoadAssetWithOverride` 함수 사용 확인

---

## 추가 리소스 (Additional Resources)

- **GitHub**: [HarmoniaKit Repository](https://github.com/yourrepo/harmoniakit)
- **Discord**: [Community Server](https://discord.gg/yourserver)
- **Wiki**: [상세 문서](https://wiki.yoursite.com/modding)
- **예제 모드**: `/Plugins/HarmoniaKit/Examples/`

---

## 라이선스 (License)

Copyright 2025 Snow Game Studio. All Rights Reserved.

---

## 변경 로그 (Changelog)

### v1.0.0 (2025-01-18)
- 초기 릴리스
- 모드 로더 구현
- 핫리로드 지원
- 에셋 오버라이드 시스템
- 데이터 테이블 패처
- 커스텀 룰셋 시스템
- Steam Workshop 기본 연동
- 충돌 감지 시스템
- 샌드박스 모드 지원
