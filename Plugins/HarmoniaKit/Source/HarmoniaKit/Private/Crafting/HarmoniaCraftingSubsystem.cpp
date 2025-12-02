// Copyright 2025 Snow Game Studio.

#include "Crafting/HarmoniaCraftingSubsystem.h"
#include "Crafting/HarmoniaCraftingConfigDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UHarmoniaCraftingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaCraftingSubsystem::Deinitialize()
{
	// Clear all active sessions
	for (auto& PlayerPair : ActiveSessions)
	{
		for (auto& SessionPair : PlayerPair.Value)
		{
			if (SessionPair.Value.TimerHandle.IsValid())
			{
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().ClearTimer(SessionPair.Value.TimerHandle);
				}
			}
		}
	}
	ActiveSessions.Empty();

	Super::Deinitialize();
}

void UHarmoniaCraftingSubsystem::SetConfigDataAsset(UHarmoniaCraftingConfigDataAsset* InConfig)
{
	ConfigAsset = InConfig;

	// Load default recipes from config
	if (ConfigAsset)
	{
		for (const FHarmoniaCraftingRecipeData& Recipe : ConfigAsset->DefaultRecipes)
		{
			RegisterRecipe(Recipe);
		}
	}
}

void UHarmoniaCraftingSubsystem::RegisterRecipe(const FHarmoniaCraftingRecipeData& Recipe)
{
	if (!Recipe.RecipeId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HarmoniaCrafting] Cannot register recipe with empty ID"));
		return;
	}

	Recipes.Add(Recipe.RecipeId.GetID(), Recipe);
}

void UHarmoniaCraftingSubsystem::UnregisterRecipe(FName RecipeID)
{
	Recipes.Remove(RecipeID);
}

bool UHarmoniaCraftingSubsystem::GetRecipe(FName RecipeID, FHarmoniaCraftingRecipeData& OutRecipe) const
{
	if (const FHarmoniaCraftingRecipeData* Found = Recipes.Find(RecipeID))
	{
		OutRecipe = *Found;
		return true;
	}
	return false;
}

TArray<FHarmoniaCraftingRecipeData> UHarmoniaCraftingSubsystem::GetAllRecipes() const
{
	TArray<FHarmoniaCraftingRecipeData> Result;
	Recipes.GenerateValueArray(Result);
	return Result;
}

