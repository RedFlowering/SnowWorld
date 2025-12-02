// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaCookingComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

UHarmoniaCookingComponent::UHarmoniaCookingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHarmoniaCookingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ?∏Î≤§?†Î¶¨ Ïª¥Ìè¨?åÌä∏ Ï∞æÍ∏∞
	if (AActor* Owner = GetOwner())
	{
		InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
	}
}

void UHarmoniaCookingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCooking)
	{
		float ElapsedTime = GetWorld()->GetTimeSeconds() - CookingStartTime;
		if (ElapsedTime >= RequiredCookingTime)
		{
			CompleteCooking();
		}
	}
}

bool UHarmoniaCookingComponent::StartCooking(FName RecipeID)
{
	if (bIsCooking || !RecipeDatabase.Contains(RecipeID))
	{
		return false;
	}

	const FCookingRecipe& Recipe = RecipeDatabase[RecipeID];

	// ?àÎ≤® Ï≤¥ÌÅ¨
	if (CookingLevel < Recipe.MinCookingLevel)
	{
		return false;
	}

	// ?àÏãú?ºÎ? ?åÍ≥† ?àÎäîÏßÄ ?ïÏù∏
	if (Recipe.bHidden && !IsRecipeKnown(RecipeID))
	{
		return false;
	}

	// ?¨Î£å Ï≤¥ÌÅ¨
	if (!HasRequiredIngredients(Recipe))
	{
		return false;
	}

	// ?¨Î£å ?åÎπÑ
	if (!ConsumeIngredients(Recipe))
	{
		return false;
	}

	CurrentRecipeID = RecipeID;
	bIsCooking = true;
	CookingStartTime = GetWorld()->GetTimeSeconds();
	RequiredCookingTime = CalculateCookingTime(Recipe.CookingTime);

	SetComponentTickEnabled(true);

	OnCookingStarted.Broadcast(RecipeID, RequiredCookingTime);

	return true;
}

void UHarmoniaCookingComponent::CancelCooking()
{
	if (!bIsCooking)
	{
		return;
	}

	bIsCooking = false;
	CurrentRecipeID = NAME_None;
	SetComponentTickEnabled(false);

	OnCookingCancelled.Broadcast();
}

float UHarmoniaCookingComponent::GetCookingProgress() const
{
	if (!bIsCooking || RequiredCookingTime <= 0.0f)
	{
		return 0.0f;
	}

	float ElapsedTime = GetWorld()->GetTimeSeconds() - CookingStartTime;
	return FMath::Clamp(ElapsedTime / RequiredCookingTime, 0.0f, 1.0f);
}

bool UHarmoniaCookingComponent::CanCookRecipe(FName RecipeID) const
{
	if (!RecipeDatabase.Contains(RecipeID))
	{
		return false;
	}

	const FCookingRecipe& Recipe = RecipeDatabase[RecipeID];

	// ?àÎ≤® Ï≤¥ÌÅ¨
	if (CookingLevel < Recipe.MinCookingLevel)
	{
		return false;
	}

	// ?àÏãú?ºÎ? ?åÍ≥† ?àÎäîÏßÄ ?ïÏù∏
	if (Recipe.bHidden && !IsRecipeKnown(RecipeID))
	{
		return false;
	}

	// ?¨Î£å Ï≤¥ÌÅ¨
	if (!HasRequiredIngredients(Recipe))
	{
		return false;
	}

	return true;
}

