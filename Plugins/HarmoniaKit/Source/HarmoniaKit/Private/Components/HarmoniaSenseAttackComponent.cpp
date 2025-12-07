// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSenseAttackComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "SenseReceiverComponent.h"
#include "SenseStimulusComponent.h"
#include "Sensors/SensorBase.h"
#include "Sensors/SensorSight.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UHarmoniaSenseAttackComponent::UHarmoniaSenseAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	bAutoActivate = true;
}

void UHarmoniaSenseAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitializeSenseStimulus)
	{
		InitializeSenseStimulus();
	}

	// Find owner's ability system if not set
	if (!OwnerAbilitySystem)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			OwnerAbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
		}
	}
}

void UHarmoniaSenseAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAttack();
	CleanupSenseReceiver();

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaSenseAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Debug visualization
	if (bIsAttacking && CurrentAttackData.TraceConfig.bShowDebugTrace)
	{
		DrawDebugAttackTrace();
	}
}

// ============================================================================
// Attack Control - Public Request Functions
// ============================================================================

void UHarmoniaSenseAttackComponent::RequestStartAttack(const FHarmoniaAttackData& InAttackData)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		// Server: Execute directly
		StartAttack(InAttackData);
	}
	else
	{
		// Client: Send to server
		ServerStartAttack(InAttackData);
	}
}

void UHarmoniaSenseAttackComponent::RequestStartAttackDefault()
{
	RequestStartAttack(AttackData);
}

void UHarmoniaSenseAttackComponent::RequestStopAttack()
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		// Server: Execute directly
		StopAttack();
	}
	else
	{
		// Client: Send to server
		ServerStopAttack();
	}
}

// ============================================================================
// Server RPCs
// ============================================================================

bool UHarmoniaSenseAttackComponent::ServerStartAttack_Validate(const FHarmoniaAttackData& InAttackData)
{
	// Validate attack data is enabled
	if (!InAttackData.bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Attack data disabled"));
		return false;
	}

	// Validate damage multiplier is reasonable
	if (InAttackData.DamageConfig.DamageMultiplier < 0.0f || InAttackData.DamageConfig.DamageMultiplier > 100.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Invalid damage multiplier %.1f"),
			InAttackData.DamageConfig.DamageMultiplier);
		return false;
	}

	// [ANTI-CHEAT] Validate critical chance is reasonable (0-100%)
	if (InAttackData.DamageConfig.CriticalChance < 0.0f || InAttackData.DamageConfig.CriticalChance > 1.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Invalid critical chance %.2f (must be 0.0-1.0)"),
			InAttackData.DamageConfig.CriticalChance);
		return false;
	}

	// [ANTI-CHEAT] Validate critical multiplier is reasonable (1x-10x)
	if (InAttackData.DamageConfig.CriticalMultiplier < 1.0f || InAttackData.DamageConfig.CriticalMultiplier > 10.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Invalid critical multiplier %.2f (must be 1.0-10.0)"),
			InAttackData.DamageConfig.CriticalMultiplier);
		return false;
	}

	// [ANTI-CHEAT] Attack rate limiting - prevent spam attacks
	if (const UWorld* World = GetWorld())
	{
		const float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastAttackRequestTime < MinAttackInterval)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Attack rate too fast (%.3fs since last attack, min: %.3fs)"),
				CurrentTime - LastAttackRequestTime, MinAttackInterval);
			return false;
		}
		// Note: We update LastAttackRequestTime in Implementation, not here
		// to avoid issues if validation passes but implementation fails
	}

	// Validate attack trace extent
	const FVector& TraceExtent = InAttackData.TraceConfig.TraceExtent;
	if (TraceExtent.X < 0.0f || TraceExtent.X > 10000.0f ||
		TraceExtent.Y < 0.0f || TraceExtent.Y > 10000.0f ||
		TraceExtent.Z < 0.0f || TraceExtent.Z > 10000.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Invalid trace extent (%.1f, %.1f, %.1f)"),
			TraceExtent.X, TraceExtent.Y, TraceExtent.Z);
		return false;
	}

	return true;
}

