// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaModSystemDefinitions.h"
#include "HarmoniaWorkshopIntegration.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaWorkshop, Log, All);

/**
 * Workshop item download status
 */
UENUM(BlueprintType)
enum class EHarmoniaWorkshopDownloadStatus : uint8
{
	NotStarted = 0 UMETA(DisplayName = "Not Started"),
	Downloading = 1 UMETA(DisplayName = "Downloading"),
	Completed = 2 UMETA(DisplayName = "Completed"),
	Failed = 3 UMETA(DisplayName = "Failed")
};

/**
 * Workshop item information
 */
USTRUCT(BlueprintType)
struct FHarmoniaWorkshopItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 ItemId = 0;

	UPROPERTY(BlueprintReadOnly)
	FString Title;

	UPROPERTY(BlueprintReadOnly)
	FString Description;

	UPROPERTY(BlueprintReadOnly)
	FString Author;

	UPROPERTY(BlueprintReadOnly)
	FString Version;

	UPROPERTY(BlueprintReadOnly)
	int32 SubscriberCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 FileSize = 0;

	UPROPERTY(BlueprintReadOnly)
	FString InstallPath;

	UPROPERTY(BlueprintReadOnly)
	EHarmoniaWorkshopDownloadStatus DownloadStatus = EHarmoniaWorkshopDownloadStatus::NotStarted;

	UPROPERTY(BlueprintReadOnly)
	bool bIsSubscribed = false;

	UPROPERTY(BlueprintReadOnly)
	bool bNeedsUpdate = false;
};

/**
 * Delegate fired when a workshop item is downloaded
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorkshopItemDownloaded, int64, ItemId, bool, bSuccess);

/**
 * Delegate fired when workshop item download progress updates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWorkshopDownloadProgress, int64, ItemId, int64, BytesDownloaded, int64, TotalBytes);

/**
 * Steam Workshop integration subsystem
 * Handles automatic mod download and update from Steam Workshop
 */
UCLASS()
class HARMONIAMODSYSTEM_API UHarmoniaWorkshopIntegration : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	/**
	 * Check if Steam Workshop is available
	 * @return True if Workshop API is available
	 */
	UFUNCTION(BlueprintPure, Category = "Workshop Integration")
	bool IsWorkshopAvailable() const;

	/**
	 * Subscribe to a workshop item
	 * @param ItemId - Steam Workshop item ID
	 * @return True if subscription initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	bool SubscribeToItem(int64 ItemId);

	/**
	 * Unsubscribe from a workshop item
	 * @param ItemId - Steam Workshop item ID
	 * @return True if unsubscription initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	bool UnsubscribeFromItem(int64 ItemId);

	/**
	 * Get all subscribed workshop items
	 * @return Array of subscribed items
	 */
	UFUNCTION(BlueprintPure, Category = "Workshop Integration")
	TArray<FHarmoniaWorkshopItem> GetSubscribedItems() const;

	/**
	 * Get workshop item information
	 * @param ItemId - Steam Workshop item ID
	 * @param OutItem - Output item information
	 * @return True if item found
	 */
	UFUNCTION(BlueprintPure, Category = "Workshop Integration")
	bool GetWorkshopItem(int64 ItemId, FHarmoniaWorkshopItem& OutItem) const;

	/**
	 * Download a workshop item
	 * @param ItemId - Steam Workshop item ID
	 * @param bHighPriority - Should this download be prioritized?
	 * @return True if download started
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	bool DownloadItem(int64 ItemId, bool bHighPriority = false);

	/**
	 * Check for updates to subscribed items
	 * @return Number of items with available updates
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	int32 CheckForUpdates();

	/**
	 * Download all pending updates
	 * @return Number of updates initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	int32 DownloadAllUpdates();

	/**
	 * Get download progress for an item
	 * @param ItemId - Steam Workshop item ID
	 * @param OutBytesDownloaded - Bytes downloaded so far
	 * @param OutTotalBytes - Total bytes to download
	 * @return True if download is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Workshop Integration")
	bool GetDownloadProgress(int64 ItemId, int64& OutBytesDownloaded, int64& OutTotalBytes) const;

	/**
	 * Install a downloaded workshop item as a mod
	 * @param ItemId - Steam Workshop item ID
	 * @return True if installation successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	bool InstallWorkshopMod(int64 ItemId);

	/**
	 * Refresh workshop item list from Steam
	 * @return True if refresh successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Workshop Integration")
	bool RefreshWorkshopItems();

	// ========================================
	// Events
	// ========================================

	/** Event fired when a workshop item is downloaded */
	UPROPERTY(BlueprintAssignable, Category = "Workshop Integration|Events")
	FOnWorkshopItemDownloaded OnWorkshopItemDownloaded;

	/** Event fired when download progress updates */
	UPROPERTY(BlueprintAssignable, Category = "Workshop Integration|Events")
	FOnWorkshopDownloadProgress OnWorkshopDownloadProgress;

protected:
	/**
	 * Initialize Steam Workshop connection
	 * @return True if initialized successfully
	 */
	bool InitializeWorkshop();

	/**
	 * Load workshop item metadata
	 * @param ItemId - Steam Workshop item ID
	 * @return True if loaded successfully
	 */
	bool LoadWorkshopItemMetadata(int64 ItemId);

	/**
	 * Handle workshop item download complete
	 * @param ItemId - Steam Workshop item ID
	 * @param bSuccess - Whether download succeeded
	 */
	void OnItemDownloadComplete(int64 ItemId, bool bSuccess);

private:
	/** Cached workshop items */
	UPROPERTY()
	TMap<int64, FHarmoniaWorkshopItem> WorkshopItems;

	/** Is Workshop API available? */
	bool bWorkshopAvailable;

	/** Active downloads (ItemId -> Progress) */
	UPROPERTY()
	TMap<int64, float> ActiveDownloads;
};
