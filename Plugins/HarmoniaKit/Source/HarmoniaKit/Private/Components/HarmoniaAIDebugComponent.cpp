// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaAIDebugComponent.h"
#include "HarmoniaLogCategories.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

UHarmoniaAIDebugComponent::UHarmoniaAIDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	DebugMode = EHarmoniaDebugMode::Off;
	bShowOnScreen = true;
	bShowInWorld = true;
	DebugColor = FColor::Yellow;
	TextScale = 1.0f;
	bLogDecisions = false;
}

void UHarmoniaAIDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());

	if (OwnerMonster)
	{
		LearningAI = OwnerMonster->FindComponentByClass<UHarmoniaLearningAIComponent>();
		TerritoryDispute = OwnerMonster->FindComponentByClass<UHarmoniaTerritoryDisputeComponent>();
	}
}

void UHarmoniaAIDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DebugMode == EHarmoniaDebugMode::Off || !OwnerMonster)
	{
		return;
	}

	DrawDebug();
}

void UHarmoniaAIDebugComponent::DrawDebug()
{
	if (bShowInWorld)
	{
		switch (DebugMode)
		{
		case EHarmoniaDebugMode::Basic:
		case EHarmoniaDebugMode::All:
			DrawBasicInfo();
			break;

		case EHarmoniaDebugMode::Learning:
			DrawLearningData();
			break;

		case EHarmoniaDebugMode::CombatPower:
			DrawCombatPower();
			break;

		default:
			break;
		}
	}

	if (bShowOnScreen && GEngine)
	{
		FString DebugText = GetDebugString();
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, DebugColor, DebugText);
	}
}

void UHarmoniaAIDebugComponent::DrawBasicInfo()
{
	if (!OwnerMonster)
	{
		return;
	}

	FVector Location = OwnerMonster->GetActorLocation() + FVector(0, 0, 150);
	FString Info = FString::Printf(TEXT("%s\nState: %s\nLevel: %d"),
		*OwnerMonster->GetName(),
		*UEnum::GetValueAsString(OwnerMonster->GetMonsterState()),
		OwnerMonster->GetMonsterLevel());

	DrawDebugString(GetWorld(), Location, Info, nullptr, DebugColor, 0.0f, true, TextScale);
}

void UHarmoniaAIDebugComponent::DrawLearningData()
{
	if (!OwnerMonster || !LearningAI)
	{
		return;
	}

	FVector Location = OwnerMonster->GetActorLocation() + FVector(0, 0, 150);
	FString Info = GetLearningAIInfo();

	DrawDebugString(GetWorld(), Location, Info, nullptr, FColor::Cyan, 0.0f, true, TextScale);
}

void UHarmoniaAIDebugComponent::DrawCombatPower()
{
	if (!OwnerMonster)
	{
		return;
	}

	FVector Location = OwnerMonster->GetActorLocation() + FVector(0, 0, 150);
	FString Info = GetCombatPowerBreakdown();

	DrawDebugString(GetWorld(), Location, Info, nullptr, FColor::Orange, 0.0f, true, TextScale);
}

FString UHarmoniaAIDebugComponent::GetDebugString() const
{
	if (!OwnerMonster)
	{
		return TEXT("No Owner");
	}

	FString DebugText;

	switch (DebugMode)
	{
	case EHarmoniaDebugMode::Basic:
		DebugText = FString::Printf(TEXT("Monster: %s | State: %s | Level: %d"),
			*OwnerMonster->GetName(),
			*UEnum::GetValueAsString(OwnerMonster->GetMonsterState()),
			OwnerMonster->GetMonsterLevel());
		break;

	case EHarmoniaDebugMode::Detailed:
		DebugText = FString::Printf(TEXT("%s\n%s\n%s"),
			*GetDebugString(),
			*GetLearningAIInfo(),
			*GetTerritoryDisputeInfo());
		break;

	case EHarmoniaDebugMode::All:
		DebugText = FString::Printf(TEXT("%s\n%s\n%s\n%s"),
			*GetDebugString(),
			*GetLearningAIInfo(),
			*GetTerritoryDisputeInfo(),
			*GetCombatPowerBreakdown());
		break;

	default:
		break;
	}

	return DebugText;
}

FString UHarmoniaAIDebugComponent::GetLearningAIInfo() const
{
	if (!LearningAI)
	{
		return TEXT("No Learning AI");
	}

	return FString::Printf(TEXT("Difficulty: %s | Patterns: %d"),
		*UEnum::GetValueAsString(LearningAI->GetCurrentDifficulty()),
		0); // Would need to expose pattern count
}

FString UHarmoniaAIDebugComponent::GetCombatPowerBreakdown() const
{
	if (!OwnerMonster)
	{
		return TEXT("");
	}

	UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent();
	if (!ASC)
	{
		return TEXT("No ASC");
	}

	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (!AttributeSet)
	{
		return TEXT("No Attributes");
	}

	return FString::Printf(TEXT("HP: %.0f/%.0f | ATK: %.0f | DEF: %.0f"),
		AttributeSet->GetHealth(),
		AttributeSet->GetMaxHealth(),
		AttributeSet->GetAttackPower(),
		AttributeSet->GetDefense());
}

FString UHarmoniaAIDebugComponent::GetTerritoryDisputeInfo() const
{
	if (!TerritoryDispute)
	{
		return TEXT("No Territory Dispute");
	}

	return FString::Printf(TEXT("Dispute State: %s"),
		*UEnum::GetValueAsString(TerritoryDispute->GetDisputeState()));
}

void UHarmoniaAIDebugComponent::LogDecision(const FString& Decision, const FString& Reason)
{
	FString LogEntry = FString::Printf(TEXT("%s: %s"), *Decision, *Reason);

	RecentDecisions.Add(LogEntry);
	if (RecentDecisions.Num() > MaxLogEntries)
	{
		RecentDecisions.RemoveAt(0);
	}

	if (bLogDecisions)
	{
		UE_LOG(LogHarmoniaAI, Log, TEXT("[AI Decision] %s - %s"), *Decision, *Reason);
	}
}
