// Copyright 2025 Snow Game Studio.

#include "UI/LifeContent/HarmoniaMusicWidget.h"
#include "Components/HarmoniaMusicComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/UniformGridPanel.h"

UHarmoniaMusicWidget::UHarmoniaMusicWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaMusicWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EndRhythmGame();
	HidePerformanceResult();
}

void UHarmoniaMusicWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsRhythmGameActive && SongProgressBar)
	{
		const float ElapsedTime = GetWorld()->GetTimeSeconds() - RhythmGameStartTime;
		const float Progress = RhythmGameDuration > 0.0f ? ElapsedTime / RhythmGameDuration : 0.0f;
		SongProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

void UHarmoniaMusicWidget::SetMusicComponent(UHarmoniaMusicComponent* InComponent)
{
	MusicComponent = InComponent;
	SetLifeContentComponent(InComponent);
}

void UHarmoniaMusicWidget::RefreshDisplay()
{
	Super::RefreshDisplay();
	RefreshMusicSheetList();
}

void UHarmoniaMusicWidget::UpdateInstrumentDisplay(EInstrumentType InstrumentType, int32 InstrumentTier,
												   float Durability, float MaxDurability)
{
	if (InstrumentNameText)
	{
		InstrumentNameText->SetText(GetInstrumentTypeName(InstrumentType));
	}

	if (InstrumentTierText)
	{
		InstrumentTierText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaMusic", "InstrumentTier", "?±Í∏â {0}"),
			FText::AsNumber(InstrumentTier)));
	}

	if (InstrumentDurabilityBar)
	{
		const float DurabilityPercent = MaxDurability > 0.0f ? Durability / MaxDurability : 0.0f;
		InstrumentDurabilityBar->SetPercent(DurabilityPercent);

		if (DurabilityPercent > 0.5f)
		{
			InstrumentDurabilityBar->SetFillColorAndOpacity(FLinearColor::Green);
		}
		else if (DurabilityPercent > 0.25f)
		{
			InstrumentDurabilityBar->SetFillColorAndOpacity(FLinearColor::Yellow);
		}
		else
		{
			InstrumentDurabilityBar->SetFillColorAndOpacity(FLinearColor::Red);
		}
	}
}

