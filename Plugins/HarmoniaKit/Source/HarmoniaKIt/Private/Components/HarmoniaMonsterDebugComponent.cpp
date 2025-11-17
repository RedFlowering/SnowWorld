// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMonsterDebugComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "Components/HarmoniaThreatComponent.h"
#include "Components/HarmoniaSenseInteractionComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

UHarmoniaMonsterDebugComponent::UHarmoniaMonsterDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
}

void UHarmoniaMonsterDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bShowDebugInfo)
	{
		return;
	}

	AHarmoniaMonsterBase* Monster = GetMonster();
	if (!Monster)
	{
		return;
	}

	// Check if we're close enough to show debug info
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC && PC->GetPawn())
		{
			float Distance = FVector::Dist(Monster->GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Distance > DebugDrawDistance)
			{
				return; // Too far, don't draw
			}
		}
	}

	FVector DrawLocation = Monster->GetActorLocation() + FVector(0, 0, 100);

	// Draw different debug categories
	if (bShowAIState)
	{
		DrawAIState(Monster, DrawLocation);
		DrawLocation.Z += 30.0f * TextScale;
	}

	if (bShowFaction)
	{
		DrawFaction(Monster, DrawLocation);
		DrawLocation.Z += 20.0f * TextScale;
	}

	if (bShowHealthBar)
	{
		DrawHealthBar(Monster, DrawLocation);
		DrawLocation.Z += 30.0f * TextScale;
	}

	if (bShowTarget)
	{
		DrawTarget(Monster);
	}

	if (bShowThreatTable)
	{
		DrawThreatTable(Monster, DrawLocation);
	}

	if (bShowSensedTargets)
	{
		DrawSensedTargets(Monster);
	}

	if (bShowPatrolInfo)
	{
		DrawPatrolInfo(Monster);
	}
}

AHarmoniaMonsterBase* UHarmoniaMonsterDebugComponent::GetMonster()
{
	if (!CachedMonster)
	{
		CachedMonster = Cast<AHarmoniaMonsterBase>(GetOwner());
	}
	return CachedMonster;
}

void UHarmoniaMonsterDebugComponent::DrawAIState(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation)
{
	if (!Monster)
	{
		return;
	}

	// Get state
	EHarmoniaMonsterState State = IHarmoniaMonsterInterface::Execute_GetMonsterState(Monster);
	FString StateString = TEXT("Unknown");

	switch (State)
	{
	case EHarmoniaMonsterState::Idle:
		StateString = TEXT("Idle");
		break;
	case EHarmoniaMonsterState::Patrol:
		StateString = TEXT("Patrol");
		break;
	case EHarmoniaMonsterState::Alert:
		StateString = TEXT("Alert");
		break;
	case EHarmoniaMonsterState::Combat:
		StateString = TEXT("Combat");
		break;
	case EHarmoniaMonsterState::Retreating:
		StateString = TEXT("Retreating");
		break;
	case EHarmoniaMonsterState::Stunned:
		StateString = TEXT("Stunned");
		break;
	case EHarmoniaMonsterState::Dead:
		StateString = TEXT("Dead");
		break;
	}

	FString DebugText = FString::Printf(TEXT("State: %s"), *StateString);
	DrawDebugString(GetWorld(), DrawLocation, DebugText, nullptr, FColor::White, 0.0f, true, TextScale);
}

void UHarmoniaMonsterDebugComponent::DrawTarget(AHarmoniaMonsterBase* Monster)
{
	if (!Monster)
	{
		return;
	}

	AActor* Target = IHarmoniaMonsterInterface::Execute_GetCurrentTarget(Monster);
	if (!Target)
	{
		return;
	}

	// Draw line to target
	FVector StartLocation = Monster->GetActorLocation();
	FVector EndLocation = Target->GetActorLocation();

	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 0.0f, 0, 3.0f);

	// Draw target name
	FVector TargetDrawLocation = EndLocation + FVector(0, 0, 50);
	FString TargetName = Target->GetName();
	float Distance = FVector::Dist(StartLocation, EndLocation);
	FString TargetText = FString::Printf(TEXT("Target: %s (%.0f)"), *TargetName, Distance);

	DrawDebugString(GetWorld(), TargetDrawLocation, TargetText, nullptr, FColor::Red, 0.0f, true, TextScale * 0.8f);
}

void UHarmoniaMonsterDebugComponent::DrawThreatTable(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation)
{
	if (!Monster)
	{
		return;
	}

	UHarmoniaThreatComponent* ThreatComp = Monster->FindComponentByClass<UHarmoniaThreatComponent>();
	if (!ThreatComp)
	{
		return;
	}

	TArray<FHarmoniaThreatEntry> AllThreatEntries = ThreatComp->GetThreatTable(true);
	if (AllThreatEntries.Num() == 0)
	{
		return;
	}

	// Get top 5 threats
	int32 MaxThreats = FMath::Min(5, AllThreatEntries.Num());

	FVector CurrentDrawLocation = DrawLocation;
	CurrentDrawLocation.Z += 20.0f * TextScale;

	// Header
	DrawDebugString(GetWorld(), CurrentDrawLocation, TEXT("--- Threat Table ---"), nullptr, FColor::Yellow, 0.0f, true, TextScale * 0.7f);
	CurrentDrawLocation.Z += 15.0f * TextScale;

	// Draw top 5 threats
	for (int32 i = 0; i < MaxThreats; ++i)
	{
		const FHarmoniaThreatEntry& Entry = AllThreatEntries[i];
		if (Entry.ThreatActor)
		{
			FString ThreatText = FString::Printf(TEXT("%d. %s: %.0f"),
				i + 1,
				*Entry.ThreatActor->GetName(),
				Entry.ThreatValue);

			FColor Color = (i == 0) ? FColor::Red : FColor::Orange;
			DrawDebugString(GetWorld(), CurrentDrawLocation, ThreatText, nullptr, Color, 0.0f, true, TextScale * 0.6f);
			CurrentDrawLocation.Z += 12.0f * TextScale;
		}
	}
}

