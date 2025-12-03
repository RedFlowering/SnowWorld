// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaSenseAttackComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "SenseReceiverComponent.h"
#include "SenseStimulusComponent.h"
#include "Sensors/SensorBase.h"
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

	// Validate damage values are reasonable
	if (InAttackData.DamageConfig.BaseDamage < 0.0f || InAttackData.DamageConfig.BaseDamage > 10000.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Invalid base damage %.1f"),
			InAttackData.DamageConfig.BaseDamage);
		return false;
	}

	if (InAttackData.DamageConfig.DamageMultiplier < 0.0f || InAttackData.DamageConfig.DamageMultiplier > 100.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartAttack: Invalid damage multiplier %.1f"),
			InAttackData.DamageConfig.DamageMultiplier);
		return false;
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

	// Activate stimulus
	SenseStimulus->SetActive(true);

	// Note: Actual sense stimulus configuration would depend on the SenseSystem plugin's API
	// This is a simplified implementation
	// In practice, you would configure the stimulus shape, range, etc. here
}

void UHarmoniaSenseAttackComponent::SetupSenseReceiver()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

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

	// Apply damage
	ApplyDamageToTarget(TargetActor, CurrentAttackData.DamageConfig, bWasCritical, HitLocation, HitNormal);

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

	// Calculate final damage
	float FinalDamage = CurrentAttackData.DamageConfig.BaseDamage * CurrentAttackData.DamageConfig.DamageMultiplier;
	if (bWasCritical)
	{
		FinalDamage *= CurrentAttackData.DamageConfig.CriticalMultiplier;
	}
	HitResult.DamageDealt = FinalDamage;

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

void UHarmoniaSenseAttackComponent::ApplyDamageToTarget(
	AActor* TargetActor,
	const FHarmoniaDamageEffectConfig& DamageConfig,
	bool bWasCritical,
	const FVector& HitLocation,
	const FVector& HitNormal)
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

	AActor* Owner = GetOwner();
	UAbilitySystemComponent* SourceASC = OwnerAbilitySystem;

	// Calculate final damage
	float FinalDamage = DamageConfig.BaseDamage * DamageConfig.DamageMultiplier;

	// Apply attack power from owner's attributes
	if (SourceASC)
	{
		const UHarmoniaAttributeSet* CombatSet = SourceASC->GetSet<UHarmoniaAttributeSet>();
		if (CombatSet)
		{
			FinalDamage *= (CombatSet->GetAttackPower() / 10.0f); // Normalize attack power
		}
	}

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
					// Set damage magnitude
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), FinalDamage);

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
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), DamagePerTick);

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
					SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), FinalDamage);
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
