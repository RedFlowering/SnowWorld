// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_PlayTaggedWeaponTrail.h"
#include "System/HarmoniaEffectCacheSubsystem.h"
#include "Definitions/HarmoniaAnimationEffectDefinitions.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UAnimNotifyState_PlayTaggedWeaponTrail::UAnimNotifyState_PlayTaggedWeaponTrail()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(200, 100, 255, 255); // Purple for trails
#endif
}

void UAnimNotifyState_PlayTaggedWeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !EffectTag.IsValid())
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	// Get effect cache subsystem
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaEffectCacheSubsystem* EffectCache = GameInstance->GetSubsystem<UHarmoniaEffectCacheSubsystem>();
	if (!EffectCache)
	{
		UE_LOG(LogAnimation, Error, TEXT("HarmoniaEffectCacheSubsystem not found! Cannot play tagged trail: %s"),
			*EffectTag.ToString());
		return;
	}

	// Get effect data from cache
	const FHarmoniaAnimationEffectData* EffectData = EffectCache->GetEffectData(EffectTag);
	if (!EffectData && bUseLazyLoading)
	{
		// Try lazy loading
		EffectData = EffectCache->GetEffectDataLazy(EffectTag, true);
	}

	if (!EffectData)
	{
		UE_LOG(LogAnimation, Warning, TEXT("Trail effect data not found for tag: %s"), *EffectTag.ToString());
		return;
	}

	// Cache mesh comp for NotifyEnd
	CachedMeshComp = MeshComp;

	// Spawn trail effect
	SpawnTrailEffect(World, MeshComp, EffectData);

	// Play trail sound
	PlayTrailSound(World, MeshComp, EffectData);
}

void UAnimNotifyState_PlayTaggedWeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Destroy trail
	DestroyTrail();

	// Clear cache
	CachedMeshComp.Reset();
}

FString UAnimNotifyState_PlayTaggedWeaponTrail::GetNotifyName_Implementation() const
{
	if (EffectTag.IsValid())
	{
		return FString::Printf(TEXT("Weapon Trail: %s"), *EffectTag.ToString());
	}
	return TEXT("Play Tagged Weapon Trail");
}

#if WITH_EDITOR
void UAnimNotifyState_PlayTaggedWeaponTrail::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Could add editor preview or validation here
}
#endif

void UAnimNotifyState_PlayTaggedWeaponTrail::SpawnTrailEffect(
	UWorld* World,
	USkeletalMeshComponent* MeshComp,
	const FHarmoniaAnimationEffectData* EffectData
)
{
	if (!World || !MeshComp || !EffectData)
	{
		return;
	}

	// Get trail socket names (with override)
	FName StartSocket = bOverrideTrailSockets ? TrailStartSocketOverride : EffectData->TrailStartSocketName;
	FName EndSocket = bOverrideTrailSockets ? TrailEndSocketOverride : EffectData->TrailEndSocketName;

	// Get trail width (with override)
	float FinalWidth = bOverrideTrailWidth ? TrailWidthOverride : EffectData->TrailWidthMultiplier;
	FinalWidth *= WidthMultiplier;

	// Get trail color (with override)
	FLinearColor FinalColor = bOverrideColor ? ColorOverride : EffectData->EffectColor;

	// Spawn Niagara system (preferred)
	if (!EffectData->NiagaraSystem.IsNull())
	{
		UNiagaraSystem* NiagaraSystem = EffectData->NiagaraSystem.LoadSynchronous();
		if (NiagaraSystem)
		{
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				NiagaraSystem,
				MeshComp,
				StartSocket,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				FVector::OneVector,
				EAttachLocation::SnapToTarget,
				true,
				ENCPoolMethod::None
			);

			if (NiagaraComp)
			{
				// Set trail parameters
				NiagaraComp->SetFloatParameter(FName("Width"), FinalWidth);
				NiagaraComp->SetColorParameter(FName("Color"), FinalColor);

				// Set trail end socket if parameter exists
				if (!EndSocket.IsNone())
				{
					NiagaraComp->SetVectorParameter(FName("TrailEnd"), MeshComp->GetSocketLocation(EndSocket));
				}

				SpawnedTrailComponent = NiagaraComp;
			}
			return;
		}
	}

	// Fallback to Cascade particles
	if (!EffectData->ParticleSystem.IsNull())
	{
		UParticleSystem* ParticleSystem = EffectData->ParticleSystem.LoadSynchronous();
		if (ParticleSystem)
		{
			UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAttached(
				ParticleSystem,
				MeshComp,
				StartSocket,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				FVector::OneVector,
				EAttachLocation::SnapToTarget,
				true
			);

			if (ParticleComp)
			{
				// Set trail parameters (if supported by particle system)
				ParticleComp->SetFloatParameter(FName("Width"), FinalWidth);
				ParticleComp->SetColorParameter(FName("Color"), FinalColor);

				SpawnedTrailComponent = ParticleComp;
			}
		}
	}
}

void UAnimNotifyState_PlayTaggedWeaponTrail::PlayTrailSound(
	UWorld* World,
	USkeletalMeshComponent* MeshComp,
	const FHarmoniaAnimationEffectData* EffectData
) const
{
	if (!World || !MeshComp || !EffectData)
	{
		return;
	}

	if (EffectData->Sound.IsNull())
	{
		return;
	}

	USoundBase* Sound = EffectData->Sound.LoadSynchronous();
	if (!Sound)
	{
		return;
	}

	// Calculate final volume
	float FinalVolume = EffectData->VolumeMultiplier * VolumeMultiplier;

	// Get start socket
	FName StartSocket = bOverrideTrailSockets ? TrailStartSocketOverride : EffectData->TrailStartSocketName;

	if (EffectData->bAttachSound && !StartSocket.IsNone())
	{
		UGameplayStatics::SpawnSoundAttached(
			Sound,
			MeshComp,
			StartSocket,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false,
			FinalVolume,
			EffectData->PitchMultiplier
		);
	}
	else
	{
		FVector Location = !StartSocket.IsNone() ?
			MeshComp->GetSocketLocation(StartSocket) :
			MeshComp->GetComponentLocation();

		UGameplayStatics::PlaySoundAtLocation(
			World,
			Sound,
			Location,
			FRotator::ZeroRotator,
			FinalVolume,
			EffectData->PitchMultiplier
		);
	}
}

void UAnimNotifyState_PlayTaggedWeaponTrail::DestroyTrail()
{
	if (!SpawnedTrailComponent)
	{
		return;
	}

	// Handle Niagara component
	if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(SpawnedTrailComponent))
	{
		NiagaraComp->Deactivate();
		NiagaraComp->DestroyComponent();
	}
	// Handle Particle System component
	else if (UParticleSystemComponent* ParticleComp = Cast<UParticleSystemComponent>(SpawnedTrailComponent))
	{
		ParticleComp->Deactivate();
		ParticleComp->DestroyComponent();
	}

	SpawnedTrailComponent = nullptr;
}
