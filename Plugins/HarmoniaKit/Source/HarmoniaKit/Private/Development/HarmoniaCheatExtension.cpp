// Copyright 2025 Snow Game Studio.

#include "Development/HarmoniaCheatExtension.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "CanvasItem.h"

UHarmoniaCheatExtension::UHarmoniaCheatExtension()
{
}

UHarmoniaCheatExtension::~UHarmoniaCheatExtension()
{
	UnregisterDebugDraw();
}

void UHarmoniaCheatExtension::RegisterDebugDraw()
{
	if (!DebugDrawDelegateHandle.IsValid())
	{
		DebugDrawDelegateHandle = UDebugDrawService::Register(
			TEXT("Game"),
			FDebugDrawDelegate::CreateUObject(this, &UHarmoniaCheatExtension::DrawDebug)
		);
	}
}

void UHarmoniaCheatExtension::UnregisterDebugDraw()
{
	if (DebugDrawDelegateHandle.IsValid())
	{
		UDebugDrawService::Unregister(DebugDrawDelegateHandle);
		DebugDrawDelegateHandle.Reset();
	}
}

void UHarmoniaCheatExtension::HarmoniaDebug(bool bEnable)
{
	bShowAll = bEnable;
	bShowStamina = bEnable;
	bShowTags = bEnable;
	bShowCombat = bEnable;

	if (bEnable)
	{
		RegisterDebugDraw();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Harmonia Debug: ON (use 'showdebug harmonia' style)"));
		}
	}
	else
	{
		UnregisterDebugDraw();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Harmonia Debug: OFF"));
		}
	}
}

void UHarmoniaCheatExtension::HarmoniaStamina()
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Ability System Component found on Player Pawn"));
		return;
	}

	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet)
	{
		float Stamina = AttributeSet->GetStamina();
		float MaxStamina = AttributeSet->GetMaxStamina();
		FString Msg = FString::Printf(TEXT("Stamina: %.1f / %.1f"), Stamina, MaxStamina);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Msg);
		}
		UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
	}
}

void UHarmoniaCheatExtension::HarmoniaTags()
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Ability System Component found on Player Pawn"));
		return;
	}

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Active Tags:"));
		for (const FGameplayTag& Tag : OwnedTags)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("  - %s"), *Tag.ToString()));
		}
	}
}

void UHarmoniaCheatExtension::DrawDebug(UCanvas* Canvas, APlayerController* PC)
{
	if (!Canvas || !PC) return;
	if (!bShowAll && !bShowStamina && !bShowTags && !bShowCombat) return;

	// Starting position
	float YPos = 50.0f;
	const float XPos = 20.0f;
	const float LineHeight = 18.0f;

	// Draw header
	UFont* Font = GEngine->GetMediumFont();
	Canvas->SetDrawColor(FColor::Cyan);
	Canvas->DrawText(Font, TEXT("=== HARMONIA DEBUG ==="), XPos, YPos);
	YPos += LineHeight * 1.5f;

	if (bShowStamina || bShowAll)
	{
		DrawStaminaDebug(Canvas, YPos);
	}

	if (bShowCombat || bShowAll)
	{
		DrawCombatDebug(Canvas, YPos);
	}

	if (bShowTags || bShowAll)
	{
		DrawTagsDebug(Canvas, YPos);
	}
}

