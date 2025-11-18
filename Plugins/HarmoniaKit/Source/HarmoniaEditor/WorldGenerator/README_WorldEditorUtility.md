# 하모니아 월드 에디터 유틸리티 (Harmonia World Editor Utility)

## 개요 (Overview)

하모니아 월드 에디터 유틸리티는 언리얼 에디터에서 월드를 시각적으로 생성하고 편집할 수 있는 도구입니다.
런타임 자동 생성과는 별개로, 에디터에서 수동으로 월드를 제작할 수 있는 기능을 제공합니다.

The Harmonia World Editor Utility is a tool for visually creating and editing worlds in Unreal Editor.
It provides manual world creation capabilities in the editor, separate from runtime auto-generation.

## 주요 기능 (Key Features)

### 1. 월드 생성 (World Generation)
- **전체 월드 생성**: 지형 + 오브젝트 완전 생성
- **지형만 생성**: 하이트맵과 바이옴만 생성
- **오브젝트만 생성**: 기존 지형에 오브젝트 배치
- **영역별 생성**: 특정 영역에만 오브젝트 생성
- **비동기 생성**: 에디터를 멈추지 않고 백그라운드 생성

### 2. 지형 편집 (Terrain Editing)
- 지형 높이기/낮추기 (Raise/Lower)
- 지형 평탄화 (Flatten)
- 지형 부드럽게 (Smooth)
- 분화구 생성 (Create Crater)
- 언덕 생성 (Create Hill)

### 3. 시각화 및 프리뷰 (Visualization & Preview)
- 빠른 프리뷰 생성
- 바이옴 경계 시각화
- 강/호수 시각화
- 도로 네트워크 시각화
- 구조물 배치 시각화
- 동굴 입구 시각화
- 자원 분포 시각화
- POI (관심 지점) 시각화

### 4. 설정 관리 (Configuration Management)
- 설정 파일 저장/로드 (JSON 형식)
- 프리셋 시스템 (Flat, Mountains, Islands, Desert)
- 기본값으로 리셋

### 5. 내보내기 (Export)
- 하이트맵 PNG 내보내기
- 바이옴 맵 PNG 내보내기

## 설치 방법 (Installation)

### 자동 설치 (Python 스크립트 사용)

1. 언리얼 에디터를 실행합니다
2. 메뉴: **Tools > Execute Python Script...**
3. 다음 파일을 선택합니다:
   ```
   Plugins/HarmoniaKit/Source/HarmoniaEditor/WorldGenerator/CreateWorldEditorWidget.py
   ```
4. 스크립트가 자동으로 에디터 유틸리티 위젯을 생성합니다

### 수동 설치 (Blueprint 직접 생성)

1. **콘텐츠 브라우저**에서 우클릭
2. **Editor Utilities > Editor Utility Widget** 선택
3. 이름을 `EUW_WorldGenerator`로 지정
4. 더블클릭하여 열기
5. **Graph** 탭에서 **Class Settings** 클릭
6. **Parent Class**를 `HarmoniaWorldEditorUtility`로 변경
7. **Designer** 탭으로 전환

### UI 레이아웃 구성 (수동 설치 시)

**Designer** 탭에서 다음 UI 요소들을 추가합니다:

#### 기본 설정 섹션
```
[Vertical Box]
  └─ [Category: Basic Settings]
      ├─ WorldSizeX (Spin Box)
      ├─ WorldSizeY (Spin Box)
      ├─ MaxHeight (Spin Box)
      ├─ SeaLevel (Slider 0.0-1.0)
      └─ Seed (Spin Box)
```

#### 생성 버튼 섹션
```
[Horizontal Box]
  ├─ [Button: Generate World]
  ├─ [Button: Generate Terrain Only]
  ├─ [Button: Generate Objects Only]
  └─ [Button: Quick Preview]
```

#### 지형 편집 섹션
```
[Horizontal Box]
  ├─ [Button: Raise Terrain]
  ├─ [Button: Lower Terrain]
  ├─ [Button: Flatten]
  └─ [Button: Smooth]
```

