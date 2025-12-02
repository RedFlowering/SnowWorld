// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaClassData.generated.h"

/**
 * ?´ë˜???°ì´???ì…‹
 * ìºë¦­???´ë˜???•ì˜, ê°ì„±, ?„ë ˆ?¤í‹°ì§€ ?°ì´?°ë? ê´€ë¦?
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaClassData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ ?´ë˜???•ì˜
	/** ëª¨ë“  ?´ë˜???•ì˜ ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class Data")
	TArray<FHarmoniaClassDefinition> ClassDefinitions;

	//~ ê°ì„± ?•ì˜
	/** ê°ì„± ?¨ê³„ë³??•ì˜ ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening Data")
	TArray<FHarmoniaAwakeningDefinition> AwakeningDefinitions;

	//~ ?„ë ˆ?¤í‹°ì§€ ?•ì˜
	/** ?„ë ˆ?¤í‹°ì§€ ?ˆë²¨ë³??•ì˜ ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige Data")
	TArray<FHarmoniaPrestigeDefinition> PrestigeDefinitions;

	//~ ? í‹¸ë¦¬í‹° ?¨ìˆ˜
	/** ?¹ì • ?´ë˜???•ì˜ ì°¾ê¸° (Blueprint?? */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Class Definition")
	bool GetClassDefinitionBP(EHarmoniaCharacterClass ClassType, FHarmoniaClassDefinition& OutClassDef) const;

	/** ?¹ì • ?´ë˜???•ì˜ ì°¾ê¸° (C++?? */
	const FHarmoniaClassDefinition* GetClassDefinition(EHarmoniaCharacterClass ClassType) const;

	/** ?¹ì • ê°ì„± ?¨ê³„ ?•ì˜ ì°¾ê¸° (Blueprint?? */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Awakening Definition")
	bool GetAwakeningDefinitionBP(EHarmoniaAwakeningTier Tier, FHarmoniaAwakeningDefinition& OutAwakeningDef) const;

	/** ?¹ì • ê°ì„± ?¨ê³„ ?•ì˜ ì°¾ê¸° (C++?? */
	const FHarmoniaAwakeningDefinition* GetAwakeningDefinition(EHarmoniaAwakeningTier Tier) const;

	/** ?¹ì • ?„ë ˆ?¤í‹°ì§€ ?ˆë²¨ ?•ì˜ ì°¾ê¸° (Blueprint?? */
	UFUNCTION(BlueprintPure, Category = "Class Data", DisplayName = "Get Prestige Definition")
	bool GetPrestigeDefinitionBP(int32 PrestigeLevel, FHarmoniaPrestigeDefinition& OutPrestigeDef) const;

	/** ?¹ì • ?„ë ˆ?¤í‹°ì§€ ?ˆë²¨ ?•ì˜ ì°¾ê¸° (C++?? */
	const FHarmoniaPrestigeDefinition* GetPrestigeDefinition(int32 PrestigeLevel) const;

	/** ì´ˆê¸° ?´ë˜??ëª©ë¡ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetStarterClasses() const;

	/** ?¹ì • ?´ë˜?¤ì—???„ì§ ê°€?¥í•œ ?ìœ„ ?´ë˜??ëª©ë¡ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	TArray<EHarmoniaCharacterClass> GetAdvancementOptions(EHarmoniaCharacterClass BaseClass) const;

	/** ?„ì§ ?”êµ¬?¬í•­ ì¶©ì¡± ?¬ë? ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool CanAdvanceToClass(EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass, int32 PlayerLevel) const;

	/** ?ìœ„ ?´ë˜???¬ë? ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	bool IsAdvancedClass(EHarmoniaCharacterClass ClassType) const;

	/** ?´ë˜???°ì–´ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Class Data")
	int32 GetClassTier(EHarmoniaCharacterClass ClassType) const;
};
