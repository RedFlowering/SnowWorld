// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "WorldGeneratorTypes.h"
#include "HarmoniaWorldEditorUtility.generated.h"

class UHarmoniaWorldGeneratorSubsystem;
class UHarmoniaWorldGeneratorEditorSubsystem;
class ALandscape;

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGenerationProgress, float, Progress, const FString&, Message);

//=============================================================================
// Supporting Structures - Must be defined before the class that uses them
//=============================================================================

/** 월드 타입 (제안 시스템용) / World type for suggestion system */
UENUM(BlueprintType)
enum class EHarmoniaWorldType : uint8
{
	Realistic		UMETA(DisplayName = "Realistic"),
	Fantasy			UMETA(DisplayName = "Fantasy"),
	Alien			UMETA(DisplayName = "Alien"),
	Archipelago		UMETA(DisplayName = "Archipelago"),
	Canyon			UMETA(DisplayName = "Canyon"),
	Plains			UMETA(DisplayName = "Plains"),
	Volcanic		UMETA(DisplayName = "Volcanic"),
	Frozen			UMETA(DisplayName = "Frozen"),
	Tropical		UMETA(DisplayName = "Tropical"),
	Highlands		UMETA(DisplayName = "Highlands")
};

/** 지형 편집 히스토리 항목 / Terrain edit history entry */
USTRUCT(BlueprintType)
struct FTerrainEditHistory
{
	GENERATED_BODY()

	/** 편집 타입 / Edit type */
	UPROPERTY()
	ETerrainModificationType EditType;

	/** 편집 위치 / Edit location */
	UPROPERTY()
	FVector Location;

	/** 영향 받은 지형 데이터 (before) / Affected terrain data (before) */
	UPROPERTY()
	TArray<int32> BeforeHeightData;

	/** 영향 받은 지형 데이터 (after) / Affected terrain data (after) */
	UPROPERTY()
	TArray<int32> AfterHeightData;

	/** 반경 / Radius */
	UPROPERTY()
	float Radius = 0.0f;

	/** 강도 / Strength */
	UPROPERTY()
	float Strength = 0.0f;

	/** 타임스탬프 / Timestamp */
	UPROPERTY()
	FDateTime Timestamp;
};

/** 검증 결과 / Validation result */
USTRUCT(BlueprintType)
struct FValidationResult
{
	GENERATED_BODY()

	/** 경고 목록 / Warning messages */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Warnings;

	/** 오류 목록 / Error messages */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Errors;

	/** 예상 메모리 사용량 (MB) / Estimated memory usage in MB */
	UPROPERTY(BlueprintReadOnly)
	float EstimatedMemoryMB = 0.0f;

	/** 예상 생성 시간 (초) / Estimated generation time in seconds */
	UPROPERTY(BlueprintReadOnly)
	float EstimatedTimeSeconds = 0.0f;

	/** 안전 여부 / Is safe to generate */
	UPROPERTY(BlueprintReadOnly)
	bool bIsSafe = true;
};

/** 월드 통계 / World statistics */
USTRUCT(BlueprintType)
struct FWorldStatistics
{
	GENERATED_BODY()

	/** 총 오브젝트 수 / Total object count */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalObjects = 0;

	/** 바이옴별 비율 / Biome percentages */
	UPROPERTY(BlueprintReadOnly)
	TMap<EBiomeType, float> BiomePercentages;

	/** 나무 수 / Tree count */
	UPROPERTY(BlueprintReadOnly)
	int32 TreeCount = 0;

	/** 바위 수 / Rock count */
	UPROPERTY(BlueprintReadOnly)
	int32 RockCount = 0;

	/** 구조물 수 / Structure count */
	UPROPERTY(BlueprintReadOnly)
	int32 StructureCount = 0;

	/** 평균 고도 / Average elevation */
	UPROPERTY(BlueprintReadOnly)
	float AverageElevation = 0.0f;

	/** 물 커버리지 (%) / Water coverage percentage */
	UPROPERTY(BlueprintReadOnly)
	float WaterCoverage = 0.0f;

	/** 강 수 / River count */
	UPROPERTY(BlueprintReadOnly)
	int32 RiverCount = 0;

	/** 호수 수 / Lake count */
	UPROPERTY(BlueprintReadOnly)
	int32 LakeCount = 0;

