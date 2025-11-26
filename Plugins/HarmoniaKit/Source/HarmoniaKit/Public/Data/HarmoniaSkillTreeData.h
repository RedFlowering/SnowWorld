// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaSkillTreeData.generated.h"

/**
 * 스킬 트리 데이터 에셋
 * 스킬 노드 정의, 해금 조건, 보상 등을 관리
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaSkillTreeData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ 스킬 트리 기본 정보
	/** 스킬 트리 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FName TreeID;

	/** 스킬 트리 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeName;

	/** 스킬 트리 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeDescription;

	/** 스킬 트리 카테고리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	EHarmoniaSkillTreeCategory Category;

	/** 스킬 트리 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Visual")
	TSoftObjectPtr<UTexture2D> TreeIcon;

	//~ 접근 조건
	/** 필요 클래스 (None이면 모든 클래스) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** 최소 레벨 요구사항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	int32 MinimumLevel = 1;

	//~ 스킬 노드
	/** 스킬 트리에 포함된 모든 스킬 노드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Nodes")
	TArray<FHarmoniaSkillNode> SkillNodes;

	//~ 유틸리티 함수
	/** ID로 스킬 노드 찾기 (Blueprint용) */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree", DisplayName = "Find Skill Node")
	bool FindSkillNodeBP(FName NodeID, FHarmoniaSkillNode& OutNode);

	/** ID로 스킬 노드 찾기 (C++용) */
	FHarmoniaSkillNode* FindSkillNode(FName NodeID);

	/** 타입별 스킬 노드 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetSkillNodesByType(EHarmoniaSkillNodeType NodeType) const;

	/** 루트 노드 (선행 조건 없는 노드) 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetRootNodes() const;

	/** 특정 노드의 자식 노드 목록 반환 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetChildNodes(FName ParentNodeID) const;

	/** 플레이어가 이 스킬 트리에 접근 가능한지 확인 */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsAccessible(EHarmoniaCharacterClass PlayerClass, int32 PlayerLevel) const;
};
