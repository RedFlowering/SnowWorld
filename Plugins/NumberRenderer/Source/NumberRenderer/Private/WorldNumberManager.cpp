// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#include "WorldNumberManager.h"

#include "NumberRenderActor.h"
#include "NumberRendererSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UWorldNumberManager::UWorldNumberManager()
{

}

UWorldNumberManager::~UWorldNumberManager()
{
	GameWorld = nullptr;
	RenderActors.Empty();
}

void UWorldNumberManager::BeginDestroy()
{
	Super::BeginDestroy();

	GameWorld = nullptr;
	RenderActors.Empty();
}

UWorld* UWorldNumberManager::GetWorld() const
{
	return Cast<UWorld>(GetOuter());
}

void UWorldNumberManager::Init(UWorld* InWorld)
{
#if !UE_SERVER
	GameWorld = InWorld;
	if (InWorld)
	{
		for (auto& Entry : UNumberRendererSettings::Get()->NumberRenderActorClasses)
		{
			if (UClass* NewRenderActorClass = Entry.Value.LoadSynchronous())
			{
				ANumberRenderActor* RenderActor = InWorld->SpawnActorDeferred<ANumberRenderActor>(NewRenderActorClass, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if (RenderActor != nullptr)
				{
					UGameplayStatics::FinishSpawningActor(RenderActor, FTransform::Identity);

					RenderActors.Add(Entry.Key, RenderActor);
				}
			}
		}
	}
#endif
}

ANumberRenderActor* UWorldNumberManager::GetRenderActor(ENumberFontType Type /*= ENumberFontType::NumberFontType_Default*/)
{
#if !UE_SERVER
	if (RenderActors.Contains(Type))
	{
		return RenderActors[Type];
	}
#endif
	return nullptr;
}

void UWorldNumberManager::Preload(FVector PreSpawnLocation)
{
#if !UE_SERVER
	for (auto& Entry : RenderActors)
	{
		FTransform Trans = FTransform::Identity;
		Trans.SetLocation(PreSpawnLocation);
		if (Entry.Value)
		{
			Entry.Value->Preload(Trans, 0.2f);
		}
	}
#endif
}
