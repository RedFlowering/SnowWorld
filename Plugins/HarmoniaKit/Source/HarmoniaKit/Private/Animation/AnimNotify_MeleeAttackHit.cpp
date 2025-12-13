// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_MeleeAttackHit.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseComponent.h"
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

	// Find combat component (attack control moved from SenseComponent)
	UHarmoniaMeleeCombatComponent* MeleeComponent = FindMeleeCombatComponent(Owner);
	if (!MeleeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_MeleeAttackHit: No HarmoniaMeleeCombatComponent found on %s"), *Owner->GetName());
		return;
	}

	// Start attack via combat component
	// Attack data is managed by MeleeCombatComponent's combo system
	MeleeComponent->StartAttack(MeleeComponent->GetCurrentAttackType());
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

UHarmoniaSenseComponent* UAnimNotify_MeleeAttackHit::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// Legacy - returns HarmoniaSenseComponent for compatibility
	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}
