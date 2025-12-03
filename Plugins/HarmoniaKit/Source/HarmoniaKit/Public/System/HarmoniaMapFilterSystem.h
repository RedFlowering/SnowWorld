// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "HarmoniaMapFilterSystem.generated.h"

/**
 * Map marker filter category
 */
USTRUCT(BlueprintType)
struct FMapFilterCategory
{
    GENERATED_BODY()

    // Category name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    FText CategoryName;

    // Category tag (for filtering)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    FGameplayTag CategoryTag;

    // Whether this category is currently visible
    UPROPERTY(BlueprintReadWrite, Category = "Filter")
    bool bVisible = true;

    // Icon for this category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    TObjectPtr<UTexture2D> Icon;

    // Color for this category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
    FLinearColor Color = FLinearColor::White;

    FMapFilterCategory()
    {
        CategoryName = FText::GetEmpty();
        CategoryTag = FGameplayTag();
        bVisible = true;
        Icon = nullptr;
        Color = FLinearColor::White;
    }

    FMapFilterCategory(const FText& InName, const FGameplayTag& InTag)
    {
        CategoryName = InName;
        CategoryTag = InTag;
        bVisible = true;
        Icon = nullptr;
        Color = FLinearColor::White;
    }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFilterChanged, FGameplayTag, CategoryTag, bool, bVisible);

/**
 * System for filtering map markers by category
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaMapFilterSystem : public UObject
{
    GENERATED_BODY()

public:
    UHarmoniaMapFilterSystem();

    // Filter categories
    UPROPERTY(BlueprintReadOnly, Category = "Filter")
    TArray<FMapFilterCategory> FilterCategories;

    // Delegate called when filter changes
    UPROPERTY(BlueprintAssignable, Category = "Filter|Events")
    FOnFilterChanged OnFilterChanged;

    // Add a filter category
    UFUNCTION(BlueprintCallable, Category = "Filter")
    void AddFilterCategory(const FMapFilterCategory& Category);

    // Remove a filter category
    UFUNCTION(BlueprintCallable, Category = "Filter")
    bool RemoveFilterCategory(const FGameplayTag& CategoryTag);

    // Set category visibility
    UFUNCTION(BlueprintCallable, Category = "Filter")
    bool SetCategoryVisible(const FGameplayTag& CategoryTag, bool bVisible);

    // Toggle category visibility
    UFUNCTION(BlueprintCallable, Category = "Filter")
    bool ToggleCategoryVisibility(const FGameplayTag& CategoryTag);

    // Check if category is visible
    UFUNCTION(BlueprintPure, Category = "Filter")
    bool IsCategoryVisible(const FGameplayTag& CategoryTag) const;

    // Check if a tag matches any visible category
    UFUNCTION(BlueprintPure, Category = "Filter")
    bool IsTagVisible(const FGameplayTag& Tag) const;

    // Get all filter categories
    UFUNCTION(BlueprintPure, Category = "Filter")
    TArray<FMapFilterCategory> GetAllCategories() const { return FilterCategories; }

    // Get visible categories
    UFUNCTION(BlueprintCallable, Category = "Filter")
    TArray<FMapFilterCategory> GetVisibleCategories() const;

    // Show all categories
    UFUNCTION(BlueprintCallable, Category = "Filter")
    void ShowAllCategories();

    // Hide all categories
    UFUNCTION(BlueprintCallable, Category = "Filter")
    void HideAllCategories();

    // Reset to default visibility
    UFUNCTION(BlueprintCallable, Category = "Filter")
    void ResetToDefaults();

protected:
    // Find category index by tag
    int32 FindCategoryIndex(const FGameplayTag& CategoryTag) const;
};