#### 시각화 섹션
```
[Horizontal Box]
  ├─ [Button: Show Biomes]
  ├─ [Button: Show Rivers]
  ├─ [Button: Show Structures]
  └─ [Button: Clear All]
```

#### 상태 표시
```
[Vertical Box]
  ├─ [Progress Bar] (Bind to: GenerationProgress)
  └─ [Text Block] (Bind to: StatusMessage)
```

### 이벤트 바인딩 (Event Binding)

각 버튼의 **OnClicked** 이벤트에 다음 함수들을 연결합니다:

- **Generate World** → `GenerateWorld()`
- **Generate Terrain Only** → `GenerateTerrainOnly()`
- **Generate Objects Only** → `GenerateObjectsOnly()`
- **Quick Preview** → `QuickPreview()`
- **Raise Terrain** → `RaiseTerrain(Location, Radius, Strength)`
- **Show Biomes** → `ShowBiomeVisualization()`
- **Clear All** → `ClearAllVisualizations()`

## 사용 방법 (Usage)

### 1. 에디터 유틸리티 위젯 열기

**방법 1**: 콘텐츠 브라우저에서 더블클릭
**방법 2**: 메뉴 > **Tools > Editor Utility Widgets > World Generator**

### 2. 기본 월드 생성

1. **Basic Settings**에서 원하는 값 설정:
   - **World Size X/Y**: 월드 크기 (타일 수, 512 권장)
   - **Max Height**: 최대 높이 (25600 권장)
   - **Sea Level**: 해수면 (0.0 ~ 1.0, 0.4 권장)
   - **Seed**: 랜덤 시드 (같은 시드 = 같은 월드)

2. **Generate World** 버튼 클릭

3. 진행률 바와 상태 메시지를 통해 생성 과정 모니터링

4. 생성 완료 후 씬에서 결과 확인

### 3. 프리셋 사용

빠른 시작을 위한 프리셋:

```cpp
// Blueprint에서 호출
ApplyPreset("Mountains")  // 높은 산맥
ApplyPreset("Islands")    // 섬 지형
ApplyPreset("Flat")       // 평탄한 지형
ApplyPreset("Desert")     // 사막 지형
```

### 4. 지형 편집

생성된 지형을 수정하려면:

1. 씬 뷰에서 편집할 위치 클릭
2. **Location**을 해당 좌표로 설정
3. **Radius**와 **Strength** 조정
4. 원하는 편집 버튼 클릭:
   - **Raise**: 지형 높이기
   - **Lower**: 지형 낮추기
   - **Flatten**: 평탄화
   - **Smooth**: 부드럽게

### 5. 시각화 및 디버깅

생성 결과를 확인하려면:

1. **Show Biomes**: 바이옴 경계와 타입 표시
2. **Show Rivers**: 강 경로 표시
3. **Show Structures**: 구조물 배치 위치 표시
4. **Clear All**: 모든 디버그 표시 제거

### 6. 설정 저장/로드

월드 설정을 재사용하려면:

```cpp
// 저장
SaveConfigToFile("C:/MyProjects/WorldConfigs/MyWorld.json")

// 로드
LoadConfigFromFile("C:/MyProjects/WorldConfigs/MyWorld.json")
```

## 고급 기능 (Advanced Features)

### 비동기 생성

큰 월드를 생성할 때 에디터가 멈추지 않도록:

```cpp
GenerateWorldAsync()  // 백그라운드에서 생성
```

진행 상황은 `OnProgressUpdateEvent`를 통해 모니터링됩니다.

### 영역별 오브젝트 생성

특정 영역에만 오브젝트를 추가하려면:

```cpp
FVector RegionCenter = FVector(25000, 25000, 0);
float RegionRadius = 10000.0f;
GenerateObjectsInRegion(RegionCenter, RegionRadius)
```

### 하이트맵/바이옴 맵 내보내기

외부 도구에서 사용하기 위해 PNG로 내보내기:

```cpp
ExportHeightmapToPNG("C:/Exports/Heightmap.png")
ExportBiomeMapToPNG("C:/Exports/BiomeMap.png")
```

