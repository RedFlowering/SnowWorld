// Copyright 2025 Snow Game Studio.
// HarmoniaCharacter Debug implementation - ALS-style showdebug support

#include "Core/HarmoniaCharacter.h"

#if !UE_BUILD_SHIPPING

#include "DisplayDebugHelpers.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameplayTagContainer.h"
#include "CanvasItem.h"

// Debug display category names
namespace HarmoniaDebugDisplayNames
{
	static const FName StatDebugDisplayName{TEXTVIEW("Harmonia.Stat")};
	static const FName CombatDebugDisplayName{TEXTVIEW("Harmonia.Combat")};
	static const FName TagsDebugDisplayName{TEXTVIEW("Harmonia.Tags")};
}

void AHarmoniaCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	const auto Scale{
		FMath::Min(static_cast<float>(Canvas->SizeX) / (1280.0f * Canvas->GetDPIScale()),
		           static_cast<float>(Canvas->SizeY) / (720.0f * Canvas->GetDPIScale()))
	};

	const auto RowOffset{12.0f * Scale};
	const auto ColumnOffset{200.0f * Scale};

	auto MaxVerticalLocation{YPos};
	auto HorizontalLocation{5.0f * Scale};

	// Check if any Harmonia debug display is enabled
	const bool bShowStats = DisplayInfo.IsDisplayOn(HarmoniaDebugDisplayNames::StatDebugDisplayName);
	const bool bShowCombat = DisplayInfo.IsDisplayOn(HarmoniaDebugDisplayNames::CombatDebugDisplayName);
	const bool bShowTags = DisplayInfo.IsDisplayOn(HarmoniaDebugDisplayNames::TagsDebugDisplayName);

	if (!bShowStats && !bShowCombat && !bShowTags)
	{
		// No Harmonia debug enabled, call parent
		Super::DisplayDebug(Canvas, DisplayInfo, YL, YPos);
		return;
	}

	// Header
	static const FText DebugModeHeaderText = FText::FromString(TEXT("=== HARMONIA DEBUG ==="));
	DisplayDebugHeader(Canvas, DebugModeHeaderText, FLinearColor::Green, Scale, HorizontalLocation, YPos);
	YPos += RowOffset;

	const auto InitialVerticalLocation{YPos};

	// Stats Debug (all attributes)
	static const FText StatsHeaderText = FText::FromString(TEXT("Harmonia.Stat (showdebug harmonia.stat)"));
	if (bShowStats)
	{
		DisplayDebugHeader(Canvas, StatsHeaderText, FLinearColor::Green, Scale, HorizontalLocation, YPos);
		DisplayDebugStats(Canvas, Scale, HorizontalLocation, YPos);
		YPos += RowOffset;
	}
	else
	{
		DisplayDebugHeader(Canvas, StatsHeaderText, FLinearColor{0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, YPos);
	}

	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, YPos);

	// Combat Debug
	static const FText CombatHeaderText = FText::FromString(TEXT("Harmonia.Combat (showdebug harmonia.combat)"));
	if (bShowCombat)
	{
		DisplayDebugHeader(Canvas, CombatHeaderText, FLinearColor::Green, Scale, HorizontalLocation, YPos);
		DisplayDebugCombat(Canvas, Scale, HorizontalLocation, YPos);
		YPos += RowOffset;
	}
	else
	{
		DisplayDebugHeader(Canvas, CombatHeaderText, FLinearColor{0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, YPos);
	}

	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, YPos);

	// Tags Debug
	static const FText TagsHeaderText = FText::FromString(TEXT("Harmonia.Tags (showdebug harmonia.tags)"));
	if (bShowTags)
	{
		DisplayDebugHeader(Canvas, TagsHeaderText, FLinearColor::Green, Scale, HorizontalLocation, YPos);
		DisplayDebugTags(Canvas, Scale, HorizontalLocation, YPos);
		YPos += RowOffset;
	}
	else
	{
		DisplayDebugHeader(Canvas, TagsHeaderText, FLinearColor{0.0f, 0.333333f, 0.0f}, Scale, HorizontalLocation, YPos);
	}

	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, YPos);
	YPos = MaxVerticalLocation;

	Super::DisplayDebug(Canvas, DisplayInfo, YL, YPos);
}

