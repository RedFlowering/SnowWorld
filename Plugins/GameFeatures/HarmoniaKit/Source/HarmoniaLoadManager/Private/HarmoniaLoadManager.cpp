// Copyright 2025 Snow Game Studio.

#include "HarmoniaLoadManager.h"
#include "HarmoniaRegistryAsset.h"

UHarmoniaLoadManager* UHarmoniaLoadManager::SingletonInstance = nullptr;

UHarmoniaLoadManager* UHarmoniaLoadManager::Get()
{
	if (!SingletonInstance)
	{
		SingletonInstance = NewObject<UHarmoniaLoadManager>();
		SingletonInstance->AddToRoot(); // GC 방지
	}
	return SingletonInstance;
}

UDataTable* UHarmoniaLoadManager::GetDataTableByKey(FName Key)
{
	if (LoadedTableMap.Contains(Key))
	{
		return LoadedTableMap[Key];
	}

	if (TSoftObjectPtr<UDataTable>* SoftPtr = SoftTableMap.Find(Key))
	{
		UDataTable* Loaded = SoftPtr->LoadSynchronous();
		if (Loaded)
		{
			LoadedTableMap.Add(Key, Loaded);
			return Loaded;
		}
	}

	return nullptr;
}

void UHarmoniaLoadManager::LoadAllTables()
{
	for (const auto& Pair : SoftTableMap)
	{
		if (!Pair.Value.IsNull())
		{
			UDataTable* Loaded = Pair.Value.LoadSynchronous();
			LoadedTableMap.Add(Pair.Key, Loaded);
		}
	}
}