## 설정 파라미터 상세 (Configuration Parameters)

### 기본 설정 (Basic Settings)

| 파라미터 | 타입 | 기본값 | 설명 |
|---------|------|--------|------|
| WorldSizeX | int32 | 512 | 월드 가로 크기 (타일) |
| WorldSizeY | int32 | 512 | 월드 세로 크기 (타일) |
| MaxHeight | float | 25600.0 | 최대 높이 (Unreal Units) |
| SeaLevel | float | 0.4 | 해수면 높이 (0.0~1.0) |
| Seed | int32 | 12345 | 랜덤 시드 |

### 노이즈 설정 (Noise Settings)

| 파라미터 | 타입 | 기본값 | 설명 |
|---------|------|--------|------|
| Octaves | int32 | 6 | 노이즈 옥타브 수 (디테일) |
| Persistence | float | 0.5 | 지속성 (0.0~1.0) |
| Lacunarity | float | 2.0 | 주파수 배율 |
| Frequency | float | 0.01 | 기본 주파수 |
| Amplitude | float | 1.0 | 진폭 |

### 침식 설정 (Erosion Settings)

| 파라미터 | 타입 | 기본값 | 설명 |
|---------|------|--------|------|
| bEnableErosion | bool | true | 침식 활성화 |
| Iterations | int32 | 50 | 침식 반복 횟수 |
| ErosionStrength | float | 0.5 | 침식 강도 |
| DepositionStrength | float | 0.5 | 퇴적 강도 |

## 이벤트 (Events)

### OnProgressUpdateEvent
생성 진행 중 호출됨
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGenerationProgress, float, Progress, const FString&, Message);
```

### OnGenerationCompleteEvent
생성 완료 시 호출됨
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationComplete, bool, bSuccess);
```

## 문제 해결 (Troubleshooting)

### 랜드스케이프가 생성되지 않음
- **bAutoCreateLandscape**가 `true`인지 확인
- 또는 수동으로 **FindExistingLandscape()** 또는 **CreateNewLandscape()** 호출

### 생성이 멈춤
- 큰 월드의 경우 **GenerateWorldAsync()** 사용
- 또는 월드 크기를 줄여서 테스트

### 시각화가 보이지 않음
- **Clear All**을 먼저 클릭한 후 다시 시각화
- 카메라 위치를 월드 중심으로 이동

### 오브젝트가 생성되지 않음
- **WorldConfig**에 오브젝트 액터 클래스가 설정되었는지 확인
- **BiomeSettings**에서 오브젝트 확률이 0보다 큰지 확인

## API 레퍼런스 (API Reference)

전체 API 문서는 다음 파일을 참조하세요:
- Header: `HarmoniaEditor/Public/WorldGenerator/HarmoniaWorldEditorUtility.h`
- Implementation: `HarmoniaEditor/Private/WorldGenerator/HarmoniaWorldEditorUtility.cpp`

## 예제 (Examples)

### 예제 1: 산악 지형 생성
```cpp
// Blueprint에서
ApplyPreset("Mountains");
WorldSizeX = 1024;
WorldSizeY = 1024;
Seed = 67890;
GenerateWorld();
```

### 예제 2: 섬 생성 후 편집
```cpp
// 섬 생성
ApplyPreset("Islands");
GenerateWorld();

// 중앙에 화산 생성
FVector VolcanoLocation = FVector(51200, 51200, 0);
CreateHill(VolcanoLocation, 5000.0f, 10000.0f);
CreateCrater(VolcanoLocation, 2000.0f, 3000.0f);
```

### 예제 3: 단계별 생성
```cpp
// 1단계: 지형만 생성
GenerateTerrainOnly();

// 2단계: 바이옴 확인
ShowBiomeVisualization();

// 3단계: 만족하면 오브젝트 추가
GenerateObjectsOnly();
```

## 라이센스 (License)

Copyright RedFlowering. All Rights Reserved.

## 지원 (Support)

문제가 발생하거나 기능 요청이 있으면 GitHub Issues에 등록해주세요.