void AHarmoniaCharacter::DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor,
                                            float Scale, float HorizontalLocation, float& VerticalLocation) const
{
	FCanvasTextItem Text{
		{HorizontalLocation, VerticalLocation},
		HeaderText,
		GEngine->GetMediumFont(),
		HeaderColor
	};

	Text.Scale = {Scale, Scale};
	Text.EnableShadow(FLinearColor::Black);

	Text.Draw(Canvas->Canvas);

	VerticalLocation += 15.0f * Scale;
}

void AHarmoniaCharacter::DisplayDebugCombat(const UCanvas* Canvas, float Scale,
                                            float HorizontalLocation, float& VerticalLocation) const
{
	VerticalLocation += 4.0f * Scale;

	FCanvasTextItem Text{
		FVector2D::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetMediumFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale * 0.75f, Scale * 0.75f};
	Text.EnableShadow(FLinearColor::Black);

	const auto RowOffset{12.0f * Scale};

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this);
	if (!ASC)
	{
		Text.SetColor(FLinearColor::Red);
		Text.Text = FText::FromString(TEXT("No AbilitySystemComponent"));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
		return;
	}

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	// Define combat-related tags to check
	struct FCombatStateCheck
	{
		FName TagName;
		const TCHAR* DisplayName;
		FLinearColor ActiveColor;
	};

	static const TArray<FCombatStateCheck> CombatStates = {
		{FName("Character.State.Blocking"), TEXT("Blocking"), FLinearColor::Blue},
		{FName("Character.State.Attacking"), TEXT("Attacking"), FLinearColor::Red},
		{FName("Character.State.Sprinting"), TEXT("Sprinting"), FLinearColor(0.0f, 1.0f, 1.0f)},
		{FName("State.InAir"), TEXT("In Air"), FLinearColor::Yellow},
		{FName("State.Charging"), TEXT("Charging"), FLinearColor(1.0f, 0.0f, 1.0f)},
		{FName("Debuff.StaminaRecoveryBlocked"), TEXT("Stamina Recovery Blocked"), FLinearColor{1.0f, 0.5f, 0.0f}},
		{FName("Ability.Cooldown.Block"), TEXT("Block Cooldown"), FLinearColor::Gray},
	};

	bool bAnyStateActive = false;
	for (const auto& StateCheck : CombatStates)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(StateCheck.TagName, false);
		if (Tag.IsValid() && OwnedTags.HasTag(Tag))
		{
			Text.SetColor(StateCheck.ActiveColor);
			Text.Text = FText::FromString(FString::Printf(TEXT("[ACTIVE] %s"), StateCheck.DisplayName));
			Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
			VerticalLocation += RowOffset;
			bAnyStateActive = true;
		}
	}

	if (!bAnyStateActive)
	{
		Text.SetColor(FLinearColor::Gray);
		Text.Text = FText::FromString(TEXT("(No active combat states)"));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
	}
}

void AHarmoniaCharacter::DisplayDebugTags(const UCanvas* Canvas, float Scale,
                                          float HorizontalLocation, float& VerticalLocation) const
{
	VerticalLocation += 4.0f * Scale;

	FCanvasTextItem Text{
		FVector2D::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetMediumFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale * 0.75f, Scale * 0.75f};
	Text.EnableShadow(FLinearColor::Black);

	const auto RowOffset{12.0f * Scale};

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this);
	if (!ASC)
	{
		Text.SetColor(FLinearColor::Red);
		Text.Text = FText::FromString(TEXT("No AbilitySystemComponent"));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
		return;
	}

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	if (OwnedTags.Num() == 0)
	{
		Text.SetColor(FLinearColor::Gray);
		Text.Text = FText::FromString(TEXT("(No active tags)"));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
		return;
	}

	// Sort tags alphabetically for readability
	TArray<FGameplayTag> SortedTags;
	OwnedTags.GetGameplayTagArray(SortedTags);
	SortedTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.ToString() < B.ToString();
	});

	for (const FGameplayTag& Tag : SortedTags)
	{
		// Color code by tag category
		FLinearColor TagColor = FLinearColor::White;
		FString TagStr = Tag.ToString();
		
		if (TagStr.StartsWith(TEXT("Character"))) TagColor = FLinearColor(0.0f, 1.0f, 1.0f);
		else if (TagStr.StartsWith(TEXT("Ability"))) TagColor = FLinearColor::Green;
		else if (TagStr.StartsWith(TEXT("State"))) TagColor = FLinearColor::Yellow;
		else if (TagStr.StartsWith(TEXT("Debuff"))) TagColor = FLinearColor::Red;
		else if (TagStr.StartsWith(TEXT("Buff"))) TagColor = FLinearColor::Blue;

		Text.SetColor(TagColor);
		Text.Text = FText::FromString(FString::Printf(TEXT("- %s"), *TagStr));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
	}
}

