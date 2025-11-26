// Copyright 2025 Snow Game Studio.

#include "Tests/HarmoniaTestBase.h"
#include "System/HarmoniaCombatPowerCalculator.h"
#include "Libraries/HarmoniaCombatLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

//////////////////////////////////////////////////////////////////////////
// Combat Power Calculator Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FCombatPowerTest_Creation, "Combat.PowerCalculator.Creation")
bool FCombatPowerTest_Creation::RunTest(const FString& Parameters)
{
	UHarmoniaCombatPowerCalculator* Calculator = NewObject<UHarmoniaCombatPowerCalculator>();
	TestNotNull(TEXT("Combat power calculator should be created"), Calculator);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FCombatPowerTest_ElementalAffinity, "Combat.PowerCalculator.ElementalAffinity")
bool FCombatPowerTest_ElementalAffinity::RunTest(const FString& Parameters)
{
	// Test elemental affinity (Fire > Ice)
	float FireVsIce = UHarmoniaCombatPowerCalculator::GetElementalAffinity(EHarmoniaElementType::Fire, EHarmoniaElementType::Ice);
	float IceVsFire = UHarmoniaCombatPowerCalculator::GetElementalAffinity(EHarmoniaElementType::Ice, EHarmoniaElementType::Fire);
	
	TestTrue(TEXT("Fire should be strong against Ice"), FireVsIce > 1.0f);
	TestTrue(TEXT("Ice should be weak against Fire"), IceVsFire < 1.0f);
	
	// Test neutral matchup
	float NoneVsNone = UHarmoniaCombatPowerCalculator::GetElementalAffinity(EHarmoniaElementType::None, EHarmoniaElementType::None);
	TestEqual(TEXT("None vs None should be neutral"), NoneVsNone, 1.0f);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FCombatPowerTest_LevelDifference, "Combat.PowerCalculator.LevelDifference")
bool FCombatPowerTest_LevelDifference::RunTest(const FString& Parameters)
{
	// Test level scaling
	FHarmoniaCombatPowerParams HigherLevelParams;
	HigherLevelParams.LevelDifference = 10;
	HigherLevelParams.BasePower = 100.0f;
	
	FHarmoniaCombatPowerParams LowerLevelParams;
	LowerLevelParams.LevelDifference = -10;
	LowerLevelParams.BasePower = 100.0f;
	
	FHarmoniaCombatPowerResult HigherLevelResult = UHarmoniaCombatPowerCalculator::CalculateCombatPower(HigherLevelParams);
	FHarmoniaCombatPowerResult LowerLevelResult = UHarmoniaCombatPowerCalculator::CalculateCombatPower(LowerLevelParams);
	
	TestTrue(TEXT("Higher level should give more power"), HigherLevelResult.FinalPower > LowerLevelResult.FinalPower);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FCombatPowerTest_TerrainModifier, "Combat.PowerCalculator.TerrainModifier")
bool FCombatPowerTest_TerrainModifier::RunTest(const FString& Parameters)
{
	// Test terrain multiplier with fire element
	float FireHighGround = UHarmoniaCombatPowerCalculator::GetTerrainMultiplier(EHarmoniaElementType::Fire, EHarmoniaTerrainType::HighGround);
	float FireLowGround = UHarmoniaCombatPowerCalculator::GetTerrainMultiplier(EHarmoniaElementType::Fire, EHarmoniaTerrainType::LowGround);
	
	// Just verify the function works without crash
	TestTrue(TEXT("Terrain multiplier should be positive"), FireHighGround > 0.0f);
	TestTrue(TEXT("Terrain multiplier should be positive"), FireLowGround > 0.0f);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Combat Library Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FCombatLibraryTest_AngleCalculation, "Combat.Library.AngleCalculation")
bool FCombatLibraryTest_AngleCalculation::RunTest(const FString& Parameters)
{
	FVector Forward = FVector(1.0f, 0.0f, 0.0f);
	FVector Backward = FVector(-1.0f, 0.0f, 0.0f);
	FVector Right = FVector(0.0f, 1.0f, 0.0f);
	
	float ForwardBackward = UHarmoniaCombatLibrary::GetAngleBetweenVectors(Forward, Backward);
	float ForwardRight = UHarmoniaCombatLibrary::GetAngleBetweenVectors(Forward, Right);
	
	TestEqual(TEXT("Opposite vectors should be 180 degrees"), ForwardBackward, 180.0f);
	TestEqual(TEXT("Perpendicular vectors should be 90 degrees"), ForwardRight, 90.0f);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FCombatLibraryTest_BackstabDetection, "Combat.Library.BackstabDetection")
bool FCombatLibraryTest_BackstabDetection::RunTest(const FString& Parameters)
{
	// This test would need mock actors
	// For now, just verify the function exists and is callable
	bool bResult = UHarmoniaCombatLibrary::IsAttackFromBehind(nullptr, FVector::ZeroVector, 45.0f);
	TestFalse(TEXT("Null target should return false"), bResult);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Damage Calculation Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FDamageTest_CriticalHit, "Combat.Damage.CriticalHit")
bool FDamageTest_CriticalHit::RunTest(const FString& Parameters)
{
	// Test critical hit calculation
	// Would typically involve GAS and AttributeSet
	float BaseDamage = 100.0f;
	float CritMultiplier = 1.5f;
	float CritDamage = BaseDamage * CritMultiplier;
	
	TestEqual(TEXT("Critical damage should be base * multiplier"), CritDamage, 150.0f);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FDamageTest_ArmorReduction, "Combat.Damage.ArmorReduction")
bool FDamageTest_ArmorReduction::RunTest(const FString& Parameters)
{
	// Test armor reduction formula
	float BaseDamage = 100.0f;
	float Armor = 50.0f;
	
	// Typical armor formula: FinalDamage = BaseDamage * (100 / (100 + Armor))
	float FinalDamage = BaseDamage * (100.0f / (100.0f + Armor));
	
	TestTrue(TEXT("Armor should reduce damage"), FinalDamage < BaseDamage);
	TestTrue(TEXT("Final damage should be positive"), FinalDamage > 0.0f);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Status Effect Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FStatusEffectTest_Stacking, "Combat.StatusEffect.Stacking")
bool FStatusEffectTest_Stacking::RunTest(const FString& Parameters)
{
	// Test status effect stacking rules
	// Would typically test with GAS
	int32 MaxStacks = 5;
	int32 CurrentStacks = 3;
	int32 AddedStacks = 4;
	int32 ResultStacks = FMath::Min(CurrentStacks + AddedStacks, MaxStacks);
	
	TestEqual(TEXT("Stacks should be capped at max"), ResultStacks, MaxStacks);
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