void UHarmoniaSenseAttackComponent::ServerStartAttack_Implementation(const FHarmoniaAttackData& InAttackData)
{
	// Update attack request time for rate limiting
	if (const UWorld* World = GetWorld())
	{
		LastAttackRequestTime = World->GetTimeSeconds();
	}

	StartAttack(InAttackData);
}

bool UHarmoniaSenseAttackComponent::ServerStopAttack_Validate()
{
	// Basic validation - always allow stopping
	return true;
}

void UHarmoniaSenseAttackComponent::ServerStopAttack_Implementation()
{
	StopAttack();
}

// ============================================================================
// Internal Attack Functions (Server-only)
// ============================================================================

void UHarmoniaSenseAttackComponent::StartAttack(const FHarmoniaAttackData& InAttackData)
{
	// Server authority check
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("StartAttack called on client - use RequestStartAttack instead"));
		return;
	}

	if (bIsAttacking)
	{
		StopAttack();
	}

	if (!InAttackData.bEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaSenseAttackComponent: Attack data is disabled"));
		return;
	}

	CurrentAttackData = InAttackData;
	bIsAttacking = true;
	HitTargets.Empty();
	HitActors.Empty();

	UWorld* World = GetWorld();
	if (World)
	{
		AttackStartTime = World->GetTimeSeconds();
	}

	// Update sense stimulus for this attack
	UpdateSenseStimulus();

	// Setup sense receiver for hit detection
	SetupSenseReceiver();

	// Broadcast attack start
	OnAttackStart.Broadcast();

	// Setup timer for continuous detection
	if (CurrentAttackData.TraceConfig.bContinuousDetection && CurrentAttackData.TraceConfig.DetectionDuration > 0.0f)
	{
		if (World)
		{
			World->GetTimerManager().SetTimer(
				AttackTimerHandle,
				this,
				&UHarmoniaSenseAttackComponent::OnAttackTimerComplete,
				CurrentAttackData.TraceConfig.DetectionDuration,
				false);
		}
	}
	else if (!CurrentAttackData.TraceConfig.bContinuousDetection)
	{
		// For single-shot, stop after a short delay to allow detection
		if (World)
		{
			World->GetTimerManager().SetTimer(
				AttackTimerHandle,
				this,
				&UHarmoniaSenseAttackComponent::OnAttackTimerComplete,
				0.1f,
				false);
		}
	}
}

void UHarmoniaSenseAttackComponent::StopAttack()
{
	// Server authority check
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("StopAttack called on client - use RequestStopAttack instead"));
		return;
	}

	if (!bIsAttacking)
	{
		return;
	}

	bIsAttacking = false;

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
	}

	// Cleanup sense receiver
	CleanupSenseReceiver();

	// Broadcast attack end
	OnAttackEnd.Broadcast();
}

void UHarmoniaSenseAttackComponent::ClearHitTargets()
{
	HitTargets.Empty();
	HitActors.Empty();
}

// ============================================================================
// Initialization
// ============================================================================

void UHarmoniaSenseAttackComponent::InitializeSenseStimulus()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create sense stimulus if not exists
	if (!SenseStimulus)
	{
		SenseStimulus = NewObject<USenseStimulusComponent>(
			Owner,
			USenseStimulusComponent::StaticClass(),
			FName("AttackSenseStimulus"));

		if (SenseStimulus)
		{
			SenseStimulus->RegisterComponent();
			SenseStimulus->SetActive(false); // Start inactive, activate during attacks
		}
	}
}