TArray<FHarmoniaCraftingRecipeData> UHarmoniaCraftingSubsystem::GetRecipesByStation(ECraftingStationType Station) const
{
	TArray<FHarmoniaCraftingRecipeData> Result;
	for (const auto& Pair : Recipes)
	{
		if (Pair.Value.RequiredStation == Station)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FHarmoniaCraftingRecipeData> UHarmoniaCraftingSubsystem::GetRecipesByCategory(FGameplayTag CategoryTag) const
{
	TArray<FHarmoniaCraftingRecipeData> Result;
	for (const auto& Pair : Recipes)
	{
		if (Pair.Value.CategoryTag == CategoryTag)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool UHarmoniaCraftingSubsystem::UnlockRecipe(APlayerController* Player, FName RecipeID)
{
	if (!Player || RecipeID.IsNone())
	{
		return false;
	}

	// Check if recipe exists
	if (!Recipes.Contains(RecipeID))
	{
		return false;
	}

	uint32 PlayerID = GetPlayerID(Player);
	TSet<FName>& UnlockedSet = PlayerUnlockedRecipes.FindOrAdd(PlayerID);

	if (UnlockedSet.Contains(RecipeID))
	{
		return false; // Already unlocked
	}

	// Check prerequisites
	const FHarmoniaCraftingRecipeData& Recipe = Recipes[RecipeID];
	for (const FName& PrereqID : Recipe.PrerequisiteRecipes)
	{
		if (!UnlockedSet.Contains(PrereqID))
		{
			return false; // Missing prerequisite
		}
	}

	UnlockedSet.Add(RecipeID);
	OnRecipeUnlocked.Broadcast(RecipeID, Player);
	return true;
}

bool UHarmoniaCraftingSubsystem::IsRecipeUnlocked(APlayerController* Player, FName RecipeID) const
{
	if (!Player)
	{
		return false;
	}

	// Check if recipe is unlocked by default
	if (const FHarmoniaCraftingRecipeData* Recipe = Recipes.Find(RecipeID))
	{
		if (Recipe->bUnlockedByDefault)
		{
			return true;
		}
	}

	uint32 PlayerID = GetPlayerID(Player);
	if (const TSet<FName>* UnlockedSet = PlayerUnlockedRecipes.Find(PlayerID))
	{
		return UnlockedSet->Contains(RecipeID);
	}
	return false;
}

TArray<FName> UHarmoniaCraftingSubsystem::GetUnlockedRecipes(APlayerController* Player) const
{
	TArray<FName> Result;

	if (!Player)
	{
		return Result;
	}

	// Add default unlocked recipes
	for (const auto& Pair : Recipes)
	{
		if (Pair.Value.bUnlockedByDefault)
		{
			Result.Add(Pair.Key);
		}
	}

	// Add player-specific unlocks
	uint32 PlayerID = GetPlayerID(Player);
	if (const TSet<FName>* UnlockedSet = PlayerUnlockedRecipes.Find(PlayerID))
	{
		for (const FName& RecipeID : *UnlockedSet)
		{
			Result.AddUnique(RecipeID);
		}
	}

	return Result;
}

TArray<FHarmoniaCraftingRecipeData> UHarmoniaCraftingSubsystem::GetAvailableRecipes(APlayerController* Player, ECraftingStationType Station) const
{
	TArray<FHarmoniaCraftingRecipeData> Result;

	if (!Player)
	{
		return Result;
	}

	TArray<FName> UnlockedIDs = GetUnlockedRecipes(Player);
	for (const FName& RecipeID : UnlockedIDs)
	{
		FHarmoniaCraftingRecipeData Recipe;
		if (GetRecipe(RecipeID, Recipe))
		{
			if (Recipe.RequiredStation == Station)
			{
				Result.Add(Recipe);
			}
		}
	}

	return Result;
}

bool UHarmoniaCraftingSubsystem::CanCraft(APlayerController* Player, FName RecipeID, FText& OutReason) const
{
	if (!Player)
	{
		OutReason = NSLOCTEXT("Crafting", "NoPlayer", "플레이어가 없습니다.");
		return false;
	}

	FHarmoniaCraftingRecipeData Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		OutReason = NSLOCTEXT("Crafting", "InvalidRecipe", "Invalid recipe.");
		return false;
	}

	// Check if unlocked
	if (!IsRecipeUnlocked(Player, RecipeID))
	{
		OutReason = NSLOCTEXT("Crafting", "RecipeLocked", "Recipe is locked.");
		return false;
	}

	// Check skill level
	int32 PlayerLevel = GetSkillLevel(Player, Recipe.RequiredStation);
	if (PlayerLevel < Recipe.RequiredLevel)
	{
		OutReason = FText::Format(
			NSLOCTEXT("Crafting", "LevelTooLow", "Crafting level is too low. (Required: {0}, Current: {1})"),
			FText::AsNumber(Recipe.RequiredLevel),
			FText::AsNumber(PlayerLevel));
		return false;
	}

	// Check materials
	if (!HasRequiredMaterials(Player, RecipeID, 1))
	{
		OutReason = NSLOCTEXT("Crafting", "InsufficientMaterials", "Insufficient materials.");
		return false;
	}

	// TODO: Check currency
	// TODO: Check crafting station availability

	OutReason = FText::GetEmpty();
	return true;
}

bool UHarmoniaCraftingSubsystem::StartCrafting(APlayerController* Player, FName RecipeID, int32 Quantity)
{
	FText Reason;
	if (!CanCraft(Player, RecipeID, Reason))
	{
		return false;
	}

	FHarmoniaCraftingRecipeData Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		return false;
	}

	// Instant craft if no crafting time
	if (Recipe.CraftingTime <= 0.0f)
	{
		FHarmoniaCraftingSessionResult Result = InstantCraft(Player, RecipeID, Quantity);
		return Result.Result == EHarmoniaCraftingResult::Success || 
			   Result.Result == EHarmoniaCraftingResult::CriticalSuccess;
	}

	// Queued crafting
	uint32 PlayerID = GetPlayerID(Player);
	TMap<FName, FCraftingSession>& Sessions = ActiveSessions.FindOrAdd(PlayerID);

	if (Sessions.Contains(RecipeID))
	{
		return false; // Already crafting this recipe
	}

	// Calculate total time with skill modifier
	FHarmoniaCraftingCategoryData Skill = GetCraftingSkill(Player, Recipe.RequiredStation);
	float TotalTime = Recipe.CraftingTime * Quantity / Skill.SpeedMultiplier;

	FCraftingSession& Session = Sessions.Add(RecipeID);
	Session.RecipeID = RecipeID;
	Session.Quantity = Quantity;
	Session.TotalTime = TotalTime;
	Session.ElapsedTime = 0.0f;

	// Consume materials upfront
	if (!ConsumeMaterials(Player, Recipe, Quantity))
	{
		Sessions.Remove(RecipeID);
		return false;
	}

	// Start timer
	if (UWorld* World = GetWorld())
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this, Player, RecipeID, Quantity]()
		{
			// Complete crafting
			FHarmoniaCraftingSessionResult Result;
			Result.RecipeId = RecipeID;

			FHarmoniaCraftingRecipeData Recipe;
			if (GetRecipe(RecipeID, Recipe))
			{
				// Calculate success
				float SuccessChance = CalculateSuccessChance(Player, RecipeID);
				float Roll = FMath::FRand();

				if (Roll <= SuccessChance * 0.1f)
				{
					Result.Result = EHarmoniaCraftingResult::CriticalSuccess;
					Result.Quality = FMath::Min(1.0f, CalculateExpectedQuality(Player, RecipeID) + 0.2f);
				}
				else if (Roll <= SuccessChance)
				{
					Result.Result = EHarmoniaCraftingResult::Success;
					Result.Quality = CalculateExpectedQuality(Player, RecipeID);
				}
				else
				{
					Result.Result = EHarmoniaCraftingResult::Failure;
					Result.Quality = 0.0f;
				}

				// Generate outputs
				if (Result.Result == EHarmoniaCraftingResult::Success || 
					Result.Result == EHarmoniaCraftingResult::CriticalSuccess)
				{
					for (const FHarmoniaCraftingOutput& Output : Recipe.Outputs)
					{
						if (FMath::FRand() <= Output.Chance)
						{
							FHarmoniaCraftingOutput ProducedOutput = Output;
							ProducedOutput.BaseQuantity *= Quantity;
							if (Result.Result == EHarmoniaCraftingResult::CriticalSuccess)
							{
								ProducedOutput.BaseQuantity = FMath::CeilToInt(ProducedOutput.BaseQuantity * 1.5f);
							}
							Result.OutputItems.Add(ProducedOutput.ItemId);
							
							// Also add to ProducedItems as FCraftingResultItem
							FCraftingResultItem ResultItem;
							ResultItem.ItemId = ProducedOutput.ItemId;
							ResultItem.Amount = ProducedOutput.BaseQuantity;
							ResultItem.Grade = ProducedOutput.Grade;
							Result.ProducedItems.Add(ResultItem);
						}
					}

					// Add experience
					float ExpMultiplier = (Result.Result == EHarmoniaCraftingResult::CriticalSuccess) ? 1.5f : 1.0f;
					Result.ExperienceGained = Recipe.ExperienceGain * Quantity * ExpMultiplier;
					AddCraftingExperience(Player, Recipe.RequiredStation, Result.ExperienceGained);

					// Grant items
					GrantCraftedItems(Player, Result);
				}
			}

			// Remove session
			uint32 PlayerID = GetPlayerID(Player);
			if (TMap<FName, FCraftingSession>* Sessions = ActiveSessions.Find(PlayerID))
			{
				Sessions->Remove(RecipeID);
			}

			OnCraftingCompleted.Broadcast(Result);
		});

		World->GetTimerManager().SetTimer(Session.TimerHandle, TimerDelegate, TotalTime, false);
	}

	OnCraftingStarted.Broadcast(RecipeID, Player, TotalTime);
	return true;
}