void UHarmoniaCheatExtension::DrawStaminaDebug(UCanvas* Canvas, float& YPos)
{
	if (!Canvas) return;
	
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	if (!ASC) return;

	const float XPos = 20.0f;
	const float LineHeight = 18.0f;
	UFont* Font = GEngine->GetSmallFont();

	// Section header
	Canvas->SetDrawColor(FColor::Green);
	Canvas->DrawText(Font, TEXT("[Stamina]"), XPos, YPos);
	YPos += LineHeight;

	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet)
	{
		float Stamina = AttributeSet->GetStamina();
		float MaxStamina = AttributeSet->GetMaxStamina();
		float Percent = MaxStamina > 0 ? (Stamina / MaxStamina) * 100.0f : 0.0f;

		// Color based on stamina level
		FColor StaminaColor = Stamina < 20.0f ? FColor::Red : (Stamina < 50.0f ? FColor::Yellow : FColor::Green);
		Canvas->SetDrawColor(StaminaColor);
		
		Canvas->DrawText(Font, FString::Printf(TEXT("  %.0f / %.0f (%.0f%%)"), Stamina, MaxStamina, Percent), XPos, YPos);
		YPos += LineHeight;

		// Draw stamina bar
		const float BarWidth = 150.0f;
		const float BarHeight = 10.0f;
		const float BarX = XPos + 10.0f;
		
		// Background
		Canvas->SetDrawColor(FColor(40, 40, 40, 200));
		Canvas->DrawTile(Canvas->DefaultTexture, BarX, YPos, BarWidth, BarHeight, 0, 0, 1, 1);
		
		// Foreground
		Canvas->SetDrawColor(StaminaColor);
		Canvas->DrawTile(Canvas->DefaultTexture, BarX, YPos, BarWidth * (Percent / 100.0f), BarHeight, 0, 0, 1, 1);
		
		YPos += LineHeight;
	}
	
	YPos += 5.0f; // Spacing
}

void UHarmoniaCheatExtension::DrawCombatDebug(UCanvas* Canvas, float& YPos)
{
	if (!Canvas) return;
	
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	if (!ASC) return;

	const float XPos = 20.0f;
	const float LineHeight = 18.0f;
	UFont* Font = GEngine->GetSmallFont();

	// Section header
	Canvas->SetDrawColor(FColor::Orange);
	Canvas->DrawText(Font, TEXT("[Combat State]"), XPos, YPos);
	YPos += LineHeight;

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	// Check common combat states
	auto CheckAndDisplayTag = [&](const FName& TagName, const TCHAR* DisplayName)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
		if (Tag.IsValid() && OwnedTags.HasTag(Tag))
		{
			Canvas->SetDrawColor(FColor::Yellow);
			Canvas->DrawText(Font, FString::Printf(TEXT("  [ACTIVE] %s"), DisplayName), XPos, YPos);
			YPos += LineHeight;
		}
	};

	CheckAndDisplayTag(FName("Character.State.Blocking"), TEXT("Blocking"));
	CheckAndDisplayTag(FName("Character.State.Attacking"), TEXT("Attacking"));
	CheckAndDisplayTag(FName("Character.State.Sprinting"), TEXT("Sprinting"));
	CheckAndDisplayTag(FName("State.InAir"), TEXT("In Air"));
	CheckAndDisplayTag(FName("Debuff.StaminaRecoveryBlocked"), TEXT("Stamina Recovery Blocked"));
	CheckAndDisplayTag(FName("Ability.Cooldown.Block"), TEXT("Block on Cooldown"));

	YPos += 5.0f; // Spacing
}

void UHarmoniaCheatExtension::DrawTagsDebug(UCanvas* Canvas, float& YPos)
{
	if (!Canvas) return;
	
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	if (!ASC) return;

	const float XPos = 20.0f;
	const float LineHeight = 16.0f;
	UFont* Font = GEngine->GetSmallFont();

	// Section header
	Canvas->SetDrawColor(FColor::Magenta);
	Canvas->DrawText(Font, TEXT("[All Active Tags]"), XPos, YPos);
	YPos += LineHeight;

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	Canvas->SetDrawColor(FColor::White);
	if (OwnedTags.Num() == 0)
	{
		Canvas->DrawText(Font, TEXT("  (none)"), XPos, YPos);
		YPos += LineHeight;
	}
	else
	{
		for (const FGameplayTag& Tag : OwnedTags)
		{
			Canvas->DrawText(Font, FString::Printf(TEXT("  - %s"), *Tag.ToString()), XPos, YPos);
			YPos += LineHeight;
		}
	}
}
