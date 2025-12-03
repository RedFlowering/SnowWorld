// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "HarmoniaModSystemDefinitions.generated.h"

/**
 * Mod load priority levels
 */
UENUM(BlueprintType)
enum class EHarmoniaModLoadPriority : uint8
{
	/** Core framework mods - loaded first */
	Critical = 0 UMETA(DisplayName = "Critical"),

	/** High priority - loaded before normal mods */
	High = 1 UMETA(DisplayName = "High"),

	/** Normal priority - default */
	Normal = 2 UMETA(DisplayName = "Normal"),

	/** Low priority - loaded after normal mods */
	Low = 3 UMETA(DisplayName = "Low"),

	/** Cosmetic/UI mods - loaded last */
	Cosmetic = 4 UMETA(DisplayName = "Cosmetic")
};

/**
 * Mod conflict severity levels
 */
UENUM(BlueprintType)
enum class EHarmoniaModConflictSeverity : uint8
{
	/** No conflict */
	None = 0 UMETA(DisplayName = "None"),

	/** Warning - may cause issues but can work */
	Warning = 1 UMETA(DisplayName = "Warning"),

	/** Error - likely to cause problems */
	Error = 2 UMETA(DisplayName = "Error"),

	/** Critical - incompatible, cannot load together */
	Critical = 3 UMETA(DisplayName = "Critical")
};

/**
 * Mod load state
 */
UENUM(BlueprintType)
enum class EHarmoniaModLoadState : uint8
{
	/** Not loaded */
	Unloaded = 0 UMETA(DisplayName = "Unloaded"),

	/** Loading in progress */
	Loading = 1 UMETA(DisplayName = "Loading"),

	/** Successfully loaded and active */
	Loaded = 2 UMETA(DisplayName = "Loaded"),

	/** Failed to load */
	Failed = 3 UMETA(DisplayName = "Failed"),

	/** Disabled by user */
	Disabled = 4 UMETA(DisplayName = "Disabled"),

	/** Conflict detected */
	Conflict = 5 UMETA(DisplayName = "Conflict")
};

/**
 * Asset override type
 */
UENUM(BlueprintType)
enum class EHarmoniaAssetOverrideType : uint8
{
	/** Static mesh override */
	StaticMesh = 0 UMETA(DisplayName = "Static Mesh"),

	/** Skeletal mesh override */
	SkeletalMesh = 1 UMETA(DisplayName = "Skeletal Mesh"),

	/** Texture override */
	Texture = 2 UMETA(DisplayName = "Texture"),

	/** Material override */
	Material = 3 UMETA(DisplayName = "Material"),

	/** Sound override */
	Sound = 4 UMETA(DisplayName = "Sound"),

	/** Animation override */
	Animation = 5 UMETA(DisplayName = "Animation"),

	/** Particle/Niagara effect override */
	Effect = 6 UMETA(DisplayName = "Effect"),

	/** Data table override */
	DataTable = 7 UMETA(DisplayName = "Data Table"),

	/** Blueprint class override */
	Blueprint = 8 UMETA(DisplayName = "Blueprint")
};

