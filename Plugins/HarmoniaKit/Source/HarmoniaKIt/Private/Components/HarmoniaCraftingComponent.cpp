// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaCraftingComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UHarmoniaCraftingComponent::UHarmoniaCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Only tick when crafting
	SetIsReplicatedByDefault(true);

	RecipeDataTable = nullptr;
	GradeConfigDataTable = nullptr;
	CategoryDataTable = nullptr;
	StationDataTable = nullptr;
	InventoryComponent = nullptr;
	CurrentStation = ECraftingStationType::None;

	// Security: Rate limiting
	LastCraftingAttempt = 0.0f;
	CraftingAttemptsThisSecond = 0;
	LastAttemptResetTime = 0.0f;
	MinTimeBetweenCrafts = 0.5f;
	MaxCraftingAttemptsPerSecond = 5;

	// Material refund settings
	bRefundMaterialsOnCancel = true;
	MaterialRefundPercentage = 0.5f;

	// Constants
	MaxItemDurability = 100.0f;
	AnyDurability = 0.0f;

	// Station distance check
	MaxStationInteractionDistance = 500.0f;
}

void UHarmoniaCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache inventory component
	GetInventoryComponent();

	// Cache DataTable configurations for fast lookups
	CacheConfigurationData();
}

void UHarmoniaCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update active crafting session
	if (ActiveSession.bIsActive)
	{
		ActiveSession.ElapsedTime += DeltaTime;

		// Broadcast progress update
		float RemainingTime = FMath::Max(0.0f, ActiveSession.TotalCastingTime - ActiveSession.ElapsedTime);
		OnCraftingProgress.Broadcast(ActiveSession.RecipeId, ActiveSession.GetProgress(), RemainingTime);

		// Check if crafting is complete
		if (ActiveSession.ElapsedTime >= ActiveSession.TotalCastingTime)
		{
			CompleteCrafting();
		}
	}
}

void UHarmoniaCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [BANDWIDTH OPTIMIZATION] Only replicate crafting data to the owning client
	// Other players don't need to know what recipes this player has learned or what they're crafting
	// This can save significant bandwidth, especially with hundreds of learned recipes
	DOREPLIFETIME_CONDITION(UHarmoniaCraftingComponent, ActiveSession, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHarmoniaCraftingComponent, LearnedRecipes, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHarmoniaCraftingComponent, CurrentStation, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHarmoniaCraftingComponent, CurrentStationTags, COND_OwnerOnly);
}

void UHarmoniaCraftingComponent::OnRep_ActiveSession()
{
	// Client-side notification when active session changes
	if (ActiveSession.bIsActive)
	{
		OnCraftingStarted.Broadcast(ActiveSession.RecipeId, ActiveSession.TotalCastingTime);

		// Note: Animation is now played via MulticastPlayCraftingAnimation
		// No need to play it here separately
	}
}

void UHarmoniaCraftingComponent::OnRep_CurrentStation()
{
	// Client-side notification when station changes
	// Can be used for UI updates
}

//~==============================================
//~ Crafting Operations
//~==============================================