void UHarmoniaSenseAttackComponent::UpdateSenseStimulus()
{
	if (!SenseStimulus)
	{
		return;
	}

	const FHarmoniaAttackTraceConfig& TraceConfig = CurrentAttackData.TraceConfig;

	// Configure response channel for attack detection using table values
	const uint8 Channel = static_cast<uint8>(FMath::Clamp(TraceConfig.SenseChannel, 0, 63));
	if (Channel > 0)
	{
		SenseStimulus->SetResponseChannel(TraceConfig.SensorTag, Channel, true);
	}
	else
	{
		// Default to channel 1 if not specified
		SenseStimulus->SetResponseChannel(TraceConfig.SensorTag, 1, true);
	}

	// Set appropriate score for detection priority (default 1.0 for reliable detection)
	SenseStimulus->SetScore(TraceConfig.SensorTag, 1.0f);

	// Activate stimulus
	SenseStimulus->SetActive(true);
}

void UHarmoniaSenseAttackComponent::SetupSenseReceiver()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FHarmoniaAttackTraceConfig& TraceConfig = CurrentAttackData.TraceConfig;

	// Create temporary sense receiver for this attack
	if (!SenseReceiver)
	{
		SenseReceiver = NewObject<USenseReceiverComponent>(
			Owner,
			USenseReceiverComponent::StaticClass(),
			FName("AttackSenseReceiver"));

		if (SenseReceiver)
		{
			SenseReceiver->RegisterComponent();

			// Create and configure sensor for attack detection using table values
			ESuccessState SuccessState;
			USensorBase* AttackSensor = SenseReceiver->CreateNewSensor(
				USensorSight::StaticClass(),
				ESensorType::Active,
				TraceConfig.SensorTag,
				ESensorThreadType::Main_Thread,
				true,
				SuccessState);

			if (AttackSensor && SuccessState == ESuccessState::Success)
			{
				// Configure sensor response channel from table
				const uint8 Channel = static_cast<uint8>(FMath::Clamp(TraceConfig.SenseChannel, 0, 63));
				if (Channel > 0)
				{
					AttackSensor->SetSenseResponseChannelBit(Channel, true);
				}
				else
				{
					// Default to channel 1 if not specified
					AttackSensor->SetSenseResponseChannelBit(1, true);
				}

				// Configure sensor sight parameters using TraceExtent as detection range
				if (USensorSight* SightSensor = Cast<USensorSight>(AttackSensor))
				{
					const float DetectionRange = TraceConfig.TraceExtent.X; // Use X as primary range
					SightSensor->SetDistanceAngleParam(
						DetectionRange,         // MaxDistance
						DetectionRange * 1.2f,  // MaxDistanceLost (slightly larger for hysteresis)
						180.0f,                 // MaxAngle (full sphere for attack detection)
						180.0f,                 // MaxAngleLost
						true,                   // TestBySingleLocation
						TraceConfig.MinimumSenseScore  // MinScore from table
					);
				}
			}
		}
	}

	// Bind to sense detection
	if (SenseReceiver)
	{
		SenseReceiver->OnNewSense.AddDynamic(this, &UHarmoniaSenseAttackComponent::OnSenseDetected);
		SenseReceiver->OnCurrentSense.AddDynamic(this, &UHarmoniaSenseAttackComponent::OnSenseDetected);
	}
}

void UHarmoniaSenseAttackComponent::CleanupSenseReceiver()
{
	if (SenseReceiver)
	{
		SenseReceiver->OnNewSense.RemoveDynamic(this, &UHarmoniaSenseAttackComponent::OnSenseDetected);
		SenseReceiver->OnCurrentSense.RemoveDynamic(this, &UHarmoniaSenseAttackComponent::OnSenseDetected);
		SenseReceiver->DestroyComponent(false);
		SenseReceiver = nullptr;
	}

	if (SenseStimulus)
	{
		SenseStimulus->SetActive(false);
	}
}

// ============================================================================
// Hit Detection
// ============================================================================