bool UHarmoniaCraftingSubsystem::CancelCrafting(APlayerController* Player, FName RecipeID)
{
	if (!Player)
	{
		return false;
	}

	uint32 PlayerID = GetPlayerID(Player);
	TMap<FName, FCraftingSession>* Sessions = ActiveSessions.Find(PlayerID);
	if (!Sessions)
	{
		return false;
	}

	FCraftingSession* Session = Sessions->Find(RecipeID);
	if (!Session)
	{
		return false;
	}

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Session->TimerHandle);
	}

	// TODO: Refund some materials based on progress

	Sessions->Remove(RecipeID);
	return true;
}

float UHarmoniaCraftingSubsystem::GetCraftingProgress(APlayerController* Player, FName RecipeID) const
{
	if (!Player)
	{
		return 0.0f;
	}

	uint32 PlayerID = GetPlayerID(Player);
	const TMap<FName, FCraftingSession>* Sessions = ActiveSessions.Find(PlayerID);
	if (!Sessions)
	{
		return 0.0f;
	}

	const FCraftingSession* Session = Sessions->Find(RecipeID);
	if (!Session || Session->TotalTime <= 0.0f)
	{
		return 0.0f;
	}

	// Calculate elapsed time from timer
	if (UWorld* World = GetWorld())
	{
		float Remaining = World->GetTimerManager().GetTimerRemaining(Session->TimerHandle);
		float Elapsed = Session->TotalTime - Remaining;
		return FMath::Clamp(Elapsed / Session->TotalTime, 0.0f, 1.0f);
	}

	return 0.0f;
}