	/** POI 수 / POI count */
	UPROPERTY(BlueprintReadOnly)
	int32 POICount = 0;

	/** 동굴 입구 수 / Cave entrance count */
	UPROPERTY(BlueprintReadOnly)
	int32 CaveEntranceCount = 0;
};

/** 월드 템플릿 / World template */
USTRUCT(BlueprintType)
struct FWorldTemplate
{
	GENERATED_BODY()

	/** 템플릿 이름 / Template name */
	UPROPERTY(BlueprintReadWrite)
	FString Name;

	/** 설명 / Description */
	UPROPERTY(BlueprintReadWrite)
	FString Description;

	/** 프리뷰 이미지 / Preview image */
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* PreviewImage = nullptr;

	/** 월드 설정 / World configuration */
	UPROPERTY(BlueprintReadWrite)
	FWorldGeneratorConfig Config;

	/** 태그 / Tags */
	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Tags;

	/** 생성 날짜 / Creation date */
	UPROPERTY(BlueprintReadWrite)
	FDateTime CreationDate;

	/** 작성자 / Author */
	UPROPERTY(BlueprintReadWrite)
	FString Author;
};

/** 배치 생성 작업 / Batch generation job */
USTRUCT()
struct FBatchGenerationJob
{
	GENERATED_BODY()

	/** 작업 인덱스 / Job index */
	int32 Index = 0;

	/** 시드 / Seed */
	int32 Seed = 0;

	/** 출력 경로 / Output path */
	FString OutputPath;

	/** 완료 여부 / Is completed */
	bool bCompleted = false;

	/** 성공 여부 / Is successful */
	bool bSuccess = false;
};

/** 월드 버전 / World version */
USTRUCT(BlueprintType)
struct FWorldVersion
{
	GENERATED_BODY()

	/** 버전 인덱스 / Version index */
	UPROPERTY(BlueprintReadOnly)
	int32 VersionIndex = 0;

	/** 타임스탬프 / Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** 월드 설정 / World configuration */
	UPROPERTY(BlueprintReadOnly)
	FWorldGeneratorConfig Config;

	/** 커밋 메시지 / Commit message */
	UPROPERTY(BlueprintReadOnly)
	FString CommitMessage;

	/** 스냅샷 이미지 / Snapshot image */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Snapshot = nullptr;

	/** 파일 경로 / File path */
	UPROPERTY(BlueprintReadOnly)
	FString FilePath;
};

/**
 * 하모니아 월드 생성을 위한 에디터 유틸리티 위젯
 * Editor Utility Widget for Harmonia World Generation
 *
 * 이 클래스는 언리얼 에디터에서 월드를 시각적으로 생성하고 편집할 수 있는 기능을 제공합니다.
 * This class provides functionality to visually create and edit worlds in Unreal Editor.
 */
