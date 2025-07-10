// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "HarmoniaProjectSettings.generated.h"

USTRUCT()
struct FHarmoniaDataTableSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category = "DataTable")
	FString FunctionName;

	UPROPERTY(EditAnywhere, config, Category = "DataTable")
	TSoftObjectPtr<UDataTable> Table;
};

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Harmonia Load Manager"))
class HARMONIALOADMANAGER_API UHarmoniaProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "Harmonia")
	TArray<FHarmoniaDataTableSetting> Tables;

private:
#if WITH_EDITOR
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	virtual FName GetSectionName() const override { return TEXT("Harmonia Load Manager"); }

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