bool UHarmoniaCraftingComponent::StartCrafting(FHarmoniaID RecipeId)
{
	// Check if already crafting
	if (ActiveSession.bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::StartCrafting - Already crafting!"));
		return false;
	}

	// Request server to start crafting
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStartCrafting(RecipeId);
		return true;
	}

	// Get recipe data
	FCraftingRecipeData RecipeData;
	if (!GetRecipeData(RecipeId, RecipeData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::StartCrafting - Recipe not found: %s"), *RecipeId.ToString());
		return false;
	}

	// Check if recipe requires learning and player knows it
	if (RecipeData.bRequiresLearning && !HasLearnedRecipe(RecipeId))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::StartCrafting - Recipe not learned: %s"), *RecipeId.ToString());
		return false;
	}

	// Check if player meets requirements
	if (!MeetsRecipeRequirements(RecipeData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::StartCrafting - Player doesn't meet requirements for recipe: %s"), *RecipeId.ToString());
		return false;
	}

	// Check if at correct crafting station
	if (!CheckStationRequirement(RecipeData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::StartCrafting - Wrong crafting station for recipe: %s (Requires: %d, Current: %d)"),
			*RecipeId.ToString(), (int32)RecipeData.RequiredStation, (int32)CurrentStation);
		return false;
	}

	// Server-side: Verify player is within range of station (anti-cheat)
	if (GetOwnerRole() == ROLE_Authority && !VerifyStationDistance())
	{
		UE_LOG(LogTemp, Error, TEXT("UHarmoniaCraftingComponent::StartCrafting - Station distance verification failed"));
		return false;
	}

	// Check if player has materials
	TArray<FCraftingMaterial> MissingMaterials;
	if (!CanCraftRecipe(RecipeId, MissingMaterials))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::StartCrafting - Missing materials for recipe: %s"), *RecipeId.ToString());
		return false;
	}

	// Consume materials
	if (!ConsumeMaterials(RecipeData.RequiredMaterials))
	{
		UE_LOG(LogTemp, Error, TEXT("UHarmoniaCraftingComponent::StartCrafting - Failed to consume materials for recipe: %s"), *RecipeId.ToString());
		return false;
	}

	// Initialize crafting session
	ActiveSession.RecipeId = RecipeId;
	ActiveSession.ElapsedTime = 0.0f;
	ActiveSession.TotalCastingTime = RecipeData.CastingTime;
	ActiveSession.bIsActive = true;
	ActiveSession.StartTime = GetWorld()->GetTimeSeconds();

	// Enable ticking only when crafting (performance optimization)
	SetComponentTickEnabled(true);

	// Broadcast start event
	OnCraftingStarted.Broadcast(RecipeId, RecipeData.CastingTime);

	// Play crafting animation on all clients (including server)
	if (RecipeData.CraftingMontage.IsValid())
	{
		UAnimMontage* Montage = RecipeData.CraftingMontage.LoadSynchronous();
		if (Montage)
		{
			// Use Multicast RPC to sync animation to all clients
			MulticastPlayCraftingAnimation(Montage);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::StartCrafting - Started crafting recipe: %s"), *RecipeId.ToString());
	return true;
}

void UHarmoniaCraftingComponent::CancelCrafting()
{
	if (!ActiveSession.bIsActive)
	{
		return;
	}

	// Request server to cancel
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerCancelCrafting();
		return;
	}

	float ProgressLost = ActiveSession.GetProgress();
	FHarmoniaID RecipeId = ActiveSession.RecipeId;

	// Refund materials based on progress (less progress = more refund)
	if (bRefundMaterialsOnCancel)
	{
		FCraftingRecipeData RecipeData;
		if (GetRecipeData(RecipeId, RecipeData))
		{
			float Progress = ActiveSession.GetProgress();
			float RefundMultiplier = MaterialRefundPercentage * (1.0f - Progress);

			UHarmoniaInventoryComponent* Inventory = GetInventoryComponent();
			if (Inventory)
			{
				for (const FCraftingMaterial& Material : RecipeData.RequiredMaterials)
				{
					if (Material.bConsumeOnCraft)
					{
						int32 RefundAmount = FMath::FloorToInt(Material.Amount * RefundMultiplier);
						if (RefundAmount > 0)
						{
							Inventory->AddItem(Material.ItemId, RefundAmount, MaxItemDurability);
							UE_LOG(LogTemp, Log, TEXT("Refunded %d x %s (%.0f%% refund)"),
								RefundAmount, *Material.ItemId.ToString(), RefundMultiplier * 100.0f);
						}
					}
				}
			}
		}
	}

	// Reset session
	ActiveSession = FActiveCraftingSession();

	// Stop animation on all clients
	MulticastStopCraftingAnimation();

	// Disable ticking when not crafting (performance optimization)
	SetComponentTickEnabled(false);

	// Broadcast cancellation
	OnCraftingCancelled.Broadcast(RecipeId, ProgressLost);

	// Notify clients
	ClientCraftingCancelled(RecipeId, ProgressLost);

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::CancelCrafting - Cancelled crafting: %s (Progress: %.2f%%)"), *RecipeId.ToString(), ProgressLost * 100.0f);
}

