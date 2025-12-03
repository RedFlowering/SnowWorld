// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaStatusEffectComponent.h"
#include "Data/HarmoniaUIConfigDataAsset.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemGlobals.h"

UHarmoniaStatusEffectComponent::UHarmoniaStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for duration display
}

void UHarmoniaStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	// Try to find ASC on owner
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
		if (ASC)
		{
			InitializeWithASC(ASC);
		}
	}
}

void UHarmoniaStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from ASC
	if (TrackedASC.IsValid())
	{
		if (OnEffectAppliedHandle.IsValid())
		{
			TrackedASC->OnGameplayEffectAppliedDelegateToSelf.Remove(OnEffectAppliedHandle);
		}
		if (OnEffectRemovedHandle.IsValid())
		{
			TrackedASC->OnAnyGameplayEffectRemovedDelegate().Remove(OnEffectRemovedHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UHarmoniaStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateEffectTimers(DeltaTime);
	CleanupExpiredEffects();
}

// ============================================================================
// GAS Integration
// ============================================================================

void UHarmoniaStatusEffectComponent::InitializeWithASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	// Unbind from previous ASC
	if (TrackedASC.IsValid() && TrackedASC.Get() != ASC)
	{
		if (OnEffectAppliedHandle.IsValid())
		{
			TrackedASC->OnGameplayEffectAppliedDelegateToSelf.Remove(OnEffectAppliedHandle);
		}
		if (OnEffectRemovedHandle.IsValid())
		{
			TrackedASC->OnAnyGameplayEffectRemovedDelegate().Remove(OnEffectRemovedHandle);
		}
	}

	TrackedASC = ASC;

	// Bind to new ASC
	OnEffectAppliedHandle = ASC->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
		this, &UHarmoniaStatusEffectComponent::OnGameplayEffectApplied);

	OnEffectRemovedHandle = ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(
		this, &UHarmoniaStatusEffectComponent::OnGameplayEffectRemoved);

	// Query existing effects
	TArray<FActiveGameplayEffectHandle> ActiveHandles = ASC->GetActiveEffects(FGameplayEffectQuery());
	for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
	{
		const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle);
		if (ActiveEffect)
		{
			FHarmoniaStatusEffectUIData UIData = CreateUIDataFromEffect(ActiveEffect->Spec, Handle);
			ActiveEffects.Add(UIData);
			OnEffectAdded.Broadcast(UIData);
		}
	}
}

// ============================================================================
// Effect Queries
// ============================================================================

TArray<FHarmoniaStatusEffectUIData> UHarmoniaStatusEffectComponent::GetEffectsByType(EHarmoniaStatusEffectType Type) const
{
	TArray<FHarmoniaStatusEffectUIData> Result;
	for (const FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectType == Type)
		{
			Result.Add(Effect);
		}
	}
	return Result;
}

TArray<FHarmoniaStatusEffectUIData> UHarmoniaStatusEffectComponent::GetEffectsByCategory(EHarmoniaStatusEffectCategory Category) const
{
	TArray<FHarmoniaStatusEffectUIData> Result;
	for (const FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.Category == Category)
		{
			Result.Add(Effect);
		}
	}
	return Result;
}

TArray<FHarmoniaStatusEffectUIData> UHarmoniaStatusEffectComponent::GetBuffs() const
{
	return GetEffectsByType(EHarmoniaStatusEffectType::Buff);
}

TArray<FHarmoniaStatusEffectUIData> UHarmoniaStatusEffectComponent::GetDebuffs() const
{
	return GetEffectsByType(EHarmoniaStatusEffectType::Debuff);
}

bool UHarmoniaStatusEffectComponent::GetEffectByID(FGuid EffectID, FHarmoniaStatusEffectUIData& OutEffect) const
{
	for (const FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectID == EffectID)
		{
			OutEffect = Effect;
			return true;
		}
	}
	return false;
}