FHarmoniaCraftingSessionResult UHarmoniaCraftingSubsystem::InstantCraft(APlayerController* Player, FName RecipeID, int32 Quantity)
{
	FHarmoniaCraftingSessionResult Result;
	Result.RecipeId = RecipeID;

	FText Reason;
	if (!CanCraft(Player, RecipeID, Reason))
	{
		Result.Result = EHarmoniaCraftingResult::InvalidRecipe;
		Result.ErrorMessage = Reason;
		return Result;
	}

	FHarmoniaCraftingRecipeData Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		Result.Result = EHarmoniaCraftingResult::InvalidRecipe;
		return Result;
	}

	// Consume materials
	if (!ConsumeMaterials(Player, Recipe, Quantity))
	{
		Result.Result = EHarmoniaCraftingResult::InsufficientMaterials;
		return Result;
	}

	// Calculate success
	float SuccessChance = CalculateSuccessChance(Player, RecipeID);
	float Roll = FMath::FRand();

	if (Roll <= SuccessChance * 0.1f)
	{
		Result.Result = EHarmoniaCraftingResult::CriticalSuccess;
		Result.Quality = FMath::Min(1.0f, CalculateExpectedQuality(Player, RecipeID) + 0.2f);
	}
	else if (Roll <= SuccessChance)
	{
		Result.Result = EHarmoniaCraftingResult::Success;
		Result.Quality = CalculateExpectedQuality(Player, RecipeID);
	}
	else
	{
		Result.Result = EHarmoniaCraftingResult::Failure;
		Result.Quality = 0.0f;
	}

	// Generate outputs
	if (Result.Result == EHarmoniaCraftingResult::Success || 
		Result.Result == EHarmoniaCraftingResult::CriticalSuccess)
	{
		for (const FHarmoniaCraftingOutput& Output : Recipe.Outputs)
		{
			if (FMath::FRand() <= Output.Chance)
			{
				FHarmoniaCraftingOutput ProducedOutput = Output;
				ProducedOutput.BaseQuantity *= Quantity;
				if (Result.Result == EHarmoniaCraftingResult::CriticalSuccess)
				{
					ProducedOutput.BaseQuantity = FMath::CeilToInt(ProducedOutput.BaseQuantity * 1.5f);
				}
				Result.OutputItems.Add(ProducedOutput.ItemId);
				
				// Also add to ProducedItems as FCraftingResultItem
				FCraftingResultItem ResultItem;
				ResultItem.ItemId = ProducedOutput.ItemId;
				ResultItem.Amount = ProducedOutput.BaseQuantity;
				ResultItem.Grade = ProducedOutput.Grade;
				Result.ProducedItems.Add(ResultItem);
			}
		}

		// Add experience
		float ExpMultiplier = (Result.Result == EHarmoniaCraftingResult::CriticalSuccess) ? 1.5f : 1.0f;
		Result.ExperienceGained = Recipe.ExperienceGain * Quantity * ExpMultiplier;
		AddCraftingExperience(Player, Recipe.RequiredStation, Result.ExperienceGained);

		// Grant items
		GrantCraftedItems(Player, Result);
	}

	OnCraftingCompleted.Broadcast(Result);
	return Result;
}