bool UHarmoniaCraftingComponent::CanCraftRecipe(FHarmoniaID RecipeId, TArray<FCraftingMaterial>& OutMissingMaterials) const
{
	OutMissingMaterials.Empty();

	// Get recipe data
	FCraftingRecipeData RecipeData;
	if (!GetRecipeData(RecipeId, RecipeData))
	{
		return false;
	}

	// Check if recipe requires learning
	if (RecipeData.bRequiresLearning && !HasLearnedRecipe(RecipeId))
	{
		return false;
	}

	// Check requirements
	if (!MeetsRecipeRequirements(RecipeData))
	{
		return false;
	}

	// Check materials
	UHarmoniaInventoryComponent* Inventory = const_cast<UHarmoniaCraftingComponent*>(this)->GetInventoryComponent();
	if (!Inventory)
	{
		return false;
	}

	for (const FCraftingMaterial& Material : RecipeData.RequiredMaterials)
	{
		int32 CurrentAmount = Inventory->GetTotalCount(Material.ItemId);
		if (CurrentAmount < Material.Amount)
		{
			OutMissingMaterials.Add(Material);
		}
	}

	return OutMissingMaterials.Num() == 0;
}

bool UHarmoniaCraftingComponent::GetRecipeData(FHarmoniaID RecipeId, FCraftingRecipeData& OutRecipeData) const
{
	if (!RecipeDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::GetRecipeData - RecipeDataTable is null!"));
		return false;
	}

	// Find recipe by row name
	static const FString ContextString(TEXT("GetRecipeData"));
	FCraftingRecipeData* FoundRecipe = RecipeDataTable->FindRow<FCraftingRecipeData>(RecipeId.Id, ContextString, false);

	if (FoundRecipe)
	{
		OutRecipeData = *FoundRecipe;
		return true;
	}

	return false;
}

TArray<FCraftingRecipeData> UHarmoniaCraftingComponent::GetAvailableRecipes(FGameplayTag CategoryTag) const
{
	TArray<FCraftingRecipeData> AvailableRecipes;

	if (!RecipeDataTable)
	{
		return AvailableRecipes;
	}

	// Get all recipes
	TArray<FCraftingRecipeData*> AllRecipes;
	RecipeDataTable->GetAllRows<FCraftingRecipeData>(TEXT("GetAvailableRecipes"), AllRecipes);

	for (FCraftingRecipeData* Recipe : AllRecipes)
	{
		if (!Recipe)
		{
			continue;
		}

		// Filter by category if specified
		if (CategoryTag.IsValid())
		{
			if (!Recipe->CategoryTags.HasTag(CategoryTag))
			{
				continue;
			}
		}

		// Check if recipe requires learning
		if (Recipe->bRequiresLearning && !HasLearnedRecipe(Recipe->RecipeId))
		{
			continue;
		}

		AvailableRecipes.Add(*Recipe);
	}

	return AvailableRecipes;
}

//~==============================================
//~ Recipe Learning System
//~==============================================

bool UHarmoniaCraftingComponent::LearnRecipe(FHarmoniaID RecipeId)
{
	// Request server to learn recipe
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLearnRecipe(RecipeId);
		return true;
	}

	// Check if already learned
	if (HasLearnedRecipe(RecipeId))
	{
		return false;
	}

	// Verify recipe exists
	FCraftingRecipeData RecipeData;
	if (!GetRecipeData(RecipeId, RecipeData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::LearnRecipe - Recipe not found: %s"), *RecipeId.ToString());
		return false;
	}

	// Add to learned recipes
	LearnedRecipes.Add(RecipeId);

	// Broadcast event
	OnRecipeLearned.Broadcast(RecipeId, RecipeData.RecipeName);

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::LearnRecipe - Learned recipe: %s"), *RecipeId.ToString());
	return true;
}

bool UHarmoniaCraftingComponent::HasLearnedRecipe(FHarmoniaID RecipeId) const
{
	return LearnedRecipes.Contains(RecipeId);
}

void UHarmoniaCraftingComponent::ForgetRecipe(FHarmoniaID RecipeId)
{
	// Server-only execution (LearnedRecipes is replicated)
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	LearnedRecipes.Remove(RecipeId);
}

//~==============================================
//~ Grade System
//~==============================================

bool UHarmoniaCraftingComponent::GetGradeConfig(EItemGrade Grade, FItemGradeConfig& OutGradeConfig) const
{
	// Use cached lookup for O(1) performance
	const FItemGradeConfig* Config = GradeConfigCache.Find(Grade);
	if (Config)
	{
		OutGradeConfig = *Config;
		return true;
	}

	return false;
}

