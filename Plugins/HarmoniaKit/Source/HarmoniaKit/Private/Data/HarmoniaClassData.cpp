// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaClassData.h"

bool UHarmoniaClassData::GetClassDefinitionBP(EHarmoniaCharacterClass ClassType, FHarmoniaClassDefinition& OutClassDef) const
{
	const FHarmoniaClassDefinition* Def = GetClassDefinition(ClassType);
	if (Def)
	{
		OutClassDef = *Def;
		return true;
	}
	return false;
}

const FHarmoniaClassDefinition* UHarmoniaClassData::GetClassDefinition(EHarmoniaCharacterClass ClassType) const
{
	for (const FHarmoniaClassDefinition& ClassDef : ClassDefinitions)
	{
		if (ClassDef.ClassType == ClassType)
		{
			return &ClassDef;
		}
	}
	return nullptr;
}

bool UHarmoniaClassData::GetAwakeningDefinitionBP(EHarmoniaAwakeningTier Tier, FHarmoniaAwakeningDefinition& OutAwakeningDef) const
{
	const FHarmoniaAwakeningDefinition* Def = GetAwakeningDefinition(Tier);
	if (Def)
	{
		OutAwakeningDef = *Def;
		return true;
	}
	return false;
}

const FHarmoniaAwakeningDefinition* UHarmoniaClassData::GetAwakeningDefinition(EHarmoniaAwakeningTier Tier) const
{
	for (const FHarmoniaAwakeningDefinition& AwakeningDef : AwakeningDefinitions)
	{
		if (AwakeningDef.Tier == Tier)
		{
			return &AwakeningDef;
		}
	}
	return nullptr;
}

bool UHarmoniaClassData::GetPrestigeDefinitionBP(int32 PrestigeLevel, FHarmoniaPrestigeDefinition& OutPrestigeDef) const
{
	const FHarmoniaPrestigeDefinition* Def = GetPrestigeDefinition(PrestigeLevel);
	if (Def)
	{
		OutPrestigeDef = *Def;
		return true;
	}
	return false;
}

const FHarmoniaPrestigeDefinition* UHarmoniaClassData::GetPrestigeDefinition(int32 PrestigeLevel) const
{
	for (const FHarmoniaPrestigeDefinition& PrestigeDef : PrestigeDefinitions)
	{
		if (PrestigeDef.PrestigeLevel == PrestigeLevel)
		{
			return &PrestigeDef;
		}
	}
	return nullptr;
}

TArray<EHarmoniaCharacterClass> UHarmoniaClassData::GetStarterClasses() const
{
	TArray<EHarmoniaCharacterClass> Result;

	// Tier 1 classes (base classes)
	Result.Add(EHarmoniaCharacterClass::Warrior);
	Result.Add(EHarmoniaCharacterClass::Rogue);
	Result.Add(EHarmoniaCharacterClass::Mage);

	return Result;
}

TArray<EHarmoniaCharacterClass> UHarmoniaClassData::GetAdvancementOptions(EHarmoniaCharacterClass BaseClass) const
{
	const FHarmoniaClassDefinition* ClassDef = GetClassDefinition(BaseClass);
	if (ClassDef)
	{
		return ClassDef->AdvancementClasses;
	}

	return TArray<EHarmoniaCharacterClass>();
}

bool UHarmoniaClassData::IsAdvancedClass(EHarmoniaCharacterClass ClassType) const
{
	// Tier 2 (advanced) classes
	const TArray<EHarmoniaCharacterClass> AdvancedClasses = {
		EHarmoniaCharacterClass::Berserker,
		EHarmoniaCharacterClass::Knight,
		EHarmoniaCharacterClass::Paladin,
		EHarmoniaCharacterClass::Assassin,
		EHarmoniaCharacterClass::Ranger,
		EHarmoniaCharacterClass::Duelist,
		EHarmoniaCharacterClass::Sorcerer,
		EHarmoniaCharacterClass::Cleric,
		EHarmoniaCharacterClass::Warlock
	};

	return AdvancedClasses.Contains(ClassType);
}

int32 UHarmoniaClassData::GetClassTier(EHarmoniaCharacterClass ClassType) const
{
	if (ClassType == EHarmoniaCharacterClass::None)
	{
		return 0;
	}

	// Tier 1 (base) classes
	if (ClassType == EHarmoniaCharacterClass::Warrior ||
		ClassType == EHarmoniaCharacterClass::Rogue ||
		ClassType == EHarmoniaCharacterClass::Mage)
	{
		return 1;
	}

	// Tier 2 (advanced) classes
	if (IsAdvancedClass(ClassType))
	{
		return 2;
	}

	return 0;
}

bool UHarmoniaClassData::CanAdvanceToClass(EHarmoniaCharacterClass CurrentClass, EHarmoniaCharacterClass TargetClass, int32 PlayerLevel) const
{
	const FHarmoniaClassDefinition* ClassDef = GetClassDefinition(CurrentClass);
	if (!ClassDef)
	{
		return false;
	}

	// Check if target class is in advancement options
	if (!ClassDef->AdvancementClasses.Contains(TargetClass))
	{
		return false;
	}

	// Check level requirement
	if (PlayerLevel < ClassDef->RequiredLevelForAdvancement)
	{
		return false;
	}

	return true;
}