bool UHarmoniaStatusEffectComponent::GetEffectByTag(FGameplayTag EffectTag, FHarmoniaStatusEffectUIData& OutEffect) const
{
	for (const FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectTag.MatchesTagExact(EffectTag))
		{
			OutEffect = Effect;
			return true;
		}
	}
	return false;
}

bool UHarmoniaStatusEffectComponent::IsEffectActive(FGameplayTag EffectTag) const
{
	for (const FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectTag.MatchesTagExact(EffectTag))
		{
			return true;
		}
	}
	return false;
}

int32 UHarmoniaStatusEffectComponent::GetEffectStackCount(FGameplayTag EffectTag) const
{
	for (const FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectTag.MatchesTagExact(EffectTag))
		{
			return Effect.StackCount;
		}
	}
	return 0;
}

// ============================================================================
// Manual Effect Management
// ============================================================================

FGuid UHarmoniaStatusEffectComponent::AddCustomEffect(const FHarmoniaStatusEffectUIData& Effect)
{
	FHarmoniaStatusEffectUIData NewEffect = Effect;
	if (!NewEffect.EffectID.IsValid())
	{
		NewEffect.EffectID = FGuid::NewGuid();
	}

	// Set start time
	NewEffect.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Calculate progress
	if (NewEffect.bHasDuration && NewEffect.TotalDuration > 0.0f)
	{
		NewEffect.DurationProgress = NewEffect.RemainingDuration / NewEffect.TotalDuration;
	}
	else
	{
		NewEffect.DurationProgress = 1.0f;
	}

	// Set border color
	NewEffect.BorderColor = GetBorderColorForType(NewEffect.EffectType);

	ActiveEffects.Add(NewEffect);
	OnEffectAdded.Broadcast(NewEffect);

	return NewEffect.EffectID;
}

bool UHarmoniaStatusEffectComponent::RemoveCustomEffect(FGuid EffectID)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (ActiveEffects[i].EffectID == EffectID && !ActiveEffects[i].ActiveHandle.IsValid())
		{
			ActiveEffects.RemoveAt(i);
			WarnedExpiringEffects.Remove(EffectID);
			OnEffectRemoved.Broadcast(EffectID);
			return true;
		}
	}
	return false;
}

bool UHarmoniaStatusEffectComponent::UpdateCustomEffectDuration(FGuid EffectID, float NewRemainingDuration)
{
	for (FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectID == EffectID && !Effect.ActiveHandle.IsValid())
		{
			Effect.RemainingDuration = NewRemainingDuration;
			if (Effect.TotalDuration > 0.0f)
			{
				Effect.DurationProgress = NewRemainingDuration / Effect.TotalDuration;
			}
			OnEffectUpdated.Broadcast(Effect);
			return true;
		}
	}
	return false;
}

bool UHarmoniaStatusEffectComponent::UpdateCustomEffectStacks(FGuid EffectID, int32 NewStackCount)
{
	for (FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.EffectID == EffectID && !Effect.ActiveHandle.IsValid())
		{
			int32 OldStackCount = Effect.StackCount;
			Effect.StackCount = FMath::Clamp(NewStackCount, 0, Effect.MaxStackCount);
			OnStackChanged.Broadcast(EffectID, Effect.StackCount);
			return true;
		}
	}
	return false;
}

// ============================================================================
// Configuration
// ============================================================================

void UHarmoniaStatusEffectComponent::RegisterEffectConfig(const FHarmoniaStatusEffectConfig& Config)
{
	if (Config.EffectTag.IsValid())
	{
		EffectConfigs.Add(Config.EffectTag, Config);
	}
}

bool UHarmoniaStatusEffectComponent::GetEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const
{
	const FHarmoniaStatusEffectConfig* Found = EffectConfigs.Find(EffectTag);
	if (Found)
	{
		OutConfig = *Found;
		return true;
	}
	return false;
}

// ============================================================================
// Internal Updates
// ============================================================================

