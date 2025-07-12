// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaLoadManager.h"
#include "HarmoniaRowIncludes.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

#define HARMONIALOADMANAGER() UHarmoniaLoadManager::Get()

#define GETITEMDATATABLE() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("Item"))
#define FINDITEMROW(RowName) static_cast<const FItemData*>(GETITEMDATATABLE() ? GETITEMDATATABLE()->FindRow<FItemData>(RowName, TEXT("FINDITEMROW")) : nullptr)
#define GETALLITEMROWS(OutRows) \
	do { \
		OutRows.Empty(); \
		if (UDataTable* Table = GETITEMDATATABLE()) { \
			for (const auto& Elem : Table->GetRowMap()) { \
				if (const FItemData* Row = reinterpret_cast<const FItemData*>(Elem.Value)) { \
					OutRows.Add(Row); \
				} \
			} \
		} \
	} while(0)