void UHarmoniaMusicWidget::StartRhythmGame(const FText& SongName, EMusicGenre Genre, float Duration)
{
	bIsRhythmGameActive = true;
	RhythmGameStartTime = GetWorld()->GetTimeSeconds();
	RhythmGameDuration = Duration;

	if (RhythmGamePanel)
	{
		RhythmGamePanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (SongNameText)
	{
		SongNameText->SetText(SongName);
	}

	if (GenreText)
	{
		GenreText->SetText(GetGenreText(Genre));
	}

	if (SongProgressBar)
	{
		SongProgressBar->SetPercent(0.0f);
	}

	// Reset score display
	UpdatePerformanceScore(0, 0, EPerformanceQuality::Poor);
}

void UHarmoniaMusicWidget::UpdateRhythmNotes(const TArray<float>& NotePositions)
{
	// Note track visualization would be updated here
	// Actual implementation would create/update note widgets based on positions
}

void UHarmoniaMusicWidget::ShowNoteHitResult(int32 NoteIndex, bool bHit, bool bPerfect)
{
	if (HitResultWidget)
	{
		HitResultWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (HitResultText)
	{
		if (bPerfect)
		{
			HitResultText->SetText(NSLOCTEXT("HarmoniaMusic", "HitPerfect", "PERFECT!"));
			HitResultText->SetColorAndOpacity(FLinearColor(1.0f, 0.84f, 0.0f)); // Gold
		}
		else if (bHit)
		{
			HitResultText->SetText(NSLOCTEXT("HarmoniaMusic", "HitGood", "GOOD"));
			HitResultText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			HitResultText->SetText(NSLOCTEXT("HarmoniaMusic", "HitMiss", "MISS"));
			HitResultText->SetColorAndOpacity(FLinearColor::Red);
		}
	}

	OnRhythmNoteHit.Broadcast(NoteIndex, bPerfect);
}

void UHarmoniaMusicWidget::EndRhythmGame()
{
	bIsRhythmGameActive = false;

	if (RhythmGamePanel)
	{
		RhythmGamePanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (HitResultWidget)
	{
		HitResultWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHarmoniaMusicWidget::UpdatePerformanceScore(int32 Score, int32 Combo, EPerformanceQuality Quality)
{
	if (ScoreText)
	{
		ScoreText->SetText(FText::AsNumber(Score));
	}

	if (ComboText)
	{
		if (Combo > 1)
		{
			ComboText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaMusic", "ComboFormat", "{0} COMBO"),
				FText::AsNumber(Combo)));
			ComboText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ComboText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (QualityText)
	{
		QualityText->SetText(GetQualityText(Quality));
		QualityText->SetColorAndOpacity(GetQualityColor(Quality));
	}
}

void UHarmoniaMusicWidget::ShowPerformanceResult(EPerformanceQuality Quality, int32 FinalScore,
												 int32 MaxCombo, const FMusicBuffEffect& BuffApplied)
{
	EndRhythmGame();

	if (PerformanceResultPanel)
	{
		PerformanceResultPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (FinalScoreText)
	{
		FinalScoreText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaMusic", "FinalScore", "?êÏàò: {0}"),
			FText::AsNumber(FinalScore)));
	}

	if (MaxComboText)
	{
		MaxComboText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaMusic", "MaxCombo", "ÏµúÎ? ÏΩ§Î≥¥: {0}"),
			FText::AsNumber(MaxCombo)));
	}

	if (FinalQualityText)
	{
		FinalQualityText->SetText(GetQualityText(Quality));
		FinalQualityText->SetColorAndOpacity(GetQualityColor(Quality));
	}

	if (BuffAppliedText)
	{
		BuffAppliedText->SetText(FormatBuffEffects(BuffApplied));
	}
}

void UHarmoniaMusicWidget::HidePerformanceResult()
{
	if (PerformanceResultPanel)
	{
		PerformanceResultPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaMusicWidget::RefreshMusicSheetList()
{
	// Music sheet list would be populated here using MusicComponent's learned sheets
}

void UHarmoniaMusicWidget::ShowMusicSheetDetails(FName MusicID, const FText& MusicName, EMusicGenre Genre,
												 int32 Difficulty, const FMusicBuffEffect& BuffEffect)
{
	if (MusicSheetDetailsPanel)
	{
		MusicSheetDetailsPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (SheetNameText)
	{
		SheetNameText->SetText(MusicName);
	}

	if (SheetGenreText)
	{
		SheetGenreText->SetText(GetGenreText(Genre));
	}

	if (SheetDifficultyText)
	{
		SheetDifficultyText->SetText(GetDifficultyText(Difficulty));
	}

	if (SheetBuffText)
	{
		SheetBuffText->SetText(FormatBuffEffects(BuffEffect));
	}
}

FText UHarmoniaMusicWidget::GetInstrumentTypeName(EInstrumentType Type) const
{
	switch (Type)
	{
	case EInstrumentType::Lute:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentLute", "Î•òÌä∏");
	case EInstrumentType::Flute:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentFlute", "?åÎ£®??);
	case EInstrumentType::Harp:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentHarp", "?òÌîÑ");
	case EInstrumentType::Drum:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentDrum", "?úÎüº");
	case EInstrumentType::Horn:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentHorn", "?∏Î•∏");
	case EInstrumentType::Violin:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentViolin", "Î∞îÏù¥?¨Î¶∞");
	case EInstrumentType::Piano:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentPiano", "?ºÏïÑ??);
	case EInstrumentType::Guitar:
		return NSLOCTEXT("HarmoniaMusic", "InstrumentGuitar", "Í∏∞Ì?");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaMusicWidget::GetGenreText(EMusicGenre Genre) const
{
	switch (Genre)
	{
	case EMusicGenre::Classical:
		return NSLOCTEXT("HarmoniaMusic", "GenreClassical", "?¥Îûò??);
	case EMusicGenre::Folk:
		return NSLOCTEXT("HarmoniaMusic", "GenreFolk", "ÎØºÏÜç");
	case EMusicGenre::Battle:
		return NSLOCTEXT("HarmoniaMusic", "GenreBattle", "?ÑÌà¨");
	case EMusicGenre::Healing:
		return NSLOCTEXT("HarmoniaMusic", "GenreHealing", "ÏπòÏú†");
	case EMusicGenre::Buff:
		return NSLOCTEXT("HarmoniaMusic", "GenreBuff", "Î≤ÑÌîÑ");
	case EMusicGenre::Debuff:
		return NSLOCTEXT("HarmoniaMusic", "GenreDebuff", "?îÎ≤Ñ??);
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaMusicWidget::GetQualityText(EPerformanceQuality Quality) const
{
	switch (Quality)
	{
	case EPerformanceQuality::Poor:
		return NSLOCTEXT("HarmoniaMusic", "QualityPoor", "Î∂ÄÏ°?);
	case EPerformanceQuality::Fair:
		return NSLOCTEXT("HarmoniaMusic", "QualityFair", "Î≥¥ÌÜµ");
	case EPerformanceQuality::Good:
		return NSLOCTEXT("HarmoniaMusic", "QualityGood", "Ï¢ãÏùå");
	case EPerformanceQuality::Great:
		return NSLOCTEXT("HarmoniaMusic", "QualityGreat", "?åÎ???);
	case EPerformanceQuality::Perfect:
		return NSLOCTEXT("HarmoniaMusic", "QualityPerfect", "?ÑÎ≤Ω");
	case EPerformanceQuality::Legendary:
		return NSLOCTEXT("HarmoniaMusic", "QualityLegendary", "?ÑÏÑ§");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaMusicWidget::GetQualityColor(EPerformanceQuality Quality) const
{
	switch (Quality)
	{
	case EPerformanceQuality::Poor:
		return FLinearColor::Gray;
	case EPerformanceQuality::Fair:
		return FLinearColor::White;
	case EPerformanceQuality::Good:
		return FLinearColor::Green;
	case EPerformanceQuality::Great:
		return FLinearColor(0.0f, 0.5f, 1.0f);
	case EPerformanceQuality::Perfect:
		return FLinearColor(0.5f, 0.0f, 0.5f);
	case EPerformanceQuality::Legendary:
		return FLinearColor(1.0f, 0.84f, 0.0f);
	default:
		return FLinearColor::White;
	}
}

FText UHarmoniaMusicWidget::GetDifficultyText(int32 Difficulty) const
{
	if (Difficulty <= 2)
	{
		return NSLOCTEXT("HarmoniaMusic", "DifficultyEasy", "?¨Ï?");
	}
	else if (Difficulty <= 4)
	{
		return NSLOCTEXT("HarmoniaMusic", "DifficultyNormal", "Î≥¥ÌÜµ");
	}
	else if (Difficulty <= 6)
	{
		return NSLOCTEXT("HarmoniaMusic", "DifficultyHard", "?¥Î†§?Ä");
	}
	else if (Difficulty <= 8)
	{
		return NSLOCTEXT("HarmoniaMusic", "DifficultyExpert", "?ÑÎ¨∏Í∞Ä");
	}
	else
	{
		return NSLOCTEXT("HarmoniaMusic", "DifficultyMaster", "ÎßàÏä§??);
	}
}

FText UHarmoniaMusicWidget::FormatBuffEffects(const FMusicBuffEffect& BuffEffect) const
{
	TArray<FString> Effects;

	if (BuffEffect.HealthRegenPerSecond > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("Ï≤¥Î†• ?¨ÏÉù +%.1f/Ï¥?), BuffEffect.HealthRegenPerSecond));
	}
	if (BuffEffect.ManaRegenPerSecond > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("ÎßàÎÇò ?¨ÏÉù +%.1f/Ï¥?), BuffEffect.ManaRegenPerSecond));
	}
	if (BuffEffect.StaminaRegenPerSecond > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("?§ÌÉúÎØ∏ÎÇò ?¨ÏÉù +%.1f/Ï¥?), BuffEffect.StaminaRegenPerSecond));
	}
	if (BuffEffect.AttackBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("Í≥µÍ≤©??+%.0f%%"), BuffEffect.AttackBonus));
	}
	if (BuffEffect.DefenseBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("Î∞©Ïñ¥??+%.0f%%"), BuffEffect.DefenseBonus));
	}
	if (BuffEffect.SpeedBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("?¥Îèô?çÎèÑ +%.0f%%"), BuffEffect.SpeedBonus));
	}
	if (BuffEffect.CriticalChanceBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("?¨Î¶¨?∞Ïª¨ +%.0f%%"), BuffEffect.CriticalChanceBonus));
	}
	if (BuffEffect.CooldownReduction > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("Ïø®Îã§??Í∞êÏÜå %.0f%%"), BuffEffect.CooldownReduction));
	}

	if (Effects.Num() == 0)
	{
		return NSLOCTEXT("HarmoniaMusic", "NoBuffEffect", "?®Í≥º ?ÜÏùå");
	}

	return FText::FromString(FString::Join(Effects, TEXT("\n")));
}
