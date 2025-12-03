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

	// Get attack data from current weapon if melee component exists
	FHarmoniaAttackData AttackData;
	bool bHasWeaponData = false;

	if (MeleeComponent)
	{
		FHarmoniaMeleeWeaponData WeaponData;
		if (MeleeComponent->GetCurrentWeaponData(WeaponData))
		{
			// Use weapon's default attack data
			AttackData.TraceConfig = WeaponData.DefaultTraceConfig;
			AttackData.DamageConfig.BaseDamage = 10.0f; // Base damage, will be modified by weapon stats
			AttackData.DamageConfig.DamageMultiplier = WeaponData.BaseDamageMultiplier * DamageMultiplier;

			// Set critical hit chance if this is a critical hit point
			if (bIsCriticalHitPoint)
			{
				AttackData.DamageConfig.bCanCritical = true;
				AttackData.DamageConfig.CriticalChance = 1.0f; // Always crit at this point
				AttackData.DamageConfig.CriticalMultiplier = CriticalDamageMultiplier;
			}

			bHasWeaponData = true;
		}
	}

	// If no weapon data, use component's default
	if (!bHasWeaponData)
	{
		AttackData = AttackComponent->AttackData;
		AttackData.DamageConfig.DamageMultiplier *= DamageMultiplier;

		if (bIsCriticalHitPoint)
		{
			AttackData.DamageConfig.bCanCritical = true;
			AttackData.DamageConfig.CriticalChance = 1.0f;
			AttackData.DamageConfig.CriticalMultiplier = CriticalDamageMultiplier;
		}
	}

	// Single-shot detection (not continuous)
	AttackData.TraceConfig.bContinuousDetection = false;

	// Trigger attack
	AttackComponent->RequestStartAttack(AttackData);
}

FString UAnimNotify_MeleeAttackHit::GetNotifyName_Implementation() const
{
	FString Name = TEXT("Melee Attack Hit");

	if (DamageMultiplier != 1.0f)
	{
		Name += FString::Printf(TEXT(" (x%.1f)"), DamageMultiplier);
	}

	if (bIsCriticalHitPoint)
	{
		Name += TEXT(" [CRIT]");
	}

	return Name;
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
