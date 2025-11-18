// Copyright RedFlowering. All Rights Reserved.

#include "WorldGenerator/HarmoniaWorldEditorUtility.h"
#include "HarmoniaWorldGeneratorSubsystem.h"
#include "WorldGenerator/HarmoniaWorldGeneratorEditorSubsystem.h"
#include "Landscape.h"
#include "LandscapeEditorObject.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "FileHelpers.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "EngineUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

UHarmoniaWorldEditorUtility::UHarmoniaWorldEditorUtility()
	: CachedWorldGenSubsystem(nullptr)
	, CachedEditorSubsystem(nullptr)
{
	// 기본 노이즈 설정 초기화
	NoiseSettings.Octaves = 6;
	NoiseSettings.Persistence = 0.5f;
	NoiseSettings.Lacunarity = 2.0f;
	NoiseSettings.Frequency = 0.01f;
	NoiseSettings.Amplitude = 1.0f;

	// 기본 침식 설정
	ErosionSettings.bEnableErosion = true;
	ErosionSettings.Iterations = 50;
	ErosionSettings.ErosionStrength = 0.5f;
	ErosionSettings.DepositionStrength = 0.5f;
	ErosionSettings.EvaporationRate = 0.01f;

	// 기본 강 설정
	RiverSettings.RiverCount = 5;
	RiverSettings.MinRiverWidth = 500.0f;
	RiverSettings.MaxRiverWidth = 2000.0f;

	// 기본 호수 설정
	LakeSettings.LakeCount = 10;
	LakeSettings.MinLakeRadius = 1000.0f;
	LakeSettings.MaxLakeRadius = 5000.0f;

	// 기본 동굴 설정
	CaveSettings.bEnableCaves = true;
	CaveSettings.CaveDensity = 0.3f;
	CaveSettings.CaveDepthMin = -5000.0f;
	CaveSettings.CaveDepthMax = -500.0f;

	// 기본 POI 설정
	POISettings.POICount = 20;
	POISettings.MinPOIDistance = 5000.0f;
}

//=============================================================================
// Subsystem Access
//=============================================================================