FHarmoniaCraftingCategoryData UHarmoniaCraftingSubsystem::GetCraftingSkill(APlayerController* Player, ECraftingStationType Station) const
{
	FHarmoniaCraftingCategoryData DefaultSkill;
	DefaultSkill.Station = Station;

	if (!Player)
	{
		return DefaultSkill;
	}

	uint32 PlayerID = GetPlayerID(Player);
	const TMap<ECraftingStationType, FHarmoniaCraftingCategoryData>* Skills = PlayerSkills.Find(PlayerID);
	if (!Skills)
	{
		return DefaultSkill;
	}

	const FHarmoniaCraftingCategoryData* Found = Skills->Find(Station);
	return Found ? *Found : DefaultSkill;
}

void UHarmoniaCraftingSubsystem::AddCraftingExperience(APlayerController* Player, ECraftingStationType Station, float Experience)
{
	if (!Player || Experience <= 0.0f)
	{
		return;
	}

	uint32 PlayerID = GetPlayerID(Player);
	TMap<ECraftingStationType, FHarmoniaCraftingCategoryData>& Skills = PlayerSkills.FindOrAdd(PlayerID);
	FHarmoniaCraftingCategoryData& Skill = Skills.FindOrAdd(Station);

	if (Skill.Station == ECraftingStationType::None)
	{
		Skill.Station = Station;
	}

	Skill.CurrentExperience += Experience;
	ProcessLevelUp(Player, Station);
}

int32 UHarmoniaCraftingSubsystem::GetSkillLevel(APlayerController* Player, ECraftingStationType Station) const
{
	return GetCraftingSkill(Player, Station).Level;
}

bool UHarmoniaCraftingSubsystem::HasRequiredMaterials(APlayerController* Player, FName RecipeID, int32 Quantity) const
{
	// TODO: Integrate with inventory system
	// For now, return true for testing
	return true;
}

TArray<FCraftingMaterial> UHarmoniaCraftingSubsystem::GetMissingMaterials(APlayerController* Player, FName RecipeID, int32 Quantity) const
{
	TArray<FCraftingMaterial> Missing;
	
	// TODO: Integrate with inventory system
	
	return Missing;
}

float UHarmoniaCraftingSubsystem::CalculateSuccessChance(APlayerController* Player, FName RecipeID) const
{
	FHarmoniaCraftingRecipeData Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		return 0.0f;
	}

	FHarmoniaCraftingCategoryData Skill = GetCraftingSkill(Player, Recipe.RequiredStation);
	float DifficultyMod = GetDifficultyModifier(Recipe.Difficulty);

	// Base chance + skill bonus - difficulty penalty
	float SuccessChance = Recipe.BaseSuccessRate + Skill.BonusSuccessRate - DifficultyMod;

	// Level advantage bonus
	int32 LevelDiff = Skill.Level - Recipe.RequiredLevel;
	if (LevelDiff > 0)
	{
		SuccessChance += LevelDiff * 0.05f; // 5% per level above requirement
	}

	return FMath::Clamp(SuccessChance, 0.0f, 1.0f);
}

