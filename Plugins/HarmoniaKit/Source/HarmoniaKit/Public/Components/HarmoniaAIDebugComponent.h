// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/HarmoniaLearningAIComponent.h"
#include "Components/HarmoniaTerritoryDisputeComponent.h"
#include "HarmoniaAIDebugComponent.generated.h"

/**
 * Debug visualization mode
 */
UENUM(BlueprintType)
enum class EHarmoniaDebugMode : uint8
{
	Off				UMETA(DisplayName = "Off"),
	Basic			UMETA(DisplayName = "Basic Info"),
	Detailed		UMETA(DisplayName = "Detailed Stats"),
	Learning		UMETA(DisplayName = "Learning AI Data"),
	CombatPower		UMETA(DisplayName = "Combat Power Breakdown"),
	All				UMETA(DisplayName = "All Information")
};

/**
 * AI Debug Component
 * Provides real-time visualization of AI state and decisions
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaAIDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaAIDebugComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Debug visualization mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	EHarmoniaDebugMode DebugMode = EHarmoniaDebugMode::Off;

	/** Show debug on screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	bool bShowOnScreen = true;

	/** Show debug in world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	bool bShowInWorld = true;

	/** Debug text color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	FColor DebugColor = FColor::Yellow;

	/** Debug text size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	float TextScale = 1.0f;

	/** Log AI decisions to output log */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Debug")
	bool bLogDecisions = false;

	/** Get formatted debug string */
	UFUNCTION(BlueprintCallable, Category = "AI Debug")
	FString GetDebugString() const;

	/** Get learning AI info */
	UFUNCTION(BlueprintCallable, Category = "AI Debug")
	FString GetLearningAIInfo() const;

	/** Get combat power breakdown */
	UFUNCTION(BlueprintCallable, Category = "AI Debug")
	FString GetCombatPowerBreakdown() const;

	/** Get territory dispute info */
	UFUNCTION(BlueprintCallable, Category = "AI Debug")
	FString GetTerritoryDisputeInfo() const;

	/** Log AI decision */
	UFUNCTION(BlueprintCallable, Category = "AI Debug")
	void LogDecision(const FString& Decision, const FString& Reason);

protected:
	/** Draw debug information */
	void DrawDebug();

	/** Draw basic info */
	void DrawBasicInfo();

	/** Draw learning AI data */
	void DrawLearningData();

	/** Draw combat power */
	void DrawCombatPower();

	UPROPERTY()
	class AHarmoniaMonsterBase* OwnerMonster;

	UPROPERTY()
	UHarmoniaLearningAIComponent* LearningAI;

	UPROPERTY()
	UHarmoniaTerritoryDisputeComponent* TerritoryDispute;

	/** Decision log */
	TArray<FString> RecentDecisions;
	static constexpr int32 MaxLogEntries = 10;
};
