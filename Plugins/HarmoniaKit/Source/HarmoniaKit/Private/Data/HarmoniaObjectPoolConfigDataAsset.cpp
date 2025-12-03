// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaObjectPoolConfigDataAsset.h"

UHarmoniaObjectPoolConfigDataAsset::UHarmoniaObjectPoolConfigDataAsset()
{
	// Default values are initialized in the struct
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