FLinearColor UHarmoniaCraftingComponent::GetGradeColor(EItemGrade Grade) const
{
	FItemGradeConfig GradeConfig;
	if (GetGradeConfig(Grade, GradeConfig))
	{
		return GradeConfig.GradeColor;
	}

	// Default colors if no config found
	switch (Grade)
	{
	case EItemGrade::Common:		return FLinearColor::White;
	case EItemGrade::Uncommon:		return FLinearColor::Green;
	case EItemGrade::Rare:			return FLinearColor::Blue;
	case EItemGrade::Epic:			return FLinearColor(0.5f, 0.0f, 1.0f); // Purple
	case EItemGrade::Legendary:		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	case EItemGrade::Mythic:		return FLinearColor::Red;
	default:						return FLinearColor::White;
	}
}

FText UHarmoniaCraftingComponent::GetGradeDisplayName(EItemGrade Grade) const
{
	FItemGradeConfig GradeConfig;
	if (GetGradeConfig(Grade, GradeConfig))
	{
		return GradeConfig.DisplayName;
	}

	// Default names if no config found
	switch (Grade)
	{
	case EItemGrade::Common:		return FText::FromString(TEXT("Common"));
	case EItemGrade::Uncommon:		return FText::FromString(TEXT("Uncommon"));
	case EItemGrade::Rare:			return FText::FromString(TEXT("Rare"));
	case EItemGrade::Epic:			return FText::FromString(TEXT("Epic"));
	case EItemGrade::Legendary:		return FText::FromString(TEXT("Legendary"));
	case EItemGrade::Mythic:		return FText::FromString(TEXT("Mythic"));
	default:						return FText::FromString(TEXT("Unknown"));
	}
}

//~==============================================
//~ Internal Crafting Logic
//~==============================================

void UHarmoniaCraftingComponent::CompleteCrafting()
{
	if (!ActiveSession.bIsActive)
	{
		return;
	}

	// Get recipe data
	FCraftingRecipeData RecipeData;
	if (!GetRecipeData(ActiveSession.RecipeId, RecipeData))
	{
		UE_LOG(LogTemp, Error, TEXT("UHarmoniaCraftingComponent::CompleteCrafting - Recipe not found: %s"), *ActiveSession.RecipeId.ToString());
		ActiveSession = FActiveCraftingSession();
		return;
	}

	// Determine result
	ECraftingResult Result = DetermineCraftingResult(RecipeData);

	// Get result items based on outcome
	TArray<FCraftingResultItem> ResultItems;
	switch (Result)
	{
	case ECraftingResult::Success:
		ResultItems = RecipeData.SuccessResults;
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::CompleteCrafting - Crafting succeeded: %s"), *ActiveSession.RecipeId.ToString());
		break;

	case ECraftingResult::CriticalSuccess:
		ResultItems = RecipeData.SuccessResults;
		ResultItems.Append(RecipeData.CriticalSuccessResults);
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::CompleteCrafting - Critical success: %s"), *ActiveSession.RecipeId.ToString());
		break;

	case ECraftingResult::Failure:
		ResultItems = RecipeData.FailureResults;
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::CompleteCrafting - Crafting failed: %s"), *ActiveSession.RecipeId.ToString());
		break;

	default:
		break;
	}

	// Distribute results
	DistributeResults(ResultItems);

	// Stop animation on all clients
	MulticastStopCraftingAnimation();

	// Broadcast completion event
	OnCraftingCompleted.Broadcast(ActiveSession.RecipeId, Result, ResultItems);

	// Notify clients
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientCraftingCompleted(ActiveSession.RecipeId, Result, ResultItems);
	}

	// Reset session
	ActiveSession = FActiveCraftingSession();

	// Disable ticking when not crafting (performance optimization)
	SetComponentTickEnabled(false);
}

bool UHarmoniaCraftingComponent::ConsumeMaterials(const TArray<FCraftingMaterial>& Materials)
{
	UHarmoniaInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		return false;
	}

	// First pass: verify all materials are available
	for (const FCraftingMaterial& Material : Materials)
	{
		if (!Material.bConsumeOnCraft)
		{
			continue;
		}

		int32 CurrentAmount = Inventory->GetTotalCount(Material.ItemId);
		if (CurrentAmount < Material.Amount)
		{
			UE_LOG(LogTemp, Error, TEXT("UHarmoniaCraftingComponent::ConsumeMaterials - Insufficient materials: %s (Required: %d, Has: %d)"),
				*Material.ItemId.ToString(), Material.Amount, CurrentAmount);
			return false;
		}
	}

	// Second pass: consume materials
	for (const FCraftingMaterial& Material : Materials)
	{
		if (!Material.bConsumeOnCraft)
		{
			continue;
		}

		// RemoveItem requires Durability parameter (use AnyDurability for any durability)
		bool bSuccess = Inventory->RemoveItem(Material.ItemId, Material.Amount, AnyDurability);
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("UHarmoniaCraftingComponent::ConsumeMaterials - Failed to remove material: %s"), *Material.ItemId.ToString());
			// Note: This shouldn't happen since we verified in first pass, but log it anyway
		}
	}

	return true;
}