bool UHarmoniaCookingComponent::ConsumeFood(FName FoodID, ECookingQuality Quality)
{
	if (!RecipeDatabase.Contains(FoodID))
	{
		return false;
	}

	const FCookingRecipe& Recipe = RecipeDatabase[FoodID];

	// ?àÏßà???∞Î•∏ Î≤ÑÌîÑ ?®Í≥º Í≥ÑÏÇ∞
	FFoodBuffEffect BuffEffect = Recipe.BaseBuffEffect;

	float QualityMultiplier = 1.0f;
	if (Recipe.QualityMultipliers.Contains(Quality))
	{
		QualityMultiplier = Recipe.QualityMultipliers[Quality];
	}

	// ?πÏÑ± Î≥¥ÎÑà???ÅÏö©
	float TotalBonus = 1.0f;
	for (const FCookingTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.BuffEffectBonus / 100.0f;
	}

	// Î≤ÑÌîÑ ?®Í≥º??Î∞∞Ïú® ?ÅÏö©
	BuffEffect.HealthRestore *= QualityMultiplier * TotalBonus;
	BuffEffect.ManaRestore *= QualityMultiplier * TotalBonus;
	BuffEffect.StaminaRestore *= QualityMultiplier * TotalBonus;
	BuffEffect.AttackBonus *= QualityMultiplier * TotalBonus;
	BuffEffect.DefenseBonus *= QualityMultiplier * TotalBonus;
	BuffEffect.SpeedBonus *= QualityMultiplier * TotalBonus;
	BuffEffect.CriticalChanceBonus *= QualityMultiplier * TotalBonus;
	BuffEffect.ExperienceBonus *= QualityMultiplier * TotalBonus;

	// ÏßÄ?çÏãúÍ∞?Î≥¥ÎÑà???ÅÏö©
	for (const FCookingTrait& Trait : ActiveTraits)
	{
		BuffEffect.Duration *= (1.0f + Trait.BuffDurationBonus / 100.0f);
	}

	ApplyBuffEffect(BuffEffect);

	OnFoodConsumed.Broadcast(FoodID, BuffEffect);

	return true;
}

bool UHarmoniaCookingComponent::HasBuff(FName BuffName) const
{
	for (const FFoodBuffEffect& Buff : ActiveBuffs)
	{
		if (Buff.BuffName == BuffName)
		{
			return true;
		}
	}
	return false;
}

void UHarmoniaCookingComponent::RemoveBuff(FName BuffName)
{
	ActiveBuffs.RemoveAll([BuffName](const FFoodBuffEffect& Buff)
	{
		return Buff.BuffName == BuffName;
	});

	if (BuffTimerHandles.Contains(BuffName))
	{
		GetWorld()->GetTimerManager().ClearTimer(BuffTimerHandles[BuffName]);
		BuffTimerHandles.Remove(BuffName);
	}
}

void UHarmoniaCookingComponent::DiscoverRecipe(FName RecipeID)
{
	if (!KnownRecipes.Contains(RecipeID))
	{
		KnownRecipes.Add(RecipeID);
		OnRecipeDiscovered.Broadcast(RecipeID);
	}
}

bool UHarmoniaCookingComponent::IsRecipeKnown(FName RecipeID) const
{
	return KnownRecipes.Contains(RecipeID);
}

bool UHarmoniaCookingComponent::GetRecipeData(FName RecipeID, FCookingRecipe& OutRecipe) const
{
	if (const FCookingRecipe* Recipe = RecipeDatabase.Find(RecipeID))
	{
		OutRecipe = *Recipe;
		return true;
	}
	return false;
}

void UHarmoniaCookingComponent::AddCookingExperience(int32 Amount)
{
	int32 ModifiedAmount = FMath::CeilToInt(Amount * ExperienceMultiplier);
	CurrentExperience += ModifiedAmount;

	CheckAndProcessLevelUp();
}

void UHarmoniaCookingComponent::AddTrait(const FCookingTrait& Trait)
{
	for (const FCookingTrait& ExistingTrait : ActiveTraits)
	{
		if (ExistingTrait.TraitName == Trait.TraitName)
		{
			return;
		}
	}

	ActiveTraits.Add(Trait);
}

void UHarmoniaCookingComponent::RemoveTrait(FName TraitName)
{
	ActiveTraits.RemoveAll([TraitName](const FCookingTrait& Trait)
	{
		return Trait.TraitName == TraitName;
	});
}

float UHarmoniaCookingComponent::GetTotalCookingSpeedBonus() const
{
	float TotalBonus = 0.0f;
	for (const FCookingTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.CookingSpeedBonus;
	}
	return TotalBonus;
}

float UHarmoniaCookingComponent::GetTotalSuccessRateBonus() const
{
	float TotalBonus = 0.0f;
	for (const FCookingTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.SuccessRateBonus;
	}
	return TotalBonus;
}

float UHarmoniaCookingComponent::GetTotalQualityBonus() const
{
	float TotalBonus = 0.0f;
	for (const FCookingTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.QualityBonus;
	}
	return TotalBonus;
}

