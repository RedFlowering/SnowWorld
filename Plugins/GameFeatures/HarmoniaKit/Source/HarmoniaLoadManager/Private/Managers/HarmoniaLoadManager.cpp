// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaLoadManager.h"
#include "Settings/HarmoniaProjectSettings.h"

UHarmoniaLoadManager* UHarmoniaLoadManager::SingletonInstance = nullptr;

UHarmoniaLoadManager* UHarmoniaLoadManager::Get()
{
	if (!SingletonInstance)
	{
		SingletonInstance = NewObject<UHarmoniaLoadManager>();
		SingletonInstance->AddToRoot();
		SingletonInstance->LoadFromProjectSettings();
	}
	return SingletonInstance;
}

void UHarmoniaLoadManager::LoadFromProjectSettings()
{
	SoftTableMap.Empty();
	LoadedTableMap.Empty();

	if (!UObjectInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HarmoniaLoadManager] Skipped loading — UObject system is uninitialized."));
		return;
	}

	const UHarmoniaProjectSettings* Settings = GetDefault<UHarmoniaProjectSettings>();
	if (!IsValid(Settings) || Settings->Tables.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HarmoniaLoadManager] No tables configured in HarmoniaProjectSettings."));
		return;
	}

	for (const FHarmoniaDataTableSetting& Setting : Settings->Tables)
	{
		if (!Setting.FunctionName.IsEmpty() && Setting.Table.IsValid() == false)
		{
			const FName Key(*Setting.FunctionName);
			SoftTableMap.Add(Key, Setting.Table);
		}
	}
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