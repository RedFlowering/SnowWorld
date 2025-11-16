# HarmoniaKit 세이브/로드 시스템

SnowWorld 게임을 위한 완전한 세이브/로드 시스템입니다.

## 주요 기능

- ✅ **로컬 PC 저장**: 서버 소유주의 PC에 세이브 파일 저장
- ✅ **스팀 클라우드**: 스팀 클라우드에 자동 동기화
- ✅ **멀티플레이어 지원**: 각 플레이어의 스팀 ID별로 데이터 저장
- ✅ **자동 저장**: 설정 가능한 간격으로 자동 저장 (기본 5분)
- ✅ **블루프린트 지원**: 블루프린트에서 쉽게 사용 가능

## 저장되는 데이터

### 플레이어 데이터
- 플레이어 위치 및 회전
- 체력, 스태미나, 최대 체력/스태미나
- 공격력, 방어력, 크리티컬 확률/배수
- 이동 속도, 공격 속도
- 인벤토리 아이템 목록
- 스탯 태그 (버프, 디버프 등)

### 월드 데이터
- 배치된 빌딩 목록 (위치, 회전, 스케일)
- 월드 시간
- 날씨 상태 (확장 가능)

## C++에서 사용하기

### 1. 세이브 시스템 가져오기

```cpp
#include "System/HarmoniaSaveGameSubsystem.h"

UHarmoniaSaveGameSubsystem* SaveSystem = GetGameInstance()->GetSubsystem<UHarmoniaSaveGameSubsystem>();
```

### 2. 게임 저장

```cpp
// 로컬 + 스팀 클라우드에 저장
bool bSuccess = SaveSystem->SaveGame(TEXT("MySlot"), true);

// 로컬에만 저장
bool bSuccess = SaveSystem->SaveGame(TEXT("MySlot"), false);
```

### 3. 게임 로드

```cpp
// 스팀 클라우드 우선, 실패 시 로컬에서 로드
bool bSuccess = SaveSystem->LoadGame(TEXT("MySlot"), true);

// 로컬에서만 로드
bool bSuccess = SaveSystem->LoadGame(TEXT("MySlot"), false);
```

### 4. 세이브 파일 삭제

```cpp
// 로컬 + 스팀 클라우드에서 삭제
bool bSuccess = SaveSystem->DeleteSaveGame(TEXT("MySlot"), true);
```

### 5. 자동 저장 설정

```cpp
// 자동 저장 활성화/비활성화
SaveSystem->SetAutoSaveEnabled(true);

// 자동 저장 간격 설정 (초 단위)
SaveSystem->SetAutoSaveInterval(300.0f); // 5분
```

### 6. 이벤트 바인딩

```cpp
// 저장 완료 이벤트
SaveSystem->OnSaveGameComplete.AddDynamic(this, &UMyClass::OnSaveComplete);

void UMyClass::OnSaveComplete(EHarmoniaSaveGameResult Result, const FString& SaveSlotName)
{
    if (Result == EHarmoniaSaveGameResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Save successful: %s"), *SaveSlotName);
    }
}

// 로드 완료 이벤트
SaveSystem->OnLoadGameComplete.AddDynamic(this, &UMyClass::OnLoadComplete);

void UMyClass::OnLoadComplete(EHarmoniaSaveGameResult Result, UHarmoniaSaveGame* SaveGameObject)
{
    if (Result == EHarmoniaSaveGameResult::Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Load successful"));
    }
}
```

## 블루프린트에서 사용하기

### 1. 게임 저장

블루프린트에서 **Save Game** 노드를 사용합니다:

```
Save Slot Name: "DefaultSave"
Use Steam Cloud: true
```

### 2. 게임 로드

**Load Game** 노드를 사용합니다:

```
Save Slot Name: "DefaultSave"
Use Steam Cloud: true
```

### 3. 세이브 파일 존재 확인

**Does Save Game Exist** 노드를 사용합니다.

### 4. 자동 저장 설정

**Set Auto Save Enabled** 및 **Set Auto Save Interval** 노드를 사용합니다.

### 5. 플레이어 정보 확인

**Get Player Steam ID** - 플레이어의 스팀 ID를 가져옵니다.
**Is Server Owner** - 현재 플레이어가 서버 소유주인지 확인합니다.

### 6. 이벤트 처리

**Get Save Game Subsystem** 노드로 서브시스템을 가져온 후,
**Bind Event to On Save Game Complete** 및 **Bind Event to On Load Game Complete**를 사용하여 이벤트를 바인딩합니다.

