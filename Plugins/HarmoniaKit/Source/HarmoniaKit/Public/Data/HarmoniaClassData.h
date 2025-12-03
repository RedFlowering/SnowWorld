// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaClassData.generated.h"

/**
 * ?�래???�이???�셋
 * 캐릭???�래???�의, 각성, ?�레?�티지 ?�이?��? 관�?
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaClassData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ ?�래???�의
	/** 모든 ?�래???�의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class Data")
	TArray<FHarmoniaClassDefinition> ClassDefinitions;

	//~ 각성 ?�의
	/** 각성 ?�계�??�의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening Data")
	TArray<FHarmoniaAwakeningDefinition> AwakeningDefinitions;

	//~ ?�레?�티지 ?�의
	/** ?�레?�티지 ?�벨�??�의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige Data")
	TArray<FHarmoniaPrestigeDefinition> PrestigeDefinitions;

	//~ ?�틸리티 ?�수
	/** ?�정 ?�래???�의 찾기 (Blueprint?? */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Class Definition")
	bool GetClassDefinitionBP(EHarmoniaCharacterClass ClassType, FHarmoniaClassDefinition& OutClassDef) const;

	/** ?�정 ?�래???�의 찾기 (C++?? */
	const FHarmoniaClassDefinition* GetClassDefinition(EHarmoniaCharacterClass ClassType) const;

	/** ?�정 각성 ?�계 ?�의 찾기 (Blueprint?? */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Awakening Definition")
	bool GetAwakeningDefinitionBP(EHarmoniaAwakeningTier Tier, FHarmoniaAwakeningDefinition& OutAwakeningDef) const;

	/** ?�정 각성 ?�계 ?�의 찾기 (C++?? */
	const FHarmoniaAwakeningDefinition* GetAwakeningDefinition(EHarmoniaAwakeningTier Tier) const;

	/** ?�정 ?�레?�티지 ?�벨 ?�의 찾기 (Blueprint?? */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Prestige Definition")
	bool GetPrestigeDefinitionBP(int32 PrestigeLevel, FHarmoniaPrestigeDefinition& OutPrestigeDef) const;

	/** ?�정 ?�레?�티지 ?�벨 ?�의 찾기 (C++?? */
	const FHarmoniaPrestigeDefinition* GetPrestigeDefinition(int32 PrestigeLevel) const;

	/** 초기 ?�래??목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetStarterClasses() const;

	/** ?�정 ?�래?�에???�직 가?�한 ?�위 ?�래??목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetAdvancementOptions(EHarmoniaCharacterClass BaseClass) const;

	/** ?�직 ?�구?�항 충족 ?��? ?�인 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool CanAdvanceToClass(EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass, int32 PlayerLevel) const;

	/** ?�위 ?�래???��? ?�인 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool IsAdvancedClass(EHarmoniaCharacterClass ClassType) const;

	/** ?�래???�어 반환 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	int32 GetClassTier(EHarmoniaCharacterClass ClassType) const;
};
