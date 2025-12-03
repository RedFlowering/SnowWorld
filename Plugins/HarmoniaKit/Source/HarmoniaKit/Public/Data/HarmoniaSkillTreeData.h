// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaSkillTreeData.generated.h"

/**
 * ?�킬 ?�리 ?�이???�셋
 * ?�킬 ?�드 ?�의, ?�금 조건, 보상 ?�을 관�?
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaSkillTreeData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ ?�킬 ?�리 기본 ?�보
	/** ?�킬 ?�리 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FName TreeID;

	/** ?�킬 ?�리 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeName;

	/** ?�킬 ?�리 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeDescription;

	/** ?�킬 ?�리 카테고리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	EHarmoniaSkillTreeCategory Category;

	/** ?�킬 ?�리 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Visual")
	TSoftObjectPtr<UTexture2D> TreeIcon;

	//~ ?�근 조건
	/** ?�요 ?�래??(None?�면 모든 ?�래?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** 최소 ?�벨 ?�구?�항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	int32 MinimumLevel = 1;

	//~ ?�킬 ?�드
	/** ?�킬 ?�리???�함??모든 ?�킬 ?�드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Nodes")
	TArray<FHarmoniaSkillNode> SkillNodes;

	//~ ?�틸리티 ?�수
	/** ID�??�킬 ?�드 찾기 (Blueprint?? */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree", DisplayName = "Find Skill Node")
	bool FindSkillNodeBP(FName NodeID, FHarmoniaSkillNode& OutNode);

	/** ID�??�킬 ?�드 찾기 (C++?? */
	FHarmoniaSkillNode* FindSkillNode(FName NodeID);

	/** ?�?�별 ?�킬 ?�드 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetSkillNodesByType(EHarmoniaSkillNodeType NodeType) const;

	/** 루트 ?�드 (?�행 조건 ?�는 ?�드) 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetRootNodes() const;

	/** ?�정 ?�드???�식 ?�드 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetChildNodes(FName ParentNodeID) const;

	/** ?�레?�어가 ???�킬 ?�리???�근 가?�한지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsAccessible(EHarmoniaCharacterClass PlayerClass, int32 PlayerLevel) const;
};
