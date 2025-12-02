// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "System/HarmoniaObjectPoolSubsystem.h"
#include "HarmoniaObjectPoolConfigDataAsset.generated.h"

/**
 * ?¤ë¸Œ?íŠ¸ ?€ ?„ì—­ ?¤ì •
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPoolGlobalSettings
{
	GENERATED_BODY()

	/** ?„ì—­ ?€ ?œì„±???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bEnablePooling = true;

	/** ?€ ?µê³„ ?˜ì§‘ ?œì„±??(?”ë²„ê·¸ìš©, ?„ë¡œ?•ì…˜?ì„œ??ë¹„í™œ?±í™” ê¶Œì¥) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bCollectStatistics = true;

	/** ?€ ì¶•ì†Œ ê²€??ì£¼ê¸° (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global", meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float ShrinkCheckInterval = 30.0f;

	/** ?€ ?ˆì—´ (ê²Œì„ ?œì‘ ??ë¯¸ë¦¬ ?ì„±) ?œì„±??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bWarmUpOnStart = true;

	/** ?€?ì„œ êº¼ë‚¸ Actor??ê¸°ë³¸ ?„ì¹˜ (?”ë©´ ë°? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	FVector HiddenLocation = FVector(0.0f, 0.0f, -100000.0f);
};

/**
 * ?¤ë¸Œ?íŠ¸ ?€ ?¤ì • ?°ì´???ì…‹
 * 
 * ?¤ë¸Œ?íŠ¸ ?€ë§??œìŠ¤?œì˜ ?„ì—­ ?¤ì • ë°?ë¯¸ë¦¬ ?•ì˜???€ êµ¬ì„±??ê´€ë¦¬í•©?ˆë‹¤.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaObjectPoolConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaObjectPoolConfigDataAsset();

	/** ?„ì—­ ?€ ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Global")
	FHarmoniaPoolGlobalSettings GlobalSettings;

	/** ë¯¸ë¦¬ ?•ì˜???€ êµ¬ì„± ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Configs")
	TArray<FHarmoniaPoolConfig> PoolConfigs;

	/** ?œê·¸ë¡??€ êµ¬ì„± ì°¾ê¸° */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByTag(FGameplayTag PoolTag, FHarmoniaPoolConfig& OutConfig) const;

	/** ?´ë˜?¤ë¡œ ?€ êµ¬ì„± ì°¾ê¸° */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByClass(TSubclassOf<AActor> ActorClass, FHarmoniaPoolConfig& OutConfig) const;
};
