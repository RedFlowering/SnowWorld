// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaWorkshopIntegration.h"
#include "System/HarmoniaModSubsystem.h"
#include "Misc/Paths.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Engine/GameInstance.h"

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

	// Steam Workshop subscription via IOnlineSubsystem
	// Note: Actual subscription is handled by Steam client
	// We open the Steam Workshop page for the user to subscribe
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
		if (ExternalUI.IsValid())
		{
			// Open Steam Workshop page for this item
			FString WorkshopURL = FString::Printf(TEXT("steam://url/CommunityFilePage/%lld"), ItemId);
			ExternalUI->ShowWebURL(WorkshopURL, FShowWebUrlParams(), FOnShowWebUrlClosedDelegate());

			UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Opened Steam Workshop page for item: %lld"), ItemId);
		}
	}

	// Track locally
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

	// Steam Workshop unsubscription via IOnlineSubsystem
	// Note: Actual unsubscription is handled by Steam client
	// We open the Steam Workshop page for the user to unsubscribe
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
		if (ExternalUI.IsValid())
		{
			// Open Steam Workshop page for this item
			FString WorkshopURL = FString::Printf(TEXT("steam://url/CommunityFilePage/%lld"), ItemId);
			ExternalUI->ShowWebURL(WorkshopURL, FShowWebUrlParams(), FOnShowWebUrlClosedDelegate());

			UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Opened Steam Workshop page for item: %lld"), ItemId);
		}
	}

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

	// Steam Workshop download via IOnlineSubsystem
	// Note: Steam client automatically downloads subscribed items
	// We can use ISteamUGC::DownloadItem() from Steamworks SDK for priority downloads
	// However, UE's IOnlineSubsystem doesn't directly expose this
	//
	// In production, you would either:
	// 1. Use Steamworks SDK directly via the Steam plugin
	// 2. Wait for Steam client to auto-download subscribed items
	// 3. Check download state periodically using ISteamUGC::GetItemState()

	// For now, mark as downloading and track it
	WorkshopItems[ItemId].DownloadStatus = EHarmoniaWorkshopDownloadStatus::Downloading;
	ActiveDownloads.Add(ItemId, 0.0f);

	// In a real implementation, you would:
	// - Call ISteamUGC::DownloadItem() if high priority
	// - Set up callbacks for download progress
	// - Monitor download completion via ISteamUGC::GetItemDownloadInfo()

	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Steam Workshop downloads are handled by Steam client automatically"));
	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("For direct download control, integrate Steamworks SDK ISteamUGC interface"));

	return true;
}