UHarmoniaWorldGeneratorSubsystem* UHarmoniaWorldEditorUtility::GetWorldGeneratorSubsystem() const
{
	if (!CachedWorldGenSubsystem && GEditor)
	{
		if (UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
		{
			if (UGameInstance* GameInstance = EditorWorld->GetGameInstance())
			{
				CachedWorldGenSubsystem = GameInstance->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>();
			}
		}
	}
	return CachedWorldGenSubsystem;
}

UHarmoniaWorldGeneratorEditorSubsystem* UHarmoniaWorldEditorUtility::GetEditorSubsystem() const
{
	if (!CachedEditorSubsystem && GEditor)
	{
		CachedEditorSubsystem = GEditor->GetEditorSubsystem<UHarmoniaWorldGeneratorEditorSubsystem>();
	}
	return CachedEditorSubsystem;
}

//=============================================================================
// Configuration Management
//=============================================================================

void UHarmoniaWorldEditorUtility::UpdateWorldConfigFromProperties()
{
	WorldConfig.Seed = Seed;
	WorldConfig.SizeX = WorldSizeX;
	WorldConfig.SizeY = WorldSizeY;
	WorldConfig.MaxHeight = MaxHeight;
	WorldConfig.SeaLevel = SeaLevel;
	WorldConfig.NoiseSettings = NoiseSettings;
	WorldConfig.ErosionSettings = ErosionSettings;
	WorldConfig.BiomeSettings = BiomeSettingsList;
	WorldConfig.RiverSettings = RiverSettings;
	WorldConfig.LakeSettings = LakeSettings;
	WorldConfig.CaveSettings = CaveSettings;
	WorldConfig.POISettings = POISettings;
	WorldConfig.EnvironmentSettings = EnvironmentSettings;
}

void UHarmoniaWorldEditorUtility::ApplyWorldConfigToProperties()
{
	Seed = WorldConfig.Seed;
	WorldSizeX = WorldConfig.SizeX;
	WorldSizeY = WorldConfig.SizeY;
	MaxHeight = WorldConfig.MaxHeight;
	SeaLevel = WorldConfig.SeaLevel;
	NoiseSettings = WorldConfig.NoiseSettings;
	ErosionSettings = WorldConfig.ErosionSettings;
	BiomeSettingsList = WorldConfig.BiomeSettings;
	RiverSettings = WorldConfig.RiverSettings;
	LakeSettings = WorldConfig.LakeSettings;
	CaveSettings = WorldConfig.CaveSettings;
	POISettings = WorldConfig.POISettings;
	EnvironmentSettings = WorldConfig.EnvironmentSettings;
}

void UHarmoniaWorldEditorUtility::ResetToDefaults()
{
	WorldSizeX = 512;
	WorldSizeY = 512;
	MaxHeight = 25600.0f;
	SeaLevel = 0.4f;
	Seed = FMath::Rand();

	NoiseSettings = FPerlinNoiseSettings();
	NoiseSettings.Octaves = 6;
	NoiseSettings.Persistence = 0.5f;
	NoiseSettings.Lacunarity = 2.0f;
	NoiseSettings.Frequency = 0.01f;
	NoiseSettings.Amplitude = 1.0f;

	ErosionSettings = FErosionSettings();
	RiverSettings = FRiverSettings();
	LakeSettings = FLakeSettings();
	CaveSettings = FCaveSettings();
	POISettings = FPOISettings();
	EnvironmentSettings = FEnvironmentSystemSettings();

	StatusMessage = TEXT("Configuration reset to defaults");
	UE_LOG(LogTemp, Log, TEXT("World generator configuration reset to defaults"));
}

bool UHarmoniaWorldEditorUtility::SaveConfigToFile(const FString& FilePath)
{
	UpdateWorldConfigFromProperties();

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// 기본 설정 저장
	JsonObject->SetNumberField(TEXT("Seed"), WorldConfig.Seed);
	JsonObject->SetNumberField(TEXT("SizeX"), WorldConfig.SizeX);
	JsonObject->SetNumberField(TEXT("SizeY"), WorldConfig.SizeY);
	JsonObject->SetNumberField(TEXT("MaxHeight"), WorldConfig.MaxHeight);
	JsonObject->SetNumberField(TEXT("SeaLevel"), WorldConfig.SeaLevel);

	// 노이즈 설정
	TSharedPtr<FJsonObject> NoiseJson = MakeShareable(new FJsonObject);
	NoiseJson->SetNumberField(TEXT("Octaves"), NoiseSettings.Octaves);
	NoiseJson->SetNumberField(TEXT("Persistence"), NoiseSettings.Persistence);
	NoiseJson->SetNumberField(TEXT("Lacunarity"), NoiseSettings.Lacunarity);
	NoiseJson->SetNumberField(TEXT("Frequency"), NoiseSettings.Frequency);
	NoiseJson->SetNumberField(TEXT("Amplitude"), NoiseSettings.Amplitude);
	JsonObject->SetObjectField(TEXT("NoiseSettings"), NoiseJson);

	// JSON을 문자열로 변환
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
	{
		if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
		{
			StatusMessage = FString::Printf(TEXT("Configuration saved to: %s"), *FilePath);
			UE_LOG(LogTemp, Log, TEXT("World configuration saved to: %s"), *FilePath);
			return true;
		}
	}

	StatusMessage = TEXT("Failed to save configuration");
	UE_LOG(LogTemp, Error, TEXT("Failed to save world configuration to: %s"), *FilePath);
	return false;
}

bool UHarmoniaWorldEditorUtility::LoadConfigFromFile(const FString& FilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		StatusMessage = TEXT("Failed to load configuration file");
		UE_LOG(LogTemp, Error, TEXT("Failed to load configuration from: %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		StatusMessage = TEXT("Failed to parse configuration file");
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON from: %s"), *FilePath);
		return false;
	}

	// 기본 설정 로드
	Seed = JsonObject->GetIntegerField(TEXT("Seed"));
	WorldSizeX = JsonObject->GetIntegerField(TEXT("SizeX"));
	WorldSizeY = JsonObject->GetIntegerField(TEXT("SizeY"));
	MaxHeight = JsonObject->GetNumberField(TEXT("MaxHeight"));
	SeaLevel = JsonObject->GetNumberField(TEXT("SeaLevel"));

	// 노이즈 설정 로드
	if (JsonObject->HasField(TEXT("NoiseSettings")))
	{
		TSharedPtr<FJsonObject> NoiseJson = JsonObject->GetObjectField(TEXT("NoiseSettings"));
		NoiseSettings.Octaves = NoiseJson->GetIntegerField(TEXT("Octaves"));
		NoiseSettings.Persistence = NoiseJson->GetNumberField(TEXT("Persistence"));
		NoiseSettings.Lacunarity = NoiseJson->GetNumberField(TEXT("Lacunarity"));
		NoiseSettings.Frequency = NoiseJson->GetNumberField(TEXT("Frequency"));
		NoiseSettings.Amplitude = NoiseJson->GetNumberField(TEXT("Amplitude"));
	}

	UpdateWorldConfigFromProperties();

	StatusMessage = FString::Printf(TEXT("Configuration loaded from: %s"), *FilePath);
	UE_LOG(LogTemp, Log, TEXT("World configuration loaded from: %s"), *FilePath);
	return true;
}

void UHarmoniaWorldEditorUtility::ApplyPreset(const FString& PresetName)
{
	if (PresetName == TEXT("Flat"))
	{
		NoiseSettings.Amplitude = 0.1f;
		NoiseSettings.Octaves = 2;
		ErosionSettings.bEnableErosion = false;
		StatusMessage = TEXT("Applied Flat World preset");
	}
	else if (PresetName == TEXT("Mountains"))
	{
		NoiseSettings.Amplitude = 2.0f;
		NoiseSettings.Octaves = 8;
		MaxHeight = 51200.0f;
		StatusMessage = TEXT("Applied Mountains preset");
	}
	else if (PresetName == TEXT("Islands"))
	{
		SeaLevel = 0.6f;
		NoiseSettings.Frequency = 0.005f;
		StatusMessage = TEXT("Applied Islands preset");
	}
	else if (PresetName == TEXT("Desert"))
	{
		RiverSettings.RiverCount = 1;
		LakeSettings.LakeCount = 2;
		StatusMessage = TEXT("Applied Desert preset");
	}
	else if (PresetName == TEXT("Archipelago"))
	{
		SeaLevel = 0.65f;
		NoiseSettings.Frequency = 0.005f;
		NoiseSettings.Octaves = 6;
		StatusMessage = TEXT("Applied Archipelago preset");
	}
	else if (PresetName == TEXT("Canyon"))
	{
		ErosionSettings.bEnableErosion = true;
		ErosionSettings.Iterations = 200;
		SeaLevel = 0.3f;
		NoiseSettings.Amplitude = 1.2f;
		StatusMessage = TEXT("Applied Canyon preset");
	}
	else if (PresetName == TEXT("Plains"))
	{
		NoiseSettings.Amplitude = 0.3f;
		NoiseSettings.Octaves = 3;
		SeaLevel = 0.4f;
		StatusMessage = TEXT("Applied Plains preset");
	}
	else if (PresetName == TEXT("Volcanic"))
	{
		MaxHeight = 51200.0f;
		NoiseSettings.Amplitude = 2.5f;
		SeaLevel = 0.5f;
		NoiseSettings.Octaves = 7;
		StatusMessage = TEXT("Applied Volcanic preset");
	}
	else if (PresetName == TEXT("Frozen"))
	{
		SeaLevel = 0.5f;
		NoiseSettings.Amplitude = 0.8f;
		StatusMessage = TEXT("Applied Frozen preset");
	}
	else if (PresetName == TEXT("Tropical"))
	{
		SeaLevel = 0.45f;
		NoiseSettings.Frequency = 0.008f;
		NoiseSettings.Octaves = 5;
		StatusMessage = TEXT("Applied Tropical preset");
	}
	else if (PresetName == TEXT("Highlands"))
	{
		MaxHeight = 40000.0f;
		NoiseSettings.Amplitude = 1.5f;
		SeaLevel = 0.3f;
		NoiseSettings.Octaves = 6;
		StatusMessage = TEXT("Applied Highlands preset");
	}
	else
	{
		StatusMessage = TEXT("Unknown preset");
	}

	UpdateWorldConfigFromProperties();
}

//=============================================================================
// Generation Control
//=============================================================================

void UHarmoniaWorldEditorUtility::GenerateWorld()
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		StatusMessage = TEXT("Error: World Generator Subsystem not found!");
		UE_LOG(LogTemp, Error, TEXT("Failed to get World Generator Subsystem"));
		return;
	}

	// 검증 (자동 검증이 활성화된 경우)
	if (bAutoValidate)
	{
		FValidationResult ValidationResult = ValidateConfiguration();
		if (!ValidationResult.bIsSafe)
		{
			StatusMessage = FString::Printf(TEXT("Configuration validation failed with %d errors!"), ValidationResult.Errors.Num());
			UE_LOG(LogTemp, Error, TEXT("Configuration validation failed"));
			for (const FString& Error : ValidationResult.Errors)
			{
				UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
			}
			return;
		}
	}

	// 타겟 랜드스케이프 확인 또는 생성
	if (!TargetLandscape)
	{
		TargetLandscape = FindExistingLandscape();
		if (!TargetLandscape && bAutoCreateLandscape)
		{
			TargetLandscape = CreateNewLandscape();
		}
	}

	if (!TargetLandscape)
	{
		StatusMessage = TEXT("Error: No landscape found or created!");
		UE_LOG(LogTemp, Error, TEXT("No target landscape available for world generation"));
		return;
	}

	// WorldConfig 업데이트
	UpdateWorldConfigFromProperties();

	bIsGenerating = true;
	GenerationProgress = 0.0f;
	GenerationStartTime = FPlatformTime::Seconds(); // 시간 측정 시작
	StatusMessage = TEXT("Starting world generation...");

	// 콜백 바인딩
	GenSubsystem->OnGenerationProgress.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationProgressCallback);
	GenSubsystem->OnGenerationComplete.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback);

	// 월드 생성 시작
	GenSubsystem->GenerateWorld(WorldConfig);

	UE_LOG(LogTemp, Log, TEXT("World generation started with size %dx%d, seed %d"), WorldSizeX, WorldSizeY, Seed);
}

void UHarmoniaWorldEditorUtility::GenerateTerrainOnly()
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		StatusMessage = TEXT("Error: World Generator Subsystem not found!");
		return;
	}

	if (!TargetLandscape)
	{
		TargetLandscape = FindExistingLandscape();
		if (!TargetLandscape && bAutoCreateLandscape)
		{
			TargetLandscape = CreateNewLandscape();
		}
	}

	if (!TargetLandscape)
	{
		StatusMessage = TEXT("Error: No landscape found!");
		return;
	}

	UpdateWorldConfigFromProperties();

	bIsGenerating = true;
	GenerationProgress = 0.0f;
	StatusMessage = TEXT("Generating terrain...");

	GenSubsystem->OnGenerationProgress.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationProgressCallback);
	GenSubsystem->OnGenerationComplete.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback);

	GenSubsystem->GenerateHeightmapOnly(WorldConfig);

	UE_LOG(LogTemp, Log, TEXT("Terrain generation started"));
}