ECraftingResult UHarmoniaCraftingComponent::DetermineCraftingResult(const FCraftingRecipeData& Recipe) const
{
	// Roll for critical success first
	if (Recipe.CriticalSuccessChance > 0.0f)
	{
		float CriticalRoll = FMath::FRand();
		if (CriticalRoll <= Recipe.CriticalSuccessChance)
		{
			return ECraftingResult::CriticalSuccess;
		}
	}

	// Roll for success/failure
	float SuccessRoll = FMath::FRand();
	if (SuccessRoll <= Recipe.BaseSuccessChance)
	{
		return ECraftingResult::Success;
	}

	return ECraftingResult::Failure;
}

void UHarmoniaCraftingComponent::DistributeResults(const TArray<FCraftingResultItem>& ResultItems)
{
	UHarmoniaInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
	{
		return;
	}

	for (const FCraftingResultItem& ResultItem : ResultItems)
	{
		// Roll for probability
		if (ResultItem.Probability < 1.0f)
		{
			float Roll = FMath::FRand();
			if (Roll > ResultItem.Probability)
			{
				UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::DistributeResults - Skipped item due to probability: %s (Roll: %.2f, Required: %.2f)"),
					*ResultItem.ItemId.ToString(), Roll, ResultItem.Probability);
				continue;
			}
		}

		// Add item to inventory (use MaxItemDurability for new items)
		bool bSuccess = Inventory->AddItem(ResultItem.ItemId, ResultItem.Amount, MaxItemDurability);
		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::DistributeResults - Added item: %s x%d"), *ResultItem.ItemId.ToString(), ResultItem.Amount);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::DistributeResults - Failed to add item: %s"), *ResultItem.ItemId.ToString());
		}
	}
}

void UHarmoniaCraftingComponent::PlayCraftingAnimation(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::PlayCraftingAnimation - Playing montage: %s"), *Montage->GetName());
	}
}

void UHarmoniaCraftingComponent::StopCraftingAnimation()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->GetCurrentActiveMontage())
	{
		AnimInstance->Montage_Stop(0.2f);
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::StopCraftingAnimation - Stopped montage"));
	}
}

bool UHarmoniaCraftingComponent::MeetsRecipeRequirements(const FCraftingRecipeData& Recipe) const
{
	// Check level requirement
	int32 CharacterLevel = GetCharacterLevel();
	if (CharacterLevel < Recipe.RequiredLevel)
	{
		return false;
	}

	// Check skill requirement (if applicable)
	if (Recipe.RequiredSkillLevel > 0)
	{
		int32 SkillLevel = GetCraftingSkillLevel();
		if (SkillLevel < Recipe.RequiredSkillLevel)
		{
			return false;
		}
	}

	return true;
}

//~==============================================
//~ Crafting Station System
//~==============================================

// Client request functions
void UHarmoniaCraftingComponent::RequestSetCurrentStation(ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SetCurrentStation(StationType, StationTags);
	}
	else
	{
		ServerSetCurrentStation(StationType, StationTags);
	}
}

void UHarmoniaCraftingComponent::RequestSetCurrentStationWithActor(AActor* StationActor, ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		SetCurrentStationWithActor(StationActor, StationType, StationTags);
	}
	else
	{
		ServerSetCurrentStationWithActor(StationActor, StationType, StationTags);
	}
}

void UHarmoniaCraftingComponent::RequestClearCurrentStation()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClearCurrentStation();
	}
	else
	{
		ServerClearCurrentStation();
	}
}

// Server-authoritative functions
void UHarmoniaCraftingComponent::SetCurrentStation(ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	// Server-only execution
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetCurrentStation called on client - use RequestSetCurrentStation instead"));
		return;
	}

	CurrentStation = StationType;
	CurrentStationTags = StationTags;

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::SetCurrentStation - Station set to: %d"), (int32)StationType);
}

