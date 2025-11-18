# 하모니아 월드 에디터 - 빠른 시작 가이드

## 5분 안에 시작하기

### 1단계: 플러그인 컴파일 (이미 완료되어 있어야 함)

프로젝트를 열면 HarmoniaKit 플러그인이 자동으로 로드됩니다.

### 2단계: 에디터 위젯 생성

**자동 방법 (추천)**:

1. 언리얼 에디터에서 **Tools > Execute Python Script...** 선택
2. 다음 경로의 파일을 선택:
   ```
   Plugins/HarmoniaKit/Source/HarmoniaEditor/WorldGenerator/CreateWorldEditorWidget.py
   ```
3. 스크립트가 자동으로 위젯을 생성하고 엽니다

**수동 방법**:

1. 콘텐츠 브라우저에서 우클릭
2. **Editor Utilities > Editor Utility Widget** 선택
3. 이름: `EUW_HarmoniaWorldGenerator`
4. 열어서 **Class Settings** 클릭
5. **Parent Class**를 `HarmoniaWorldEditorUtility`로 변경

### 3단계: 간단한 UI 만들기

Designer 탭에서:

1. **Canvas Panel** 추가 (루트)
2. **Vertical Box** 추가
3. 다음 버튼들 추가:
   - `Generate World` 버튼
   - `Quick Preview` 버튼
   - `Clear All` 버튼
4. **Progress Bar** 추가
5. **Text Block** (상태 메시지용) 추가

### 4단계: 이벤트 연결

Graph 탭으로 전환:

#### Generate World 버튼:
1. 버튼 선택 → Details 패널에서 **OnClicked** 옆 `+` 클릭
2. 노드 생성됨 → 우클릭 → "Generate World" 검색
3. `Generate World` 함수 연결

#### Quick Preview 버튼:
1. 동일한 방법으로 **OnClicked** 이벤트 추가
2. `Quick Preview` 함수 연결

#### Clear All 버튼:
1. 동일한 방법으로 **OnClicked** 이벤트 추가
2. `Clear All Visualizations` 함수 연결

#### Progress Bar 바인딩:
1. Progress Bar 선택
2. **Percent** 옆 **Bind** 클릭 → **Create Binding**
3. 생성된 함수에서 `Get Generation Progress` 노드 추가하고 Return Value에 연결

#### Status Text 바인딩:
1. Text Block 선택
2. **Text** 옆 **Bind** 클릭 → **Create Binding**
3. `Get Status Message` 노드 추가하고 Return Value에 연결

### 5단계: 컴파일 및 저장

1. 상단의 **Compile** 버튼 클릭
2. **Save** 버튼 클릭

### 6단계: 사용하기!

1. 콘텐츠 브라우저에서 위젯을 더블클릭하여 실행
2. 또는 **Tools > Editor Utility Widgets > [위젯 이름]**

## 첫 월드 생성해보기

### 테스트용 작은 월드:

1. 위젯 실행
2. **World Size X**: 256
3. **World Size Y**: 256
4. **Seed**: 12345 (원하는 숫자)
5. **Quick Preview** 버튼 클릭

약 10-30초 후 작은 테스트 월드가 생성됩니다.

### 실제 크기 월드:

1. **World Size X**: 512
2. **World Size Y**: 512
3. **Max Height**: 25600
4. **Sea Level**: 0.4
5. **Seed**: (원하는 랜덤 숫자)
6. **Generate World** 버튼 클릭

생성 시간: 약 1-5분 (컴퓨터 사양에 따라)

## 프리셋 사용하기

빠르게 시작하려면 프리셋을 사용하세요:

### Graph에서 버튼 이벤트에 연결:

```
OnClicked (Preset Button)
  → Apply Preset (PresetName = "Mountains")
  → Generate World
```

**사용 가능한 프리셋**:
- `"Flat"` - 평평한 지형
- `"Mountains"` - 산악 지형
- `"Islands"` - 섬 지형
- `"Desert"` - 사막 지형

## 지형 편집하기

1. 월드 생성 후 씬 뷰에서 편집하고 싶은 위치 찾기
2. 해당 위치의 좌표 확인 (예: X=10000, Y=10000, Z=0)
3. Graph에서:

```blueprint
OnClicked (Raise Button)
  → Raise Terrain
      Location: (10000, 10000, 0)
      Radius: 2000
      Strength: 1000
```

## 바이옴 확인하기

생성된 월드의 바이옴을 시각적으로 보려면:

1. **Show Biomes** 버튼 클릭
2. 씬 뷰에서 색상으로 표시된 바이옴 경계 확인
3. 확인 후 **Clear All** 버튼으로 제거

## 설정 저장하기

마음에 드는 설정을 재사용하려면:

### Graph에서:

```blueprint
OnClicked (Save Button)
  → Save Config To File
      File Path: "C:/MyConfigs/MyWorld.json"
```

### 불러오기:

```blueprint
OnClicked (Load Button)
  → Load Config From File
      File Path: "C:/MyConfigs/MyWorld.json"
  → Generate World
```

## 진행 상황 모니터링

### 이벤트 바인딩 (Graph 탭):

```blueprint
Event Construct
  → Bind Event to On Progress Update Event
      Event: UpdateProgressUI

UpdateProgressUI (Custom Event)
  → Set Progress Bar Percent (Progress)
  → Set Status Text (Message)
```

## 비동기 생성 (큰 월드용)

에디터가 멈추지 않게 하려면:

```blueprint
OnClicked (Generate Async Button)
  → Generate World Async
```

생성 중에도 에디터를 사용할 수 있습니다.

## 문제 해결

### "Subsystem not found" 오류
→ 에디터를 재시작하고 플러그인이 활성화되어 있는지 확인

### 랜드스케이프가 안 보임
→ `bAutoCreateLandscape`를 `true`로 설정
→ 또는 World Outliner에서 Landscape 액터 찾기

### 생성이 너무 느림
→ 월드 크기를 줄이세요 (256x256부터 시작)
→ 또는 `Generate World Async` 사용

### 오브젝트가 안 보임
→ WorldConfig에서 BiomeSettings를 확인하세요
→ 오브젝트 액터 클래스가 설정되어 있는지 확인

## 다음 단계

더 자세한 내용은 다음 문서를 참조하세요:
- `README_WorldEditorUtility.md` - 전체 문서
- `HarmoniaWorldEditorUtility.h` - API 레퍼런스

즐거운 월드 제작 되세요! 🎮🌍
