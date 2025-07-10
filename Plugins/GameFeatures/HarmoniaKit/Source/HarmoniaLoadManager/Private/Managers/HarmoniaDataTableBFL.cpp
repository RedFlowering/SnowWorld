// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaDataTableBFL.h"

UDataTable* UHarmoniaDataTableBFL::GetCosmeticDataTable()
{
    return UHarmoniaLoadManager::Get()->GetDataTableByKey(TEXT("Cosmetic"));
}