void UHarmoniaCraftingComponent::ClearCurrentStation()
{
	// Server-only execution
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClearCurrentStation called on client - use RequestClearCurrentStation instead"));
		return;
	}

	CurrentStation = ECraftingStationType::None;
	CurrentStationTags.Reset();
	CurrentStationActor = nullptr;

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::ClearCurrentStation - Station cleared"));
}

bool UHarmoniaCraftingComponent::GetStationData(ECraftingStationType StationType, FCraftingStationData& OutStationData) const
{
	// Use cached lookup for O(1) performance
	const FCraftingStationData* Station = StationDataCache.Find(StationType);
	if (Station)
	{
		OutStationData = *Station;
		return true;
	}

	return false;
}

bool UHarmoniaCraftingComponent::CheckStationRequirement(const FCraftingRecipeData& RecipeData) const
{
	// If recipe doesn't require a station, it can be crafted anywhere
	if (RecipeData.RequiredStation == ECraftingStationType::None)
	{
		return true;
	}

	// Check if current station matches required station
	if (CurrentStation != RecipeData.RequiredStation)
	{
		return false;
	}

	// For custom stations, check tags
	if (RecipeData.RequiredStation == ECraftingStationType::Custom)
	{
		if (RecipeData.RequiredStationTags.Num() > 0)
		{
			// Check if current station has all required tags
			for (const FGameplayTag& RequiredTag : RecipeData.RequiredStationTags)
			{
				if (!CurrentStationTags.HasTag(RequiredTag))
				{
					return false;
				}
			}
		}
	}

	return true;
}

TArray<FCraftingRecipeData> UHarmoniaCraftingComponent::GetRecipesForCurrentStation(FGameplayTag CategoryTag) const
{
	TArray<FCraftingRecipeData> AvailableRecipes;

	if (!RecipeDataTable)
	{
		return AvailableRecipes;
	}

	// Get all recipes
	TArray<FCraftingRecipeData*> AllRecipes;
	RecipeDataTable->GetAllRows<FCraftingRecipeData>(TEXT("GetRecipesForCurrentStation"), AllRecipes);

	for (FCraftingRecipeData* Recipe : AllRecipes)
	{
		if (!Recipe)
		{
			continue;
		}

		// Filter by station requirement
		if (!CheckStationRequirement(*Recipe))
		{
			continue;
		}

		// Filter by category if specified
		if (CategoryTag.IsValid())
		{
			if (!Recipe->CategoryTags.HasTag(CategoryTag))
			{
				continue;
			}
		}

		// Check if recipe requires learning
		if (Recipe->bRequiresLearning && !HasLearnedRecipe(Recipe->RecipeId))
		{
			continue;
		}

		AvailableRecipes.Add(*Recipe);
	}

	return AvailableRecipes;
}

//~==============================================
//~ Server/Client RPCs
//~==============================================

void UHarmoniaCraftingComponent::ServerStartCrafting_Implementation(FHarmoniaID RecipeId)
{
	StartCrafting(RecipeId);
}

bool UHarmoniaCraftingComponent::ServerStartCrafting_Validate(FHarmoniaID RecipeId)
{
	// Reject invalid recipe IDs
	if (!RecipeId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartCrafting_Validate: Invalid RecipeId"));
		return false;
	}

	// Reject if already crafting (prevent spam)
	if (ActiveSession.bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerStartCrafting_Validate: Already crafting"));
		return false;
	}

	// Rate limiting
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Reset counter every second
	if (CurrentTime - LastAttemptResetTime >= 1.0f)
	{
		CraftingAttemptsThisSecond = 0;
		LastAttemptResetTime = CurrentTime;
	}

	// Check minimum time between crafts
	if (CurrentTime - LastCraftingAttempt < MinTimeBetweenCrafts)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Player %s: Crafting spam detected (too fast)"),
			*GetOwner()->GetName());
		return false;
	}

	// Check attempts per second
	if (CraftingAttemptsThisSecond >= MaxCraftingAttemptsPerSecond)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Player %s: Too many crafting attempts per second"),
			*GetOwner()->GetName());
		return false;
	}

	LastCraftingAttempt = CurrentTime;
	CraftingAttemptsThisSecond++;

	return true;
}

void UHarmoniaCraftingComponent::ServerCancelCrafting_Implementation()
{
	CancelCrafting();
}

bool UHarmoniaCraftingComponent::ServerCancelCrafting_Validate()
{
	return true;
}

