// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaSkillTreeData.generated.h"

/**
 * ?¤í‚¬ ?¸ë¦¬ ?°ì´???ì…‹
 * ?¤í‚¬ ?¸ë“œ ?•ì˜, ?´ê¸ˆ ì¡°ê±´, ë³´ìƒ ?±ì„ ê´€ë¦?
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaSkillTreeData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ ?¤í‚¬ ?¸ë¦¬ ê¸°ë³¸ ?•ë³´
	/** ?¤í‚¬ ?¸ë¦¬ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FName TreeID;

	/** ?¤í‚¬ ?¸ë¦¬ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeName;

	/** ?¤í‚¬ ?¸ë¦¬ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeDescription;

	/** ?¤í‚¬ ?¸ë¦¬ ì¹´í…Œê³ ë¦¬ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	EHarmoniaSkillTreeCategory Category;

	/** ?¤í‚¬ ?¸ë¦¬ ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Visual")
	TSoftObjectPtr<UTexture2D> TreeIcon;

	//~ ?‘ê·¼ ì¡°ê±´
	/** ?„ìš” ?´ë˜??(None?´ë©´ ëª¨ë“  ?´ë˜?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** ìµœì†Œ ?ˆë²¨ ?”êµ¬?¬í•­ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	int32 MinimumLevel = 1;

	//~ ?¤í‚¬ ?¸ë“œ
	/** ?¤í‚¬ ?¸ë¦¬???¬í•¨??ëª¨ë“  ?¤í‚¬ ?¸ë“œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Nodes")
	TArray<FHarmoniaSkillNode> SkillNodes;

	//~ ? í‹¸ë¦¬í‹° ?¨ìˆ˜
	/** IDë¡??¤í‚¬ ?¸ë“œ ì°¾ê¸° (Blueprint?? */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree", DisplayName = "Find Skill Node")
	bool FindSkillNodeBP(FName NodeID, FHarmoniaSkillNode& OutNode);

	/** IDë¡??¤í‚¬ ?¸ë“œ ì°¾ê¸° (C++?? */
	FHarmoniaSkillNode* FindSkillNode(FName NodeID);

	/** ?€?…ë³„ ?¤í‚¬ ?¸ë“œ ëª©ë¡ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetSkillNodesByType(EHarmoniaSkillNodeType NodeType) const;

	/** ë£¨íŠ¸ ?¸ë“œ (? í–‰ ì¡°ê±´ ?†ëŠ” ?¸ë“œ) ëª©ë¡ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetRootNodes() const;

	/** ?¹ì • ?¸ë“œ???ì‹ ?¸ë“œ ëª©ë¡ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetChildNodes(FName ParentNodeID) const;

	/** ?Œë ˆ?´ì–´ê°€ ???¤í‚¬ ?¸ë¦¬???‘ê·¼ ê°€?¥í•œì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsAccessible(EHarmoniaCharacterClass PlayerClass, int32 PlayerLevel) const;
};