void UHarmoniaWorldEditorUtility::GenerateBiomeMapOnly()
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		StatusMessage = TEXT("Error: World Generator Subsystem not found!");
		return;
	}

	UpdateWorldConfigFromProperties();

	StatusMessage = TEXT("Generating biome map...");

	GeneratedBiomeData = GenSubsystem->GenerateBiomeMap(WorldConfig);

	StatusMessage = FString::Printf(TEXT("Biome map generated with %d biomes"), GeneratedBiomeData.Num());
	UE_LOG(LogTemp, Log, TEXT("Biome map generated with %d entries"), GeneratedBiomeData.Num());
}

void UHarmoniaWorldEditorUtility::GenerateObjectsOnly()
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		StatusMessage = TEXT("Error: World Generator Subsystem not found!");
		return;
	}

	UpdateWorldConfigFromProperties();

	StatusMessage = TEXT("Generating objects...");

	// 오브젝트 생성은 비동기로 처리되므로 콜백 바인딩
	bIsGenerating = true;
	GenSubsystem->OnGenerationProgress.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationProgressCallback);
	GenSubsystem->OnGenerationComplete.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback);

	// 전체 월드에 오브젝트 생성
	FVector WorldCenter = FVector(WorldSizeX * 50.0f, WorldSizeY * 50.0f, 0.0f);
	float WorldRadius = FMath::Max(WorldSizeX, WorldSizeY) * 100.0f;
	GenSubsystem->GenerateObjectsInRegion(WorldCenter, WorldRadius, WorldConfig);

	UE_LOG(LogTemp, Log, TEXT("Object generation started"));
}

void UHarmoniaWorldEditorUtility::GenerateObjectsInRegion(FVector RegionCenter, float RegionRadius)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		StatusMessage = TEXT("Error: World Generator Subsystem not found!");
		return;
	}

	UpdateWorldConfigFromProperties();

	StatusMessage = FString::Printf(TEXT("Generating objects in region (radius: %.0f)..."), RegionRadius);

	bIsGenerating = true;
	GenSubsystem->OnGenerationProgress.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationProgressCallback);
	GenSubsystem->OnGenerationComplete.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback);

	GenSubsystem->GenerateObjectsInRegion(RegionCenter, RegionRadius, WorldConfig);

	UE_LOG(LogTemp, Log, TEXT("Regional object generation started at %s with radius %.0f"), *RegionCenter.ToString(), RegionRadius);
}

void UHarmoniaWorldEditorUtility::GenerateWorldAsync()
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		StatusMessage = TEXT("Error: World Generator Subsystem not found!");
		return;
	}

	if (!TargetLandscape)
	{
		TargetLandscape = FindExistingLandscape();
		if (!TargetLandscape && bAutoCreateLandscape)
		{
			TargetLandscape = CreateNewLandscape();
		}
	}

	if (!TargetLandscape)
	{
		StatusMessage = TEXT("Error: No landscape found!");
		return;
	}

	UpdateWorldConfigFromProperties();

	bIsGenerating = true;
	GenerationProgress = 0.0f;
	StatusMessage = TEXT("Starting async world generation...");

	GenSubsystem->OnGenerationProgress.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationProgressCallback);
	GenSubsystem->OnGenerationComplete.AddUniqueDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback);

	GenSubsystem->GenerateWorldAsync(WorldConfig);

	UE_LOG(LogTemp, Log, TEXT("Async world generation started"));
}

void UHarmoniaWorldEditorUtility::CancelGeneration()
{
	bIsGenerating = false;
	GenerationProgress = 0.0f;
	StatusMessage = TEXT("Generation cancelled");

	UE_LOG(LogTemp, Warning, TEXT("World generation cancelled by user"));
}

//=============================================================================
// Callbacks
//=============================================================================

void UHarmoniaWorldEditorUtility::OnGenerationProgressCallback(float Progress, const FString& Message)
{
	GenerationProgress = Progress;
	StatusMessage = Message;

	OnProgressUpdateEvent.Broadcast(Progress, Message);

	UE_LOG(LogTemp, Verbose, TEXT("Generation progress: %.1f%% - %s"), Progress * 100.0f, *Message);
}

void UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback(bool bSuccess)
{
	bIsGenerating = false;
	GenerationProgress = 1.0f;

	// 생성 시간 측정 완료
	if (GenerationStartTime > 0.0)
	{
		LastGenerationTimeSeconds = FPlatformTime::Seconds() - GenerationStartTime;
	}

	if (bSuccess)
	{
		StatusMessage = FString::Printf(TEXT("World generation completed successfully in %.1f seconds!"), LastGenerationTimeSeconds);
		UE_LOG(LogTemp, Log, TEXT("World generation completed successfully in %.1f seconds"), LastGenerationTimeSeconds);

		// 통계 새로고침
		RefreshStatistics();
	}
	else
	{
		StatusMessage = TEXT("World generation failed!");
		UE_LOG(LogTemp, Error, TEXT("World generation failed"));
	}

	OnGenerationCompleteEvent.Broadcast(bSuccess);

	// 콜백 언바인딩
	if (UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem())
	{
		GenSubsystem->OnGenerationProgress.RemoveDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationProgressCallback);
		GenSubsystem->OnGenerationComplete.RemoveDynamic(this, &UHarmoniaWorldEditorUtility::OnGenerationCompleteCallback);
	}
}

//=============================================================================
// Preview & Visualization
//=============================================================================

void UHarmoniaWorldEditorUtility::QuickPreview()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (!EditorSub)
	{
		StatusMessage = TEXT("Error: Editor Subsystem not found!");
		return;
	}

	UpdateWorldConfigFromProperties();

	StatusMessage = TEXT("Generating quick preview...");

	EditorSub->EditorQuickPreview(WorldConfig);

	StatusMessage = TEXT("Quick preview generated");
	UE_LOG(LogTemp, Log, TEXT("Quick preview generated"));
}

void UHarmoniaWorldEditorUtility::ShowBiomeVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		EditorSub->DrawBiomeDebugVisualization(GEditor->GetEditorWorldContext().World(), GeneratedBiomeData);
		StatusMessage = TEXT("Biome visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ShowRiverVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		TArray<FRoadSegmentData> Rivers; // 실제로는 생성된 강 데이터를 사용
		EditorSub->DrawRiverDebugVisualization(GEditor->GetEditorWorldContext().World(), Rivers);
		StatusMessage = TEXT("River visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ShowRoadVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		TArray<FRoadSegmentData> Roads; // 실제로는 생성된 도로 데이터를 사용
		EditorSub->DrawRoadDebugVisualization(GEditor->GetEditorWorldContext().World(), Roads);
		StatusMessage = TEXT("Road visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ShowStructureVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		EditorSub->DrawStructureDebugVisualization(GEditor->GetEditorWorldContext().World(), GeneratedObjectData);
		StatusMessage = TEXT("Structure visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ShowCaveVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		TArray<FVector> CaveEntrances; // 실제로는 생성된 동굴 데이터를 사용
		EditorSub->DrawCaveDebugVisualization(GEditor->GetEditorWorldContext().World(), CaveEntrances);
		StatusMessage = TEXT("Cave visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ShowResourceVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		TArray<FOreVeinData> OreVeins; // 실제로는 생성된 자원 데이터를 사용
		EditorSub->DrawResourceDebugVisualization(GEditor->GetEditorWorldContext().World(), OreVeins);
		StatusMessage = TEXT("Resource visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ShowPOIVisualization()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		EditorSub->DrawPOIDebugVisualization(GEditor->GetEditorWorldContext().World(), GeneratedObjectData);
		StatusMessage = TEXT("POI visualization shown");
	}
}

void UHarmoniaWorldEditorUtility::ClearAllVisualizations()
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (EditorSub && GEditor && GEditor->GetEditorWorldContext().World())
	{
		EditorSub->ClearDebugVisualizations(GEditor->GetEditorWorldContext().World());
		StatusMessage = TEXT("All visualizations cleared");
	}
}

//=============================================================================
// Terrain Modification
//=============================================================================

void UHarmoniaWorldEditorUtility::RaiseTerrain(FVector Location, float Radius, float Strength)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (GenSubsystem)
	{
		GenSubsystem->RaiseTerrain(Location, Radius, Strength);
		StatusMessage = FString::Printf(TEXT("Raised terrain at %s"), *Location.ToString());
	}
}

void UHarmoniaWorldEditorUtility::LowerTerrain(FVector Location, float Radius, float Strength)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (GenSubsystem)
	{
		GenSubsystem->LowerTerrain(Location, Radius, Strength);
		StatusMessage = FString::Printf(TEXT("Lowered terrain at %s"), *Location.ToString());
	}
}

void UHarmoniaWorldEditorUtility::FlattenTerrain(FVector Location, float Radius, float TargetHeight)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (GenSubsystem)
	{
		GenSubsystem->FlattenTerrain(Location, Radius, TargetHeight);
		StatusMessage = FString::Printf(TEXT("Flattened terrain at %s"), *Location.ToString());
	}
}