void UHarmoniaCraftingComponent::ServerLearnRecipe_Implementation(FHarmoniaID RecipeId)
{
	LearnRecipe(RecipeId);
}

bool UHarmoniaCraftingComponent::ServerLearnRecipe_Validate(FHarmoniaID RecipeId)
{
	// Verify recipe ID is valid
	if (!RecipeId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerLearnRecipe_Validate: Invalid RecipeId"));
		return false;
	}

	// Verify recipe exists in data table
	FCraftingRecipeData RecipeData;
	if (!GetRecipeData(RecipeId, RecipeData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Player %s: Attempted to learn non-existent recipe %s"),
			*GetOwner()->GetName(), *RecipeId.ToString());
		return false;
	}

	// Prevent learning the same recipe multiple times (spam)
	if (HasLearnedRecipe(RecipeId))
	{
		return false; // Already learned, reject silently
	}

	return true;
}

void UHarmoniaCraftingComponent::ClientCraftingCompleted_Implementation(FHarmoniaID RecipeId, ECraftingResult Result, const TArray<FCraftingResultItem>& ResultItems)
{
	// Client-side notification (server already broadcasted via OnCraftingCompleted)
}

void UHarmoniaCraftingComponent::ClientCraftingCancelled_Implementation(FHarmoniaID RecipeId, float ProgressLost)
{
	// Client-side notification (server already broadcasted via OnCraftingCancelled)
}

void UHarmoniaCraftingComponent::MulticastPlayCraftingAnimation_Implementation(UAnimMontage* Montage)
{
	// Play animation on all clients (and server)
	PlayCraftingAnimation(Montage);
}

void UHarmoniaCraftingComponent::MulticastStopCraftingAnimation_Implementation()
{
	// Stop animation on all clients (and server)
	StopCraftingAnimation();
}

//~==============================================
//~ Save/Load System
//~==============================================

FLearnedRecipeSaveData UHarmoniaCraftingComponent::GetLearnedRecipesSaveData() const
{
	FLearnedRecipeSaveData SaveData;
	SaveData.LearnedRecipes = LearnedRecipes;
	return SaveData;
}

void UHarmoniaCraftingComponent::LoadLearnedRecipesFromSaveData(const FLearnedRecipeSaveData& SaveData)
{
	LearnedRecipes = SaveData.LearnedRecipes;
}

//~==============================================
//~ Utility Functions
//~==============================================

UHarmoniaInventoryComponent* UHarmoniaCraftingComponent::GetInventoryComponent()
{
	if (!InventoryComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
		}
	}

	return InventoryComponent;
}

int32 UHarmoniaCraftingComponent::GetCharacterLevel() const
{
	// Default implementation - override in subclass if you have a level system
	// For now, return a high level so requirements are always met
	return 9999;
}

int32 UHarmoniaCraftingComponent::GetCraftingSkillLevel() const
{
	// Default implementation - override in subclass if you have a skill system
	// For now, return a high level so requirements are always met
	return 9999;
}

void UHarmoniaCraftingComponent::CacheConfigurationData()
{
	// Cache grade configs for fast O(1) lookups
	GradeConfigCache.Empty();
	if (GradeConfigDataTable)
	{
		TArray<FItemGradeConfig*> AllGradeConfigs;
		GradeConfigDataTable->GetAllRows<FItemGradeConfig>(TEXT("CacheGradeConfig"), AllGradeConfigs);
		for (FItemGradeConfig* Config : AllGradeConfigs)
		{
			if (Config)
			{
				GradeConfigCache.Add(Config->Grade, *Config);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::CacheConfigurationData - Cached %d grade configs"), GradeConfigCache.Num());
	}

	// Cache station data for fast O(1) lookups
	StationDataCache.Empty();
	if (StationDataTable)
	{
		TArray<FCraftingStationData*> AllStations;
		StationDataTable->GetAllRows<FCraftingStationData>(TEXT("CacheStationData"), AllStations);
		for (FCraftingStationData* Station : AllStations)
		{
			if (Station)
			{
				StationDataCache.Add(Station->StationType, *Station);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::CacheConfigurationData - Cached %d station configs"), StationDataCache.Num());
	}
}

bool UHarmoniaCraftingComponent::VerifyStationDistance() const
{
	if (CurrentStation == ECraftingStationType::None)
	{
		return true; // Hand crafting, no station needed
	}

	if (!CurrentStationActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Player %s: No station actor set but station type is %d"),
			*GetOwner()->GetName(), (int32)CurrentStation);
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	float Distance = FVector::Dist(Owner->GetActorLocation(), CurrentStationActor->GetActorLocation());

	if (Distance > MaxStationInteractionDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] Player %s: Too far from station (Distance: %.2f, Max: %.2f)"),
			*GetOwner()->GetName(), Distance, MaxStationInteractionDistance);
		return false;
	}

	return true;
}

void UHarmoniaCraftingComponent::SetCurrentStationWithActor(AActor* StationActor, ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	// Server-only execution
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetCurrentStationWithActor called on client - use RequestSetCurrentStationWithActor instead"));
		return;
	}

	CurrentStation = StationType;
	CurrentStationTags = StationTags;
	CurrentStationActor = StationActor;

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::SetCurrentStationWithActor - Station set to: %d with actor: %s"),
		(int32)StationType, *GetNameSafe(StationActor));
}