UCLASS(Blueprintable, BlueprintType)
class HARMONIAEDITOR_API UHarmoniaWorldEditorUtility : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UHarmoniaWorldEditorUtility();

	//=============================================================================
	// World Generation Configuration
	//=============================================================================

	/** 현재 월드 생성 설정 / Current world generation configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
	FWorldGeneratorConfig WorldConfig;

	/** 생성할 월드의 크기 (타일 수) / World size in tiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Basic", meta = (ClampMin = "64", ClampMax = "8192"))
	int32 WorldSizeX = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Basic", meta = (ClampMin = "64", ClampMax = "8192"))
	int32 WorldSizeY = 512;

	/** 최대 높이 (Unreal Units) / Maximum height in Unreal Units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Basic", meta = (ClampMin = "100.0", ClampMax = "100000.0"))
	float MaxHeight = 25600.0f;

	/** 해수면 높이 / Sea level height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Basic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SeaLevel = 0.4f;

	/** 랜덤 시드 (같은 시드 = 같은 월드) / Random seed (same seed = same world) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Basic")
	int32 Seed = 12345;

	/** 펄린 노이즈 설정 / Perlin noise settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Terrain")
	FPerlinNoiseSettings NoiseSettings;

	/** 침식 설정 / Erosion settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Terrain")
	FErosionSettings ErosionSettings;

	/** 바이옴 설정 목록 / Biome settings list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Biomes")
	TArray<FBiomeSettings> BiomeSettingsList;

	/** 강 생성 설정 / River generation settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Water")
	FRiverSettings RiverSettings;

	/** 호수 생성 설정 / Lake generation settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Water")
	FLakeSettings LakeSettings;

	/** 동굴 생성 설정 / Cave generation settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Features")
	FCaveSettings CaveSettings;

	/** POI 생성 설정 / POI generation settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Features")
	FPOISettings POISettings;

	/** 환경 시스템 설정 / Environment system settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Environment")
	FEnvironmentSystemSettings EnvironmentSettings;

	//=============================================================================
	// Generation Control
	//=============================================================================

	/** 전체 월드 생성 (지형 + 오브젝트) / Generate complete world (terrain + objects) */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void GenerateWorld();

	/** 지형만 생성 (오브젝트 제외) / Generate terrain only (no objects) */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void GenerateTerrainOnly();

	/** 바이옴 맵만 생성 / Generate biome map only */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void GenerateBiomeMapOnly();

	/** 오브젝트만 생성 (지형은 기존 유지) / Generate objects only (keep existing terrain) */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void GenerateObjectsOnly();

	/** 특정 영역의 오브젝트만 생성 / Generate objects in specific region */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void GenerateObjectsInRegion(FVector RegionCenter, float RegionRadius);

	/** 비동기 월드 생성 시작 / Start async world generation */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void GenerateWorldAsync();

	/** 월드 생성 취소 / Cancel world generation */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Actions")
	void CancelGeneration();

	//=============================================================================
	// Preview & Visualization
	//=============================================================================

	/** 빠른 프리뷰 생성 (작은 크기) / Quick preview generation (small size) */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void QuickPreview();

	/** 바이옴 디버그 시각화 표시 / Show biome debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowBiomeVisualization();

	/** 강 디버그 시각화 표시 / Show river debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowRiverVisualization();

	/** 도로 디버그 시각화 표시 / Show road debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowRoadVisualization();

	/** 구조물 디버그 시각화 표시 / Show structure debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowStructureVisualization();

	/** 동굴 디버그 시각화 표시 / Show cave debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowCaveVisualization();

	/** 리소스 디버그 시각화 표시 / Show resource debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowResourceVisualization();

	/** POI 디버그 시각화 표시 / Show POI debug visualization */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ShowPOIVisualization();

	/** 모든 디버그 시각화 제거 / Clear all debug visualizations */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	void ClearAllVisualizations();

	//=============================================================================
	// Terrain Modification
	//=============================================================================

	/** 지형 높이기 / Raise terrain */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain Edit")
	void RaiseTerrain(FVector Location, float Radius, float Strength);

	/** 지형 낮추기 / Lower terrain */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain Edit")
	void LowerTerrain(FVector Location, float Radius, float Strength);

	/** 지형 평탄화 / Flatten terrain */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain Edit")
	void FlattenTerrain(FVector Location, float Radius, float TargetHeight);

	/** 지형 부드럽게 / Smooth terrain */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain Edit")
	void SmoothTerrain(FVector Location, float Radius, int32 Iterations);

	/** 분화구 생성 / Create crater */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain Edit")
	void CreateCrater(FVector Location, float Radius, float Depth);

	/** 언덕 생성 / Create hill */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Terrain Edit")
	void CreateHill(FVector Location, float Radius, float Height);

	//=============================================================================
	// Configuration Management
	//=============================================================================

	/** 설정을 기본값으로 리셋 / Reset configuration to defaults */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Config")
	void ResetToDefaults();

	/** 설정을 파일로 저장 / Save configuration to file */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Config")
	bool SaveConfigToFile(const FString& FilePath);

	/** 파일에서 설정 로드 / Load configuration from file */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Config")
	bool LoadConfigFromFile(const FString& FilePath);

	/** 프리셋 적용 / Apply preset */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Config")
	void ApplyPreset(const FString& PresetName);

	//=============================================================================
	// Export Functions
	//=============================================================================

	/** 하이트맵을 PNG로 내보내기 / Export heightmap to PNG */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Export")
	bool ExportHeightmapToPNG(const FString& FilePath);

	/** 바이옴 맵을 PNG로 내보내기 / Export biome map to PNG */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Export")
	bool ExportBiomeMapToPNG(const FString& FilePath);

	//=============================================================================
	// Status & Progress
	//=============================================================================

	/** 현재 생성 진행률 (0.0 ~ 1.0) / Current generation progress (0.0 ~ 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Status")
	float GenerationProgress = 0.0f;

	/** 현재 상태 메시지 / Current status message */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Status")
	FString StatusMessage;

	/** 생성 중인지 여부 / Whether generation is in progress */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Status")
	bool bIsGenerating = false;

	/** 생성 완료 이벤트 / Generation complete event */
	UPROPERTY(BlueprintAssignable, Category = "World Generation|Events")
	FOnGenerationComplete OnGenerationCompleteEvent;

	/** 진행 상황 업데이트 이벤트 / Progress update event */
	UPROPERTY(BlueprintAssignable, Category = "World Generation|Events")
	FOnGenerationProgress OnProgressUpdateEvent;

	//=============================================================================
	// Landscape Management
	//=============================================================================

	/** 타겟 랜드스케이프 액터 / Target landscape actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Landscape")
	ALandscape* TargetLandscape;

	/** 랜드스케이프 자동 생성 여부 / Whether to auto-create landscape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Landscape")
	bool bAutoCreateLandscape = true;

	/** 새 랜드스케이프 생성 / Create new landscape */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Landscape")
	ALandscape* CreateNewLandscape();

	/** 기존 랜드스케이프 찾기 / Find existing landscape */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Landscape")
	ALandscape* FindExistingLandscape();

	//=============================================================================
	// NEW: Real-time Preview & Thumbnails
	//=============================================================================

	/** 실시간 프리뷰 썸네일 생성 / Generate real-time preview thumbnail */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	UTexture2D* GeneratePreviewThumbnail(int32 ThumbnailSize = 256);

	/** 현재 설정으로 미니맵 생성 / Generate minimap with current settings */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Preview")
	UTexture2D* GenerateMinimap(int32 Width = 512, int32 Height = 512);

	/** 캐시된 프리뷰 썸네일 / Cached preview thumbnail */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Preview")
	UTexture2D* CachedPreviewThumbnail;

	/** 자동 프리뷰 업데이트 활성화 / Enable auto preview update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Preview")
	bool bAutoUpdatePreview = false;

	//=============================================================================
	// NEW: Undo/Redo System
	//=============================================================================

	/** 마지막 편집 되돌리기 / Undo last edit */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Undo")
	bool UndoLastEdit();

	/** 되돌린 편집 다시 실행 / Redo last undone edit */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Undo")
	bool RedoLastEdit();

	/** Undo 가능 여부 / Can undo */
	UFUNCTION(BlueprintPure, Category = "World Generation|Undo")
	bool CanUndo() const;

	/** Redo 가능 여부 / Can redo */
	UFUNCTION(BlueprintPure, Category = "World Generation|Undo")
	bool CanRedo() const;

	/** 편집 히스토리 초기화 / Clear edit history */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Undo")
	void ClearEditHistory();

	/** 최대 히스토리 크기 / Maximum history size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Undo", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxHistorySize = 50;

	//=============================================================================
	// NEW: Brush-based Terrain Editing
	//=============================================================================

	/** 브러시 스트로크 시작 / Begin brush stroke */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Brush")
	void BeginBrushStroke(FVector StartLocation);

	/** 브러시 스트로크 업데이트 / Update brush stroke */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Brush")
	void UpdateBrushStroke(FVector CurrentLocation);

	/** 브러시 스트로크 종료 / End brush stroke */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Brush")
	void EndBrushStroke();

	/** 현재 브러시 타입 / Current brush type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Brush")
	ETerrainModificationType CurrentBrushType = ETerrainModificationType::Raise;

	/** 브러시 강도 / Brush strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Brush", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
	float BrushStrength = 100.0f;

	/** 브러시 반경 / Brush radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Brush", meta = (ClampMin = "100.0", ClampMax = "50000.0"))
	float BrushRadius = 2000.0f;

	/** 브러시 모양 텍스처 / Brush shape texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Brush")
	UTexture2D* BrushShapeTexture;

	/** 브러시 감쇠 타입 / Brush falloff type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Brush")
	ETerrainFalloffType BrushFalloffType = ETerrainFalloffType::Smooth;

	//=============================================================================
	// NEW: Validation & Warning System
	//=============================================================================

	/** 현재 설정 검증 / Validate current configuration */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Validation")
	FValidationResult ValidateConfiguration();

	/** 예상 메모리 사용량 계산 (MB) / Calculate estimated memory usage in MB */
	UFUNCTION(BlueprintPure, Category = "World Generation|Validation")
	float GetEstimatedMemoryUsageMB() const;

	/** 예상 생성 시간 계산 (초) / Calculate estimated generation time in seconds */
	UFUNCTION(BlueprintPure, Category = "World Generation|Validation")
	float GetEstimatedGenerationTimeSeconds() const;

	/** 설정이 안전한지 확인 / Check if configuration is safe */
	UFUNCTION(BlueprintPure, Category = "World Generation|Validation")
	bool IsConfigurationSafe() const;

	/** 자동 검증 활성화 / Enable auto validation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation|Validation")
	bool bAutoValidate = true;

	//=============================================================================
	// NEW: World Statistics Dashboard
	//=============================================================================

	/** 월드 통계 가져오기 / Get world statistics */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Statistics")
	FWorldStatistics GetWorldStatistics();

	/** 통계 새로고침 / Refresh statistics */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Statistics")
	void RefreshStatistics();

	/** 캐시된 통계 데이터 / Cached statistics data */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Statistics")
	FWorldStatistics CachedStatistics;

	/** 마지막 생성 시간 (초) / Last generation time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Statistics")
	float LastGenerationTimeSeconds = 0.0f;

	//=============================================================================
	// NEW: Template Gallery System
	//=============================================================================

	/** 사용 가능한 템플릿 목록 가져오기 / Get available templates */
	UFUNCTION(BlueprintPure, Category = "World Generation|Templates")
	TArray<FWorldTemplate> GetAvailableTemplates() const;

	/** 템플릿 적용 / Apply template */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Templates")
	bool ApplyTemplate(const FWorldTemplate& Template);

	/** 현재 설정을 템플릿으로 저장 / Save current config as template */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Templates")
	bool SaveAsTemplate(const FString& TemplateName, const FString& Description, UTexture2D* PreviewImage = nullptr);

	/** 템플릿 로드 / Load template by name */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Templates")
	bool LoadTemplate(const FString& TemplateName);

	/** 템플릿 삭제 / Delete template */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Templates")
	bool DeleteTemplate(const FString& TemplateName);

	//=============================================================================
	// NEW: Batch World Generation
	//=============================================================================

	/** 배치로 여러 월드 생성 / Generate multiple worlds in batch */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Batch")
	void BatchGenerateWorlds(int32 Count, const FString& OutputPath, bool bRandomizeSeeds);

	/** 배치 생성 취소 / Cancel batch generation */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Batch")
	void CancelBatchGeneration();

	/** 배치 생성 진행률 / Batch generation progress */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Batch")
	float BatchProgress = 0.0f;

	/** 배치 생성 중인지 여부 / Whether batch generation is in progress */
	UPROPERTY(BlueprintReadOnly, Category = "World Generation|Batch")
	bool bIsBatchGenerating = false;

	//=============================================================================
	// NEW: Smart Suggestion System
	//=============================================================================

	/** 원하는 월드 타입에 대한 설정 제안 / Get suggested config for desired world type */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Suggestions")
	FWorldGeneratorConfig GetSuggestedConfig(EHarmoniaWorldType WorldType) const;

	/** 현재 설정에 대한 최적화 제안 / Get optimization suggestions for current config */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Suggestions")
	TArray<FString> GetOptimizationSuggestions() const;

	/** 설정 자동 최적화 / Auto-optimize configuration */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Suggestions")
	void AutoOptimizeConfiguration();

	/** 랜덤 시드 생성 / Generate random seed */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Suggestions")
	int32 GenerateRandomSeed();

	//=============================================================================
	// NEW: Version Control System
	//=============================================================================

	/** 현재 설정을 버전으로 저장 / Save current config as version */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Versions")
	bool SaveVersion(const FString& CommitMessage);

	/** 특정 버전 로드 / Load specific version */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Versions")
	bool LoadVersion(int32 VersionIndex);

	/** 두 버전 비교 / Compare two versions */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Versions")
	FString CompareVersions(int32 Version1Index, int32 Version2Index);

	/** 버전 히스토리 가져오기 / Get version history */
	UFUNCTION(BlueprintPure, Category = "World Generation|Versions")
	TArray<FWorldVersion> GetVersionHistory() const;

	/** 버전 히스토리 초기화 / Clear version history */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Versions")
	void ClearVersionHistory();

	/** 특정 버전 삭제 / Delete specific version */
	UFUNCTION(BlueprintCallable, Category = "World Generation|Versions")
	bool DeleteVersion(int32 VersionIndex);

	//=============================================================================
	// NEW: Advanced Presets
	//=============================================================================

	/** 향상된 프리셋 목록 / Enhanced preset list */
	UFUNCTION(BlueprintPure, Category = "World Generation|Config")
	TArray<FString> GetAvailablePresets() const;

	/** 프리셋 설명 가져오기 / Get preset description */
	UFUNCTION(BlueprintPure, Category = "World Generation|Config")
	FString GetPresetDescription(const FString& PresetName) const;

