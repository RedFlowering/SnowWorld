// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaMapFilterSystem.h"

UHarmoniaMapFilterSystem::UHarmoniaMapFilterSystem()
{
}

void UHarmoniaMapFilterSystem::AddFilterCategory(const FMapFilterCategory& Category)
{
    // Check if category already exists
    int32 Index = FindCategoryIndex(Category.CategoryTag);
    if (Index != INDEX_NONE)
    {
        // Update existing
        FilterCategories[Index] = Category;
    }
    else
    {
        // Add new
        FilterCategories.Add(Category);
    }
}

bool UHarmoniaMapFilterSystem::RemoveFilterCategory(const FGameplayTag& CategoryTag)
{
    int32 Index = FindCategoryIndex(CategoryTag);
    if (Index != INDEX_NONE)
    {
        FilterCategories.RemoveAt(Index);
        return true;
    }
    return false;
}

bool UHarmoniaMapFilterSystem::SetCategoryVisible(const FGameplayTag& CategoryTag, bool bVisible)
{
    int32 Index = FindCategoryIndex(CategoryTag);
    if (Index != INDEX_NONE)
    {
        FilterCategories[Index].bVisible = bVisible;
        OnFilterChanged.Broadcast(CategoryTag, bVisible);
        return true;
    }
    return false;
}

bool UHarmoniaMapFilterSystem::ToggleCategoryVisibility(const FGameplayTag& CategoryTag)
{
    int32 Index = FindCategoryIndex(CategoryTag);
    if (Index != INDEX_NONE)
    {
        FilterCategories[Index].bVisible = !FilterCategories[Index].bVisible;
        OnFilterChanged.Broadcast(CategoryTag, FilterCategories[Index].bVisible);
        return true;
    }
    return false;
}

bool UHarmoniaMapFilterSystem::IsCategoryVisible(const FGameplayTag& CategoryTag) const
{
    int32 Index = FindCategoryIndex(CategoryTag);
    if (Index != INDEX_NONE)
    {
        return FilterCategories[Index].bVisible;
    }
    return true; // Default visible if not found
}

bool UHarmoniaMapFilterSystem::IsTagVisible(const FGameplayTag& Tag) const
{
    if (!Tag.IsValid())
    {
        return true;
    }

    // Check if tag matches any category
    for (const FMapFilterCategory& Category : FilterCategories)
    {
        if (Tag.MatchesTag(Category.CategoryTag))
        {
            return Category.bVisible;
        }
    }

    return true; // Default visible if no matching category
}

TArray<FMapFilterCategory> UHarmoniaMapFilterSystem::GetVisibleCategories() const
{
    TArray<FMapFilterCategory> Result;

    for (const FMapFilterCategory& Category : FilterCategories)
    {
        if (Category.bVisible)
        {
            Result.Add(Category);
        }
    }

    return Result;
}

void UHarmoniaMapFilterSystem::ShowAllCategories()
{
    for (FMapFilterCategory& Category : FilterCategories)
    {
        if (!Category.bVisible)
        {
            Category.bVisible = true;
            OnFilterChanged.Broadcast(Category.CategoryTag, true);
        }
    }
}

void UHarmoniaMapFilterSystem::HideAllCategories()
{
    for (FMapFilterCategory& Category : FilterCategories)
    {
        if (Category.bVisible)
        {
            Category.bVisible = false;
            OnFilterChanged.Broadcast(Category.CategoryTag, false);
        }
    }
}

void UHarmoniaMapFilterSystem::ResetToDefaults()
{
    // Reset all to visible
    ShowAllCategories();
}

int32 UHarmoniaMapFilterSystem::FindCategoryIndex(const FGameplayTag& CategoryTag) const
{
    for (int32 i = 0; i < FilterCategories.Num(); ++i)
    {
        if (FilterCategories[i].CategoryTag.MatchesTagExact(CategoryTag))
        {
            return i;
        }
    }
    return INDEX_NONE;
}