void UHarmoniaSenseAttackComponent::OnSenseDetected(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli)
{
	if (!bIsAttacking || !SensorPtr)
	{
		return;
	}

	// Check if this is the correct sensor
	if (SensorPtr->SensorTag != CurrentAttackData.TraceConfig.SensorTag)
	{
		return;
	}

	// Check channel
	if (Channel != CurrentAttackData.TraceConfig.SenseChannel)
	{
		return;
	}

	// Process each detected stimulus
	for (const FSensedStimulus& Stimulus : SensedStimuli)
	{
		// Check sense score threshold
		if (Stimulus.Score < CurrentAttackData.TraceConfig.MinimumSenseScore)
		{
			continue;
		}

		// Process hit
		ProcessHitTarget(Stimulus);

		// Check if we've hit max targets
		if (HitTargets.Num() >= CurrentAttackData.TraceConfig.MaxTargets)
		{
			StopAttack();
			return;
		}
	}
}

bool UHarmoniaSenseAttackComponent::ProcessHitTarget(const FSensedStimulus& Stimulus)
{
	if (!Stimulus.StimulusComponent.IsValid())
	{
		return false;
	}

	AActor* TargetActor = Stimulus.StimulusComponent->GetOwner();
	if (!ShouldHitTarget(TargetActor, Stimulus))
	{
		return false;
	}

	// Check if already hit (for hit-once tracking)
	if (CurrentAttackData.TraceConfig.bHitOncePerTarget && HitActors.Contains(TargetActor))
	{
		return false;
	}

	// Calculate critical hit
	const bool bWasCritical = CalculateCriticalHit(CurrentAttackData.DamageConfig.CriticalChance);

	// Get hit location and normal
	FVector HitLocation = Stimulus.SensedPoints.Num() > 0 ? Stimulus.SensedPoints[0].SensedPoint : GetOwner()->GetActorLocation();
	FVector HitNormal = (GetOwner()->GetActorLocation() - HitLocation).GetSafeNormal();
	float HitDistance = Stimulus.SensedPoints.Num() > 0 ? (HitLocation - GetTraceLocation()).Length() : 0.0f;

	// Apply damage and get actual damage dealt
	float ActualDamage = ApplyDamageToTarget(TargetActor, CurrentAttackData.DamageConfig, bWasCritical, HitLocation, HitNormal);

	// Apply hit reaction
	const FVector HitDirection = (HitLocation - GetTraceLocation()).GetSafeNormal();
	ApplyHitReaction(TargetActor, CurrentAttackData.HitReactionConfig, bWasCritical, HitLocation, HitDirection);

	// Record hit
	FHarmoniaAttackHitResult HitResult;
	HitResult.HitActor = TargetActor;
	HitResult.HitComponent = Stimulus.StimulusComponent.Get();
	HitResult.HitLocation = HitLocation;
	HitResult.HitNormal = HitNormal;
	HitResult.Distance = HitDistance;
	HitResult.bWasCriticalHit = bWasCritical;
	HitResult.SenseScore = Stimulus.Score;

	if (UWorld* World = GetWorld())
	{
		HitResult.HitTime = World->GetTimeSeconds();
	}

	// Use actual damage from ApplyDamageToTarget
	HitResult.DamageDealt = ActualDamage;

	HitTargets.Add(HitResult);
	HitActors.Add(TargetActor);

	// Broadcast hit event
	OnAttackHit.Broadcast(HitResult);

	return true;
}

bool UHarmoniaSenseAttackComponent::ShouldHitTarget(AActor* TargetActor, const FSensedStimulus& Stimulus) const
{
	if (!TargetActor)
	{
		return false;
	}

	// Don't hit self
	AActor* Owner = GetOwner();
	if (TargetActor == Owner)
	{
		return false;
	}

	// Check if target has ability system (needed for damage)
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!TargetASC)
	{
		return false;
	}

	return true;
}

bool UHarmoniaSenseAttackComponent::CalculateCriticalHit(float CritChance) const
{
	if (!CurrentAttackData.DamageConfig.bCanCritical)
	{
		return false;
	}

	// Get critical chance from owner's attributes if available
	float FinalCritChance = CritChance;

	if (OwnerAbilitySystem)
	{
		const UHarmoniaAttributeSet* CombatSet = OwnerAbilitySystem->GetSet<UHarmoniaAttributeSet>();
		if (CombatSet)
		{
			FinalCritChance = CombatSet->GetCriticalChance();
		}
	}

	return FMath::FRand() < FinalCritChance;
}

