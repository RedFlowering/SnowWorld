// Copyright 2025 Snow Game Studio.

#include "Journal/HarmoniaJournalEntry.h"

UHarmoniaJournalEntry::UHarmoniaJournalEntry()
	: Title(FText::FromString("New Entry"))
	, Content(FText::FromString("Content..."))
	, Image(nullptr)
	, bHiddenUntilUnlocked(true)
{
}
