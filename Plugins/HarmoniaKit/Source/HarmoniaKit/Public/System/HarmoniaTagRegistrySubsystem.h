// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "HarmoniaTagRegistrySubsystem.generated.h"

class UDataTable;
struct FHarmoniaTagDefinition;
struct FHarmoniaAttributeDefinition;

/**
 * Subsystem for managing data-driven gameplay tags and attributes
 * 
 * This subsystem:
 * - Loads tags from INI configuration files
 * - Loads tags from DataTables
 * - Provides tag lookup by name or category
 * - Manages attribute-to-tag mappings
 * - Validates tag and attribute configurations
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaTagRegistrySubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Subsystem Lifecycle
	// ============================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Get the subsystem instance */
	static UHarmoniaTagRegistrySubsystem* Get();

	// ============================================================================
	// Tag Registration
	// ============================================================================

	/**
	 * Register tags from the configured INI file
	 * @return Number of tags registered
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Tags")
	int32 RegisterTagsFromIni();

	/**
	 * Register tags from a specific INI file
	 * @param IniPath Path to the INI file relative to project root
	 * @return Number of tags registered
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Tags")
	int32 RegisterTagsFromIniPath(const FString& IniPath);

	/**
	 * Register tags from a DataTable
	 * @param DataTable The DataTable containing FHarmoniaTagDefinition rows
	 * @return Number of tags registered
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Tags")
	int32 RegisterTagsFromDataTable(UDataTable* DataTable);

	/**
	 * Register a single tag
	 * @param TagName Full tag name (e.g., "Ability.Attack.Melee")
	 * @param Comment Description of the tag
	 * @return Whether registration was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Tags")
	bool RegisterTag(const FString& TagName, const FString& Comment = TEXT(""));

	// ============================================================================
	// Attribute Registration
	// ============================================================================

	/**
	 * Register attributes from a DataTable
	 * @param DataTable The DataTable containing FHarmoniaAttributeDefinition rows
	 * @return Number of attributes registered
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Attributes")
	int32 RegisterAttributesFromDataTable(UDataTable* DataTable);

	/**
	 * Register an attribute-to-tag mapping
	 * @param AttributeName Name of the attribute
	 * @param Tag Associated gameplay tag
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Attributes")
	void RegisterAttributeMapping(const FString& AttributeName, FGameplayTag Tag);

	// ============================================================================
	// Tag Lookup
	// ============================================================================

	/**
	 * Find a tag by its string name
	 * @param TagString The tag name string
	 * @param bMatchPartial Whether to allow partial matching
	 * @return The found tag, or empty tag if not found
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Tags")
	FGameplayTag FindTagByName(const FString& TagString, bool bMatchPartial = false) const;

	/**
	 * Get all registered tags in a category
	 * @param Category The category prefix (e.g., "Ability", "State")
	 * @return Container with matching tags
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Tags")
	FGameplayTagContainer GetTagsByCategory(const FString& Category) const;

	/**
	 * Get all registered tags
	 * @return All registered tags
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Tags")
	FGameplayTagContainer GetAllRegisteredTags() const;

	/**
	 * Check if a tag is registered
	 * @param TagString The tag name string
	 * @return Whether the tag exists
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Tags")
	bool IsTagRegistered(const FString& TagString) const;

	// ============================================================================
	// Attribute Lookup
	// ============================================================================

	/**
	 * Get tag associated with an attribute name
	 * @param AttributeName Name of the attribute
	 * @return Associated gameplay tag
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Attributes")
	FGameplayTag GetTagForAttribute(const FString& AttributeName) const;

	/**
	 * Get attribute name from a tag
	 * @param Tag The gameplay tag
	 * @return Attribute name, or empty string if not found
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Attributes")
	FString GetAttributeForTag(const FGameplayTag& Tag) const;

	/**
	 * Get all registered attribute names
	 * @return Array of attribute names
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Attributes")
	TArray<FString> GetAllAttributeNames() const;

	// ============================================================================
	// Validation
	// ============================================================================

	/**
	 * Validate all registered tags exist in the GameplayTags system
	 * @return Array of error messages for invalid tags
	 */
	UFUNCTION(BlueprintCallable, Category = "HarmoniaKit|Validation")
	TArray<FString> ValidateRegisteredTags() const;

	/**
	 * Get count of registered tags
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Tags")
	int32 GetRegisteredTagCount() const { return RegisteredTags.Num(); }

	/**
	 * Get count of registered attributes
	 */
	UFUNCTION(BlueprintPure, Category = "HarmoniaKit|Attributes")
	int32 GetRegisteredAttributeCount() const { return AttributeToTagMap.Num(); }

protected:
	/** Parse INI file and extract tag definitions */
	TArray<TPair<FString, FString>> ParseTagsFromIniContent(const FString& IniContent);

	/** Log registered tags if debug logging is enabled */
	void LogRegisteredTags() const;

	/** Log registered attributes if debug logging is enabled */
	void LogRegisteredAttributes() const;

private:
	/** Set of all registered tag names */
	TSet<FString> RegisteredTags;

	/** Map from attribute name to gameplay tag */
	TMap<FString, FGameplayTag> AttributeToTagMap;

	/** Map from gameplay tag to attribute name (reverse lookup) */
	TMap<FGameplayTag, FString> TagToAttributeMap;

	/** Whether initialization has completed */
	bool bInitialized = false;
};