/**
 * Mod dependency information
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaModDependency
{
	GENERATED_BODY()

	/** Unique ID of the required mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Dependency")
	FName ModId;

	/** Minimum required version (semver format: Major.Minor.Patch) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Dependency")
	FString MinVersion;

	/** Maximum compatible version (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Dependency")
	FString MaxVersion;

	/** Is this dependency optional? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Dependency")
	bool bOptional = false;

	FHarmoniaModDependency()
		: ModId(NAME_None)
		, MinVersion(TEXT("0.0.0"))
		, MaxVersion(TEXT(""))
		, bOptional(false)
	{}
};

/**
 * Mod incompatibility information
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaModIncompatibility
{
	GENERATED_BODY()

	/** ID of the incompatible mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Incompatibility")
	FName ModId;

	/** Reason for incompatibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Incompatibility")
	FString Reason;

	/** Version range this incompatibility applies to (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Incompatibility")
	FString VersionRange;

	FHarmoniaModIncompatibility()
		: ModId(NAME_None)
		, Reason(TEXT(""))
		, VersionRange(TEXT("*"))
	{}
};

/**
 * Asset override definition
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaAssetOverride
{
	GENERATED_BODY()

	/** Original asset path to override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Override")
	FSoftObjectPath OriginalAssetPath;

	/** New asset path to use instead */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Override")
	FSoftObjectPath OverrideAssetPath;

	/** Type of asset being overridden */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Override")
	EHarmoniaAssetOverrideType OverrideType;

	/** Optional: Tag to identify this override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Override")
	FGameplayTag OverrideTag;

	FHarmoniaAssetOverride()
		: OverrideType(EHarmoniaAssetOverrideType::StaticMesh)
	{}
};

/**
 * Data table patch operation
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaDataTablePatch
{
	GENERATED_BODY()

	/** Target data table to patch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table Patch")
	FSoftObjectPath TargetDataTable;

	/** Row name to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table Patch")
	FName RowName;

	/** Column/property name to modify */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table Patch")
	FName PropertyName;

	/** New value (as JSON string) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table Patch")
	FString NewValue;

	/** Operation type: "Set", "Add", "Multiply", "Append" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Table Patch")
	FString Operation;

	FHarmoniaDataTablePatch()
		: RowName(NAME_None)
		, PropertyName(NAME_None)
		, Operation(TEXT("Set"))
	{}
};

/**
 * Custom ruleset definition (e.g., Nuzlocke mode)
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaCustomRuleset
{
	GENERATED_BODY()

	/** Unique ID for this ruleset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruleset")
	FName RulesetId;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruleset")
	FText DisplayName;

	/** Description of the rules */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruleset")
	FText Description;

	/** Tags that define the ruleset behaviors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruleset")
	FGameplayTagContainer RuleTags;

	/** Config values for the ruleset (JSON format) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruleset")
	TMap<FName, FString> ConfigValues;

	/** Is this a difficulty modifier? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruleset")
	bool bIsDifficultyModifier = false;

	FHarmoniaCustomRuleset()
		: RulesetId(NAME_None)
		, bIsDifficultyModifier(false)
	{}
};

/**
 * Mod conflict information
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaModConflict
{
	GENERATED_BODY()

	/** First mod involved in conflict */
	UPROPERTY(BlueprintReadOnly, Category = "Mod Conflict")
	FName ModA;

	/** Second mod involved in conflict */
	UPROPERTY(BlueprintReadOnly, Category = "Mod Conflict")
	FName ModB;

	/** Severity of the conflict */
	UPROPERTY(BlueprintReadOnly, Category = "Mod Conflict")
	EHarmoniaModConflictSeverity Severity;

	/** Description of the conflict */
	UPROPERTY(BlueprintReadOnly, Category = "Mod Conflict")
	FString Description;

	/** Conflicting resources (asset paths, data table rows, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Mod Conflict")
	TArray<FString> ConflictingResources;

	FHarmoniaModConflict()
		: ModA(NAME_None)
		, ModB(NAME_None)
		, Severity(EHarmoniaModConflictSeverity::None)
	{}
};

/**
 * Main mod information structure
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaModInfo
{
	GENERATED_BODY()

	/** Unique mod identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FName ModId;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FText DisplayName;

	/** Author name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FString Author;

	/** Mod version (semver format) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FString Version;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FText Description;

	/** Load priority */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	EHarmoniaModLoadPriority LoadPriority;

	/** Current load state */
	UPROPERTY(BlueprintReadOnly, Category = "Mod Info")
	EHarmoniaModLoadState LoadState;

	/** Dependencies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	TArray<FHarmoniaModDependency> Dependencies;

	/** Incompatibilities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	TArray<FHarmoniaModIncompatibility> Incompatibilities;

	/** Asset overrides */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	TArray<FHarmoniaAssetOverride> AssetOverrides;

	/** Data table patches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	TArray<FHarmoniaDataTablePatch> DataTablePatches;

	/** Custom rulesets provided by this mod */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	TArray<FHarmoniaCustomRuleset> CustomRulesets;

	/** Mod root directory path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FString ModRootPath;

	/** Blueprint classes to load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	TArray<FSoftClassPath> BlueprintClasses;

	/** Steam Workshop item ID (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	int64 WorkshopItemId;

	/** Should this mod run in sandbox mode? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	bool bSandboxMode;

	/** Is hot-reload enabled for this mod? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	bool bHotReloadEnabled;

	/** Tags for categorization/filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FGameplayTagContainer ModTags;

	/** Homepage/URL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Info")
	FString Homepage;

	FHarmoniaModInfo()
		: ModId(NAME_None)
		, Version(TEXT("1.0.0"))
		, LoadPriority(EHarmoniaModLoadPriority::Normal)
		, LoadState(EHarmoniaModLoadState::Unloaded)
		, WorkshopItemId(0)
		, bSandboxMode(false)
		, bHotReloadEnabled(true)
	{}

	/** Check if this mod is currently loaded */
	bool IsLoaded() const
	{
		return LoadState == EHarmoniaModLoadState::Loaded;
	}

	/** Check if this mod has failed to load */
	bool HasFailed() const
	{
		return LoadState == EHarmoniaModLoadState::Failed;
	}

	/** Compare versions (returns true if Version >= Other) */
	bool IsVersionCompatible(const FString& RequiredVersion) const;
};

/**
 * Data table row for mod configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAMODSYSTEM_API FHarmoniaModConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Mod information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mod Config")
	FHarmoniaModInfo ModInfo;
};
