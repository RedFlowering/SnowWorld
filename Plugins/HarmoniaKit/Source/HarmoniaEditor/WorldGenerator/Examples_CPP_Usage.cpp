// Copyright RedFlowering. All Rights Reserved.

/*
 * 하모니아 월드 에디터 유틸리티 C++ 사용 예제
 * Harmonia World Editor Utility C++ Usage Examples
 *
 * 이 파일은 컴파일되지 않는 예제 코드입니다.
 * This file contains example code that is not meant to be compiled.
 *
 * C++에서 월드 에디터 유틸리티를 프로그래밍 방식으로 사용하는 방법을 보여줍니다.
 * Shows how to use the World Editor Utility programmatically from C++.
 */

#if 0 // Example code - not compiled

#include "WorldGenerator/HarmoniaWorldEditorUtility.h"
#include "HarmoniaWorldGeneratorSubsystem.h"

//=============================================================================
// 예제 1: 에디터 유틸리티 인스턴스 생성 및 기본 월드 생성
// Example 1: Creating Editor Utility Instance and Basic World Generation
//=============================================================================

void Example1_CreateAndGenerateWorld()
{
	// 에디터 유틸리티 생성
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	if (!EditorUtility)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create editor utility"));
		return;
	}

	// 기본 설정
	EditorUtility->WorldSizeX = 512;
	EditorUtility->WorldSizeY = 512;
	EditorUtility->MaxHeight = 25600.0f;
	EditorUtility->SeaLevel = 0.4f;
	EditorUtility->Seed = FMath::Rand(); // 랜덤 시드

	// 노이즈 설정
	EditorUtility->NoiseSettings.Octaves = 6;
	EditorUtility->NoiseSettings.Persistence = 0.5f;
	EditorUtility->NoiseSettings.Lacunarity = 2.0f;
	EditorUtility->NoiseSettings.Frequency = 0.01f;
	EditorUtility->NoiseSettings.Amplitude = 1.0f;

	// 랜드스케이프 자동 생성 활성화
	EditorUtility->bAutoCreateLandscape = true;

	// 월드 생성 시작
	EditorUtility->GenerateWorld();

	UE_LOG(LogTemp, Log, TEXT("World generation started with seed: %d"), EditorUtility->Seed);
}

//=============================================================================
// 예제 2: 프리셋을 사용한 빠른 생성
// Example 2: Quick Generation with Presets
//=============================================================================

void Example2_GenerateWithPreset()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 산악 지형 프리셋 적용
	EditorUtility->ApplyPreset(TEXT("Mountains"));

	// 월드 크기 설정
	EditorUtility->WorldSizeX = 1024;
	EditorUtility->WorldSizeY = 1024;

	// 생성
	EditorUtility->GenerateWorld();
}

//=============================================================================
// 예제 3: 단계별 생성 (지형 → 바이옴 확인 → 오브젝트)
// Example 3: Step-by-Step Generation (Terrain → Biome Check → Objects)
//=============================================================================

void Example3_StepByStepGeneration()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 설정
	EditorUtility->WorldSizeX = 512;
	EditorUtility->WorldSizeY = 512;
	EditorUtility->Seed = 42;

	// 1단계: 지형만 생성
	EditorUtility->GenerateTerrainOnly();

	// 2단계: 바이옴 맵 생성 및 시각화
	EditorUtility->GenerateBiomeMapOnly();
	EditorUtility->ShowBiomeVisualization();

	// 필요시 설정 조정...

	// 3단계: 만족하면 오브젝트 배치
	EditorUtility->GenerateObjectsOnly();
}

//=============================================================================
// 예제 4: 비동기 생성 with 진행 상황 모니터링
// Example 4: Async Generation with Progress Monitoring
//=============================================================================

// 진행 콜백
UFUNCTION()
void OnWorldGenerationProgress(float Progress, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("Generation Progress: %.1f%% - %s"), Progress * 100.0f, *Message);
}

// 완료 콜백
UFUNCTION()
void OnWorldGenerationComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("World generation completed successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("World generation failed!"));
	}
}

void Example4_AsyncGenerationWithCallbacks()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 콜백 바인딩
	EditorUtility->OnProgressUpdateEvent.AddDynamic(this, &ThisClass::OnWorldGenerationProgress);
	EditorUtility->OnGenerationCompleteEvent.AddDynamic(this, &ThisClass::OnWorldGenerationComplete);

	// 설정
	EditorUtility->WorldSizeX = 1024;
	EditorUtility->WorldSizeY = 1024;
	EditorUtility->Seed = 12345;

	// 비동기 생성 시작
	EditorUtility->GenerateWorldAsync();

	// 에디터는 멈추지 않고 계속 작업 가능
}

