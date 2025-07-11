// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaLoadManager.h"
#include "HarmoniaRowIncludes.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

#define HARMONIALOADMANAGER() UHarmoniaLoadManager::Get()

#define GETCOSMETICDATATABLE() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("Cosmetic"))
#define FINDCOSMETICROW(RowName) static_cast<const FGameplayTag*>(GETCOSMETICDATATABLE() ? GETCOSMETICDATATABLE()->FindRow<FGameplayTag>(RowName, TEXT("FINDCOSMETICROW")) : nullptr)
#define GETALLCOSMETICROWS(OutRows) \
	do { \
		OutRows.Empty(); \
		if (UDataTable* Table = GETCOSMETICDATATABLE()) { \
			for (const auto& Elem : Table->GetRowMap()) { \
				if (const FGameplayTag* Row = reinterpret_cast<const FGameplayTag*>(Elem.Value)) { \
					OutRows.Add(Row); \
				} \
			} \
		} \
	} while(0)