## 멀티플레이어 동작 방식

### 리슨 서버 (Listen Server)

1. **서버 소유주** (호스트):
   - 서버 소유주의 PC에 세이브 파일 저장
   - 스팀 클라우드에 백업
   - 자신의 스팀 ID로 데이터 저장

2. **참가한 친구들**:
   - 각자의 스팀 ID를 키로 사용하여 데이터 저장
   - 서버 소유주의 세이브 파일에 함께 저장됨
   - 나중에 같은 서버에 재접속 시 자동으로 데이터 로드

### 데이터 구조 예시

```
세이브 파일: "MyWorld.sav"
├── ServerOwnerSteamID: "76561198012345678"
├── PlayerDataMap:
│   ├── "76561198012345678" (서버 소유주)
│   │   ├── PlayerName: "Host"
│   │   ├── PlayerLocation: (100, 200, 50)
│   │   ├── Attributes: { Health: 80, Stamina: 60, ... }
│   │   └── InventoryItems: [...]
│   ├── "76561198087654321" (친구1)
│   │   ├── PlayerName: "Friend1"
│   │   └── ...
│   └── "76561198011111111" (친구2)
│       └── ...
└── WorldData:
    └── PlacedBuildings: [...]
```

## 스팀 클라우드 설정

### DefaultEngine.ini 설정

스팀 클라우드를 사용하려면 프로젝트의 `Config/DefaultEngine.ini`에 다음을 추가하세요:

```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
GameServerQueryPort=27015
bRelaunchInSteam=false
GameVersion=1.0.0.0
bVACEnabled=0
bAllowP2PPacketRelay=true
P2PConnectionTimeout=90

[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
```

**참고**: `SteamDevAppId=480`은 테스트용입니다. 실제 게임에서는 자신의 스팀 앱 ID를 사용하세요.

### steam_appid.txt

프로젝트 루트에 `steam_appid.txt` 파일을 만들고 앱 ID를 입력하세요:

```
480
```

## 주의사항

### 1. 세이브 파일 호환성

세이브 파일 버전이 다르면 로드 시 경고가 표시됩니다.
`UHarmoniaSaveGame::SaveVersion`을 업데이트하고 호환성 처리 로직을 추가하세요.

### 2. 인벤토리 아이템

현재 인벤토리 아이템은 `ItemDefinition` 경로로 저장됩니다.
커스텀 데이터가 있는 경우 `FHarmoniaSavedInventoryItem` 구조체를 확장하세요.

### 3. 빌딩 시스템

HarmoniaBuildingInstanceManager와의 통합은 해당 API에 따라 구현해야 합니다.
현재는 기본 구조만 제공됩니다.

### 4. 네트워크 동기화

세이브/로드는 **서버에서만** 수행해야 합니다.
클라이언트가 직접 저장하면 데이터 불일치가 발생할 수 있습니다.

## 확장 가이드

### 새 데이터 추가하기

1. **`FHarmoniaPlayerSaveData` 또는 `FHarmoniaWorldSaveData`에 필드 추가**:

```cpp
UPROPERTY(SaveGame)
int32 PlayerLevel = 1;
```

2. **저장/로드 로직 업데이트**:

```cpp
void UHarmoniaSaveGameSubsystem::SavePlayerData(...)
{
    PlayerData.PlayerLevel = GetPlayerLevel(PlayerState);
}

void UHarmoniaSaveGameSubsystem::LoadPlayerData(...)
{
    SetPlayerLevel(PlayerState, PlayerData.PlayerLevel);
}
```

3. **세이브 버전 증가**:

```cpp
// HarmoniaSaveGame.h
UPROPERTY(SaveGame)
int32 SaveVersion = 2; // 1 -> 2로 증가
```

## 파일 구조

```
Plugins/HarmoniaKit/Source/HarmoniaKit/
├── Public/System/
│   ├── HarmoniaSaveGame.h                  // 세이브 데이터 구조
│   ├── HarmoniaSaveGameSubsystem.h         // 세이브/로드 시스템
│   └── HarmoniaSaveGameLibrary.h           // 블루프린트 함수 라이브러리
└── Private/System/
    ├── HarmoniaSaveGame.cpp
    ├── HarmoniaSaveGameSubsystem.cpp
    └── HarmoniaSaveGameLibrary.cpp
```

## 라이선스

Copyright 2025 Snow Game Studio. All Rights Reserved.
