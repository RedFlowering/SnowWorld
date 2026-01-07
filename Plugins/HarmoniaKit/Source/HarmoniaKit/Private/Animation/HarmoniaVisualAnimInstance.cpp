// Copyright 2025 Snow Game Studio.

#include "Animation/HarmoniaVisualAnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaVisualAnimInstance)

UHarmoniaVisualAnimInstance::UHarmoniaVisualAnimInstance()
{
}

void UHarmoniaVisualAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		return;
	}

	// Try to get parent character (this mesh might be attached to a character or be a child actor)
	ParentCharacter = Cast<ACharacter>(OwningActor->GetParentActor());
	
	// If no parent actor, try the owning actor itself
	if (!ParentCharacter.IsValid())
	{
		ParentCharacter = Cast<ACharacter>(OwningActor);
	}

	// Get parent's main mesh AnimInstance
	if (ParentCharacter.IsValid() && ParentCharacter->GetMesh())
	{
		ParentAnimInstance = ParentCharacter->GetMesh()->GetAnimInstance();
	}

#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (World && !World->IsGameWorld())
	{
		// Editor preview - use defaults if needed
	}
#endif
}

void UHarmoniaVisualAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UHarmoniaVisualAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Ensure parent references are valid
	if (!ParentCharacter.IsValid())
	{
		return;
	}

	// Sync dodge state from parent
	SyncDodgeStateFromParent();

	// Additional visual mesh specific updates can be added here:
	// - IK adjustments
	// - Equipment/cosmetic animations
	// - Layered animation blending
}

void UHarmoniaVisualAnimInstance::SyncDodgeStateFromParent()
{
	if (!ParentAnimInstance.IsValid())
	{
		return;
	}
}
