// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_HarmoniaAttackCheck.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_HarmoniaAttackCheck::UAnimNotify_HarmoniaAttackCheck()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 100, 100, 255);
#endif
}

void UAnimNotify_HarmoniaAttackCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	// Find combat component (attack control moved from SenseComponent to MeleeCombatComponent)
	UHarmoniaMeleeCombatComponent* CombatComp = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_HarmoniaAttackCheck: No HarmoniaMeleeCombatComponent found on %s"), *Owner->GetName());
		return;
	}

	// Stop previous attack if requested
	if (bStopPreviousAttack && CombatComp->IsAttacking())
	{
		CombatComp->EndAttack();
	}

	// Start attack via combat component
	CombatComp->StartAttack(CombatComp->GetCurrentAttackType());
}

FString UAnimNotify_HarmoniaAttackCheck::GetNotifyName_Implementation() const
{
	if (AttackComponentName.IsNone())
	{
		return TEXT("Harmonia Attack Check");
	}

	return FString::Printf(TEXT("Harmonia Attack Check (%s)"), *AttackComponentName.ToString());
}

UHarmoniaSenseComponent* UAnimNotify_HarmoniaAttackCheck::FindAttackComponent(AActor* Owner) const
{
	if (!Owner)
	{
		return nullptr;
	}

	// Legacy - returns HarmoniaSenseComponent for compatibility
	return Owner->FindComponentByClass<UHarmoniaSenseComponent>();
}