void AHarmoniaCharacter::DisplayDebugStats(const UCanvas* Canvas, float Scale,
                                           float HorizontalLocation, float& VerticalLocation) const
{
	VerticalLocation += 4.0f * Scale;

	FCanvasTextItem Text{
		FVector2D::ZeroVector,
		FText::GetEmpty(),
		GEngine->GetMediumFont(),
		FLinearColor::White
	};

	Text.Scale = {Scale * 0.75f, Scale * 0.75f};
	Text.EnableShadow(FLinearColor::Black);

	const auto RowOffset{12.0f * Scale};
	const auto ColumnOffset{100.0f * Scale};
	const auto ValueOffset{200.0f * Scale};

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this);
	if (!ASC)
	{
		Text.SetColor(FLinearColor::Red);
		Text.Text = FText::FromString(TEXT("No AbilitySystemComponent"));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
		return;
	}

	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (!AttributeSet)
	{
		Text.SetColor(FLinearColor::Red);
		Text.Text = FText::FromString(TEXT("No HarmoniaAttributeSet"));
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});
		VerticalLocation += RowOffset;
		return;
	}

	// Helper lambda to draw an attribute with bar
	auto DrawAttributeBar = [&](const TCHAR* Label, float Current, float Max, const FLinearColor& BaseColor)
	{
		const float Percent = Max > 0 ? (Current / Max) * 100.0f : 0.0f;
		const float LowThreshold = Max * 0.2f;
		const float MedThreshold = Max * 0.5f;
		
		// Color based on percentage
		FLinearColor AttrColor = Current < LowThreshold ? FLinearColor::Red : 
		                         (Current < MedThreshold ? FLinearColor::Yellow : BaseColor);

		// Label
		Text.SetColor(FLinearColor::White);
		Text.Text = FText::FromString(Label);
		Text.Draw(Canvas->Canvas, {HorizontalLocation, VerticalLocation});

		// Value
		Text.SetColor(AttrColor);
		Text.Text = FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max));
		Text.Draw(Canvas->Canvas, {HorizontalLocation + ColumnOffset, VerticalLocation});

		// Bar
		const float BarWidth = 120.0f * Scale;
		const float BarHeight = 8.0f * Scale;
		const float BarX = HorizontalLocation + ValueOffset;

		// Background bar
		FCanvasTileItem BackgroundBar(
			FVector2D(BarX, VerticalLocation + 2.0f * Scale),
			FVector2D(BarWidth, BarHeight),
			FLinearColor(0.1f, 0.1f, 0.1f, 0.8f)
		);
		BackgroundBar.BlendMode = SE_BLEND_Translucent;
		Canvas->Canvas->DrawItem(BackgroundBar);

		// Foreground bar
		FCanvasTileItem ForegroundBar(
			FVector2D(BarX, VerticalLocation + 2.0f * Scale),
			FVector2D(BarWidth * (Percent / 100.0f), BarHeight),
			AttrColor
		);
		ForegroundBar.BlendMode = SE_BLEND_Translucent;
		Canvas->Canvas->DrawItem(ForegroundBar);

		VerticalLocation += RowOffset + 4.0f * Scale;
	};

	// Draw all attributes
	DrawAttributeBar(TEXT("Health"), AttributeSet->GetHealth(), AttributeSet->GetMaxHealth(), FLinearColor::Green);
	DrawAttributeBar(TEXT("Mana"), AttributeSet->GetMana(), AttributeSet->GetMaxMana(), FLinearColor::Blue);
	DrawAttributeBar(TEXT("Stamina"), AttributeSet->GetStamina(), AttributeSet->GetMaxStamina(), FLinearColor::Yellow);
	DrawAttributeBar(TEXT("Poise"), AttributeSet->GetPoise(), AttributeSet->GetMaxPoise(), FLinearColor(0.6f, 0.3f, 0.0f)); // Brown/Orange for Poise
	DrawAttributeBar(TEXT("Ultimate"), AttributeSet->GetUltimateGauge(), AttributeSet->GetMaxUltimateGauge(), FLinearColor(1.0f, 0.0f, 1.0f)); // Magenta for Ultimate
}

#endif // !UE_BUILD_SHIPPING
