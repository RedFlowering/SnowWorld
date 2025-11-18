// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaWorkshopIntegration.h"
#include "System/HarmoniaModSubsystem.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogHarmoniaWorkshop);

void UHarmoniaWorkshopIntegration::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Initializing Harmonia Workshop Integration"));

	bWorkshopAvailable = InitializeWorkshop();

	if (!bWorkshopAvailable)
	{
		UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Steam Workshop is not available"));
	}

	WorkshopItems.Empty();
	ActiveDownloads.Empty();
}

void UHarmoniaWorkshopIntegration::Deinitialize()
{
	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Deinitializing Harmonia Workshop Integration"));

	WorkshopItems.Empty();
	ActiveDownloads.Empty();

	Super::Deinitialize();
}

bool UHarmoniaWorkshopIntegration::IsWorkshopAvailable() const
{
	return bWorkshopAvailable;
}

bool UHarmoniaWorkshopIntegration::SubscribeToItem(int64 ItemId)
{
	if (!bWorkshopAvailable)
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop not available"));
		return false;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Subscribing to workshop item: %lld"), ItemId);

	// TODO: Implement Steam Workshop subscription via IOnlineSubsystem
	// For now, just track locally
	if (!WorkshopItems.Contains(ItemId))
	{
		FHarmoniaWorkshopItem NewItem;
		NewItem.ItemId = ItemId;
		NewItem.bIsSubscribed = true;
		NewItem.DownloadStatus = EHarmoniaWorkshopDownloadStatus::NotStarted;
		WorkshopItems.Add(ItemId, NewItem);

		// Auto-download on subscribe
		DownloadItem(ItemId, false);
	}
	else
	{
		WorkshopItems[ItemId].bIsSubscribed = true;
	}

	return true;
}

bool UHarmoniaWorkshopIntegration::UnsubscribeFromItem(int64 ItemId)
{
	if (!bWorkshopAvailable)
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop not available"));
		return false;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Unsubscribing from workshop item: %lld"), ItemId);

	// TODO: Implement Steam Workshop unsubscription via IOnlineSubsystem

	if (WorkshopItems.Contains(ItemId))
	{
		WorkshopItems[ItemId].bIsSubscribed = false;
	}

	return true;
}

TArray<FHarmoniaWorkshopItem> UHarmoniaWorkshopIntegration::GetSubscribedItems() const
{
	TArray<FHarmoniaWorkshopItem> SubscribedItems;

	for (const auto& Pair : WorkshopItems)
	{
		if (Pair.Value.bIsSubscribed)
		{
			SubscribedItems.Add(Pair.Value);
		}
	}

	return SubscribedItems;
}

bool UHarmoniaWorkshopIntegration::GetWorkshopItem(int64 ItemId, FHarmoniaWorkshopItem& OutItem) const
{
	if (WorkshopItems.Contains(ItemId))
	{
		OutItem = WorkshopItems[ItemId];
		return true;
	}

	return false;
}

bool UHarmoniaWorkshopIntegration::DownloadItem(int64 ItemId, bool bHighPriority)
{
	if (!bWorkshopAvailable)
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop not available"));
		return false;
	}

	if (!WorkshopItems.Contains(ItemId))
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop item %lld not found"), ItemId);
		return false;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Downloading workshop item: %lld (Priority: %s)"),
		ItemId, bHighPriority ? TEXT("High") : TEXT("Normal"));

	// TODO: Implement Steam Workshop download via IOnlineSubsystem
	// For now, simulate download
	WorkshopItems[ItemId].DownloadStatus = EHarmoniaWorkshopDownloadStatus::Downloading;
	ActiveDownloads.Add(ItemId, 0.0f);

	// Simulate successful download (in real implementation, this would be async)
	// OnItemDownloadComplete(ItemId, true);

	return true;
}

