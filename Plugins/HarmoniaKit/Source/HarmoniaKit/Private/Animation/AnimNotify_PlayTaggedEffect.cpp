// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_PlayTaggedEffect.h"
#include "System/HarmoniaEffectCacheSubsystem.h"
#include "Definitions/HarmoniaAnimationEffectDefinitions.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UAnimNotify_PlayTaggedEffect::UAnimNotify_PlayTaggedEffect()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(100, 200, 255, 255); // Blue for tagged effects
#endif
}

void UAnimNotify_PlayTaggedEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

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
		UE_LOG(LogAnimation, Error, TEXT("HarmoniaEffectCacheSubsystem not found! Cannot play tagged effect: %s"),
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
		UE_LOG(LogAnimation, Warning, TEXT("Effect data not found for tag: %s"), *EffectTag.ToString());
		return;
	}

	// Get spawn transform
	FVector SpawnLocation;
	FRotator SpawnRotation;
	GetSpawnTransform(MeshComp, EffectData, SpawnLocation, SpawnRotation);

	// Spawn visual effect
	SpawnVisualEffect(World, MeshComp, EffectData, SpawnLocation, SpawnRotation);

	// Play sound effect
	PlaySoundEffect(World, MeshComp, EffectData, SpawnLocation, SpawnRotation);
}

FString UAnimNotify_PlayTaggedEffect::GetNotifyName_Implementation() const
{
	if (EffectTag.IsValid())
	{
		return FString::Printf(TEXT("Play Effect: %s"), *EffectTag.ToString());
	}
	return TEXT("Play Tagged Effect");
}

#if WITH_EDITOR
void UAnimNotify_PlayTaggedEffect::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Could add editor preview or validation here
}
#endif

