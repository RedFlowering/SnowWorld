// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HarmoniaSaveInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHarmoniaSaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that need to save/load state
 */
class HARMONIALOADMANAGER_API IHarmoniaSaveInterface
{
	GENERATED_BODY()

public:
	/** Return binary data to save */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	TArray<uint8> GetSaveData();

	/** Restore state from binary data */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void LoadSaveData(const TArray<uint8>& Data);

	/** Get unique ID for this actor (for mapping save data) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	FString GetUniqueSaveId();
};
