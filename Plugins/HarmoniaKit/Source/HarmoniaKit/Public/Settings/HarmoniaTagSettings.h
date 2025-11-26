// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "HarmoniaTagSettings.generated.h"

/**
 * Tag definition for data-driven tag registration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaTagDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/** The gameplay tag string (e.g., "Ability.Attack.Melee.Combo1") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FString TagName;

	/** Description of what this tag is used for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FString Description;

	/** Category for organization (e.g., "Input", "Ability", "State") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FString Category;

	/** Whether this tag should be available in blueprints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	bool bBlueprintAccessible = true;

	/** Whether this tag is critical (must exist for systems to work) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	bool bCritical = false;

	FHarmoniaTagDefinition()
		: bBlueprintAccessible(true)
		, bCritical(false)
	{}
};

/**
 * Attribute definition for data-driven attribute mapping
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAttributeDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/** Attribute name (e.g., "Health", "MaxHealth", "AttackPower") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FString AttributeName;

	/** Associated gameplay tag for this attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FGameplayTag AttributeTag;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FText DisplayName;

	/** Description of the attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FText Description;

	/** Minimum value (for validation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float MinValue = 0.0f;

	/** Maximum value (0 = no max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float MaxValue = 0.0f;

	/** Default value for new characters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float DefaultValue = 0.0f;

	/** Is this a percentage-based attribute? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	bool bIsPercentage = false;

	/** Category for organization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FString Category;

	FHarmoniaAttributeDefinition()
		: MinValue(0.0f)
		, MaxValue(0.0f)
		, DefaultValue(0.0f)
		, bIsPercentage(false)
	{}
};

/**
 * Settings for HarmoniaKit tag and attribute management
 * Access via UHarmoniaTagSettings::Get()
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Harmonia Tag Settings"))
class HARMONIAKIT_API UHarmoniaTagSettings : public UObject
{
	GENERATED_BODY()

public:
	UHarmoniaTagSettings();

	/** Get the settings instance */
	static UHarmoniaTagSettings* Get();

	// ============================================================================
	// Tag Configuration
	// ============================================================================

	/** Path to the gameplay tags configuration INI file */
	UPROPERTY(config, EditAnywhere, Category = "Tags", meta = (DisplayName = "Custom Tags INI Path"))
	FString CustomTagsIniPath = TEXT("Plugins/HarmoniaKit/Config/HarmoniaGameplayTags.ini");

	/** DataTable containing additional tag definitions */
	UPROPERTY(config, EditAnywhere, Category = "Tags", meta = (DisplayName = "Tag Definitions DataTable"))
	TSoftObjectPtr<UDataTable> TagDefinitionsTable;

	/** Whether to automatically register tags from INI on startup */
	UPROPERTY(config, EditAnywhere, Category = "Tags", meta = (DisplayName = "Auto-Register INI Tags"))
	bool bAutoRegisterIniTags = true;

	/** Whether to automatically register tags from DataTable on startup */
	UPROPERTY(config, EditAnywhere, Category = "Tags", meta = (DisplayName = "Auto-Register DataTable Tags"))
	bool bAutoRegisterDataTableTags = true;

	// ============================================================================
	// Attribute Configuration
	// ============================================================================

	/** DataTable containing attribute definitions */
	UPROPERTY(config, EditAnywhere, Category = "Attributes", meta = (DisplayName = "Attribute Definitions DataTable"))
	TSoftObjectPtr<UDataTable> AttributeDefinitionsTable;

	/** Whether to validate attribute values at runtime */
	UPROPERTY(config, EditAnywhere, Category = "Attributes", meta = (DisplayName = "Validate Attribute Values"))
	bool bValidateAttributeValues = true;

	// ============================================================================
	// Debug
	// ============================================================================

	/** Log registered tags on startup */
	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bLogRegisteredTags = false;

	/** Log registered attributes on startup */
	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bLogRegisteredAttributes = false;
};
