// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaAchievementSystemDefinitions.generated.h"

/**
 * Achievement state enum
 */
UENUM(BlueprintType)
enum class EAchievementState : uint8
{
	Locked			UMETA(DisplayName = "Locked"),			// 잠김
	Unlocked		UMETA(DisplayName = "Unlocked"),		// 해금됨
	Hidden			UMETA(DisplayName = "Hidden"),			// 숨겨짐 (조건 달성 전까지 보이지 않음)
	MAX				UMETA(Hidden)
};

/**
 * Achievement type enum
 */
UENUM(BlueprintType)
enum class EAchievementType : uint8
{
	Standard		UMETA(DisplayName = "Standard"),		// 일반 업적 (단발성)
	Progressive		UMETA(DisplayName = "Progressive"),		// 진행형 업적 (누적 카운트)
	MAX				UMETA(Hidden)
};

/**
 * Achievement definition structure
 * Defines the static data for an achievement
 */
USTRUCT(BlueprintType)
struct FHarmoniaAchievementDefinition : public FTableRowBase
{
	GENERATED_BODY()

	// Achievement ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FHarmoniaID AchievementId;

	// Display Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Description;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	TSoftObjectPtr<UTexture2D> Icon;

	// Achievement Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EAchievementType Type = EAchievementType::Standard;

	// Is Hidden?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsHidden = false;

	// Target Progress (for Progressive type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement", meta = (EditCondition = "Type == EAchievementType::Progressive"))
	int32 TargetProgress = 1;

	// Steam Achievement ID (for integration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FString SteamAchievementId;

	// Reward Tags (can be used to grant titles, cosmetics, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FGameplayTagContainer RewardTags;

	FHarmoniaAchievementDefinition()
		: AchievementId()
		, DisplayName()
		, Description()
		, Icon(nullptr)
		, Type(EAchievementType::Standard)
		, bIsHidden(false)
		, TargetProgress(1)
		, SteamAchievementId()
		, RewardTags()
	{}
};

/**
 * Achievement runtime state
 */
USTRUCT(BlueprintType)
struct FAchievementState
{
	GENERATED_BODY()

	// Achievement ID
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FHarmoniaID AchievementId;

	// Current State
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	EAchievementState State = EAchievementState::Locked;

	// Current Progress
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	int32 CurrentProgress = 0;

	// Unlock Date
	UPROPERTY(BlueprintReadOnly, Category = "Achievement")
	FDateTime UnlockDate;

	FAchievementState()
		: AchievementId()
		, State(EAchievementState::Locked)
		, CurrentProgress(0)
		, UnlockDate(0)
	{}
};
