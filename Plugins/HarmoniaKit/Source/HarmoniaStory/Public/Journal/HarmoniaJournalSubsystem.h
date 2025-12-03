// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaJournalSubsystem.generated.h"

class UHarmoniaJournalEntry;

/**
 * Delegate for journal events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJournalEntryUnlocked, FHarmoniaID, EntryId);

/**
 * Subsystem to manage journal and lore entries
 */
UCLASS()
class HARMONIASTORY_API UHarmoniaJournalSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Unlock a journal entry */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void UnlockEntry(FHarmoniaID EntryId);

	/** Check if an entry is unlocked */
	UFUNCTION(BlueprintPure, Category = "Journal")
	bool IsEntryUnlocked(FHarmoniaID EntryId) const;

	/** Get all unlocked entries */
	UFUNCTION(BlueprintPure, Category = "Journal")
	TArray<FHarmoniaID> GetUnlockedEntries() const;

public:
	/** Called when an entry is unlocked */
	UPROPERTY(BlueprintAssignable, Category = "Journal|Events")
	FOnJournalEntryUnlocked OnJournalEntryUnlocked;

protected:
	/** Set of unlocked entry IDs */
	UPROPERTY(SaveGame)
	TSet<FHarmoniaID> UnlockedEntries;
};
