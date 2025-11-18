// Copyright 2024 Snow Game Studio.

#include "UI/HarmoniaBossHealthBarWidget.h"
#include "Monsters/HarmoniaBossMonster.h"
#include "Character/LyraHealthComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetMathLibrary.h"

UHarmoniaBossHealthBarWidget::UHarmoniaBossHealthBarWidget(const FObjectInitializer& ObjectInitializer)
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

void UHarmoniaBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BossCharacter)
	{
		BindToBoss();
	}
}

void UHarmoniaBossHealthBarWidget::NativeDestruct()
{
	UnbindFromBoss();

	Super::NativeDestruct();
}

void UHarmoniaBossHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

void UHarmoniaBossHealthBarWidget::SetBossCharacter(AHarmoniaBossMonster* InBossCharacter)
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

void UHarmoniaBossHealthBarWidget::ShowBossHealthBar_Implementation()
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

		UpdateBossName(BossCharacter->MonsterData ? BossCharacter->MonsterData->MonsterName : FText(), BossCharacter->BossTitle);
		UpdatePhaseDisplay(BossCharacter->GetCurrentPhase(), BossCharacter->GetTotalPhases());
	}
}

void UHarmoniaBossHealthBarWidget::HideBossHealthBar_Implementation()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

//~=============================================================================
// Update Functions
//~=============================================================================

void UHarmoniaBossHealthBarWidget::UpdateHealthBar_Implementation(float HealthPercent)
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

void UHarmoniaBossHealthBarWidget::UpdateBossName_Implementation(const FText& InBossName, const FText& InBossTitle)
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

void UHarmoniaBossHealthBarWidget::UpdatePhaseDisplay_Implementation(int32 CurrentPhase, int32 MaxPhases)
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

void UHarmoniaBossHealthBarWidget::OnBossHealthChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator)
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

void UHarmoniaBossHealthBarWidget::OnBossPhaseChanged(int32 OldPhase, int32 NewPhase, const FHarmoniaBossPhase& PhaseData)
{
	if (!BossCharacter)
	{
		return;
	}

	UpdatePhaseDisplay(NewPhase, BossCharacter->GetTotalPhases());
	PlayPhaseTransitionAnimation(NewPhase);
}

//~=============================================================================
// Animation Events
//~=============================================================================

void UHarmoniaBossHealthBarWidget::PlayPhaseTransitionAnimation_Implementation(int32 NewPhase)
{
	// Blueprint can implement phase transition animations
}

void UHarmoniaBossHealthBarWidget::PlayDamageTakenAnimation_Implementation(float DamagePercent)
{
	// Blueprint can implement damage flash/shake animations
}

//~=============================================================================
// Private Functions
//~=============================================================================

void UHarmoniaBossHealthBarWidget::UnbindFromBoss()
{
	if (!BossCharacter)
	{
		return;
	}

	// Unbind from health component
	if (ULyraHealthComponent* HealthComp = ULyraHealthComponent::FindHealthComponent(BossCharacter))
	{
		HealthComp->OnHealthChanged.RemoveDynamic(this, &UHarmoniaBossHealthBarWidget::OnBossHealthChanged);
	}

	// Unbind from boss events
	BossCharacter->OnBossPhaseChanged.RemoveDynamic(this, &UHarmoniaBossHealthBarWidget::OnBossPhaseChanged);
}

void UHarmoniaBossHealthBarWidget::BindToBoss()
{
	if (!BossCharacter)
	{
		return;
	}

	// Bind to health component
	if (ULyraHealthComponent* HealthComp = ULyraHealthComponent::FindHealthComponent(BossCharacter))
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &UHarmoniaBossHealthBarWidget::OnBossHealthChanged);

		// Initialize health display
		float HealthPercent = HealthComp->GetHealthNormalized();
		TargetHealth = HealthPercent;
		CurrentDisplayedHealth = HealthPercent;
	}

	// Bind to boss events
	BossCharacter->OnBossPhaseChanged.AddDynamic(this, &UHarmoniaBossHealthBarWidget::OnBossPhaseChanged);

	// Update name and phase
	UpdateBossName(BossCharacter->MonsterData ? BossCharacter->MonsterData->MonsterName : FText(), BossCharacter->BossTitle);
	UpdatePhaseDisplay(BossCharacter->GetCurrentPhase(), BossCharacter->GetTotalPhases());
}

FLinearColor UHarmoniaBossHealthBarWidget::GetHealthBarColor(float HealthPercent) const
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