int32 UHarmoniaWorkshopIntegration::CheckForUpdates()
{
	if (!bWorkshopAvailable)
	{
		return 0;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Checking for workshop item updates..."));

	// TODO: Implement Steam Workshop update check via IOnlineSubsystem
	int32 UpdateCount = 0;

	for (auto& Pair : WorkshopItems)
	{
		if (Pair.Value.bIsSubscribed)
		{
			// Check if update available (stub)
			// In real implementation, query Steam Workshop API
		}
	}

	return UpdateCount;
}

int32 UHarmoniaWorkshopIntegration::DownloadAllUpdates()
{
	if (!bWorkshopAvailable)
	{
		return 0;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Downloading all workshop updates..."));

	int32 DownloadCount = 0;

	for (auto& Pair : WorkshopItems)
	{
		if (Pair.Value.bIsSubscribed && Pair.Value.bNeedsUpdate)
		{
			if (DownloadItem(Pair.Key, false))
			{
				DownloadCount++;
			}
		}
	}

	return DownloadCount;
}

bool UHarmoniaWorkshopIntegration::GetDownloadProgress(int64 ItemId, int64& OutBytesDownloaded, int64& OutTotalBytes) const
{
	if (!ActiveDownloads.Contains(ItemId))
	{
		return false;
	}

	if (WorkshopItems.Contains(ItemId))
	{
		OutTotalBytes = WorkshopItems[ItemId].FileSize;
		OutBytesDownloaded = static_cast<int64>(ActiveDownloads[ItemId] * OutTotalBytes);
		return true;
	}

	return false;
}

bool UHarmoniaWorkshopIntegration::InstallWorkshopMod(int64 ItemId)
{
	if (!WorkshopItems.Contains(ItemId))
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop item %lld not found"), ItemId);
		return false;
	}

	FHarmoniaWorkshopItem& Item = WorkshopItems[ItemId];

	if (Item.DownloadStatus != EHarmoniaWorkshopDownloadStatus::Completed)
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop item %lld not downloaded yet"), ItemId);
		return false;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Installing workshop mod: %lld"), ItemId);

	// Get mod subsystem
	UHarmoniaModSubsystem* ModSubsystem = GetGameInstance()->GetSubsystem<UHarmoniaModSubsystem>();
	if (!ModSubsystem)
	{
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Mod subsystem not available"));
		return false;
	}

	// Add workshop install path to mod search paths if not already there
	FString WorkshopModsPath = FPaths::ProjectDir() / TEXT("Mods/Workshop/");

	// Discover and load the mod
	ModSubsystem->DiscoverMods();

	// Find mod by workshop ID (mods should specify WorkshopItemId in their ModInfo.json)
	TArray<FHarmoniaModInfo> AllMods = ModSubsystem->GetAllMods();
	for (const FHarmoniaModInfo& ModInfo : AllMods)
	{
		if (ModInfo.WorkshopItemId == ItemId)
		{
			FString ErrorMessage;
			if (ModSubsystem->LoadMod(ModInfo.ModId, ErrorMessage))
			{
				UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Successfully installed workshop mod: %s"), *ModInfo.ModId.ToString());
				return true;
			}
			else
			{
				UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Failed to load workshop mod: %s"), *ErrorMessage);
				return false;
			}
		}
	}

	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Workshop mod %lld not found in mod list"), ItemId);
	return false;
}

bool UHarmoniaWorkshopIntegration::RefreshWorkshopItems()
{
	if (!bWorkshopAvailable)
	{
		return false;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Refreshing workshop items..."));

	// TODO: Implement Steam Workshop item list refresh via IOnlineSubsystem

	return true;
}

bool UHarmoniaWorkshopIntegration::InitializeWorkshop()
{
	// TODO: Initialize Steam Workshop via IOnlineSubsystem
	// Check if Steam is available and user is logged in

	// For now, assume not available unless Steam is properly configured
	return false;
}

bool UHarmoniaWorkshopIntegration::LoadWorkshopItemMetadata(int64 ItemId)
{
	// TODO: Load workshop item metadata from Steam API

	return false;
}

void UHarmoniaWorkshopIntegration::OnItemDownloadComplete(int64 ItemId, bool bSuccess)
{
	if (!WorkshopItems.Contains(ItemId))
	{
		return;
	}

	if (bSuccess)
	{
		WorkshopItems[ItemId].DownloadStatus = EHarmoniaWorkshopDownloadStatus::Completed;
		UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Workshop item %lld downloaded successfully"), ItemId);
	}
	else
	{
		WorkshopItems[ItemId].DownloadStatus = EHarmoniaWorkshopDownloadStatus::Failed;
		UE_LOG(LogHarmoniaWorkshop, Error, TEXT("Workshop item %lld download failed"), ItemId);
	}

	ActiveDownloads.Remove(ItemId);

	// Broadcast event
	OnWorkshopItemDownloaded.Broadcast(ItemId, bSuccess);

	// Auto-install if successful
	if (bSuccess)
	{
		InstallWorkshopMod(ItemId);
	}
}