void UHarmoniaWorldEditorUtility::SmoothTerrain(FVector Location, float Radius, int32 Iterations)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (GenSubsystem)
	{
		GenSubsystem->SmoothTerrain(Location, Radius, Iterations);
		StatusMessage = FString::Printf(TEXT("Smoothed terrain at %s"), *Location.ToString());
	}
}

void UHarmoniaWorldEditorUtility::CreateCrater(FVector Location, float Radius, float Depth)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (GenSubsystem)
	{
		GenSubsystem->CreateCrater(Location, Radius, Depth);
		StatusMessage = FString::Printf(TEXT("Created crater at %s"), *Location.ToString());
	}
}

void UHarmoniaWorldEditorUtility::CreateHill(FVector Location, float Radius, float Height)
{
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (GenSubsystem)
	{
		GenSubsystem->CreateHill(Location, Radius, Height);
		StatusMessage = FString::Printf(TEXT("Created hill at %s"), *Location.ToString());
	}
}

//=============================================================================
// Export Functions
//=============================================================================

bool UHarmoniaWorldEditorUtility::ExportHeightmapToPNG(const FString& FilePath)
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (!EditorSub)
	{
		StatusMessage = TEXT("Error: Editor Subsystem not found!");
		return false;
	}

	TArray<int32> HeightData; // 실제로는 생성된 하이트맵 데이터를 사용

	if (EditorSub->ExportHeightmapToPNG(FilePath, HeightData, WorldSizeX, WorldSizeY))
	{
		StatusMessage = FString::Printf(TEXT("Heightmap exported to: %s"), *FilePath);
		return true;
	}

	StatusMessage = TEXT("Failed to export heightmap");
	return false;
}

bool UHarmoniaWorldEditorUtility::ExportBiomeMapToPNG(const FString& FilePath)
{
	UHarmoniaWorldGeneratorEditorSubsystem* EditorSub = GetEditorSubsystem();
	if (!EditorSub)
	{
		StatusMessage = TEXT("Error: Editor Subsystem not found!");
		return false;
	}

	if (EditorSub->ExportBiomeMapToPNG(FilePath, GeneratedBiomeData, WorldSizeX, WorldSizeY))
	{
		StatusMessage = FString::Printf(TEXT("Biome map exported to: %s"), *FilePath);
		return true;
	}

	StatusMessage = TEXT("Failed to export biome map");
	return false;
}

//=============================================================================
// Landscape Management
//=============================================================================

ALandscape* UHarmoniaWorldEditorUtility::CreateNewLandscape()
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot create landscape: No editor world"));
		return nullptr;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();

	// 새 랜드스케이프 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(TEXT("HarmoniaGeneratedLandscape"));

	ALandscape* NewLandscape = World->SpawnActor<ALandscape>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (NewLandscape)
	{
		StatusMessage = TEXT("New landscape created");
		UE_LOG(LogTemp, Log, TEXT("Created new landscape actor"));
	}
	else
	{
		StatusMessage = TEXT("Failed to create landscape");
		UE_LOG(LogTemp, Error, TEXT("Failed to create landscape actor"));
	}

	return NewLandscape;
}

ALandscape* UHarmoniaWorldEditorUtility::FindExistingLandscape()
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
	{
		return nullptr;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();

	// 월드에서 첫 번째 랜드스케이프 찾기
	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		ALandscape* Landscape = *It;
		if (Landscape && !Landscape->IsActorBeingDestroyed())
		{
			StatusMessage = FString::Printf(TEXT("Found existing landscape: %s"), *Landscape->GetName());
			UE_LOG(LogTemp, Log, TEXT("Found existing landscape: %s"), *Landscape->GetName());
			return Landscape;
		}
	}

	StatusMessage = TEXT("No existing landscape found");
	return nullptr;
}

//=============================================================================
// NEW: Real-time Preview & Thumbnails
//=============================================================================

UTexture2D* UHarmoniaWorldEditorUtility::GeneratePreviewThumbnail(int32 ThumbnailSize)
{
	UpdateWorldConfigFromProperties();

	// 작은 크기로 하이트맵 생성
	TArray<int32> HeightData = GenerateHeightDataForPreview(ThumbnailSize, ThumbnailSize);

	// 텍스처 생성
	CachedPreviewThumbnail = CreateTextureFromHeightData(HeightData, ThumbnailSize, ThumbnailSize);

	StatusMessage = FString::Printf(TEXT("Generated preview thumbnail (%dx%d)"), ThumbnailSize, ThumbnailSize);
	return CachedPreviewThumbnail;
}

UTexture2D* UHarmoniaWorldEditorUtility::GenerateMinimap(int32 Width, int32 Height)
{
	TArray<int32> HeightData = GenerateHeightDataForPreview(Width, Height);
	UTexture2D* Minimap = CreateTextureFromHeightData(HeightData, Width, Height);

	StatusMessage = FString::Printf(TEXT("Generated minimap (%dx%d)"), Width, Height);
	return Minimap;
}

//=============================================================================
// NEW: Undo/Redo System
//=============================================================================

bool UHarmoniaWorldEditorUtility::UndoLastEdit()
{
	if (!CanUndo())
	{
		StatusMessage = TEXT("Nothing to undo");
		return false;
	}

	const FTerrainEditHistory& Edit = EditHistory[CurrentHistoryIndex];
	ApplyEdit(Edit, true); // true = undo (apply before data)

	CurrentHistoryIndex--;

	StatusMessage = FString::Printf(TEXT("Undone edit: %s"), *Edit.Location.ToString());
	UE_LOG(LogTemp, Log, TEXT("Undone terrain edit at %s"), *Edit.Location.ToString());
	return true;
}

bool UHarmoniaWorldEditorUtility::RedoLastEdit()
{
	if (!CanRedo())
	{
		StatusMessage = TEXT("Nothing to redo");
		return false;
	}

	CurrentHistoryIndex++;
	const FTerrainEditHistory& Edit = EditHistory[CurrentHistoryIndex];
	ApplyEdit(Edit, false); // false = redo (apply after data)

	StatusMessage = FString::Printf(TEXT("Redone edit: %s"), *Edit.Location.ToString());
	UE_LOG(LogTemp, Log, TEXT("Redone terrain edit at %s"), *Edit.Location.ToString());
	return true;
}

