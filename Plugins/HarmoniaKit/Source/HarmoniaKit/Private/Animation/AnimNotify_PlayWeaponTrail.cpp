// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_PlayWeaponTrail.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_PlayWeaponTrail::UAnimNotify_PlayWeaponTrail()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(128, 255, 128, 255); // Green for weapon trails
#endif
}

void UAnimNotify_PlayWeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn trail VFX
	if (TrailNiagaraSystem)
	{
		// Use Niagara
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagaraSystem,
			MeshComp,
			StartSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(1.0f),
			EAttachLocation::SnapToTarget,
			false, // Don't auto-destroy, we'll manage it
			ENCPoolMethod::None
		);

		if (NiagaraComp)
		{
			// Set trail parameters
			NiagaraComp->SetFloatParameter(FName("WidthMultiplier"), WidthMultiplier);
			NiagaraComp->SetColorParameter(FName("TrailColor"), TrailColor);

			// Set end socket if it exists
			if (!EndSocketName.IsNone() && MeshComp->DoesSocketExist(EndSocketName))
			{
				NiagaraComp->SetVectorParameter(FName("EndLocation"), MeshComp->GetSocketLocation(EndSocketName));
			}

			SpawnedTrailComponent = NiagaraComp;
		}
	}
	else if (TrailParticleSystem)
	{
		// Fallback to Cascade
		UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAttached(
			TrailParticleSystem,
			MeshComp,
			StartSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(WidthMultiplier),
			EAttachLocation::SnapToTarget,
			false // Don't auto-destroy
		);

		if (ParticleComp)
		{
			SpawnedTrailComponent = ParticleComp;
		}
	}

	// Play swoosh sound
	if (SwooshSound)
	{
		UGameplayStatics::SpawnSoundAttached(
			SwooshSound,
			MeshComp,
			StartSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false,
			VolumeMultiplier,
			PitchMultiplier
		);
	}
}

void UAnimNotify_PlayWeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Deactivate and destroy trail
	if (SpawnedTrailComponent)
	{
		if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(SpawnedTrailComponent))
		{
			NiagaraComp->Deactivate();
			NiagaraComp->DestroyComponent();
		}
		else if (UParticleSystemComponent* ParticleComp = Cast<UParticleSystemComponent>(SpawnedTrailComponent))
		{
			ParticleComp->Deactivate();
			ParticleComp->DestroyComponent();
		}

		SpawnedTrailComponent = nullptr;
	}
}

FString UAnimNotify_PlayWeaponTrail::GetNotifyName_Implementation() const
{
	FString Name = TEXT("Play Weapon Trail");

	if (TrailNiagaraSystem)
	{
		Name += FString::Printf(TEXT(" (%s)"), *TrailNiagaraSystem->GetName());
	}
	else if (TrailParticleSystem)
	{
		Name += FString::Printf(TEXT(" (%s)"), *TrailParticleSystem->GetName());
	}

	return Name;
}
