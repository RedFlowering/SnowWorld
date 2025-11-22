# Harmonia World Generator

시드 기반 프로시저럴 월드 생성 시스템

## 주요 기능

### 1. 시드 기반 결정적 생성 (Deterministic Generation)
- 동일한 시드로 멀티플레이에서 동일한 월드 생성 보장
- 위치 기반 시드를 사용하여 청크별 독립적 생성 가능
- FRandomStream을 활용한 예측 가능한 랜덤 생성

### 2. 청크 기반 처리 (Chunk-Based Processing)
- 대용량 맵 생성 시 에디터 무한 루프 방지
- 청크 단위로 분할 처리하여 성능 최적화
- 기본 청크 크기: 64x64 타일 (설정 가능)

### 3. 멀티 옥타브 펄린 노이즈
- 자연스러운 지형 생성
- 옥타브, 주파수, 진폭 등 세밀한 조정 가능
- Earth-like, Turbulence, Ridged 등 다양한 노이즈 타입 지원

### 4. 유연한 오브젝트 배치
- 타입별 확률 기반 오브젝트 생성
- 높이, 경사도 기반 배치 검증
- 지역별 독립적인 오브젝트 생성 가능

## 사용 방법

### Blueprint에서 사용

```cpp
// WorldGenerator Subsystem 가져오기
UHarmoniaWorldGeneratorSubsystem* WorldGen = GetGameInstance()->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>();

// 설정 구성
FWorldGeneratorConfig Config;
Config.Seed = 12345;  // 고정 시드로 동일한 월드 생성
Config.SizeX = 512;
Config.SizeY = 512;
Config.ChunkSize = 64;  // 무한 루프 방지용 청크 크기
Config.ObjectDensity = 0.008f;
Config.SeaLevel = 0.42f;

// 노이즈 설정
Config.NoiseSettings.Octaves = 6;
Config.NoiseSettings.Persistence = 0.5f;
Config.NoiseSettings.Lacunarity = 2.0f;
Config.NoiseSettings.Frequency = 1.0f;

// 오브젝트 타입 확률 설정
Config.ObjectTypeProbabilities.Add(EWorldObjectType::Tree, 50.0f);
Config.ObjectTypeProbabilities.Add(EWorldObjectType::Rock, 30.0f);
Config.ObjectTypeProbabilities.Add(EWorldObjectType::Resource, 20.0f);

// 액터 클래스 매핑
TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap;
ActorClassMap.Add(EWorldObjectType::Tree, TreeActorClass);
ActorClassMap.Add(EWorldObjectType::Rock, RockActorClass);

// 월드 생성
TArray<int32> HeightData;
TArray<FWorldObjectData> Objects;
WorldGen->GenerateWorld(Config, HeightData, Objects, ActorClassMap);
```

### 고급 사용법

#### 1. Heightmap만 생성 (오브젝트 없이)
```cpp
TArray<int32> HeightData;
WorldGen->GenerateHeightmapOnly(Config, HeightData);
```

#### 2. 특정 지역의 오브젝트만 생성 (스트리밍 월드)
```cpp
WorldGen->GenerateObjectsInRegion(
    Config,
    HeightData,
    ActorClassMap,
    MinX, MinY,  // 지역 시작점
    MaxX, MaxY,  // 지역 끝점
    OutObjects
);
```

## 성능 최적화

### 무한 루프 방지
- `ChunkSize`: 한 번에 처리할 타일 수 (기본값: 64)
  - 작을수록 안전하지만 오버헤드 증가
  - 큰 값일수록 빠르지만 에디터가 멈춘 것처럼 보일 수 있음

### 진행 상황 로깅
```cpp
Config.bEnableProgressLogging = true;  // 진행 상황 로그 활성화
```

### 메모리 최적화
- 오브젝트 배열은 예상 개수만큼 미리 Reserve
- HeightData는 SetNumUninitialized로 빠른 할당

## 멀티플레이 동기화

### 시드 동기화 방법
1. 서버에서 시드 생성
2. 클라이언트에게 시드 전송
3. 모든 클라이언트가 동일한 시드로 월드 생성

```cpp
// 서버
int32 WorldSeed = FMath::Rand();
ReplicateWorldSeed(WorldSeed);

// 클라이언트
void OnWorldSeedReceived(int32 Seed)
{
    FWorldGeneratorConfig Config;
    Config.Seed = Seed;
    // ... 나머지 설정
    WorldGen->GenerateWorld(Config, HeightData, Objects, ActorClassMap);
}
```

## 설정 가이드

### 지형 설정
- `SizeX, SizeY`: 월드 크기 (2의 제곱수 권장)
- `MaxHeight`: 최대 지형 높이 (언리얼 단위)
- `SeaLevel`: 해수면 높이 (0.0 ~ 1.0)

### 노이즈 설정
- `Octaves`: 레이어 수 (높을수록 디테일 증가, 6 권장)
- `Persistence`: 각 옥타브의 진폭 감소율 (0.5 권장)
- `Lacunarity`: 각 옥타브의 주파수 증가율 (2.0 권장)
- `Frequency`: 기본 주파수 (높을수록 빠른 변화)

### 오브젝트 설정
- `ObjectDensity`: 오브젝트 밀도 (0.0 ~ 1.0)
  - 0.001 = 매우 희소
  - 0.01 = 적당
  - 0.1 = 매우 조밀

## 알려진 제한사항

1. 매우 큰 맵 (1024x1024 이상)은 생성 시간이 오래 걸릴 수 있음
   - 해결방법: ChunkSize를 32 이하로 설정하고 진행 로깅 활성화

2. 오브젝트 타입 확률의 합이 0이면 오브젝트가 생성되지 않음
   - 최소 하나의 타입에 0보다 큰 확률 설정 필요

3. 시드가 0일 때도 정상 작동하지만, 다른 값 사용 권장

## 문제 해결

### 에디터가 멈춰 보임
```cpp
Config.ChunkSize = 32;  // 청크 크기 감소
Config.bEnableProgressLogging = true;  // 로그 활성화
```

### 오브젝트가 생성되지 않음
- ObjectTypeProbabilities에 하나 이상의 타입 추가 확인
- ObjectDensity가 0보다 큰지 확인
- ActorClassMap에 해당 타입의 클래스가 있는지 확인

### 멀티플레이에서 월드가 다르게 생성됨
- 모든 클라이언트가 동일한 Config.Seed 사용하는지 확인
- Config의 모든 파라미터가 동일한지 확인

## 버전 히스토리

### v2.0 (2025-11-15)
- 청크 기반 생성 시스템으로 리팩토링
- 무한 루프 방지 로직 추가
- 시드 기반 결정적 생성 보장 강화
- 진행 상황 로깅 기능 추가
- 지역별 오브젝트 생성 기능 추가
- 메모리 최적화
- 다양한 노이즈 타입 추가 (Turbulence, Ridged)

### v1.0 (초기 버전)
- 기본 Perlin 노이즈 기반 지형 생성
- 간단한 오브젝트 배치