bool UHarmoniaWorldEditorUtility::CanUndo() const
{
	return CurrentHistoryIndex >= 0;
}

bool UHarmoniaWorldEditorUtility::CanRedo() const
{
	return CurrentHistoryIndex < EditHistory.Num() - 1;
}

void UHarmoniaWorldEditorUtility::ClearEditHistory()
{
	EditHistory.Empty();
	CurrentHistoryIndex = -1;
	StatusMessage = TEXT("Edit history cleared");
	UE_LOG(LogTemp, Log, TEXT("Cleared terrain edit history"));
}

//=============================================================================
// NEW: Brush-based Terrain Editing
//=============================================================================

void UHarmoniaWorldEditorUtility::BeginBrushStroke(FVector StartLocation)
{
	bIsBrushStrokeActive = true;
	LastBrushLocation = StartLocation;

	// 첫 브러시 적용
	UpdateBrushStroke(StartLocation);

	StatusMessage = FString::Printf(TEXT("Brush stroke started at %s"), *StartLocation.ToString());
}

void UHarmoniaWorldEditorUtility::UpdateBrushStroke(FVector CurrentLocation)
{
	if (!bIsBrushStrokeActive)
	{
		return;
	}

	// 브러시 타입에 따라 지형 수정
	switch (CurrentBrushType)
	{
	case ETerrainModificationType::Raise:
		RaiseTerrain(CurrentLocation, BrushRadius, BrushStrength);
		break;
	case ETerrainModificationType::Lower:
		LowerTerrain(CurrentLocation, BrushRadius, BrushStrength);
		break;
	case ETerrainModificationType::Smooth:
		SmoothTerrain(CurrentLocation, BrushRadius, 1);
		break;
	case ETerrainModificationType::Flatten:
		FlattenTerrain(CurrentLocation, BrushRadius, CurrentLocation.Z);
		break;
	default:
		break;
	}

	LastBrushLocation = CurrentLocation;
}

void UHarmoniaWorldEditorUtility::EndBrushStroke()
{
	bIsBrushStrokeActive = false;
	StatusMessage = TEXT("Brush stroke ended");
}

//=============================================================================
// NEW: Validation & Warning System
//=============================================================================

FValidationResult UHarmoniaWorldEditorUtility::ValidateConfiguration()
{
	FValidationResult Result;

	// 메모리 검증
	float EstimatedMemory = GetEstimatedMemoryUsageMB();
	Result.EstimatedMemoryMB = EstimatedMemory;

	if (EstimatedMemory > 4096.0f) // 4GB
	{
		Result.Errors.Add(TEXT("Estimated memory usage exceeds 4GB! This may cause crashes."));
		Result.bIsSafe = false;
	}
	else if (EstimatedMemory > 2048.0f) // 2GB
	{
		Result.Warnings.Add(TEXT("High memory usage detected (>2GB). Ensure sufficient RAM available."));
	}

	// 월드 크기 검증
	int32 TotalSize = WorldSizeX * WorldSizeY;
	if (TotalSize > 4096 * 4096)
	{
		Result.Errors.Add(TEXT("World size is too large! Maximum recommended: 4096x4096"));
		Result.bIsSafe = false;
	}
	else if (TotalSize > 2048 * 2048)
	{
		Result.Warnings.Add(TEXT("Large world size detected. Generation may take significant time."));
	}

	// 생성 시간 예측
	float EstimatedTime = GetEstimatedGenerationTimeSeconds();
	Result.EstimatedTimeSeconds = EstimatedTime;

	if (EstimatedTime > 600.0f) // 10분
	{
		Result.Warnings.Add(FString::Printf(TEXT("Estimated generation time: %.1f minutes. Consider using async generation."), EstimatedTime / 60.0f));
	}

	// 침식 검증
	if (ErosionSettings.bEnableErosion && ErosionSettings.Iterations > 200)
	{
		Result.Warnings.Add(TEXT("High erosion iteration count may significantly increase generation time."));
	}

	// 검증 완료
	if (Result.Errors.Num() == 0)
	{
		StatusMessage = TEXT("Configuration is valid");
	}
	else
	{
		StatusMessage = FString::Printf(TEXT("Configuration has %d errors"), Result.Errors.Num());
	}

	return Result;
}

float UHarmoniaWorldEditorUtility::GetEstimatedMemoryUsageMB() const
{
	// 대략적인 메모리 계산
	int32 TotalTiles = WorldSizeX * WorldSizeY;

	// 하이트맵: 4 bytes per tile (int32)
	float HeightmapMB = (TotalTiles * sizeof(int32)) / (1024.0f * 1024.0f);

	// 바이옴 데이터: ~100 bytes per tile (구조체)
	float BiomeMB = (TotalTiles * 100) / (1024.0f * 1024.0f);

	// 오브젝트 데이터: 가정 - 타일당 0.1개, 오브젝트당 200 bytes
	float ObjectsMB = (TotalTiles * 0.1f * 200) / (1024.0f * 1024.0f);

	// 총합 + 오버헤드 20%
	float TotalMB = (HeightmapMB + BiomeMB + ObjectsMB) * 1.2f;

	return TotalMB;
}

float UHarmoniaWorldEditorUtility::GetEstimatedGenerationTimeSeconds() const
{
	// 매우 대략적인 예측 (실제로는 하드웨어에 따라 다름)
	int32 TotalTiles = WorldSizeX * WorldSizeY;

	// 기본 생성 시간: 타일당 0.0001초
	float BaseTime = TotalTiles * 0.0001f;

	// 노이즈 옥타브에 따른 배율
	float NoiseFactor = 1.0f + (NoiseSettings.Octaves / 6.0f);

	// 침식 시간
	float ErosionTime = 0.0f;
	if (ErosionSettings.bEnableErosion)
	{
		ErosionTime = ErosionSettings.Iterations * 0.1f;
	}

	// 오브젝트 배치 시간
	float ObjectTime = (TotalTiles * 0.1f) * 0.001f; // 가정: 타일당 0.1개 오브젝트

	return (BaseTime * NoiseFactor) + ErosionTime + ObjectTime;
}

bool UHarmoniaWorldEditorUtility::IsConfigurationSafe() const
{
	FValidationResult Result = const_cast<UHarmoniaWorldEditorUtility*>(this)->ValidateConfiguration();
	return Result.bIsSafe;
}

//=============================================================================
// NEW: World Statistics Dashboard
//=============================================================================

FWorldStatistics UHarmoniaWorldEditorUtility::GetWorldStatistics()
{
	RefreshStatistics();
	return CachedStatistics;
}

