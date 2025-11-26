// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaClassData.generated.h"

/**
 * 클래스 데이터 에셋
 * 캐릭터 클래스 정의, 각성, 프레스티지 데이터를 관리
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaClassData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ 클래스 정의
	/** 모든 클래스 정의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class Data")
	TArray<FHarmoniaClassDefinition> ClassDefinitions;

	//~ 각성 정의
	/** 각성 단계별 정의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening Data")
	TArray<FHarmoniaAwakeningDefinition> AwakeningDefinitions;

	//~ 프레스티지 정의
	/** 프레스티지 레벨별 정의 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige Data")
	TArray<FHarmoniaPrestigeDefinition> PrestigeDefinitions;

	//~ 유틸리티 함수
	/** 특정 클래스 정의 찾기 (Blueprint용) */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Class Definition")
	bool GetClassDefinitionBP(EHarmoniaCharacterClass ClassType, FHarmoniaClassDefinition& OutClassDef) const;

	/** 특정 클래스 정의 찾기 (C++용) */
	const FHarmoniaClassDefinition* GetClassDefinition(EHarmoniaCharacterClass ClassType) const;

	/** 특정 각성 단계 정의 찾기 (Blueprint용) */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Awakening Definition")
	bool GetAwakeningDefinitionBP(EHarmoniaAwakeningTier Tier, FHarmoniaAwakeningDefinition& OutAwakeningDef) const;

	/** 특정 각성 단계 정의 찾기 (C++용) */
	const FHarmoniaAwakeningDefinition* GetAwakeningDefinition(EHarmoniaAwakeningTier Tier) const;

	/** 특정 프레스티지 레벨 정의 찾기 (Blueprint용) */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Prestige Definition")
	bool GetPrestigeDefinitionBP(int32 PrestigeLevel, FHarmoniaPrestigeDefinition& OutPrestigeDef) const;

	/** 특정 프레스티지 레벨 정의 찾기 (C++용) */
	const FHarmoniaPrestigeDefinition* GetPrestigeDefinition(int32 PrestigeLevel) const;

	/** 초기 클래스 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetStarterClasses() const;

	/** 특정 클래스에서 전직 가능한 상위 클래스 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetAdvancementOptions(EHarmoniaCharacterClass BaseClass) const;

	/** 전직 요구사항 충족 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool CanAdvanceToClass(EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass, int32 PlayerLevel) const;

	/** 상위 클래스 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool IsAdvancedClass(EHarmoniaCharacterClass ClassType) const;

	/** 클래스 티어 반환 */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	int32 GetClassTier(EHarmoniaCharacterClass ClassType) const;
};