// ============================================================================
// Damage Application
// ============================================================================

float UHarmoniaSenseAttackComponent::ApplyDamageToTarget(
	AActor* TargetActor,
	const FHarmoniaDamageEffectConfig& DamageConfig,
	bool bWasCritical,
	const FVector& HitLocation,
	const FVector& HitNormal)
{
	if (!TargetActor)
	{
		return 0.0f;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!TargetASC)
	{
		return 0.0f;
	}

	AActor* Owner = GetOwner();
	UAbilitySystemComponent* SourceASC = OwnerAbilitySystem;

	// Calculate final damage using AttackPower from attributes as base
	float FinalDamage = 0.0f;

	if (SourceASC)
	{
		const UHarmoniaAttributeSet* CombatSet = SourceASC->GetSet<UHarmoniaAttributeSet>();
		if (CombatSet)
		{
			// Use AttackPower as base damage (includes weapon, stats, buffs)
			FinalDamage = CombatSet->GetAttackPower();
		}
	}

	// Apply combo/attack multiplier from table
	FinalDamage *= DamageConfig.DamageMultiplier;

	// Apply critical multiplier
	if (bWasCritical)
	{
		float CritMultiplier = DamageConfig.CriticalMultiplier;

		if (SourceASC)
		{
			const UHarmoniaAttributeSet* CombatSet = SourceASC->GetSet<UHarmoniaAttributeSet>();
			if (CombatSet)
			{
				CritMultiplier = CombatSet->GetCriticalDamage();
			}
		}

		FinalDamage *= CritMultiplier;
	}

	// Apply damage based on type
	switch (DamageConfig.DamageType)
	{
	case EHarmoniaDamageType::Instant:
		{
			// Apply instant damage via Gameplay Effect
			if (DamageConfig.DamageEffectClass)
			{
				FGameplayEffectContextHandle EffectContext = CreateDamageEffectContext(TargetActor, HitLocation, HitNormal);
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageConfig.DamageEffectClass, 1.0f, EffectContext);

				if (SpecHandle.IsValid())
				{
					// Set damage magnitude using configured tag
					if (DamageConfig.SetByCallerDamageTag.IsValid())
					{
						SpecHandle.Data->SetSetByCallerMagnitude(DamageConfig.SetByCallerDamageTag, FinalDamage);
					}

					// Add damage tags
					SpecHandle.Data->DynamicGrantedTags.AppendTags(DamageConfig.DamageTags);

					// Apply effect
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
			else
			{
				// Fallback: Apply damage directly to attribute
				const UHarmoniaAttributeSet* TargetCombatSet = TargetASC->GetSet<UHarmoniaAttributeSet>();
				if (TargetCombatSet)
				{
					TargetASC->ApplyModToAttribute(
						TargetCombatSet->GetDamageAttribute(),
						EGameplayModOp::Additive,
						FinalDamage);
				}
			}
		}
		break;

	case EHarmoniaDamageType::Duration:
		{
			// Apply DoT effect
			if (DamageConfig.DamageEffectClass)
			{
				FGameplayEffectContextHandle EffectContext = CreateDamageEffectContext(TargetActor, HitLocation, HitNormal);
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageConfig.DamageEffectClass, 1.0f, EffectContext);

				if (SpecHandle.IsValid())
				{
					// Calculate damage per tick
					const float DamagePerTick = FinalDamage / (DamageConfig.DurationSeconds / DamageConfig.TickInterval);
					if (DamageConfig.SetByCallerDamageTag.IsValid())
					{
						SpecHandle.Data->SetSetByCallerMagnitude(DamageConfig.SetByCallerDamageTag, DamagePerTick);
					}

					// Add damage tags
					SpecHandle.Data->DynamicGrantedTags.AppendTags(DamageConfig.DamageTags);

					// Apply effect
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
		break;

	case EHarmoniaDamageType::Explosion:
		{
			// For explosion, damage falloff is handled separately
			// Apply reduced damage based on distance from center
			const float Distance = FVector::Dist(HitLocation, GetTraceLocation());
			const float DistanceRatio = FMath::Clamp(Distance / DamageConfig.ExplosionRadius, 0.0f, 1.0f);
			const float Falloff = FMath::Pow(1.0f - DistanceRatio, DamageConfig.ExplosionFalloff);
			FinalDamage *= Falloff;

			if (DamageConfig.DamageEffectClass)
			{
				FGameplayEffectContextHandle EffectContext = CreateDamageEffectContext(TargetActor, HitLocation, HitNormal);
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageConfig.DamageEffectClass, 1.0f, EffectContext);

				if (SpecHandle.IsValid())
				{
					if (DamageConfig.SetByCallerDamageTag.IsValid())
					{
						SpecHandle.Data->SetSetByCallerMagnitude(DamageConfig.SetByCallerDamageTag, FinalDamage);
					}
					SpecHandle.Data->DynamicGrantedTags.AppendTags(DamageConfig.DamageTags);
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
		break;

	case EHarmoniaDamageType::Percentage:
		{
			// Apply percentage-based damage (ignores defense)
			const UHarmoniaAttributeSet* TargetCombatSet = TargetASC->GetSet<UHarmoniaAttributeSet>();
			if (TargetCombatSet)
			{
				const float PercentageDamage = TargetCombatSet->GetMaxHealth() * (FinalDamage / 100.0f);
				TargetASC->ApplyModToAttribute(
					TargetCombatSet->GetDamageAttribute(),
					EGameplayModOp::Additive,
					PercentageDamage);
			}
		}
		break;

	default:
		break;
	}

	// Apply additional effects
	if (SourceASC)
	{
		for (const TSubclassOf<UGameplayEffect>& AdditionalEffect : DamageConfig.AdditionalEffects)
		{
			if (AdditionalEffect)
			{
				FGameplayEffectContextHandle EffectContext = CreateDamageEffectContext(TargetActor, HitLocation, HitNormal);
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AdditionalEffect, 1.0f, EffectContext);

				if (SpecHandle.IsValid())
				{
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
	}

	return FinalDamage;
}

void UHarmoniaSenseAttackComponent::ApplyHitReaction(
	AActor* TargetActor,
	const FHarmoniaHitReactionConfig& ReactionConfig,
	bool bWasCritical,
	const FVector& HitLocation,
	const FVector& HitDirection)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	// Trigger Gameplay Cue
	if (ReactionConfig.GameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitLocation;
		CueParams.Normal = HitDirection;
		CueParams.Instigator = GetOwner();
		CueParams.EffectCauser = GetOwner();
		CueParams.TargetAttachComponent = TargetActor->GetRootComponent();
		CueParams.RawMagnitude = bWasCritical ? 2.0f : 1.0f;

		TargetASC->ExecuteGameplayCue(ReactionConfig.GameplayCueTag, CueParams);
	}

	// Apply impact force (if target has physics)
	if (ReactionConfig.ImpactForce > 0.0f)
	{
		if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
		{
			if (RootPrimitive->IsSimulatingPhysics())
			{
				FVector ForceDirection = ReactionConfig.ImpactDirection.IsNearlyZero() ? HitDirection : ReactionConfig.ImpactDirection;
				ForceDirection.Normalize();

				RootPrimitive->AddImpulse(ForceDirection * ReactionConfig.ImpactForce, NAME_None, true);
			}
		}
	}

	// Apply camera shake to the attacker's player controller
	if (ReactionConfig.CameraShakeClass)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController());
			if (PC)
			{
				PC->ClientStartCameraShake(ReactionConfig.CameraShakeClass, ReactionConfig.CameraShakeScale);
			}
		}
	}

	// Apply hit pause (time dilation effect)
	if (ReactionConfig.bApplyHitPause && ReactionConfig.HitPauseDuration > 0.0f)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// Store original time dilation
			const float OriginalTimeDilation = World->GetWorldSettings()->TimeDilation;

			// Apply hit pause (slow down time)
			World->GetWorldSettings()->SetTimeDilation(0.1f);

			// Set timer to restore normal time
			FTimerHandle HitPauseTimerHandle;
			World->GetTimerManager().SetTimer(
				HitPauseTimerHandle,
				[World, OriginalTimeDilation]()
				{
					if (World && World->GetWorldSettings())
					{
						World->GetWorldSettings()->SetTimeDilation(OriginalTimeDilation);
					}
				},
				ReactionConfig.HitPauseDuration * 0.1f, // Adjust for dilated time
				false
			);
		}
	}
}

FGameplayEffectContextHandle UHarmoniaSenseAttackComponent::CreateDamageEffectContext(
	AActor* TargetActor,
	const FVector& HitLocation,
	const FVector& HitNormal) const
{
	FGameplayEffectContextHandle ContextHandle;

	if (OwnerAbilitySystem)
	{
		ContextHandle = OwnerAbilitySystem->MakeEffectContext();
		ContextHandle.AddInstigator(GetOwner(), GetOwner());
		ContextHandle.AddHitResult(FHitResult(TargetActor, nullptr, HitLocation, HitNormal));
		ContextHandle.AddOrigin(GetTraceLocation());
	}

	return ContextHandle;
}

// ============================================================================
// Helpers
// ============================================================================

void UHarmoniaSenseAttackComponent::OnAttackTimerComplete()
{
	StopAttack();
}

FVector UHarmoniaSenseAttackComponent::GetTraceLocation() const
{
	if (!CurrentAttackData.TraceConfig.SocketName.IsNone())
	{
		if (USceneComponent* ParentComp = GetAttachParent())
		{
			if (ParentComp->DoesSocketExist(CurrentAttackData.TraceConfig.SocketName))
			{
				return ParentComp->GetSocketLocation(CurrentAttackData.TraceConfig.SocketName);
			}
		}
	}

	return GetComponentLocation() + CurrentAttackData.TraceConfig.TraceOffset;
}

FRotator UHarmoniaSenseAttackComponent::GetTraceRotation() const
{
	if (!CurrentAttackData.TraceConfig.SocketName.IsNone())
	{
		if (USceneComponent* ParentComp = GetAttachParent())
		{
			if (ParentComp->DoesSocketExist(CurrentAttackData.TraceConfig.SocketName))
			{
				return ParentComp->GetSocketRotation(CurrentAttackData.TraceConfig.SocketName);
			}
		}
	}

	return GetComponentRotation();
}

void UHarmoniaSenseAttackComponent::DrawDebugAttackTrace() const
{
	if (!GetWorld())
	{
		return;
	}

	const FVector Location = GetTraceLocation();
	const FRotator Rotation = GetTraceRotation();
	const FVector Extent = CurrentAttackData.TraceConfig.TraceExtent;
	const FColor DebugColor = bIsAttacking ? FColor::Green : FColor::Red;

	switch (CurrentAttackData.TraceConfig.TraceShape)
	{
	case EHarmoniaAttackTraceShape::Box:
		DrawDebugBox(GetWorld(), Location, Extent, Rotation.Quaternion(), DebugColor, false, -1.0f, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Sphere:
		DrawDebugSphere(GetWorld(), Location, Extent.X, 12, DebugColor, false, -1.0f, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Capsule:
		DrawDebugCapsule(GetWorld(), Location, Extent.Z, Extent.X, Rotation.Quaternion(), DebugColor, false, -1.0f, 0, 2.0f);
		break;

	case EHarmoniaAttackTraceShape::Line:
		{
			const FVector EndLocation = Location + Rotation.Vector() * Extent.X;
			DrawDebugLine(GetWorld(), Location, EndLocation, DebugColor, false, -1.0f, 0, 2.0f);
		}
		break;

	default:
		break;
	}
}
