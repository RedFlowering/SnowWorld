// Copyright 2024 Snow Game Studio.

#include "BossHealthBarWidget.h"
#include "Character/BossCharacter.h"
#include "Character/LyraHealthComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetMathLibrary.h"

UBossHealthBarWidget::UBossHealthBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVisible = false;
	CurrentDisplayedHealth = 1.0f;
	TargetHealth = 1.0f;

	// Default colors
	FullHealthColor = FLinearColor::Green;
	MediumHealthColor = FLinearColor::Yellow;
	LowHealthColor = FLinearColor::Red;

	MediumHealthThreshold = 0.5f;
	LowHealthThreshold = 0.25f;

	bShowHealthPercentage = true;
	bShowPhaseIndicator = true;
	bSmoothHealthBarTransition = true;
	HealthBarInterpolationSpeed = 5.0f;
}

void UBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BossCharacter)
	{
		BindToBoss();
	}
}

void UBossHealthBarWidget::NativeDestruct()
{
	UnbindFromBoss();

	Super::NativeDestruct();
}

void UBossHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Smooth health bar interpolation
	if (bSmoothHealthBarTransition && FMath::Abs(CurrentDisplayedHealth - TargetHealth) > 0.001f)
	{
		CurrentDisplayedHealth = FMath::FInterpTo(
			CurrentDisplayedHealth,
			TargetHealth,
			InDeltaTime,
			HealthBarInterpolationSpeed
		);

		if (HealthBar)
		{
			HealthBar->SetPercent(CurrentDisplayedHealth);
			HealthBar->SetFillColorAndOpacity(GetHealthBarColor(CurrentDisplayedHealth));
		}

		// Update percentage text
		if (bShowHealthPercentage && HealthPercentText)
		{
			int32 HealthPercent = FMath::RoundToInt(CurrentDisplayedHealth * 100.0f);
			HealthPercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), HealthPercent)));
		}
	}
}

//~=============================================================================
// Boss Binding
//~=============================================================================

void UBossHealthBarWidget::SetBossCharacter(ABossCharacter* InBossCharacter)
{
	if (BossCharacter == InBossCharacter)
	{
		return;
	}

	UnbindFromBoss();

	BossCharacter = InBossCharacter;

	if (BossCharacter && IsInViewport())
	{
		BindToBoss();
	}
}

//~=============================================================================
// Display Control
//~=============================================================================

void UBossHealthBarWidget::ShowBossHealthBar_Implementation()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);

	// Update initial display
	if (BossCharacter)
	{
		if (ULyraHealthComponent* HealthComp = ULyraHealthComponent::FindHealthComponent(BossCharacter))
		{
			float HealthPercent = HealthComp->GetHealthNormalized();
			TargetHealth = HealthPercent;
			CurrentDisplayedHealth = HealthPercent;
			UpdateHealthBar(HealthPercent);
		}

		UpdateBossName(BossCharacter->BossName, BossCharacter->BossTitle);
		UpdatePhaseDisplay(BossCharacter->GetCurrentPhase(), BossCharacter->GetMaxPhases());
	}
}

void UBossHealthBarWidget::HideBossHealthBar_Implementation()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

//~=============================================================================
// Update Functions
//~=============================================================================

void UBossHealthBarWidget::UpdateHealthBar_Implementation(float HealthPercent)
{
	TargetHealth = FMath::Clamp(HealthPercent, 0.0f, 1.0f);

	if (!bSmoothHealthBarTransition)
	{
		CurrentDisplayedHealth = TargetHealth;
	}

	if (HealthBar)
	{
		if (!bSmoothHealthBarTransition)
		{
			HealthBar->SetPercent(TargetHealth);
			HealthBar->SetFillColorAndOpacity(GetHealthBarColor(TargetHealth));
		}
	}

	// Update percentage text
	if (bShowHealthPercentage && HealthPercentText)
	{
		int32 HealthPercentInt = FMath::RoundToInt(TargetHealth * 100.0f);
		HealthPercentText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), HealthPercentInt)));
	}
}

void UBossHealthBarWidget::UpdateBossName_Implementation(const FText& InBossName, const FText& InBossTitle)
{
	if (BossNameText)
	{
		BossNameText->SetText(InBossName);
	}

	if (BossTitleText)
	{
		BossTitleText->SetText(InBossTitle);
	}
}

void UBossHealthBarWidget::UpdatePhaseDisplay_Implementation(int32 CurrentPhase, int32 MaxPhases)
{
	if (!bShowPhaseIndicator || !PhaseText)
	{
		return;
	}

	FText PhaseDisplayText = FText::FromString(FString::Printf(TEXT("Phase %d/%d"), CurrentPhase + 1, MaxPhases));
	PhaseText->SetText(PhaseDisplayText);
}

