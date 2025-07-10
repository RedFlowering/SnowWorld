// Copyright 2025 Snow Game Studio.

#pragma once

#include "Managers/HarmoniaLoadManager.h"

#define HARMONIALOADMANAGER() UHarmoniaLoadManager::Get()

#define GETCOSMETICDATATABLE() HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("Cosmetic"))