float UHarmoniaCraftingSubsystem::CalculateExpectedQuality(APlayerController* Player, FName RecipeID) const
{
	FHarmoniaCraftingRecipeData Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		return 0.5f;
	}

	FHarmoniaCraftingCategoryData Skill = GetCraftingSkill(Player, Recipe.RequiredStation);

	// Base quality from skill
	float BaseQuality = 0.5f + (Skill.Level * 0.05f) + Skill.BonusQuality;

	// Level advantage bonus
	int32 LevelDiff = Skill.Level - Recipe.RequiredLevel;
	if (LevelDiff > 0)
	{
		BaseQuality += LevelDiff * 0.02f;
	}

	return FMath::Clamp(BaseQuality, 0.0f, 1.0f);
}

bool UHarmoniaCraftingSubsystem::ConsumeMaterials(APlayerController* Player, const FHarmoniaCraftingRecipeData& Recipe, int32 Quantity)
{
	// TODO: Integrate with inventory system to consume materials
	return true;
}

bool UHarmoniaCraftingSubsystem::GrantCraftedItems(APlayerController* Player, const FHarmoniaCraftingSessionResult& Result)
{
	// TODO: Integrate with inventory system to grant items
	return true;
}

float UHarmoniaCraftingSubsystem::GetDifficultyModifier(EHarmoniaRecipeDifficulty Difficulty) const
{
	if (ConfigAsset)
	{
		switch (Difficulty)
		{
			case EHarmoniaRecipeDifficulty::Trivial: return ConfigAsset->TrivialDifficultyPenalty;
			case EHarmoniaRecipeDifficulty::Easy: return ConfigAsset->EasyDifficultyPenalty;
			case EHarmoniaRecipeDifficulty::Normal: return ConfigAsset->NormalDifficultyPenalty;
			case EHarmoniaRecipeDifficulty::Hard: return ConfigAsset->HardDifficultyPenalty;
			case EHarmoniaRecipeDifficulty::Expert: return ConfigAsset->ExpertDifficultyPenalty;
			case EHarmoniaRecipeDifficulty::Master: return ConfigAsset->MasterDifficultyPenalty;
			case EHarmoniaRecipeDifficulty::Legendary: return ConfigAsset->LegendaryDifficultyPenalty;
			default: return 0.0f;
		}
	}

	// Default values
	switch (Difficulty)
	{
		case EHarmoniaRecipeDifficulty::Trivial: return -0.1f;
		case EHarmoniaRecipeDifficulty::Easy: return 0.0f;
		case EHarmoniaRecipeDifficulty::Normal: return 0.1f;
		case EHarmoniaRecipeDifficulty::Hard: return 0.2f;
		case EHarmoniaRecipeDifficulty::Expert: return 0.35f;
		case EHarmoniaRecipeDifficulty::Master: return 0.5f;
		case EHarmoniaRecipeDifficulty::Legendary: return 0.7f;
		default: return 0.0f;
	}
}

void UHarmoniaCraftingSubsystem::ProcessLevelUp(APlayerController* Player, ECraftingStationType Station)
{
	if (!Player)
	{
		return;
	}

	uint32 PlayerID = GetPlayerID(Player);
	TMap<ECraftingStationType, FHarmoniaCraftingCategoryData>* Skills = PlayerSkills.Find(PlayerID);
	if (!Skills)
	{
		return;
	}

	FHarmoniaCraftingCategoryData* Skill = Skills->Find(Station);
	if (!Skill)
	{
		return;
	}

	while (Skill->CurrentExperience >= Skill->ExperienceToNextLevel)
	{
		Skill->CurrentExperience -= Skill->ExperienceToNextLevel;
		Skill->Level++;

		// Increase exp requirement
		Skill->ExperienceToNextLevel *= 1.5f;

		// Improve skill bonuses
		Skill->BonusSuccessRate += 0.02f;
		Skill->BonusQuality += 0.01f;
		Skill->SpeedMultiplier += 0.05f;

		OnCraftingSkillLevelUp.Broadcast(Station, Skill->Level);
	}
}

uint32 UHarmoniaCraftingSubsystem::GetPlayerID(APlayerController* Player) const
{
	if (!Player)
	{
		return 0;
	}
	return Player->GetUniqueID();
}
