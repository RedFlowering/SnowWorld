// Copyright 2025 Snow Game Studio.

#include "HarmoniaLoadManager.h"
#include "HarmoniaRegistryAsset.h"
#include "Engine/AssetManager.h"

UHarmoniaLoadManager* UHarmoniaLoadManager::SingletonInstance = nullptr;

UHarmoniaLoadManager* UHarmoniaLoadManager::Get()
{
	if (!SingletonInstance)
	{
		SingletonInstance = NewObject<UHarmoniaLoadManager>();
		SingletonInstance->AddToRoot(); // GC 방지
		SingletonInstance->AutoLoadRegistry();
	}
	return SingletonInstance;
}

void UHarmoniaLoadManager::AutoLoadRegistry()
{
	if (bIsInitialized)
	{
		return;
	}

	// AssetManager를 통해 UHarmoniaRegistryAsset 타입의 모든 에셋 검색
	UAssetManager& AssetManager = UAssetManager::Get();
	
	TArray<FAssetData> AssetDataList;
	AssetManager.GetAssetRegistry().GetAssetsByClass(UHarmoniaRegistryAsset::StaticClass()->GetClassPathName(), AssetDataList);

	if (AssetDataList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Harmonia] No HarmoniaRegistryAsset found in the project. DataTable lookups will fail."));
		return;
	}

	// 첫 번째 Registry 에셋 로드 (여러 개가 있을 경우 첫 번째만 사용)
	if (AssetDataList.Num() > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Harmonia] Multiple HarmoniaRegistryAsset found. Using: %s"), *AssetDataList[0].AssetName.ToString());
	}

	if (UHarmoniaRegistryAsset* Registry = Cast<UHarmoniaRegistryAsset>(AssetDataList[0].GetAsset()))
	{
		InitializeFromRegistry(Registry);
		UE_LOG(LogTemp, Log, TEXT("[Harmonia] Auto-loaded registry: %s with %d entries"), *AssetDataList[0].AssetName.ToString(), Registry->Entries.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Harmonia] Failed to load HarmoniaRegistryAsset: %s"), *AssetDataList[0].AssetName.ToString());
	}
}

void UHarmoniaLoadManager::InitializeFromRegistry(const UHarmoniaRegistryAsset* Registry)
{
	if (!Registry)
	{
		UE_LOG(LogTemp, Error, TEXT("[Harmonia] InitializeFromRegistry called with null Registry"));
		return;
	}

	SoftTableMap.Empty();
	LoadedTableMap.Empty();

	for (const FHarmoniaDataTableEntry& Entry : Registry->Entries)
	{
		if (!Entry.FunctionName.IsEmpty() && !Entry.Table.IsNull())
		{
			FName Key = FName(*Entry.FunctionName);
			SoftTableMap.Add(Key, Entry.Table);
			UE_LOG(LogTemp, Verbose, TEXT("[Harmonia] Registered DataTable: %s -> %s"), *Entry.FunctionName, *Entry.Table.GetAssetName());
		}
	}

	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] LoadManager initialized with %d DataTable entries"), SoftTableMap.Num());
}

UDataTable* UHarmoniaLoadManager::GetDataTableByKey(FName Key)
{
	// 아직 초기화되지 않았다면 자동 초기화 시도
	if (!bIsInitialized)
	{
		AutoLoadRegistry();
	}

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
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Harmonia] Failed to load DataTable for key: %s"), *Key.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Harmonia] DataTable key not found: %s"), *Key.ToString());
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
			if (Loaded)
			{
				LoadedTableMap.Add(Pair.Key, Loaded);
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[Harmonia] Pre-loaded %d DataTables"), LoadedTableMap.Num());
}