int32 UHarmoniaWorkshopIntegration::CheckForUpdates()
{
	if (!bWorkshopAvailable)
	{
		return 0;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Checking for workshop item updates..."));

	// Steam Workshop update check via IOnlineSubsystem
	// Note: Steam client automatically detects and downloads updates
	// We can check item state using ISteamUGC::GetItemState() from Steamworks SDK
	//
	// Item states include:
	// - k_EItemStateNeedsUpdate: Item needs an update
	// - k_EItemStateDownloading: Item is currently downloading
	// - k_EItemStateDownloadPending: Item download is pending

	int32 UpdateCount = 0;

	for (auto& Pair : WorkshopItems)
	{
		if (Pair.Value.bIsSubscribed)
		{
			// In a real implementation, you would:
			// - Call ISteamUGC::GetItemState() for each subscribed item
			// - Check if state includes k_EItemStateNeedsUpdate flag
			// - Update bNeedsUpdate flag accordingly
			//
			// Example (pseudo-code):
			// uint32 ItemState = SteamUGC()->GetItemState(ItemId);
			// if (ItemState & k_EItemStateNeedsUpdate)
			// {
			//     Pair.Value.bNeedsUpdate = true;
			//     UpdateCount++;
			// }
		}
	}

	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Update checking requires Steamworks SDK ISteamUGC interface"));
	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Steam client handles updates automatically for subscribed items"));

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

	// Steam Workshop item list refresh via IOnlineSubsystem
	// Use ISteamUGC::GetSubscribedItems() from Steamworks SDK
	//
	// The process involves:
	// 1. Call ISteamUGC::GetNumSubscribedItems() to get count
	// 2. Allocate array for PublishedFileId_t items
	// 3. Call ISteamUGC::GetSubscribedItems() to get all subscribed item IDs
	// 4. For each item, load metadata using LoadWorkshopItemMetadata()
	// 5. Check installation status using ISteamUGC::GetItemInstallInfo()
	//
	// In a real implementation with Steamworks SDK:
	/*
	uint32 NumSubscribed = SteamUGC()->GetNumSubscribedItems();
	TArray<PublishedFileId_t> SubscribedItems;
	SubscribedItems.SetNum(NumSubscribed);

	uint32 NumRetrieved = SteamUGC()->GetSubscribedItems(SubscribedItems.GetData(), NumSubscribed);

	for (uint32 i = 0; i < NumRetrieved; i++)
	{
		PublishedFileId_t ItemId = SubscribedItems[i];

		// Check if item is installed
		uint64 SizeOnDisk;
		TCHAR InstallFolder[1024];
		uint32 TimeStamp;
		bool bIsInstalled = SteamUGC()->GetItemInstallInfo(ItemId, &SizeOnDisk, InstallFolder, 1024, &TimeStamp);

		// Load/update metadata for this item
		LoadWorkshopItemMetadata(ItemId);
	}
	*/

	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Refreshing items requires Steamworks SDK ISteamUGC interface"));
	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Use GetNumSubscribedItems() and GetSubscribedItems()"));

	return true;
}

bool UHarmoniaWorkshopIntegration::InitializeWorkshop()
{
	// Initialize Steam Workshop via IOnlineSubsystem
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("No Online Subsystem available"));
		return false;
	}

	// Check if this is the Steam subsystem
	if (OnlineSubsystem->GetSubsystemName() != FName(TEXT("Steam")))
	{
		UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Workshop integration only available with Steam (current: %s)"),
			*OnlineSubsystem->GetSubsystemName().ToString());
		return false;
	}

	// Check if external UI interface is available (used for Workshop)
	IOnlineExternalUIPtr ExternalUI = OnlineSubsystem->GetExternalUIInterface();
	if (!ExternalUI.IsValid())
	{
		UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Steam External UI interface not available"));
		return false;
	}

	UE_LOG(LogHarmoniaWorkshop, Log, TEXT("Steam Workshop initialized successfully"));
	return true;
}

bool UHarmoniaWorkshopIntegration::LoadWorkshopItemMetadata(int64 ItemId)
{
	// Load workshop item metadata from Steam API
	// Use ISteamUGC::RequestUGCDetails() from Steamworks SDK
	//
	// The process involves:
	// 1. Call ISteamUGC::CreateQueryUGCDetailsRequest() with the ItemId
	// 2. Call ISteamUGC::SendQueryUGCRequest() to send the query
	// 3. Handle the SteamUGCQueryCompleted_t callback
	// 4. Extract metadata using ISteamUGC::GetQueryUGCResult()
	//
	// Metadata includes:
	// - Title (m_rgchTitle)
	// - Description (m_rgchDescription)
	// - File size (m_nFileSize)
	// - Owner (m_ulSteamIDOwner)
	// - Subscriptions count (m_unNumSubscriptions)
	// - Install path (ISteamUGC::GetItemInstallInfo)
	//
	// In a real implementation with Steamworks SDK:
	/*
	UGCQueryHandle_t QueryHandle = SteamUGC()->CreateQueryUGCDetailsRequest(&ItemId, 1);
	SteamAPICall_t APICall = SteamUGC()->SendQueryUGCRequest(QueryHandle);
	// Register callback for SteamUGCQueryCompleted_t
	// On callback, parse results and update WorkshopItems map
	*/

	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Loading item metadata requires Steamworks SDK ISteamUGC interface"));
	UE_LOG(LogHarmoniaWorkshop, Warning, TEXT("Use CreateQueryUGCDetailsRequest() and SendQueryUGCRequest()"));

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
