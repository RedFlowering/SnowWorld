// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaObjectPoolConfigDataAsset.h"

UHarmoniaObjectPoolConfigDataAsset::UHarmoniaObjectPoolConfigDataAsset()
{
	// 기본값은 구조체에서 초기화됨
}

bool UHarmoniaObjectPoolConfigDataAsset::GetPoolConfigByTag(FGameplayTag PoolTag, FHarmoniaPoolConfig& OutConfig) const
{
	for (const FHarmoniaPoolConfig& Config : PoolConfigs)
	{
		if (Config.PoolTag.MatchesTagExact(PoolTag))
		{
			OutConfig = Config;
			return true;
		}
	}
	return false;
}

bool UHarmoniaObjectPoolConfigDataAsset::GetPoolConfigByClass(TSubclassOf<AActor> ActorClass, FHarmoniaPoolConfig& OutConfig) const
{
	for (const FHarmoniaPoolConfig& Config : PoolConfigs)
	{
		if (Config.ActorClass == ActorClass)
		{
			OutConfig = Config;
			return true;
		}
	}
	return false;
}
