// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * Base class for HarmoniaKit automation tests
 */
class FHarmoniaTestBase : public FAutomationTestBase
{
public:
	FHarmoniaTestBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

protected:
	/** Setup test world */
	static UWorld* CreateTestWorld();
	
	/** Cleanup test world */
	static void DestroyTestWorld(UWorld* World);
};

/**
 * Test flags for HarmoniaKit tests
 */
#define HARMONIA_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority)

/**
 * Macro for defining Harmonia simple tests
 */
#define HARMONIA_SIMPLE_TEST(TestClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestClass, "HarmoniaKit." TestName, HARMONIA_TEST_FLAGS)

/**
 * Macro for defining Harmonia complex tests
 */
#define HARMONIA_COMPLEX_TEST(TestClass, TestName) \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestClass, "HarmoniaKit." TestName, HARMONIA_TEST_FLAGS)
