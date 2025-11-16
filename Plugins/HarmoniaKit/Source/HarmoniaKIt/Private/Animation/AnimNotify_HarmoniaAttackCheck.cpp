// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotify_HarmoniaAttackCheck.h"
#include "Components/HarmoniaSenseAttackComponent.h"
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

	// Find attack component
	UHarmoniaSenseAttackComponent* AttackComponent = FindAttackComponent(Owner);
	if (!AttackComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_HarmoniaAttackCheck: No HarmoniaSenseAttackComponent found on %s"), *Owner->GetName());
		return;
	}

	// Stop previous attack if requested
	if (bStopPreviousAttack && AttackComponent->IsAttacking())
	{
		AttackComponent->StopAttack();
	}

	// Start attack with appropriate data
	if (bUseCustomAttackData)
	{
		AttackComponent->StartAttack(CustomAttackData);
	}
	else
	{
		AttackComponent->StartAttackDefault();
	}
}

FString UAnimNotify_HarmoniaAttackCheck::GetNotifyName_Implementation() const
{
	if (AttackComponentName.IsNone())
	{
		return TEXT("Harmonia Attack Check");
	}

	return FString::Printf(TEXT("Harmonia Attack Check (%s)"), *AttackComponentName.ToString());
}

UHarmoniaSenseAttackComponent* UAnimNotify_HarmoniaAttackCheck::FindAttackComponent(AActor* Owner) const
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

		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_HarmoniaAttackCheck: Could not find component named '%s' on %s"), *AttackComponentName.ToString(), *Owner->GetName());
		return nullptr;
	}

	// Otherwise, return first found component
	return Owner->FindComponentByClass<UHarmoniaSenseAttackComponent>();
}
