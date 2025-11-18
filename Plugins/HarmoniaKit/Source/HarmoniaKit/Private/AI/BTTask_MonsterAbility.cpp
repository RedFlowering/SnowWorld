// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_MonsterAbility.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"

UBTTask_MonsterAbility::UBTTask_MonsterAbility()
{
	NodeName = "Monster Ability (GAS)";
	bNotifyTick = false;

	// Set up blackboard key filter
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterAbility, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MonsterAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Check if task is enabled
	if (!bTaskEnabled)
	{
		return EBTNodeResult::Succeeded;
	}

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	// Check conditions
	if (!CheckConditions(Monster, OwnerComp))
	{
		return EBTNodeResult::Failed;
	}

	// Find ability to activate
	TSubclassOf<UGameplayAbility> AbilityToActivate = FindAbilityToActivate(Monster);
	if (!AbilityToActivate)
	{
		return EBTNodeResult::Failed;
	}

	// Get Ability System Component
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	// Check if ability is already active (if checking cooldown)
	if (bCheckCooldown)
	{
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityToActivate);
		if (Spec && Spec->IsActive())
		{
			return EBTNodeResult::Failed;
		}
	}

	// Try to activate ability
	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(AbilityToActivate);
	if (!AbilitySpec)
	{
		// Ability not granted, fail
		return EBTNodeResult::Failed;
	}

	bool bSuccess = ASC->TryActivateAbility(AbilitySpec->Handle);

	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

TSubclassOf<UGameplayAbility> UBTTask_MonsterAbility::FindAbilityToActivate(AHarmoniaMonsterBase* Monster) const
{
	if (!Monster)
	{
		return nullptr;
	}

	// Priority 1: Specific ability class
	if (AbilityClass)
	{
		return AbilityClass;
	}

	// Priority 2: Search by tags
	if (!AbilityTags.IsEmpty())
	{
		UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
		if (!ASC)
		{
			return nullptr;
		}

		TArray<FGameplayAbilitySpec*> MatchingSpecs;

		// Find all abilities with matching tags
		for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.Ability)
			{
				// Get ability tags from the ability class
				const FGameplayTagContainer& AbilityTagContainer = Spec.Ability->GetAssetTags();

				if (AbilityTagContainer.HasAny(AbilityTags))
				{
					MatchingSpecs.Add(&Spec);
				}
			}
		}

		if (MatchingSpecs.Num() == 0)
		{
			return nullptr;
		}

		// Select ability
		if (bRandomSelection && MatchingSpecs.Num() > 1)
		{
			int32 RandomIndex = FMath::RandRange(0, MatchingSpecs.Num() - 1);
			return MatchingSpecs[RandomIndex]->Ability->GetClass();
		}
		else
		{
			return MatchingSpecs[0]->Ability->GetClass();
		}
	}

	return nullptr;
}

bool UBTTask_MonsterAbility::CheckConditions(AHarmoniaMonsterBase* Monster, UBehaviorTreeComponent& OwnerComp) const
{
	if (!Monster)
	{
		return false;
	}

	// Check health percentage
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (ASC)
	{
		float CurrentHealth = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetHealthAttribute());
		float MaxHealth = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetMaxHealthAttribute());

		if (MaxHealth > 0.0f)
		{
			float HealthPercent = CurrentHealth / MaxHealth;

			if (HealthPercent < MinHealthPercent || HealthPercent > MaxHealthPercent)
			{
				return false;
			}
		}
	}

	// Check target requirement
	if (bRequiresTarget)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (!BlackboardComp)
		{
			return false;
		}

		AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
		if (!Target)
		{
			return false;
		}

		// Check target distance
		if (!CheckTargetDistance(Monster, Target))
		{
			return false;
		}
	}

	return true;
}

bool UBTTask_MonsterAbility::CheckTargetDistance(AHarmoniaMonsterBase* Monster, AActor* Target) const
{
	if (!Monster || !Target)
	{
		return false;
	}

	float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());

	return Distance >= MinDistanceToTarget && Distance <= MaxDistanceToTarget;
}

FString UBTTask_MonsterAbility::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();

	if (!bTaskEnabled)
	{
		Description += TEXT("\n[DISABLED]");
		return Description;
	}

	// Show ability info
	if (AbilityClass)
	{
		Description += FString::Printf(TEXT("\nAbility: %s"), *AbilityClass->GetName());
	}
	else if (!AbilityTags.IsEmpty())
	{
		Description += FString::Printf(TEXT("\nTags: %s"), *AbilityTags.ToStringSimple());
		if (bRandomSelection)
		{
			Description += TEXT(" (Random)");
		}
	}

	// Show conditions
	if (MinHealthPercent > 0.0f || MaxHealthPercent < 1.0f)
	{
		Description += FString::Printf(TEXT("\nHP: %.0f%% - %.0f%%"),
			MinHealthPercent * 100.0f,
			MaxHealthPercent * 100.0f);
	}

	if (MinDistanceToTarget > 0.0f || MaxDistanceToTarget < 99999.0f)
	{
		Description += FString::Printf(TEXT("\nDist: %.0f - %.0f"),
			MinDistanceToTarget,
			MaxDistanceToTarget);
	}

	return Description;
}