void UHarmoniaMonsterDebugComponent::DrawSensedTargets(AHarmoniaMonsterBase* Monster)
{
	if (!Monster)
	{
		return;
	}

	UHarmoniaSenseInteractionComponent* SenseComp = Monster->FindComponentByClass<UHarmoniaSenseInteractionComponent>();
	if (!SenseComp)
	{
		return;
	}

	TArray<FInteractableTargetInfo> TrackedTargets = SenseComp->GetAllInteractableTargets();

	for (const FInteractableTargetInfo& TargetInfo : TrackedTargets)
	{
		if (!TargetInfo.IsValid())
		{
			continue;
		}

		FVector StartLocation = Monster->GetActorLocation();
		FVector EndLocation = TargetInfo.TargetActor->GetActorLocation();

		// Different color based on priority
		FColor LineColor = FColor::Green;
		if (TargetInfo.Priority > 500.0f)
		{
			LineColor = FColor::Yellow;
		}
		if (TargetInfo.Priority > 1000.0f)
		{
			LineColor = FColor::Orange;
		}

		DrawDebugLine(GetWorld(), StartLocation, EndLocation, LineColor, false, 0.0f, 0, 1.0f);

		// Draw sensor tag
		FVector MidPoint = (StartLocation + EndLocation) / 2.0f;
		FString SensorText = FString::Printf(TEXT("[%s]"), *TargetInfo.SensorTag.ToString());
		DrawDebugString(GetWorld(), MidPoint, SensorText, nullptr, LineColor, 0.0f, false, TextScale * 0.5f);
	}
}

void UHarmoniaMonsterDebugComponent::DrawPatrolInfo(AHarmoniaMonsterBase* Monster)
{
	if (!Monster)
	{
		return;
	}

	// This would require access to AI controller's patrol data
	// For now, just draw a patrol radius sphere
	UHarmoniaMonsterData* MonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(Monster);
	if (MonsterData && MonsterData->PatrolRadius > 0.0f)
	{
		FVector HomeLocation = Monster->GetActorLocation(); // Simplified
		DrawDebugSphere(GetWorld(), HomeLocation, MonsterData->PatrolRadius, 16, FColor::Cyan, false, 0.0f, 0, 1.0f);
	}
}

void UHarmoniaMonsterDebugComponent::DrawHealthBar(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation)
{
	if (!Monster)
	{
		return;
	}

	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Get health attributes from HarmoniaAttributeSet
	float CurrentHealth = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetHealthAttribute());
	float MaxHealth = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetMaxHealthAttribute());

	float HealthPercent = (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;

	// Draw health text
	FString HealthText = FString::Printf(TEXT("HP: %.0f / %.0f (%.0f%%)"),
		CurrentHealth,
		MaxHealth,
		HealthPercent * 100.0f);

	FColor HealthColor = FColor::Green;
	if (HealthPercent < 0.3f)
	{
		HealthColor = FColor::Red;
	}
	else if (HealthPercent < 0.6f)
	{
		HealthColor = FColor::Yellow;
	}

	DrawDebugString(GetWorld(), DrawLocation, HealthText, nullptr, HealthColor, 0.0f, true, TextScale * 0.8f);
}

void UHarmoniaMonsterDebugComponent::DrawFaction(AHarmoniaMonsterBase* Monster, const FVector& DrawLocation)
{
	if (!Monster)
	{
		return;
	}

	UHarmoniaMonsterData* MonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(Monster);
	if (!MonsterData)
	{
		return;
	}

	const FHarmoniaFactionSettings& Faction = MonsterData->FactionSettings;

	FString FactionName = TEXT("Unknown");
	switch (Faction.Faction)
	{
	case EHarmoniaMonsterFaction::Neutral:
		FactionName = TEXT("Neutral");
		break;
	case EHarmoniaMonsterFaction::PlayerHostile:
		FactionName = TEXT("Hostile");
		break;
	case EHarmoniaMonsterFaction::PlayerFriendly:
		FactionName = TEXT("Friendly");
		break;
	case EHarmoniaMonsterFaction::Monster1:
		FactionName = TEXT("Monster 1");
		break;
	case EHarmoniaMonsterFaction::Monster2:
		FactionName = TEXT("Monster 2");
		break;
	case EHarmoniaMonsterFaction::Monster3:
		FactionName = TEXT("Monster 3");
		break;
	case EHarmoniaMonsterFaction::Monster4:
		FactionName = TEXT("Monster 4");
		break;
	default:
		FactionName = TEXT("Custom");
		break;
	}

	FString FactionText = FString::Printf(TEXT("Faction: %s"), *FactionName);
	FColor FactionColor = FColor::Cyan;

	if (Faction.Faction == EHarmoniaMonsterFaction::PlayerHostile)
	{
		FactionColor = FColor::Red;
	}
	else if (Faction.Faction == EHarmoniaMonsterFaction::PlayerFriendly)
	{
		FactionColor = FColor::Green;
	}

	DrawDebugString(GetWorld(), DrawLocation, FactionText, nullptr, FactionColor, 0.0f, true, TextScale * 0.7f);
}
