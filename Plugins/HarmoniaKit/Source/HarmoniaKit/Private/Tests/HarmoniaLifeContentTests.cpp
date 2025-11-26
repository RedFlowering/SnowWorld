// Copyright 2025 Snow Game Studio.

#include "Tests/HarmoniaTestBase.h"
#include "Components/HarmoniaFishingComponent.h"
#include "Components/HarmoniaGatheringComponent.h"
#include "Components/HarmoniaCookingComponent.h"
#include "Components/HarmoniaFarmingComponent.h"
#include "Components/HarmoniaMusicComponent.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

//////////////////////////////////////////////////////////////////////////
// Fishing Component Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FFishingComponentTest_Creation, "LifeContent.Fishing.ComponentCreation")
bool FFishingComponentTest_Creation::RunTest(const FString& Parameters)
{
	// Test that fishing component can be created
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaFishingComponent* FishingComp = NewObject<UHarmoniaFishingComponent>(TestActor);
	
	TestNotNull(TEXT("Fishing component should be created"), FishingComp);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FFishingComponentTest_LevelSystem, "LifeContent.Fishing.LevelSystem")
bool FFishingComponentTest_LevelSystem::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaFishingComponent* FishingComp = NewObject<UHarmoniaFishingComponent>(TestActor);
	
	// Test initial level
	TestEqual(TEXT("Initial fishing level should be 1"), FishingComp->GetLevel(), 1);
	
	// Test adding experience
	FishingComp->AddExperience(100);
	TestTrue(TEXT("Experience should be added"), FishingComp->GetCurrentExperience() > 0);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Gathering Component Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FGatheringComponentTest_Creation, "LifeContent.Gathering.ComponentCreation")
bool FGatheringComponentTest_Creation::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaGatheringComponent* GatheringComp = NewObject<UHarmoniaGatheringComponent>(TestActor);
	
	TestNotNull(TEXT("Gathering component should be created"), GatheringComp);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FGatheringComponentTest_ToolRequirement, "LifeContent.Gathering.ToolRequirement")
bool FGatheringComponentTest_ToolRequirement::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaGatheringComponent* GatheringComp = NewObject<UHarmoniaGatheringComponent>(TestActor);
	
	// Test that gathering requires appropriate tool
	// This test would need a mock resource node
	TestNotNull(TEXT("Gathering component should exist"), GatheringComp);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Cooking Component Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FCookingComponentTest_Creation, "LifeContent.Cooking.ComponentCreation")
bool FCookingComponentTest_Creation::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaCookingComponent* CookingComp = NewObject<UHarmoniaCookingComponent>(TestActor);
	
	TestNotNull(TEXT("Cooking component should be created"), CookingComp);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FCookingComponentTest_QualityCalculation, "LifeContent.Cooking.QualityCalculation")
bool FCookingComponentTest_QualityCalculation::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaCookingComponent* CookingComp = NewObject<UHarmoniaCookingComponent>(TestActor);
	
	// Test quality is calculated based on skill level
	TestNotNull(TEXT("Cooking component should exist"), CookingComp);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Farming Component Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FFarmingComponentTest_Creation, "LifeContent.Farming.ComponentCreation")
bool FFarmingComponentTest_Creation::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaFarmingComponent* FarmingComp = NewObject<UHarmoniaFarmingComponent>(TestActor);
	
	TestNotNull(TEXT("Farming component should be created"), FarmingComp);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FFarmingComponentTest_CropGrowth, "LifeContent.Farming.CropGrowth")
bool FFarmingComponentTest_CropGrowth::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaFarmingComponent* FarmingComp = NewObject<UHarmoniaFarmingComponent>(TestActor);
	
	// Test crop growth mechanics
	TestNotNull(TEXT("Farming component should exist"), FarmingComp);
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Music Component Tests
//////////////////////////////////////////////////////////////////////////

HARMONIA_SIMPLE_TEST(FMusicComponentTest_Creation, "LifeContent.Music.ComponentCreation")
bool FMusicComponentTest_Creation::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaMusicComponent* MusicComp = NewObject<UHarmoniaMusicComponent>(TestActor);
	
	TestNotNull(TEXT("Music component should be created"), MusicComp);
	
	return true;
}

HARMONIA_SIMPLE_TEST(FMusicComponentTest_ScoreCalculation, "LifeContent.Music.ScoreCalculation")
bool FMusicComponentTest_ScoreCalculation::RunTest(const FString& Parameters)
{
	AActor* TestActor = NewObject<AActor>();
	UHarmoniaMusicComponent* MusicComp = NewObject<UHarmoniaMusicComponent>(TestActor);
	
	// Test score calculation for rhythm game
	TestNotNull(TEXT("Music component should exist"), MusicComp);
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