void UHarmoniaStatusEffectComponent::UpdateEffectTimers(float DeltaTime)
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (!Effect.bHasDuration)
		{
			continue;
		}

		// For GAS effects, query the ASC for remaining time
		if (Effect.ActiveHandle.IsValid() && TrackedASC.IsValid())
		{
			TArray<float> Durations = TrackedASC->GetActiveEffectsDuration(FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(Effect.EffectTag.GetSingleTagContainer()));
			float RemainingTime = Durations.Num() > 0 ? Durations[0] : 0.0f;
			Effect.RemainingDuration = RemainingTime;
		}
		else
		{
			// For custom effects, manually decrease
			Effect.RemainingDuration -= DeltaTime;
		}

		// Update progress
		if (Effect.TotalDuration > 0.0f)
		{
			Effect.DurationProgress = FMath::Clamp(Effect.RemainingDuration / Effect.TotalDuration, 0.0f, 1.0f);
		}

		// Check for expiring soon
		bool bWasExpiringSoon = Effect.bIsExpiringSoon;
		Effect.bIsExpiringSoon = Effect.RemainingDuration <= ExpiringSoonThreshold && Effect.RemainingDuration > 0.0f;

		// Fire expiring event (only once per effect)
		if (Effect.bIsExpiringSoon && !bWasExpiringSoon && !WarnedExpiringEffects.Contains(Effect.EffectID))
		{
			WarnedExpiringEffects.Add(Effect.EffectID);
			OnEffectExpiring.Broadcast(Effect);
		}
	}
}

void UHarmoniaStatusEffectComponent::OnGameplayEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	// Check if we already have this effect (for stacking)
	for (FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.ActiveHandle == Handle)
		{
			// Already tracked
			return;
		}
	}

	FHarmoniaStatusEffectUIData UIData = CreateUIDataFromEffect(Spec, Handle);
	ActiveEffects.Add(UIData);
	OnEffectAdded.Broadcast(UIData);
}

void UHarmoniaStatusEffectComponent::OnGameplayEffectRemoved(const FActiveGameplayEffect& RemovedEffect)
{
	FActiveGameplayEffectHandle Handle = RemovedEffect.Handle;

	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		if (ActiveEffects[i].ActiveHandle == Handle)
		{
			FGuid RemovedID = ActiveEffects[i].EffectID;
			ActiveEffects.RemoveAt(i);
			WarnedExpiringEffects.Remove(RemovedID);
			OnEffectRemoved.Broadcast(RemovedID);
			return;
		}
	}
}

void UHarmoniaStatusEffectComponent::OnGameplayEffectStackChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount)
{
	for (FHarmoniaStatusEffectUIData& Effect : ActiveEffects)
	{
		if (Effect.ActiveHandle == Handle)
		{
			Effect.StackCount = NewStackCount;
			OnStackChanged.Broadcast(Effect.EffectID, NewStackCount);
			OnEffectUpdated.Broadcast(Effect);
			return;
		}
	}
}

