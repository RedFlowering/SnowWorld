// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaJournalEntry.generated.h"

class UTexture2D;

/**
 * Data asset representing a journal or lore entry
 */
UCLASS(BlueprintType)
class HARMONIASTORY_API UHarmoniaJournalEntry : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaJournalEntry();

	/** Unique ID for this entry */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal")
	FHarmoniaID EntryId;

	/** Title of the entry */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal")
	FText Title;

	/** Main content text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal", meta = (MultiLine = true))
	FText Content;

	/** Optional image */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal")
	UTexture2D* Image;

	/** Category ID (e.g., "Bestiary", "History") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal")
	FHarmoniaID CategoryId;

	/** Whether this entry is hidden until unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal")
	bool bHiddenUntilUnlocked = true;
};
