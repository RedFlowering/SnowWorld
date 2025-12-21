// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaBiomeSoundVolume.h"
#include "System/HarmoniaSoundCacheSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogHarmoniaBiomeSound, Log, All);

AHarmoniaBiomeSoundVolume::AHarmoniaBiomeSoundVolume()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHarmoniaBiomeSoundVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!IsPlayerPawn(OtherActor))
	{
		return;
	}

	UE_LOG(LogHarmoniaBiomeSound, Verbose, TEXT("Player entered biome sound volume: %s"), *GetName());
	StartSounds();
}

void AHarmoniaBiomeSoundVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (!IsPlayerPawn(OtherActor))
	{
		return;
	}

	UE_LOG(LogHarmoniaBiomeSound, Verbose, TEXT("Player exited biome sound volume: %s"), *GetName());
	StopSounds();
}

void AHarmoniaBiomeSoundVolume::StartSounds()
{
	// Stop any existing sounds first
	StopSounds();

	if (SoundTags.IsEmpty())
	{
		UE_LOG(LogHarmoniaBiomeSound, Warning, TEXT("BiomeSoundVolume %s has no SoundTags set"), *GetName());
		return;
	}

	// Get subsystem
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaSoundCacheSubsystem* SoundSubsystem = GameInstance->GetSubsystem<UHarmoniaSoundCacheSubsystem>();
	if (!SoundSubsystem)
	{
		UE_LOG(LogHarmoniaBiomeSound, Warning, TEXT("HarmoniaSoundCacheSubsystem not found"));
		return;
	}

	// Play each sound via subsystem
	for (const FGameplayTag& Tag : SoundTags)
	{
		if (SoundSubsystem->PlayManagedSound(this, Tag, VolumeMultiplier))
		{
			ActiveSoundTags.Add(Tag);
		}
	}

	UE_LOG(LogHarmoniaBiomeSound, Log, TEXT("Started %d biome sounds in volume: %s"), ActiveSoundTags.Num(), *GetName());
}

void AHarmoniaBiomeSoundVolume::StopSounds()
{
	if (ActiveSoundTags.Num() == 0)
	{
		return;
	}

	// Get subsystem
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (!GameInstance)
	{
		ActiveSoundTags.Empty();
		return;
	}

	UHarmoniaSoundCacheSubsystem* SoundSubsystem = GameInstance->GetSubsystem<UHarmoniaSoundCacheSubsystem>();
	if (!SoundSubsystem)
	{
		ActiveSoundTags.Empty();
		return;
	}

	// Stop each sound via subsystem
	for (const FGameplayTag& Tag : ActiveSoundTags)
	{
		SoundSubsystem->StopManagedSound(Tag, FadeOutDuration);
	}

	UE_LOG(LogHarmoniaBiomeSound, Log, TEXT("Stopped %d biome sounds in volume: %s"), ActiveSoundTags.Num(), *GetName());
	ActiveSoundTags.Empty();
}

bool AHarmoniaBiomeSoundVolume::IsPlayerPawn(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	APawn* Pawn = Cast<APawn>(Actor);
	if (!Pawn)
	{
		return false;
	}

	return Pawn->IsLocallyControlled() && Pawn->IsPlayerControlled();
}