void UAnimNotify_PlayTaggedEffect::GetSpawnTransform(
	USkeletalMeshComponent* MeshComp,
	const FHarmoniaAnimationEffectData* EffectData,
	FVector& OutLocation,
	FRotator& OutRotation
) const
{
	if (!MeshComp || !EffectData)
	{
		OutLocation = FVector::ZeroVector;
		OutRotation = FRotator::ZeroRotator;
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	// Get offset and rotation (with overrides)
	FVector LocationOffset = bOverrideLocationOffset ? LocationOffsetOverride : EffectData->LocationOffset;
	FRotator RotationOffset = bOverrideRotationOffset ? RotationOffsetOverride : EffectData->RotationOffset;

	// Try to get hit location if requested
	if (EffectData->bUseHitLocation && Owner)
	{
		FVector HitLocation;
		FVector HitNormal;
		if (GetLastHitResult(Owner, HitLocation, HitNormal))
		{
			OutLocation = HitLocation + LocationOffset;

			if (EffectData->bAlignToHitNormal)
			{
				OutRotation = HitNormal.Rotation() + RotationOffset;
			}
			else
			{
				OutRotation = RotationOffset;
			}
			return;
		}
	}

	// Fallback to socket or component location
	FName SocketName = OverrideSocketName.IsNone() ? EffectData->AttachSocketName : OverrideSocketName;

	if (!SocketName.IsNone() && MeshComp->DoesSocketExist(SocketName))
	{
		OutLocation = MeshComp->GetSocketLocation(SocketName) + LocationOffset;
		OutRotation = MeshComp->GetSocketRotation(SocketName) + RotationOffset;
	}
	else
	{
		OutLocation = MeshComp->GetComponentLocation() + LocationOffset;
		OutRotation = MeshComp->GetComponentRotation() + RotationOffset;
	}
}

bool UAnimNotify_PlayTaggedEffect::GetLastHitResult(AActor* Owner, FVector& OutLocation, FVector& OutNormal) const
{
	if (!Owner)
	{
		return false;
	}

	// Find attack component
	UHarmoniaSenseAttackComponent* AttackComponent = Owner->FindComponentByClass<UHarmoniaSenseAttackComponent>();
	if (!AttackComponent)
	{
		return false;
	}

	// Get last hit targets
	TArray<FHarmoniaAttackHitResult> HitTargets = AttackComponent->GetHitTargets();
	if (HitTargets.Num() > 0)
	{
		// Use most recent hit
		const FHarmoniaAttackHitResult& LastHit = HitTargets.Last();
		OutLocation = LastHit.HitLocation;
		OutNormal = LastHit.HitNormal;
		return true;
	}

	return false;
}

void UAnimNotify_PlayTaggedEffect::SpawnVisualEffect(
	UWorld* World,
	USkeletalMeshComponent* MeshComp,
	const FHarmoniaAnimationEffectData* EffectData,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation
) const
{
	if (!World || !MeshComp || !EffectData)
	{
		return;
	}

	// Calculate final scale
	FVector FinalScale = bOverrideScale ? ScaleOverride : EffectData->EffectScale;
	FinalScale *= ScaleMultiplier;

	// Get socket name (with override)
	FName SocketName = OverrideSocketName.IsNone() ? EffectData->AttachSocketName : OverrideSocketName;
	FVector LocationOffset = bOverrideLocationOffset ? LocationOffsetOverride : EffectData->LocationOffset;
	FRotator RotationOffset = bOverrideRotationOffset ? RotationOffsetOverride : EffectData->RotationOffset;

	// Spawn Niagara system (preferred)
	if (!EffectData->NiagaraSystem.IsNull())
	{
		UNiagaraSystem* NiagaraSystem = EffectData->NiagaraSystem.LoadSynchronous();
		if (NiagaraSystem)
		{
			if (EffectData->bAttachToSocket && !SocketName.IsNone())
			{
				UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
					NiagaraSystem,
					MeshComp,
					SocketName,
					LocationOffset,
					RotationOffset,
					FinalScale,
					EAttachLocation::SnapToTarget,
					true,
					ENCPoolMethod::None
				);

				// Set color if supported
				if (NiagaraComp)
				{
					NiagaraComp->SetColorParameter(FName("Color"), EffectData->EffectColor);
				}
			}
			else
			{
				UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					World,
					NiagaraSystem,
					SpawnLocation,
					SpawnRotation,
					FinalScale
				);

				// Set color if supported
				if (NiagaraComp)
				{
					NiagaraComp->SetColorParameter(FName("Color"), EffectData->EffectColor);
				}
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
			if (EffectData->bAttachToSocket && !SocketName.IsNone())
			{
				UGameplayStatics::SpawnEmitterAttached(
					ParticleSystem,
					MeshComp,
					SocketName,
					LocationOffset,
					RotationOffset,
					FinalScale,
					EAttachLocation::SnapToTarget,
					true
				);
			}
			else
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					World,
					ParticleSystem,
					SpawnLocation,
					SpawnRotation,
					FinalScale,
					true
				);
			}
		}
	}
}

void UAnimNotify_PlayTaggedEffect::PlaySoundEffect(
	UWorld* World,
	USkeletalMeshComponent* MeshComp,
	const FHarmoniaAnimationEffectData* EffectData,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation
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

	// Get socket name (with override)
	FName SocketName = OverrideSocketName.IsNone() ? EffectData->AttachSocketName : OverrideSocketName;
	FVector LocationOffset = bOverrideLocationOffset ? LocationOffsetOverride : EffectData->LocationOffset;
	FRotator RotationOffset = bOverrideRotationOffset ? RotationOffsetOverride : EffectData->RotationOffset;

	if (EffectData->bAttachSound && !SocketName.IsNone())
	{
		UGameplayStatics::SpawnSoundAttached(
			Sound,
			MeshComp,
			SocketName,
			LocationOffset,
			RotationOffset,
			EAttachLocation::SnapToTarget,
			false,
			FinalVolume,
			EffectData->PitchMultiplier
		);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			Sound,
			SpawnLocation,
			SpawnRotation,
			FinalVolume,
			EffectData->PitchMultiplier
		);
	}
}