FHarmoniaStatusEffectUIData UHarmoniaStatusEffectComponent::CreateUIDataFromEffect(const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	FHarmoniaStatusEffectUIData UIData;
	UIData.ActiveHandle = Handle;
	UIData.EffectID = FGuid::NewGuid();

	// Get effect class
	if (Spec.Def)
	{
		UIData.EffectClass = Spec.Def->GetClass();

		// Try to get asset tags for identification
		const FGameplayTagContainer& AssetTags = Spec.Def->GetAssetTags();
		if (AssetTags.Num() > 0)
		{
			UIData.EffectTag = AssetTags.First();
		}
	}

	// Check for configured display data
	FHarmoniaStatusEffectConfig Config;
	if (UIData.EffectTag.IsValid() && GetEffectConfig(UIData.EffectTag, Config))
	{
		UIData.DisplayName = Config.DisplayName;
		UIData.Description = Config.DescriptionTemplate;
		UIData.IconTexture = Config.Icon;
		UIData.EffectType = Config.EffectType;
		UIData.Category = Config.Category;
		UIData.DisplayPriority = Config.Priority;
	}
	else
	{
		// Default display data from effect
		if (Spec.Def)
		{
			UIData.DisplayName = FText::FromString(Spec.Def->GetName());
		}
	}

	// Duration
	UIData.TotalDuration = Spec.GetDuration();
	UIData.RemainingDuration = UIData.TotalDuration;
	UIData.bHasDuration = UIData.TotalDuration > 0.0f;
	UIData.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	UIData.DurationProgress = 1.0f;

	// Stacking
	UIData.StackCount = Spec.GetStackCount();
	if (Spec.Def)
	{
		UIData.MaxStackCount = Spec.Def->StackLimitCount;
	}

	// Magnitude
	UIData.MagnitudeText = FormatMagnitudeText(Spec);

	// Colors
	UIData.BorderColor = GetBorderColorForType(UIData.EffectType);

	return UIData;
}

FLinearColor UHarmoniaStatusEffectComponent::GetBorderColorForType(EHarmoniaStatusEffectType Type) const
{
	// Use data asset if available
	if (UIConfigAsset)
	{
		return UIConfigAsset->GetBorderColorForType(Type);
	}

	// Fallback to default colors
	switch (Type)
	{
	case EHarmoniaStatusEffectType::Buff:
		return FLinearColor(0.2f, 0.8f, 0.2f); // Green

	case EHarmoniaStatusEffectType::Debuff:
		return FLinearColor(0.8f, 0.2f, 0.2f); // Red

	case EHarmoniaStatusEffectType::Neutral:
	default:
		return FLinearColor(0.5f, 0.5f, 0.5f); // Gray
	}
}

void UHarmoniaStatusEffectComponent::SetUIConfigDataAsset(UHarmoniaUIConfigDataAsset* ConfigAsset)
{
	UIConfigAsset = ConfigAsset;

	if (ConfigAsset)
	{
		// Apply timing settings from data asset
		ExpiringSoonThreshold = ConfigAsset->StatusEffectTiming.ExpiringSoonThreshold;
		PrimaryComponentTick.TickInterval = ConfigAsset->StatusEffectTiming.UpdateInterval;

		// Register predefined effect configs
		for (const FHarmoniaStatusEffectConfig& Config : ConfigAsset->PredefinedEffectConfigs)
		{
			RegisterEffectConfig(Config);
		}
	}
}

FText UHarmoniaStatusEffectComponent::FormatMagnitudeText(const FGameplayEffectSpec& Spec) const
{
	// Try to get the first modifier magnitude
	if (Spec.Def && Spec.Def->Modifiers.Num() > 0)
	{
		float Magnitude = 0.0f;
		Spec.Def->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(0, Magnitude);

		if (FMath::Abs(Magnitude) > 0.01f)
		{
			if (Magnitude > 0)
			{
				return FText::FromString(FString::Printf(TEXT("+%.0f%%"), Magnitude * 100.0f));
			}
			else
			{
				return FText::FromString(FString::Printf(TEXT("%.0f%%"), Magnitude * 100.0f));
			}
		}
	}

	return FText::GetEmpty();
}

void UHarmoniaStatusEffectComponent::CleanupExpiredEffects()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
	{
		// Only cleanup custom effects (GAS effects are handled by OnGameplayEffectRemoved)
		if (!ActiveEffects[i].ActiveHandle.IsValid() &&
			ActiveEffects[i].bHasDuration &&
			ActiveEffects[i].RemainingDuration <= 0.0f)
		{
			FGuid RemovedID = ActiveEffects[i].EffectID;
			ActiveEffects.RemoveAt(i);
			WarnedExpiringEffects.Remove(RemovedID);
			OnEffectRemoved.Broadcast(RemovedID);
		}
	}
}
