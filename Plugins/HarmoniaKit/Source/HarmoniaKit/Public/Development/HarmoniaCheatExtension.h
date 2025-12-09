// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "Debug/DebugDrawService.h"
#include "HarmoniaCheatExtension.generated.h"

class UCanvas;

/**
 * UHarmoniaCheatExtension
 *
 * Cheat Manager Extension for HarmoniaKit specific debug commands.
 * Supports showdebug-style persistent debug display.
 * 
 * Usage:
 *   showdebug harmonia       - Toggle all Harmonia debug info
 *   showdebug harmonia.stamina - Toggle stamina display only
 *   showdebug harmonia.tags    - Toggle gameplay tags display only
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCheatExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UHarmoniaCheatExtension();
	virtual ~UHarmoniaCheatExtension();

	// Toggle Harmonia debug display (legacy command, prefer showdebug harmonia)
	UFUNCTION(Exec)
	void HarmoniaDebug(bool bEnable);

	// One-shot stamina display
	UFUNCTION(Exec)
	void HarmoniaStamina();

	// One-shot gameplay tags display
	UFUNCTION(Exec)
	void HarmoniaTags();

protected:
	// Debug draw callback for UDebugDrawService
	void DrawDebug(UCanvas* Canvas, APlayerController* PC);

	// Register/Unregister debug draw delegate
	void RegisterDebugDraw();
	void UnregisterDebugDraw();

	// Individual draw functions
	void DrawStaminaDebug(UCanvas* Canvas, float& YPos);
	void DrawTagsDebug(UCanvas* Canvas, float& YPos);
	void DrawCombatDebug(UCanvas* Canvas, float& YPos);

	// Debug draw delegate handle
	FDelegateHandle DebugDrawDelegateHandle;

	// Debug display flags
	bool bShowStamina = false;
	bool bShowTags = false;
	bool bShowCombat = false;
	bool bShowAll = false;
};
