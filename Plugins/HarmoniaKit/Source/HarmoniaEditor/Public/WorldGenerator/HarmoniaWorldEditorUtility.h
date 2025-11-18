// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "WorldGeneratorTypes.h"
#include "HarmoniaWorldEditorUtility.generated.h"

class UHarmoniaWorldGeneratorSubsystem;
class UHarmoniaWorldGeneratorEditorSubsystem;
class ALandscape;

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
	void OnGenerationProgressCallback(float Progress, const FString& Message);

	/** 생성 완료 콜백 / Generation complete callback */
	UFUNCTION()
	void OnGenerationCompleteCallback(bool bSuccess);

private:
	/** 서브시스템 캐시 / Cached subsystems */
	UPROPERTY(Transient)
	UHarmoniaWorldGeneratorSubsystem* CachedWorldGenSubsystem;

	UPROPERTY(Transient)
	UHarmoniaWorldGeneratorEditorSubsystem* CachedEditorSubsystem;

	/** 생성된 바이옴 데이터 / Generated biome data */
	TArray<FBiomeData> GeneratedBiomeData;

	/** 생성된 오브젝트 데이터 / Generated object data */
	TArray<FWorldObjectData> GeneratedObjectData;
};
