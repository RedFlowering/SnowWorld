// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Summon.generated.h"

class AHarmoniaMonsterBase;
class UHarmoniaMonsterData;

/**
 * Summon configuration for spawning minions
 */
USTRUCT(BlueprintType)
struct FHarmoniaSummonConfig
{
	GENERATED_BODY()

	/** Monster data to summon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	TSoftObjectPtr<UHarmoniaMonsterData> MonsterDataToSummon;

	/** Number of monsters to summon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	int32 SummonCount = 1;

	/** Spawn radius around summoner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	float SpawnRadius = 300.0f;

	/** Summoned monsters lifetime (0 = permanent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	float SummonedLifetime = 30.0f;

	/** Should summoned monsters inherit summoner's level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	bool bInheritSummonerLevel = true;

	FHarmoniaSummonConfig()
		: SummonCount(1)
		, SpawnRadius(300.0f)
		, SummonedLifetime(30.0f)
		, bInheritSummonerLevel(true)
	{
	}
};

/**
 * Summoner Monster Ability
 * - Summons additional monsters during combat
 * - Can summon multiple types
 * - Summoned monsters can be temporary or permanent
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.6.3 for tag configuration
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Summon : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Summon();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Summon configurations (can summon multiple types) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	TArray<FHarmoniaSummonConfig> SummonConfigs;

	/** Maximum number of active summons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	int32 MaxActiveSummons = 5;

	/** Summon animation montage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	UAnimMontage* SummonMontage;

	/** Visual effect for summoning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summon")
	TSubclassOf<AActor> SummonEffect;

	/** Currently active summons */
	UPROPERTY()
	TArray<AHarmoniaMonsterBase*> ActiveSummons;

	/** Perform summoning */
	UFUNCTION(BlueprintCallable, Category = "Summon")
	void PerformSummon();

	/** Summon specific monster type */
	void SummonMonster(const FHarmoniaSummonConfig& Config);

	/** Clean up dead summons from list */
	void CleanupDeadSummons();

	/** Called when a summoned monster dies */
	UFUNCTION()
	void OnSummonedMonsterDied(AHarmoniaMonsterBase* DeadMonster, AActor* Killer);

	/** Destroy all summoned monsters */
	void DestroyAllSummons();

	/** Get spawn location for summoned monster */
	FVector GetSummonSpawnLocation(float Radius) const;
};