protected:
	//=============================================================================
	// Internal Functions
	//=============================================================================

	/** 서브시스템 레퍼런스 가져오기 / Get subsystem references */
	UHarmoniaWorldGeneratorSubsystem* GetWorldGeneratorSubsystem() const;
	UHarmoniaWorldGeneratorEditorSubsystem* GetEditorSubsystem() const;

	/** WorldConfig 업데이트 / Update WorldConfig from properties */
	void UpdateWorldConfigFromProperties();

	/** 프로퍼티에서 WorldConfig 적용 / Apply WorldConfig to properties */
	void ApplyWorldConfigToProperties();

	/** 생성 진행 콜백 / Generation progress callback */
	UFUNCTION()
	void OnGenerationProgressCallback(float Progress);

	/** 생성 완료 콜백 / Generation complete callback */
	UFUNCTION()
	void OnGenerationCompleteCallback(const TArray<int32>& HeightData, const TArray<FWorldObjectData>& Objects, bool bSuccess);

private:
	/** 서브시스템 캐시 / Cached subsystems */
	UPROPERTY(Transient)
	mutable UHarmoniaWorldGeneratorSubsystem* CachedWorldGenSubsystem;

	UPROPERTY(Transient)
	mutable UHarmoniaWorldGeneratorEditorSubsystem* CachedEditorSubsystem;

	/** 생성된 바이옴 데이터 / Generated biome data */
	TArray<FBiomeData> GeneratedBiomeData;

	/** 생성된 오브젝트 데이터 / Generated object data */
	TArray<FWorldObjectData> GeneratedObjectData;

	/** 생성된 광석 자원 데이터 / Generated ore vein data */
	TArray<FOreVeinData> GeneratedOreVeinData;

	//=============================================================================
	// NEW: Internal Data Structures
	//=============================================================================

	/** Undo/Redo 편집 히스토리 / Edit history for undo/redo */
	TArray<FTerrainEditHistory> EditHistory;
	int32 CurrentHistoryIndex = -1;

	/** 브러시 스트로크 활성 여부 / Whether brush stroke is active */
	bool bIsBrushStrokeActive = false;
	FVector LastBrushLocation = FVector::ZeroVector;

	/** 배치 생성 작업 목록 / Batch generation job list */
	TArray<FBatchGenerationJob> BatchJobs;
	int32 CurrentBatchJobIndex = 0;

	/** 버전 히스토리 / Version history */
	TArray<FWorldVersion> VersionHistory;

	/** 생성 시작 시간 / Generation start time */
	double GenerationStartTime = 0.0;

	/** 내부 헬퍼 함수들 / Internal helper functions */
	void RecordEditForUndo(const FTerrainEditHistory& Edit);
	void ApplyEdit(const FTerrainEditHistory& Edit, bool bIsUndo);
	TArray<int32> GenerateHeightDataForPreview(int32 Width, int32 Height);
	UTexture2D* CreateTextureFromHeightData(const TArray<int32>& HeightData, int32 Width, int32 Height);
	void ProcessNextBatchJob();
	FString GenerateTemplateFilePath(const FString& TemplateName) const;
	FString GenerateVersionFilePath(int32 VersionIndex) const;
};
