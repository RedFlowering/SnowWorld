// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_MeleeAttackHit.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "GameFramework/Actor.h"

UAnimNotify_MeleeAttackHit::UAnimNotify_MeleeAttackHit()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 200, 100, 255); // Orange for melee hit
#endif
}

void UAnimNotify_MeleeAttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	// Find components
	UHarmoniaMeleeCombatComponent* MeleeComponent = FindMeleeCombatComponent(Owner);
	UHarmoniaSenseAttackComponent* AttackComponent = FindAttackComponent(Owner);

	if (!AttackComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_MeleeAttackHit: No HarmoniaSenseAttackComponent found on %s"), *Owner->GetName());
		return;
	}

	// Get attack data from MeleeCombatComponent (includes combo step data)
	FHarmoniaAttackData AttackData;
	
	if (MeleeComponent && MeleeComponent->GetCurrentComboAttackData(AttackData))
	{
		// Use combo attack data from MeleeCombatComponent
		AttackComponent->RequestStartAttack(AttackData);
	}
	else
	{
		// Fallback: Use component's default attack data
		AttackComponent->RequestStartAttackDefault();
	}
}

FString UAnimNotify_MeleeAttackHit::GetNotifyName_Implementation() const
{
	return TEXT("Melee Attack Hit");
}

UHarmoniaMeleeCombatComponent* UAnimNotify_MeleeAttackHit::FindMeleeCombatComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
}

UHarmoniaSenseAttackComponent* UAnimNotify_MeleeAttackHit::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// If component name is specified, find by name
	if (!AttackComponentName.IsNone())
	{
		TArray<UHarmoniaSenseAttackComponent*> AttackComponents;
		Owner->GetComponents<UHarmoniaSenseAttackComponent>(AttackComponents);

		for (UHarmoniaSenseAttackComponent* Component : AttackComponents)
		{
			if (Component && Component->GetFName() == AttackComponentName)
			{
				return Component;
			}
		}

		return nullptr;
	}

	// Otherwise, return first found component
	return Owner->FindComponentByClass<UHarmoniaSenseAttackComponent>();
}