void UHarmoniaCookingComponent::CompleteCooking()
{
	if (!RecipeDatabase.Contains(CurrentRecipeID))
	{
		CancelCooking();
		return;
	}

	const FCookingRecipe& Recipe = RecipeDatabase[CurrentRecipeID];

	// ?îÎ¶¨ Í≤∞Í≥º Í≥ÑÏÇ∞
	FCookingResult Result = CalculateCookingResult(Recipe);

	// Í≤ΩÌóòÏπ??çÎìù
	if (Result.bSuccess)
	{
		AddCookingExperience(Result.Experience);

		// ?àÏãú?ºÎ? Ï≤òÏùå ÎßåÎì† Í≤ΩÏö∞ ?ÑÍ∞ê??Ï∂îÍ?
		DiscoverRecipe(CurrentRecipeID);
	}

	OnCookingCompleted.Broadcast(Result);

	// ?îÎ¶¨ Ï¢ÖÎ£å
	bIsCooking = false;
	CurrentRecipeID = NAME_None;
	SetComponentTickEnabled(false);
}

FCookingResult UHarmoniaCookingComponent::CalculateCookingResult(const FCookingRecipe& Recipe)
{
	FCookingResult Result;
	Result.RecipeID = CurrentRecipeID;

	// ?±Í≥µÎ•?Í≥ÑÏÇ∞
	float SuccessRate = BaseSuccessRate + GetTotalSuccessRateBonus();
	
	// ?àÎ≤® Ï∞®Ïù¥???∞Î•∏ ?±Í≥µÎ•?Ï°∞Ï†ï
	int32 LevelDiff = CookingLevel - Recipe.MinCookingLevel;
	SuccessRate += LevelDiff * 2.0f;

	// ?úÏù¥?ÑÏóê ?∞Î•∏ ?±Í≥µÎ•?Ï°∞Ï†ï
	SuccessRate -= (Recipe.Difficulty - 5) * 5.0f;

	SuccessRate = FMath::Clamp(SuccessRate, 10.0f, 95.0f);

	// ?±Í≥µ ?êÏ†ï
	Result.bSuccess = FMath::FRand() * 100.0f <= SuccessRate;

	if (Result.bSuccess)
	{
		// ?àÏßà Í≤∞Ï†ï
		Result.Quality = DetermineCookingQuality(Recipe.Difficulty);
		Result.bPerfect = Result.Quality == ECookingQuality::Masterpiece;

		// Î≤ÑÌîÑ ?®Í≥º ?§Ï†ï
		Result.BuffEffect = Recipe.BaseBuffEffect;

		// Í≤ΩÌóòÏπ?Í≥ÑÏÇ∞
		float ExpMultiplier = 1.0f;
		for (const FCookingTrait& Trait : ActiveTraits)
		{
			ExpMultiplier += Trait.ExperienceBonus / 100.0f;
		}

		Result.Experience = FMath::CeilToInt(Recipe.ExperienceReward * ExpMultiplier);

		// ?ÑÎ≤Ω???îÎ¶¨??Í≤ΩÏö∞ Î≥¥ÎÑà??
		if (Result.bPerfect)
		{
			Result.Experience = FMath::CeilToInt(Result.Experience * 1.5f);
		}
	}
	else
	{
		Result.Quality = ECookingQuality::Failed;
		Result.Experience = Recipe.ExperienceReward / 4;
	}

	return Result;
}

ECookingQuality UHarmoniaCookingComponent::DetermineCookingQuality(int32 Difficulty)
{
	float QualityRoll = FMath::FRand() * 100.0f;
	float QualityBonus = GetTotalQualityBonus();

	// ?àÏßà Í∏∞Ï???Í≥ÑÏÇ∞ (?úÏù¥?ÑÍ? ?íÏùÑ?òÎ°ù ?íÏ? ?àÏßà ?¨ÏÑ± ?¥Î†§?Ä)
	float MasterpieceThreshold = FMath::Max(5.0f, 15.0f - QualityBonus - (Difficulty * 0.5f));
	float ExcellentThreshold = FMath::Max(15.0f, 30.0f - QualityBonus - (Difficulty * 0.3f));
	float GoodThreshold = FMath::Max(30.0f, 50.0f - QualityBonus);
	float NormalThreshold = 70.0f;

	if (QualityRoll <= MasterpieceThreshold)
	{
		return ECookingQuality::Masterpiece;
	}
	else if (QualityRoll <= ExcellentThreshold)
	{
		return ECookingQuality::Excellent;
	}
	else if (QualityRoll <= GoodThreshold)
	{
		return ECookingQuality::Good;
	}
	else if (QualityRoll <= NormalThreshold)
	{
		return ECookingQuality::Normal;
	}
	else
	{
		return ECookingQuality::Poor;
	}
}