//~=============================================================================
// Event Handlers
//~=============================================================================

void UBossHealthBarWidget::OnBossHealthChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	if (!HealthComponent)
	{
		return;
	}

	float HealthPercent = HealthComponent->GetHealthNormalized();
	UpdateHealthBar(HealthPercent);

	// Play damage animation if health decreased
	if (NewValue < OldValue)
	{
		float DamagePercent = (OldValue - NewValue) / HealthComponent->GetMaxHealth();
		PlayDamageTakenAnimation(DamagePercent);
	}
}

void UBossHealthBarWidget::OnBossPhaseChanged(int32 OldPhase, int32 NewPhase)
{
	if (!BossCharacter)
	{
		return;
	}

	UpdatePhaseDisplay(NewPhase, BossCharacter->GetMaxPhases());
	PlayPhaseTransitionAnimation(NewPhase);
}

void UBossHealthBarWidget::OnBossEncounterStart(ABossCharacter* Boss)
{
	ShowBossHealthBar();
}

void UBossHealthBarWidget::OnBossEncounterEnd(ABossCharacter* Boss, bool bDefeated)
{
	if (bDefeated)
	{
		// Could add a delay before hiding
		HideBossHealthBar();
	}
}

//~=============================================================================
// Animation Events
//~=============================================================================

void UBossHealthBarWidget::PlayPhaseTransitionAnimation_Implementation(int32 NewPhase)
{
	// Blueprint can implement phase transition animations
}

void UBossHealthBarWidget::PlayDamageTakenAnimation_Implementation(float DamagePercent)
{
	// Blueprint can implement damage flash/shake animations
}

//~=============================================================================
// Private Functions
//~=============================================================================

void UBossHealthBarWidget::UnbindFromBoss()
{
	if (!BossCharacter)
	{
		return;
	}

	// Unbind from health component
	if (ULyraHealthComponent* HealthComp = ULyraHealthComponent::FindHealthComponent(BossCharacter))
	{
		HealthComp->OnHealthChanged.RemoveDynamic(this, &UBossHealthBarWidget::OnBossHealthChanged);
	}

	// Unbind from boss events
	BossCharacter->OnBossPhaseChanged.RemoveDynamic(this, &UBossHealthBarWidget::OnBossPhaseChanged);
	BossCharacter->OnBossEncounterStart.RemoveDynamic(this, &UBossHealthBarWidget::OnBossEncounterStart);
	BossCharacter->OnBossEncounterEnd.RemoveDynamic(this, &UBossHealthBarWidget::OnBossEncounterEnd);
}

void UBossHealthBarWidget::BindToBoss()
{
	if (!BossCharacter)
	{
		return;
	}

	// Bind to health component
	if (ULyraHealthComponent* HealthComp = ULyraHealthComponent::FindHealthComponent(BossCharacter))
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &UBossHealthBarWidget::OnBossHealthChanged);

		// Initialize health display
		float HealthPercent = HealthComp->GetHealthNormalized();
		TargetHealth = HealthPercent;
		CurrentDisplayedHealth = HealthPercent;
	}

	// Bind to boss events
	BossCharacter->OnBossPhaseChanged.AddDynamic(this, &UBossHealthBarWidget::OnBossPhaseChanged);
	BossCharacter->OnBossEncounterStart.AddDynamic(this, &UBossHealthBarWidget::OnBossEncounterStart);
	BossCharacter->OnBossEncounterEnd.AddDynamic(this, &UBossHealthBarWidget::OnBossEncounterEnd);

	// Update name and phase
	UpdateBossName(BossCharacter->BossName, BossCharacter->BossTitle);
	UpdatePhaseDisplay(BossCharacter->GetCurrentPhase(), BossCharacter->GetMaxPhases());
}

FLinearColor UBossHealthBarWidget::GetHealthBarColor(float HealthPercent) const
{
	if (HealthPercent <= LowHealthThreshold)
	{
		return LowHealthColor;
	}
	else if (HealthPercent <= MediumHealthThreshold)
	{
		// Interpolate between low and medium
		float Alpha = (HealthPercent - LowHealthThreshold) / (MediumHealthThreshold - LowHealthThreshold);
		return FMath::Lerp(LowHealthColor, MediumHealthColor, Alpha);
	}
	else
	{
		// Interpolate between medium and full
		float Alpha = (HealthPercent - MediumHealthThreshold) / (1.0f - MediumHealthThreshold);
		return FMath::Lerp(MediumHealthColor, FullHealthColor, Alpha);
	}
}