//=============================================================================
// 예제 5: 지형 편집
// Example 5: Terrain Editing
//=============================================================================

void Example5_TerrainEditing()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 먼저 기본 지형 생성
	EditorUtility->ApplyPreset(TEXT("Flat"));
	EditorUtility->GenerateTerrainOnly();

	// 월드 중앙에 화산 만들기
	FVector VolcanoCenter(25600.0f, 25600.0f, 0.0f);

	// 1. 큰 언덕 생성 (화산 본체)
	EditorUtility->CreateHill(VolcanoCenter, 8000.0f, 15000.0f);

	// 2. 분화구 생성 (화산 정상)
	FVector CraterLocation = VolcanoCenter + FVector(0, 0, 15000.0f);
	EditorUtility->CreateCrater(CraterLocation, 3000.0f, 5000.0f);

	// 3. 주변 지형 부드럽게
	EditorUtility->SmoothTerrain(VolcanoCenter, 12000.0f, 5);

	UE_LOG(LogTemp, Log, TEXT("Volcano created at %s"), *VolcanoCenter.ToString());
}

//=============================================================================
// 예제 6: 특정 영역에만 오브젝트 추가
// Example 6: Adding Objects to Specific Region
//=============================================================================

void Example6_RegionalObjectPlacement()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 지형은 이미 생성되어 있다고 가정

	// 특정 영역 정의 (예: 숲 지역)
	FVector ForestCenter(30000.0f, 30000.0f, 0.0f);
	float ForestRadius = 10000.0f;

	// 해당 영역에만 나무와 오브젝트 생성
	EditorUtility->GenerateObjectsInRegion(ForestCenter, ForestRadius);

	UE_LOG(LogTemp, Log, TEXT("Objects placed in forest region at %s"), *ForestCenter.ToString());

	// 다른 영역 (예: 마을)
	FVector VillageCenter(10000.0f, 10000.0f, 0.0f);
	float VillageRadius = 5000.0f;

	EditorUtility->GenerateObjectsInRegion(VillageCenter, VillageRadius);
}

//=============================================================================
// 예제 7: 설정 저장 및 로드
// Example 7: Configuration Save and Load
//=============================================================================

void Example7_SaveAndLoadConfiguration()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 산악 설정 구성
	EditorUtility->ApplyPreset(TEXT("Mountains"));
	EditorUtility->WorldSizeX = 2048;
	EditorUtility->WorldSizeY = 2048;
	EditorUtility->MaxHeight = 51200.0f;
	EditorUtility->Seed = 99999;

	// 고급 노이즈 설정
	EditorUtility->NoiseSettings.Octaves = 8;
	EditorUtility->NoiseSettings.Persistence = 0.6f;
	EditorUtility->NoiseSettings.Amplitude = 2.0f;

	// 설정 저장
	FString ConfigPath = FPaths::ProjectSavedDir() / TEXT("WorldConfigs/MountainWorld.json");
	if (EditorUtility->SaveConfigToFile(ConfigPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Configuration saved to: %s"), *ConfigPath);
	}

	// 나중에 로드
	UHarmoniaWorldEditorUtility* NewUtility = NewObject<UHarmoniaWorldEditorUtility>();
	if (NewUtility->LoadConfigFromFile(ConfigPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Configuration loaded from: %s"), *ConfigPath);

		// 같은 설정으로 새 월드 생성
		NewUtility->GenerateWorld();
	}
}

//=============================================================================
// 예제 8: 시각화 및 디버깅
// Example 8: Visualization and Debugging
//=============================================================================

void Example8_VisualizationAndDebugging()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 월드 생성
	EditorUtility->GenerateWorld();

	// 바이옴 경계 시각화
	EditorUtility->ShowBiomeVisualization();

	// 강 경로 시각화
	EditorUtility->ShowRiverVisualization();

	// 구조물 배치 시각화
	EditorUtility->ShowStructureVisualization();

	// 동굴 입구 시각화
	EditorUtility->ShowCaveVisualization();

	// 자원 분포 시각화
	EditorUtility->ShowResourceVisualization();

	// POI 시각화
	EditorUtility->ShowPOIVisualization();

	// 모든 시각화 제거
	// EditorUtility->ClearAllVisualizations();
}