void UHarmoniaCookingComponent::ApplyBuffEffect(const FFoodBuffEffect& BuffEffect)
{
	// ?§ÌÉù Í∞Ä???¨Î? ?ïÏù∏
	if (!BuffEffect.bStackable)
	{
		// Í∏∞Ï°¥ Î≤ÑÌîÑ ?úÍ±∞
		RemoveBuff(BuffEffect.BuffName);
	}

	// Î≤ÑÌîÑ Ï∂îÍ?
	ActiveBuffs.Add(BuffEffect);

	// ?Ä?¥Î®∏ ?§Ï†ï
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &UHarmoniaCookingComponent::OnBuffExpired, BuffEffect.BuffName);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		TimerDelegate,
		BuffEffect.Duration,
		false
	);

	BuffTimerHandles.Add(BuffEffect.BuffName, TimerHandle);
}

void UHarmoniaCookingComponent::OnBuffExpired(FName BuffName)
{
	RemoveBuff(BuffName);
}

void UHarmoniaCookingComponent::CheckAndProcessLevelUp()
{
	int32 ExpNeeded = GetExperienceForNextLevel();

	while (CurrentExperience >= ExpNeeded)
	{
		CurrentExperience -= ExpNeeded;
		CookingLevel++;

		OnCookingLevelUp.Broadcast(CookingLevel, 1);

		ExpNeeded = GetExperienceForNextLevel();
	}
}

float UHarmoniaCookingComponent::CalculateCookingTime(float BaseTime) const
{
	float SpeedBonus = GetTotalCookingSpeedBonus();
	float TimeMultiplier = 1.0f / (1.0f + SpeedBonus / 100.0f);

	return BaseTime * TimeMultiplier;
}

bool UHarmoniaCookingComponent::HasRequiredIngredients(const FCookingRecipe& Recipe) const
{
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasRequiredIngredients: No inventory component found"));
		return false;
	}

	for (const FCookingIngredient& Ingredient : Recipe.RequiredIngredients)
	{
		// ?†ÌÉù???¨Î£å??Í±¥ÎÑà?Ä
		if (Ingredient.bOptional)
		{
			continue;
		}

		// FName??FHarmoniaIDÎ°?Î≥Ä??(IngredientIDÍ∞Ä ?ÑÏù¥??ID?Ä ?ôÏùº?òÎã§Í≥?Í∞Ä??
		FHarmoniaID ItemID(Ingredient.IngredientID);

		int32 TotalCount = InventoryComponent->GetTotalCount(ItemID);
		if (TotalCount < Ingredient.Quantity)
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaCookingComponent::ConsumeIngredients(const FCookingRecipe& Recipe)
{
	if (!InventoryComponent)
	{
		return false;
	}

	// Î®ºÏ? Î™®Îì† ?¨Î£åÍ∞Ä ?àÎäîÏßÄ ?§Ïãú ?ïÏù∏ (?ôÏãú??Î¨∏Ï†ú Î∞©Ï?)
	if (!HasRequiredIngredients(Recipe))
	{
		return false;
	}

	// ?¨Î£å ?åÎπÑ
	for (const FCookingIngredient& Ingredient : Recipe.RequiredIngredients)
	{
		if (Ingredient.bOptional)
		{
			continue;
		}

		FHarmoniaID ItemID(Ingredient.IngredientID);

		// ?∏Î≤§?†Î¶¨?êÏÑú ?¨Î£å ?úÍ±∞
		if (!InventoryComponent->RemoveItem(ItemID, Ingredient.Quantity, -1.0f))
		{
			UE_LOG(LogTemp, Error, TEXT("ConsumeIngredients: Failed to remove ingredient %s"), *Ingredient.IngredientID.ToString());
			return false;
		}
	}

	return true;
}
