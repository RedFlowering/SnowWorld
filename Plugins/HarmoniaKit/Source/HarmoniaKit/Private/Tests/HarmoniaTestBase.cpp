// Copyright 2025 Snow Game Studio.

#include "Tests/HarmoniaTestBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

UWorld* FHarmoniaTestBase::CreateTestWorld()
{
	// Create a new empty world for testing
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	
	if (World)
	{
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(World);
		
		World->InitializeActorsForPlay(FURL());
		World->BeginPlay();
	}
	
	return World;
}

void FHarmoniaTestBase::DestroyTestWorld(UWorld* World)
{
	if (World)
	{
		// Cleanup world
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.World() == World)
			{
				GEngine->DestroyWorldContext(World);
				break;
			}
		}
		
		World->DestroyWorld(false);
	}
}

#endif // WITH_DEV_AUTOMATION_TESTS