//=============================================================================
// 예제 9: 하이트맵/바이옴 맵 내보내기
// Example 9: Exporting Heightmap/Biome Map
//=============================================================================

void Example9_ExportMaps()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 월드 생성
	EditorUtility->WorldSizeX = 1024;
	EditorUtility->WorldSizeY = 1024;
	EditorUtility->GenerateWorld();

	// 하이트맵 내보내기
	FString HeightmapPath = FPaths::ProjectSavedDir() / TEXT("Exports/Heightmap.png");
	if (EditorUtility->ExportHeightmapToPNG(HeightmapPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Heightmap exported to: %s"), *HeightmapPath);
	}

	// 바이옴 맵 내보내기
	FString BiomePath = FPaths::ProjectSavedDir() / TEXT("Exports/BiomeMap.png");
	if (EditorUtility->ExportBiomeMapToPNG(BiomePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Biome map exported to: %s"), *BiomePath);
	}
}

//=============================================================================
// 예제 10: 복잡한 월드 시나리오 - 화산 섬 생성
// Example 10: Complex World Scenario - Volcanic Island Creation
//=============================================================================

void Example10_CreateVolcanicIsland()
{
	UHarmoniaWorldEditorUtility* EditorUtility = NewObject<UHarmoniaWorldEditorUtility>();

	// 섬 지형 기본 생성
	EditorUtility->ApplyPreset(TEXT("Islands"));
	EditorUtility->WorldSizeX = 1024;
	EditorUtility->WorldSizeY = 1024;
	EditorUtility->SeaLevel = 0.65f; // 높은 해수면으로 작은 섬들 생성
	EditorUtility->Seed = 88888;

	// 지형만 먼저 생성
	EditorUtility->GenerateTerrainOnly();

	// 섬 위치들 (예시)
	TArray<FVector> IslandLocations = {
		FVector(30000, 30000, 0),
		FVector(70000, 50000, 0),
		FVector(50000, 80000, 0)
	};

	// 각 섬에 화산 추가
	for (const FVector& IslandCenter : IslandLocations)
	{
		// 화산 본체
		EditorUtility->CreateHill(IslandCenter, 8000.0f, 12000.0f);

		// 화산 분화구
		FVector CraterPos = IslandCenter + FVector(0, 0, 12000.0f);
		EditorUtility->CreateCrater(CraterPos, 2500.0f, 4000.0f);

		// 주변 부드럽게
		EditorUtility->SmoothTerrain(IslandCenter, 10000.0f, 3);

		UE_LOG(LogTemp, Log, TEXT("Volcanic island created at %s"), *IslandCenter.ToString());
	}

	// 바이옴 재생성
	EditorUtility->GenerateBiomeMapOnly();

	// 각 섬에 열대 식물 오브젝트 배치
	for (const FVector& IslandCenter : IslandLocations)
	{
		EditorUtility->GenerateObjectsInRegion(IslandCenter, 12000.0f);
	}

	// 시각화로 확인
	EditorUtility->ShowBiomeVisualization();
	EditorUtility->ShowStructureVisualization();

	UE_LOG(LogTemp, Log, TEXT("Volcanic island world completed!"));
}

//=============================================================================
// 예제 11: 서브시스템 직접 사용
// Example 11: Direct Subsystem Usage
//=============================================================================

void Example11_DirectSubsystemUsage()
{
	// 게임 인스턴스에서 서브시스템 가져오기
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		if (UGameInstance* GameInstance = EditorWorld->GetGameInstance())
		{
			UHarmoniaWorldGeneratorSubsystem* GenSubsystem =
				GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>();

			if (GenSubsystem)
			{
				// WorldConfig 직접 구성
				FWorldGeneratorConfig Config;
				Config.Seed = 12345;
				Config.SizeX = 512;
				Config.SizeY = 512;
				Config.MaxHeight = 25600.0f;
				Config.SeaLevel = 0.4f;

				// 노이즈 설정
				Config.NoiseSettings.Octaves = 6;
				Config.NoiseSettings.Persistence = 0.5f;
				Config.NoiseSettings.Lacunarity = 2.0f;
				Config.NoiseSettings.Frequency = 0.01f;
				Config.NoiseSettings.Amplitude = 1.0f;

				// 월드 생성
				GenSubsystem->GenerateWorld(Config);

				UE_LOG(LogTemp, Log, TEXT("World generated via subsystem"));
			}
		}
	}
}

#endif // Example code
