// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_PlayHitEffect.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UAnimNotify_PlayHitEffect::UAnimNotify_PlayHitEffect()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 128, 0, 255); // Orange for hit effects
#endif
}

void UAnimNotify_PlayHitEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	// Get spawn transform
	FVector SpawnLocation;
	FRotator SpawnRotation;
	GetSpawnTransform(MeshComp, SpawnLocation, SpawnRotation);

	// Spawn VFX
	if (HitNiagaraSystem)
	{
		// Niagara takes priority
		if (bAttachToSocket && !AttachSocketName.IsNone())
		{
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				HitNiagaraSystem,
				MeshComp,
				AttachSocketName,
				LocationOffset,
				RotationOffset,
				EffectScale,
				EAttachLocation::SnapToTarget,
				true,
				ENCPoolMethod::None
			);
		}
		else
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				HitNiagaraSystem,
				SpawnLocation,
				SpawnRotation,
				EffectScale
			);
		}
	}
	else if (HitParticleSystem)
	{
		// Fallback to cascade particles
		if (bAttachToSocket && !AttachSocketName.IsNone())
		{
			UGameplayStatics::SpawnEmitterAttached(
				HitParticleSystem,
				MeshComp,
				AttachSocketName,
				LocationOffset,
				RotationOffset,
				EffectScale,
				EAttachLocation::SnapToTarget,
				true
			);
		}
		else
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				HitParticleSystem,
				SpawnLocation,
				SpawnRotation,
				EffectScale,
				true
			);
		}
	}

	// Play SFX
	if (HitSound)
	{
		if (bAttachSound && !AttachSocketName.IsNone())
		{
			UGameplayStatics::SpawnSoundAttached(
				HitSound,
				MeshComp,
				AttachSocketName,
				LocationOffset,
				RotationOffset,
				EAttachLocation::SnapToTarget,
				false,
				VolumeMultiplier,
				PitchMultiplier
			);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(
				World,
				HitSound,
				SpawnLocation,
				SpawnRotation,
				VolumeMultiplier,
				PitchMultiplier
			);
		}
	}
}

FString UAnimNotify_PlayHitEffect::GetNotifyName_Implementation() const
{
	FString Name = TEXT("Play Hit Effect");

	if (HitNiagaraSystem)
	{
		Name += FString::Printf(TEXT(" (%s)"), *HitNiagaraSystem->GetName());
	}
	else if (HitParticleSystem)
	{
		Name += FString::Printf(TEXT(" (%s)"), *HitParticleSystem->GetName());
	}

	return Name;
}

void UAnimNotify_PlayHitEffect::GetSpawnTransform(USkeletalMeshComponent* MeshComp, FVector& OutLocation, FRotator& OutRotation) const
{
	if (!MeshComp)
	{
		OutLocation = FVector::ZeroVector;
		OutRotation = FRotator::ZeroRotator;
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	// Try to get hit location if requested
	if (bUseHitLocation && Owner)
	{
		FVector HitLocation;
		FVector HitNormal;
		if (GetLastHitResult(Owner, HitLocation, HitNormal))
		{
			OutLocation = HitLocation + LocationOffset;

			if (bAlignToHitNormal)
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
	if (!AttachSocketName.IsNone() && MeshComp->DoesSocketExist(AttachSocketName))
	{
		OutLocation = MeshComp->GetSocketLocation(AttachSocketName) + LocationOffset;
		OutRotation = MeshComp->GetSocketRotation(AttachSocketName) + RotationOffset;
	}
	else
	{
		OutLocation = MeshComp->GetComponentLocation() + LocationOffset;
		OutRotation = MeshComp->GetComponentRotation() + RotationOffset;
	}
}

bool UAnimNotify_PlayHitEffect::GetLastHitResult(AActor* Owner, FVector& OutLocation, FVector& OutNormal) const
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
