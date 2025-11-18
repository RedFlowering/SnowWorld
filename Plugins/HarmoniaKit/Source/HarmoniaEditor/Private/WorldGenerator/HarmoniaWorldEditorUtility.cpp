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

	if (bSuccess)
	{
		StatusMessage = TEXT("World generation completed successfully!");
		UE_LOG(LogTemp, Log, TEXT("World generation completed successfully"));
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