void UHarmoniaWorldEditorUtility::RefreshStatistics()
{
	CachedStatistics = FWorldStatistics();

	// 오브젝트 통계
	CachedStatistics.TotalObjects = GeneratedObjectData.Num();

	for (const FWorldObjectData& ObjData : GeneratedObjectData)
	{
		switch (ObjData.ObjectType)
		{
		case EWorldObjectType::Tree:
			CachedStatistics.TreeCount++;
			break;
		case EWorldObjectType::Rock:
			CachedStatistics.RockCount++;
			break;
		case EWorldObjectType::Structure:
			CachedStatistics.StructureCount++;
			break;
		default:
			break;
		}

		if (ObjData.bIsGroupCenter)
		{
			CachedStatistics.StructureCount++;
		}
	}

	// 바이옴 통계
	TMap<EBiomeType, int32> BiomeCounts;
	for (const FBiomeData& Biome : GeneratedBiomeData)
	{
		if (!BiomeCounts.Contains(Biome.BiomeType))
		{
			BiomeCounts.Add(Biome.BiomeType, 0);
		}
		BiomeCounts[Biome.BiomeType]++;
	}

	// 바이옴 비율 계산
	int32 TotalBiomes = GeneratedBiomeData.Num();
	if (TotalBiomes > 0)
	{
		for (const auto& Pair : BiomeCounts)
		{
			float Percentage = (Pair.Value / (float)TotalBiomes) * 100.0f;
			CachedStatistics.BiomePercentages.Add(Pair.Key, Percentage);
		}
	}

	// 평균 고도 계산
	float TotalElevation = 0.0f;
	for (const FBiomeData& Biome : GeneratedBiomeData)
	{
		TotalElevation += Biome.Height;
	}
	if (GeneratedBiomeData.Num() > 0)
	{
		CachedStatistics.AverageElevation = TotalElevation / GeneratedBiomeData.Num();
	}

	// 물 커버리지 계산
	if (CachedStatistics.BiomePercentages.Contains(EBiomeType::Ocean))
	{
		CachedStatistics.WaterCoverage = CachedStatistics.BiomePercentages[EBiomeType::Ocean];
	}

	StatusMessage = FString::Printf(TEXT("Statistics refreshed: %d objects, %.1f%% water"),
		CachedStatistics.TotalObjects, CachedStatistics.WaterCoverage);
}

//=============================================================================
// NEW: Template Gallery System
//=============================================================================

TArray<FWorldTemplate> UHarmoniaWorldEditorUtility::GetAvailableTemplates() const
{
	TArray<FWorldTemplate> Templates;

	// 템플릿 디렉토리 스캔
	FString TemplateDir = FPaths::ProjectSavedDir() / TEXT("WorldTemplates");

	TArray<FString> TemplateFiles;
	IFileManager::Get().FindFiles(TemplateFiles, *(TemplateDir / TEXT("*.json")), true, false);

	for (const FString& FileName : TemplateFiles)
	{
		FString FilePath = TemplateDir / FileName;
		FString JsonString;

		if (FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			// JSON 파싱하여 템플릿 로드
			// (간략화된 구현)
			FWorldTemplate Template;
			Template.Name = FPaths::GetBaseFilename(FileName);
			Templates.Add(Template);
		}
	}

	return Templates;
}

bool UHarmoniaWorldEditorUtility::ApplyTemplate(const FWorldTemplate& Template)
{
	WorldConfig = Template.Config;
	ApplyWorldConfigToProperties();

	StatusMessage = FString::Printf(TEXT("Applied template: %s"), *Template.Name);
	UE_LOG(LogTemp, Log, TEXT("Applied template: %s"), *Template.Name);
	return true;
}

bool UHarmoniaWorldEditorUtility::SaveAsTemplate(const FString& TemplateName, const FString& Description, UTexture2D* PreviewImage)
{
	UpdateWorldConfigFromProperties();

	FWorldTemplate Template;
	Template.Name = TemplateName;
	Template.Description = Description;
	Template.PreviewImage = PreviewImage;
	Template.Config = WorldConfig;
	Template.CreationDate = FDateTime::Now();
	Template.Author = FPlatformProcess::UserName();

	// JSON으로 저장
	FString FilePath = GenerateTemplateFilePath(TemplateName);

	// 디렉토리 생성
	FString Dir = FPaths::GetPath(FilePath);
	if (!IFileManager::Get().DirectoryExists(*Dir))
	{
		IFileManager::Get().MakeDirectory(*Dir, true);
	}

	// 저장 (간략화된 구현)
	bool bSuccess = SaveConfigToFile(FilePath);

	if (bSuccess)
	{
		StatusMessage = FString::Printf(TEXT("Template saved: %s"), *TemplateName);
		UE_LOG(LogTemp, Log, TEXT("Saved template: %s"), *TemplateName);
	}

	return bSuccess;
}

bool UHarmoniaWorldEditorUtility::LoadTemplate(const FString& TemplateName)
{
	FString FilePath = GenerateTemplateFilePath(TemplateName);

	if (!FPaths::FileExists(FilePath))
	{
		StatusMessage = FString::Printf(TEXT("Template not found: %s"), *TemplateName);
		return false;
	}

	bool bSuccess = LoadConfigFromFile(FilePath);

	if (bSuccess)
	{
		StatusMessage = FString::Printf(TEXT("Template loaded: %s"), *TemplateName);
		UE_LOG(LogTemp, Log, TEXT("Loaded template: %s"), *TemplateName);
	}

	return bSuccess;
}

bool UHarmoniaWorldEditorUtility::DeleteTemplate(const FString& TemplateName)
{
	FString FilePath = GenerateTemplateFilePath(TemplateName);

	if (!FPaths::FileExists(FilePath))
	{
		StatusMessage = FString::Printf(TEXT("Template not found: %s"), *TemplateName);
		return false;
	}

	bool bSuccess = IFileManager::Get().Delete(*FilePath);

	if (bSuccess)
	{
		StatusMessage = FString::Printf(TEXT("Template deleted: %s"), *TemplateName);
		UE_LOG(LogTemp, Log, TEXT("Deleted template: %s"), *TemplateName);
	}

	return bSuccess;
}

//=============================================================================
// NEW: Batch World Generation
//=============================================================================

void UHarmoniaWorldEditorUtility::BatchGenerateWorlds(int32 Count, const FString& OutputPath, bool bRandomizeSeeds)
{
	if (bIsBatchGenerating)
	{
		StatusMessage = TEXT("Batch generation already in progress");
		return;
	}

	// 작업 목록 생성
	BatchJobs.Empty();
	for (int32 i = 0; i < Count; i++)
	{
		FBatchGenerationJob Job;
		Job.Index = i;
		Job.Seed = bRandomizeSeeds ? FMath::Rand() : (Seed + i);
		Job.OutputPath = FString::Printf(TEXT("%s/World_%03d"), *OutputPath, i);
		BatchJobs.Add(Job);
	}

	CurrentBatchJobIndex = 0;
	bIsBatchGenerating = true;
	BatchProgress = 0.0f;

	StatusMessage = FString::Printf(TEXT("Starting batch generation of %d worlds"), Count);
	UE_LOG(LogTemp, Log, TEXT("Started batch generation: %d worlds"), Count);

	// 첫 작업 시작
	ProcessNextBatchJob();
}

void UHarmoniaWorldEditorUtility::CancelBatchGeneration()
{
	if (!bIsBatchGenerating)
	{
		return;
	}

	bIsBatchGenerating = false;
	BatchProgress = 0.0f;
	StatusMessage = TEXT("Batch generation cancelled");
	UE_LOG(LogTemp, Warning, TEXT("Batch generation cancelled"));
}

//=============================================================================
// NEW: Smart Suggestion System
//=============================================================================

