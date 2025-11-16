// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaLoadManager.h"
#include "HarmoniaRowIncludes.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

#define HARMONIALOADMANAGER() UHarmoniaLoadManager::Get()

#define GETITEMDATATABLE() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("Item"))
#define GETGAMEPLAYTAGSDATATABLE() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("GamePlayTags"))