// Server RPC implementations
void UHarmoniaCraftingComponent::ServerSetCurrentStation_Implementation(ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	SetCurrentStation(StationType, StationTags);
}

bool UHarmoniaCraftingComponent::ServerSetCurrentStation_Validate(ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	// Anti-cheat: Validate station type is valid
	if (StationType == ECraftingStationType::None)
	{
		return true; // Allow clearing station
	}

	// Validate station type exists in data
	FCraftingStationData StationData;
	if (!GetStationData(StationType, StationData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSetCurrentStation: Invalid StationType %d"), (int32)StationType);
		return false;
	}

	return true;
}

void UHarmoniaCraftingComponent::ServerSetCurrentStationWithActor_Implementation(AActor* StationActor, ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	// Validate station actor distance
	if (StationActor && GetOwner())
	{
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), StationActor->GetActorLocation());
		if (Distance > MaxStationInteractionDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("ServerSetCurrentStationWithActor: Station too far (Distance: %.1f, Max: %.1f)"),
				Distance, MaxStationInteractionDistance);
			return;
		}
	}

	SetCurrentStationWithActor(StationActor, StationType, StationTags);
}

bool UHarmoniaCraftingComponent::ServerSetCurrentStationWithActor_Validate(AActor* StationActor, ECraftingStationType StationType, FGameplayTagContainer StationTags)
{
	// Anti-cheat: Validate station type
	if (StationType == ECraftingStationType::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSetCurrentStationWithActor: Cannot set None with actor"));
		return false;
	}

	// Validate station type exists
	FCraftingStationData StationData;
	if (!GetStationData(StationType, StationData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSetCurrentStationWithActor: Invalid StationType %d"), (int32)StationType);
		return false;
	}

	// Validate station actor exists
	if (!StationActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSetCurrentStationWithActor: StationActor is null"));
		return false;
	}

	// Validate distance (2x buffer for network lag)
	if (GetOwner())
	{
		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), StationActor->GetActorLocation());
		if (Distance > MaxStationInteractionDistance * 2.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSetCurrentStationWithActor: Station too far (Distance: %.1f, Max: %.1f)"),
				Distance, MaxStationInteractionDistance * 2.0f);
			return false;
		}
	}

	return true;
}

void UHarmoniaCraftingComponent::ServerClearCurrentStation_Implementation()
{
	ClearCurrentStation();
}

//~==============================================
//~ Debug
//~==============================================

#if WITH_EDITOR
void UHarmoniaCraftingComponent::Debug_LearnAllRecipes()
{
	if (!RecipeDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::Debug_LearnAllRecipes - RecipeDataTable is null!"));
		return;
	}

	TArray<FCraftingRecipeData*> AllRecipes;
	RecipeDataTable->GetAllRows<FCraftingRecipeData>(TEXT("Debug_LearnAllRecipes"), AllRecipes);

	for (FCraftingRecipeData* Recipe : AllRecipes)
	{
		if (Recipe)
		{
			LearnRecipe(Recipe->RecipeId);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::Debug_LearnAllRecipes - Learned %d recipes"), AllRecipes.Num());
}

void UHarmoniaCraftingComponent::Debug_InstantCraft()
{
	if (!ActiveSession.bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaCraftingComponent::Debug_InstantCraft - No active crafting session!"));
		return;
	}

	// Set elapsed time to completion
	ActiveSession.ElapsedTime = ActiveSession.TotalCastingTime;

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaCraftingComponent::Debug_InstantCraft - Instantly completing crafting"));
}
#endif