FWorldGeneratorConfig UHarmoniaWorldEditorUtility::GetSuggestedConfig(EHarmoniaWorldType WorldType) const
{
	FWorldGeneratorConfig SuggestedConfig = WorldConfig;

	switch (WorldType)
	{
	case EHarmoniaWorldType::Realistic:
		SuggestedConfig.NoiseSettings.Octaves = 6;
		SuggestedConfig.NoiseSettings.Persistence = 0.5f;
		SuggestedConfig.SeaLevel = 0.4f;
		SuggestedConfig.ErosionSettings.bEnableErosion = true;
		break;

	case EHarmoniaWorldType::Fantasy:
		SuggestedConfig.NoiseSettings.Octaves = 8;
		SuggestedConfig.NoiseSettings.Amplitude = 2.0f;
		SuggestedConfig.MaxHeight = 51200.0f;
		break;

	case EHarmoniaWorldType::Alien:
		SuggestedConfig.NoiseSettings.Octaves = 10;
		SuggestedConfig.NoiseSettings.Frequency = 0.02f;
		SuggestedConfig.SeaLevel = 0.2f;
		break;

	case EHarmoniaWorldType::Archipelago:
		SuggestedConfig.SeaLevel = 0.65f;
		SuggestedConfig.NoiseSettings.Frequency = 0.005f;
		break;

	case EHarmoniaWorldType::Canyon:
		SuggestedConfig.ErosionSettings.bEnableErosion = true;
		SuggestedConfig.ErosionSettings.Iterations = 200;
		SuggestedConfig.SeaLevel = 0.3f;
		break;

	case EHarmoniaWorldType::Plains:
		SuggestedConfig.NoiseSettings.Amplitude = 0.3f;
		SuggestedConfig.NoiseSettings.Octaves = 3;
		break;

	case EHarmoniaWorldType::Volcanic:
		SuggestedConfig.MaxHeight = 51200.0f;
		SuggestedConfig.NoiseSettings.Amplitude = 2.5f;
		SuggestedConfig.SeaLevel = 0.5f;
		break;

	case EHarmoniaWorldType::Frozen:
		SuggestedConfig.SeaLevel = 0.5f;
		// 추가 환경 설정 필요
		break;

	case EHarmoniaWorldType::Tropical:
		SuggestedConfig.SeaLevel = 0.45f;
		SuggestedConfig.NoiseSettings.Frequency = 0.008f;
		break;

	case EHarmoniaWorldType::Highlands:
		SuggestedConfig.MaxHeight = 40000.0f;
		SuggestedConfig.NoiseSettings.Amplitude = 1.5f;
		SuggestedConfig.SeaLevel = 0.3f;
		break;

	default:
		break;
	}

	return SuggestedConfig;
}

TArray<FString> UHarmoniaWorldEditorUtility::GetOptimizationSuggestions() const
{
	TArray<FString> Suggestions;

	// 메모리 최적화
	if (GetEstimatedMemoryUsageMB() > 1024.0f)
	{
		Suggestions.Add(TEXT("Consider reducing world size to decrease memory usage"));
	}

	// 시간 최적화
	if (NoiseSettings.Octaves > 8)
	{
		Suggestions.Add(TEXT("Reduce noise octaves (currently > 8) to speed up generation"));
	}

	if (ErosionSettings.bEnableErosion && ErosionSettings.Iterations > 100)
	{
		Suggestions.Add(TEXT("Reduce erosion iterations to speed up generation"));
	}

	// 오브젝트 최적화
	int32 EstimatedObjects = (WorldSizeX * WorldSizeY) / 100; // 대략적
	if (EstimatedObjects > 50000)
	{
		Suggestions.Add(TEXT("High object count expected. Consider using LOD or culling"));
	}

	// 일반 제안
	if (Suggestions.Num() == 0)
	{
		Suggestions.Add(TEXT("Configuration looks optimized!"));
	}

	return Suggestions;
}

void UHarmoniaWorldEditorUtility::AutoOptimizeConfiguration()
{
	// 자동 최적화 로직
	bool bWasOptimized = false;

	// 메모리가 너무 크면 크기 축소
	if (GetEstimatedMemoryUsageMB() > 2048.0f)
	{
		WorldSizeX = FMath::Min(WorldSizeX, 1024);
		WorldSizeY = FMath::Min(WorldSizeY, 1024);
		bWasOptimized = true;
	}

	// 옥타브가 너무 많으면 감소
	if (NoiseSettings.Octaves > 8)
	{
		NoiseSettings.Octaves = 8;
		bWasOptimized = true;
	}

	// 침식 반복이 너무 많으면 감소
	if (ErosionSettings.Iterations > 100)
	{
		ErosionSettings.Iterations = 100;
		bWasOptimized = true;
	}

	if (bWasOptimized)
	{
		UpdateWorldConfigFromProperties();
		StatusMessage = TEXT("Configuration auto-optimized");
		UE_LOG(LogTemp, Log, TEXT("Configuration auto-optimized"));
	}
	else
	{
		StatusMessage = TEXT("Configuration already optimal");
	}
}

int32 UHarmoniaWorldEditorUtility::GenerateRandomSeed()
{
	Seed = FMath::Rand();
	StatusMessage = FString::Printf(TEXT("Generated random seed: %d"), Seed);
	return Seed;
}

//=============================================================================
// NEW: Version Control System
//=============================================================================

bool UHarmoniaWorldEditorUtility::SaveVersion(const FString& CommitMessage)
{
	UpdateWorldConfigFromProperties();

	FWorldVersion Version;
	Version.VersionIndex = VersionHistory.Num();
	Version.Timestamp = FDateTime::Now();
	Version.Config = WorldConfig;
	Version.CommitMessage = CommitMessage;
	Version.FilePath = GenerateVersionFilePath(Version.VersionIndex);

	// 프리뷰 스냅샷 생성
	Version.Snapshot = GeneratePreviewThumbnail(256);

	// 파일로 저장
	bool bSuccess = SaveConfigToFile(Version.FilePath);

	if (bSuccess)
	{
		VersionHistory.Add(Version);
		StatusMessage = FString::Printf(TEXT("Version %d saved: %s"), Version.VersionIndex, *CommitMessage);
		UE_LOG(LogTemp, Log, TEXT("Saved version %d: %s"), Version.VersionIndex, *CommitMessage);
	}

	return bSuccess;
}

bool UHarmoniaWorldEditorUtility::LoadVersion(int32 VersionIndex)
{
	if (VersionIndex < 0 || VersionIndex >= VersionHistory.Num())
	{
		StatusMessage = TEXT("Invalid version index");
		return false;
	}

	const FWorldVersion& Version = VersionHistory[VersionIndex];

	bool bSuccess = LoadConfigFromFile(Version.FilePath);

	if (bSuccess)
	{
		StatusMessage = FString::Printf(TEXT("Loaded version %d: %s"), VersionIndex, *Version.CommitMessage);
		UE_LOG(LogTemp, Log, TEXT("Loaded version %d"), VersionIndex);
	}

	return bSuccess;
}

FString UHarmoniaWorldEditorUtility::CompareVersions(int32 Version1Index, int32 Version2Index)
{
	if (Version1Index < 0 || Version1Index >= VersionHistory.Num() ||
		Version2Index < 0 || Version2Index >= VersionHistory.Num())
	{
		return TEXT("Invalid version indices");
	}

	const FWorldVersion& V1 = VersionHistory[Version1Index];
	const FWorldVersion& V2 = VersionHistory[Version2Index];

	FString Comparison;
	Comparison += FString::Printf(TEXT("Version %d vs Version %d\n\n"), Version1Index, Version2Index);

	// 크기 비교
	if (V1.Config.SizeX != V2.Config.SizeX || V1.Config.SizeY != V2.Config.SizeY)
	{
		Comparison += FString::Printf(TEXT("Size: %dx%d -> %dx%d\n"),
			V1.Config.SizeX, V1.Config.SizeY, V2.Config.SizeX, V2.Config.SizeY);
	}

	// 시드 비교
	if (V1.Config.Seed != V2.Config.Seed)
	{
		Comparison += FString::Printf(TEXT("Seed: %d -> %d\n"), V1.Config.Seed, V2.Config.Seed);
	}

	// 높이 비교
	if (V1.Config.MaxHeight != V2.Config.MaxHeight)
	{
		Comparison += FString::Printf(TEXT("Max Height: %.0f -> %.0f\n"), V1.Config.MaxHeight, V2.Config.MaxHeight);
	}

	if (Comparison.IsEmpty())
	{
		Comparison = TEXT("No significant differences found");
	}

	return Comparison;
}

