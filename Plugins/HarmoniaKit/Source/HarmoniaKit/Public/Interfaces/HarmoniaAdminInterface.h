// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HarmoniaAdminInterface.generated.h"

/**
 * Admin permission levels for sensitive operations
 */
UENUM(BlueprintType)
enum class EHarmoniaAdminLevel : uint8
{
	None = 0,		// Regular player, no admin permissions
	Moderator = 1,	// Can use some admin commands (kick, mute, etc.)
	Admin = 2,		// Full admin permissions (clear inventory, spawn items, etc.)
	Owner = 3		// Server owner, all permissions
};

UINTERFACE(MinimalAPI, Blueprintable)
class UHarmoniaAdminInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for checking admin permissions
 * Implement this in your PlayerState or PlayerController to provide admin functionality
 */
class HARMONIAKIT_API IHarmoniaAdminInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the admin level of this player
	 * @return The admin level of this player
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Admin")
	EHarmoniaAdminLevel GetAdminLevel() const;
	virtual EHarmoniaAdminLevel GetAdminLevel_Implementation() const { return EHarmoniaAdminLevel::None; }

	/**
	 * Check if this player has at least the specified admin level
	 * @param RequiredLevel The minimum admin level required
	 * @return True if the player has sufficient permissions
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Admin")
	bool HasAdminPermission(EHarmoniaAdminLevel RequiredLevel) const;
	virtual bool HasAdminPermission_Implementation(EHarmoniaAdminLevel RequiredLevel) const
	{
		return GetAdminLevel() >= RequiredLevel;
	}
};