TArray<FWorldVersion> UHarmoniaWorldEditorUtility::GetVersionHistory() const
{
	return VersionHistory;
}

void UHarmoniaWorldEditorUtility::ClearVersionHistory()
{
	VersionHistory.Empty();
	StatusMessage = TEXT("Version history cleared");
	UE_LOG(LogTemp, Log, TEXT("Cleared version history"));
}

bool UHarmoniaWorldEditorUtility::DeleteVersion(int32 VersionIndex)
{
	if (VersionIndex < 0 || VersionIndex >= VersionHistory.Num())
	{
		StatusMessage = TEXT("Invalid version index");
		return false;
	}

	const FWorldVersion& Version = VersionHistory[VersionIndex];

	// 파일 삭제
	bool bSuccess = IFileManager::Get().Delete(*Version.FilePath);

	if (bSuccess)
	{
		VersionHistory.RemoveAt(VersionIndex);
		StatusMessage = FString::Printf(TEXT("Deleted version %d"), VersionIndex);
		UE_LOG(LogTemp, Log, TEXT("Deleted version %d"), VersionIndex);
	}

	return bSuccess;
}

//=============================================================================
// NEW: Advanced Presets
//=============================================================================

TArray<FString> UHarmoniaWorldEditorUtility::GetAvailablePresets() const
{
	return {
		TEXT("Flat"),
		TEXT("Mountains"),
		TEXT("Islands"),
		TEXT("Desert"),
		TEXT("Archipelago"),
		TEXT("Canyon"),
		TEXT("Plains"),
		TEXT("Volcanic"),
		TEXT("Frozen"),
		TEXT("Tropical"),
		TEXT("Highlands")
	};
}

FString UHarmoniaWorldEditorUtility::GetPresetDescription(const FString& PresetName) const
{
	if (PresetName == TEXT("Flat"))
		return TEXT("Flat terrain with minimal elevation changes");
	else if (PresetName == TEXT("Mountains"))
		return TEXT("High mountain ranges with dramatic elevation");
	else if (PresetName == TEXT("Islands"))
		return TEXT("Island chains surrounded by ocean");
	else if (PresetName == TEXT("Desert"))
		return TEXT("Arid desert landscape with minimal water");
	else if (PresetName == TEXT("Archipelago"))
		return TEXT("Many small islands scattered across ocean");
	else if (PresetName == TEXT("Canyon"))
		return TEXT("Deep canyons carved by erosion");
	else if (PresetName == TEXT("Plains"))
		return TEXT("Gently rolling plains with low elevation");
	else if (PresetName == TEXT("Volcanic"))
		return TEXT("Volcanic landscape with steep peaks");
	else if (PresetName == TEXT("Frozen"))
		return TEXT("Frozen tundra with ice coverage");
	else if (PresetName == TEXT("Tropical"))
		return TEXT("Tropical islands with lush vegetation");
	else if (PresetName == TEXT("Highlands"))
		return TEXT("Elevated plateau regions");
	else
		return TEXT("Unknown preset");
}

//=============================================================================
// NEW: Internal Helper Functions
//=============================================================================

void UHarmoniaWorldEditorUtility::RecordEditForUndo(const FTerrainEditHistory& Edit)
{
	// Redo 스택 제거 (새 편집이 들어오면)
	if (CurrentHistoryIndex < EditHistory.Num() - 1)
	{
		EditHistory.RemoveAt(CurrentHistoryIndex + 1, EditHistory.Num() - CurrentHistoryIndex - 1);
	}

	// 새 편집 추가
	EditHistory.Add(Edit);
	CurrentHistoryIndex++;

	// 최대 히스토리 크기 체크
	if (EditHistory.Num() > MaxHistorySize)
	{
		EditHistory.RemoveAt(0);
		CurrentHistoryIndex--;
	}
}

void UHarmoniaWorldEditorUtility::ApplyEdit(const FTerrainEditHistory& Edit, bool bIsUndo)
{
	// 실제 지형 데이터에 적용
	// (간략화된 구현 - 실제로는 Landscape API 사용)
	UHarmoniaWorldGeneratorSubsystem* GenSubsystem = GetWorldGeneratorSubsystem();
	if (!GenSubsystem)
	{
		return;
	}

	const TArray<int32>& DataToApply = bIsUndo ? Edit.BeforeHeightData : Edit.AfterHeightData;

	// Landscape에 데이터 적용 로직
	// ...
}

TArray<int32> UHarmoniaWorldEditorUtility::GenerateHeightDataForPreview(int32 Width, int32 Height)
{
	TArray<int32> HeightData;
	HeightData.SetNum(Width * Height);

	// 간단한 Perlin 노이즈로 하이트맵 생성
	for (int32 Y = 0; Y < Height; Y++)
	{
		for (int32 X = 0; X < Width; X++)
		{
			float NoiseValue = FMath::PerlinNoise2D(FVector2D(
				X * NoiseSettings.Frequency,
				Y * NoiseSettings.Frequency
			));

			// -1~1을 0~65535로 변환 (16-bit heightmap)
			int32 HeightValue = FMath::Clamp(
				FMath::RoundToInt((NoiseValue + 1.0f) * 0.5f * 65535.0f),
				0, 65535
			);

			HeightData[Y * Width + X] = HeightValue;
		}
	}

	return HeightData;
}

UTexture2D* UHarmoniaWorldEditorUtility::CreateTextureFromHeightData(const TArray<int32>& HeightData, int32 Width, int32 Height)
{
	if (HeightData.Num() != Width * Height)
	{
		return nullptr;
	}

	// 텍스처 생성
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		return nullptr;
	}

	// 픽셀 데이터 작성
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	uint8* DestData = static_cast<uint8*>(Data);

	for (int32 i = 0; i < HeightData.Num(); i++)
	{
		uint8 GrayValue = HeightData[i] / 256; // 16-bit to 8-bit

		DestData[i * 4 + 0] = GrayValue; // B
		DestData[i * 4 + 1] = GrayValue; // G
		DestData[i * 4 + 2] = GrayValue; // R
		DestData[i * 4 + 3] = 255;       // A
	}

	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

void UHarmoniaWorldEditorUtility::ProcessNextBatchJob()
{
	if (!bIsBatchGenerating || CurrentBatchJobIndex >= BatchJobs.Num())
	{
		// 배치 완료
		bIsBatchGenerating = false;
		BatchProgress = 1.0f;
		StatusMessage = TEXT("Batch generation completed");
		UE_LOG(LogTemp, Log, TEXT("Batch generation completed"));
		return;
	}

	FBatchGenerationJob& Job = BatchJobs[CurrentBatchJobIndex];

	// 시드 설정
	Seed = Job.Seed;
	UpdateWorldConfigFromProperties();

	// 월드 생성 (간략화)
	GenerateWorld();

	// 내보내기
	ExportHeightmapToPNG(Job.OutputPath + TEXT("_heightmap.png"));

	Job.bCompleted = true;
	Job.bSuccess = true;

	// 진행률 업데이트
	BatchProgress = (float)(CurrentBatchJobIndex + 1) / BatchJobs.Num();

	CurrentBatchJobIndex++;

	// 다음 작업 처리
	ProcessNextBatchJob();
}

FString UHarmoniaWorldEditorUtility::GenerateTemplateFilePath(const FString& TemplateName) const
{
	return FPaths::ProjectSavedDir() / TEXT("WorldTemplates") / (TemplateName + TEXT(".json"));
}

FString UHarmoniaWorldEditorUtility::GenerateVersionFilePath(int32 VersionIndex) const
{
	return FPaths::ProjectSavedDir() / TEXT("WorldVersions") / FString::Printf(TEXT("Version_%03d.json"), VersionIndex);
}
