// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaDataTablePatcher.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY(LogHarmoniaDataTablePatcher);

void UHarmoniaDataTablePatcher::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Initializing Harmonia Data Table Patcher"));

	OriginalValues.Empty();
	ActivePatches.Empty();
	PatchesByMod.Empty();
	DataTableCache.Empty();
}

void UHarmoniaDataTablePatcher::Deinitialize()
{
	UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Deinitializing Harmonia Data Table Patcher"));

	RevertAllPatches();

	Super::Deinitialize();
}

bool UHarmoniaDataTablePatcher::ApplyPatch(const FHarmoniaDataTablePatch& Patch, FName ModId, FHarmoniaPatchResult& OutResult)
{
	OutResult = FHarmoniaPatchResult();

	// Validate patch
	FString ErrorMessage;
	if (!ValidatePatch(Patch, ErrorMessage))
	{
		OutResult.ErrorMessage = ErrorMessage;
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("Invalid patch: %s"), *ErrorMessage);
		return false;
	}

	// Load data table
	UDataTable* DataTable = nullptr;

	if (DataTableCache.Contains(Patch.TargetDataTable))
	{
		DataTable = DataTableCache[Patch.TargetDataTable];
	}
	else
	{
		DataTable = Cast<UDataTable>(Patch.TargetDataTable.TryLoad());
		if (DataTable)
		{
			DataTableCache.Add(Patch.TargetDataTable, DataTable);
		}
	}

	if (!DataTable)
	{
		OutResult.ErrorMessage = FString::Printf(TEXT("Failed to load data table: %s"), *Patch.TargetDataTable.ToString());
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("%s"), *OutResult.ErrorMessage);
		return false;
	}

	// Find row
	uint8* RowData = DataTable->FindRowUnchecked(Patch.RowName);
	if (!RowData)
	{
		OutResult.ErrorMessage = FString::Printf(TEXT("Row '%s' not found in table"), *Patch.RowName.ToString());
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("%s"), *OutResult.ErrorMessage);
		return false;
	}

	// Find property
	FProperty* Property = DataTable->GetRowStruct()->FindPropertyByName(Patch.PropertyName);
	if (!Property)
	{
		OutResult.ErrorMessage = FString::Printf(TEXT("Property '%s' not found in row struct"), *Patch.PropertyName.ToString());
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("%s"), *OutResult.ErrorMessage);
		return false;
	}

	// Generate patch key
	FString PatchKey = GeneratePatchKey(Patch.TargetDataTable, Patch.RowName, Patch.PropertyName);

	// Backup original value if not already backed up
	if (!OriginalValues.Contains(PatchKey))
	{
		FString OriginalValue = GetPropertyValueAsString(Property, RowData);
		OriginalValues.Add(PatchKey, OriginalValue);
		OutResult.OldValue = OriginalValue;
	}
	else
	{
		OutResult.OldValue = OriginalValues[PatchKey];
	}

	// Apply patch operation
	if (!ApplyPatchOperation(Property, RowData, Patch, OutResult))
	{
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("Failed to apply patch operation: %s"), *OutResult.ErrorMessage);
		return false;
	}

	// Register patch
	ActivePatches.Add(PatchKey, Patch);

	// Track by mod
	if (!PatchesByMod.Contains(ModId))
	{
		PatchesByMod.Add(ModId, FHarmoniaPatchKeyArray());
	}
	PatchesByMod[ModId].PatchKeys.AddUnique(PatchKey);

	OutResult.bSuccess = true;
	OutResult.NewValue = GetPropertyValueAsString(Property, RowData);

	UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Applied patch: %s.%s.%s = %s (was: %s) [Mod: %s]"),
		*Patch.TargetDataTable.ToString(),
		*Patch.RowName.ToString(),
		*Patch.PropertyName.ToString(),
		*OutResult.NewValue,
		*OutResult.OldValue,
		*ModId.ToString());

	return true;
}

int32 UHarmoniaDataTablePatcher::ApplyPatches(const TArray<FHarmoniaDataTablePatch>& Patches, FName ModId)
{
	int32 AppliedCount = 0;

	for (const FHarmoniaDataTablePatch& Patch : Patches)
	{
		FHarmoniaPatchResult Result;
		if (ApplyPatch(Patch, ModId, Result))
		{
			AppliedCount++;
		}
	}

	UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Applied %d/%d patches for mod: %s"),
		AppliedCount, Patches.Num(), *ModId.ToString());

	return AppliedCount;
}

bool UHarmoniaDataTablePatcher::RevertPatch(const FHarmoniaDataTablePatch& Patch, FName ModId)
{
	FString PatchKey = GeneratePatchKey(Patch.TargetDataTable, Patch.RowName, Patch.PropertyName);

	if (!ActivePatches.Contains(PatchKey))
	{
		UE_LOG(LogHarmoniaDataTablePatcher, Warning, TEXT("Patch not active: %s"), *PatchKey);
		return false;
	}

	// Load data table
	UDataTable* DataTable = Cast<UDataTable>(Patch.TargetDataTable.TryLoad());
	if (!DataTable)
	{
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("Failed to load data table: %s"), *Patch.TargetDataTable.ToString());
		return false;
	}

	// Find row
	uint8* RowData = DataTable->FindRowUnchecked(Patch.RowName);
	if (!RowData)
	{
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("Row not found: %s"), *Patch.RowName.ToString());
		return false;
	}

	// Find property
	FProperty* Property = DataTable->GetRowStruct()->FindPropertyByName(Patch.PropertyName);
	if (!Property)
	{
		UE_LOG(LogHarmoniaDataTablePatcher, Error, TEXT("Property not found: %s"), *Patch.PropertyName.ToString());
		return false;
	}

	// Restore original value
	if (OriginalValues.Contains(PatchKey))
	{
		FString OriginalValue = OriginalValues[PatchKey];
		if (SetPropertyValueFromString(Property, RowData, OriginalValue))
		{
			UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Reverted patch: %s = %s"), *PatchKey, *OriginalValue);
		}
	}

	// Remove from active patches
	ActivePatches.Remove(PatchKey);

	// Remove from mod tracking
	if (PatchesByMod.Contains(ModId))
	{
		PatchesByMod[ModId].PatchKeys.Remove(PatchKey);

		if (PatchesByMod[ModId].PatchKeys.Num() == 0)
		{
			PatchesByMod.Remove(ModId);
		}
	}

	return true;
}

int32 UHarmoniaDataTablePatcher::RevertAllModPatches(FName ModId)
{
	if (!PatchesByMod.Contains(ModId))
	{
		return 0;
	}

	TArray<FString> PatchKeys = PatchesByMod[ModId].PatchKeys;
	int32 RevertedCount = 0;

	for (const FString& PatchKey : PatchKeys)
	{
		if (ActivePatches.Contains(PatchKey))
		{
			const FHarmoniaDataTablePatch& Patch = ActivePatches[PatchKey];
			if (RevertPatch(Patch, ModId))
			{
				RevertedCount++;
			}
		}
	}

	UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Reverted %d patches for mod: %s"), RevertedCount, *ModId.ToString());

	return RevertedCount;
}

void UHarmoniaDataTablePatcher::RevertAllPatches()
{
	UE_LOG(LogHarmoniaDataTablePatcher, Log, TEXT("Reverting all patches..."));

	TArray<FName> ModIds;
	PatchesByMod.GetKeys(ModIds);

	for (const FName& ModId : ModIds)
	{
		RevertAllModPatches(ModId);
	}

	OriginalValues.Empty();
	ActivePatches.Empty();
	PatchesByMod.Empty();
	DataTableCache.Empty();
}

bool UHarmoniaDataTablePatcher::GetOriginalValue(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName, FString& OutOriginalValue) const
{
	FString PatchKey = GeneratePatchKey(TablePath, RowName, PropertyName);

	if (OriginalValues.Contains(PatchKey))
	{
		OutOriginalValue = OriginalValues[PatchKey];
		return true;
	}

	return false;
}

bool UHarmoniaDataTablePatcher::IsValuePatched(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName) const
{
	FString PatchKey = GeneratePatchKey(TablePath, RowName, PropertyName);
	return ActivePatches.Contains(PatchKey);
}

TMap<FString, FHarmoniaDataTablePatch> UHarmoniaDataTablePatcher::GetAllActivePatches() const
{
	return ActivePatches;
}

TArray<FHarmoniaDataTablePatch> UHarmoniaDataTablePatcher::GetModPatches(FName ModId) const
{
	TArray<FHarmoniaDataTablePatch> Patches;

	if (PatchesByMod.Contains(ModId))
	{
		const TArray<FString>& PatchKeys = PatchesByMod[ModId].PatchKeys;

		for (const FString& Key : PatchKeys)
		{
			if (ActivePatches.Contains(Key))
			{
				Patches.Add(ActivePatches[Key]);
			}
		}
	}

	return Patches;
}

bool UHarmoniaDataTablePatcher::ValidatePatch(const FHarmoniaDataTablePatch& Patch, FString& OutErrorMessage) const
{
	if (!Patch.TargetDataTable.IsValid())
	{
		OutErrorMessage = TEXT("Target data table path is invalid");
		return false;
	}

	if (Patch.RowName == NAME_None)
	{
		OutErrorMessage = TEXT("Row name is invalid");
		return false;
	}

	if (Patch.PropertyName == NAME_None)
	{
		OutErrorMessage = TEXT("Property name is invalid");
		return false;
	}

	if (Patch.NewValue.IsEmpty())
	{
		OutErrorMessage = TEXT("New value is empty");
		return false;
	}

	if (Patch.Operation.IsEmpty())
	{
		OutErrorMessage = TEXT("Operation is empty");
		return false;
	}

	return true;
}

bool UHarmoniaDataTablePatcher::ApplyPatchOperation(FProperty* Property, void* DataPtr, const FHarmoniaDataTablePatch& Patch, FHarmoniaPatchResult& OutResult)
{
	if (Patch.Operation == TEXT("Set"))
	{
		// Direct set operation
		return SetPropertyValueFromString(Property, DataPtr, Patch.NewValue);
	}
	else if (Patch.Operation == TEXT("Add"))
	{
		// Add to numeric value
		if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			double CurrentValue = NumericProperty->GetFloatingPointPropertyValue(DataPtr);
			double AddValue = FCString::Atod(*Patch.NewValue);
			double NewValue = CurrentValue + AddValue;

			if (NumericProperty->IsInteger())
			{
				NumericProperty->SetIntPropertyValue(DataPtr, static_cast<int64>(NewValue));
			}
			else
			{
				NumericProperty->SetFloatingPointPropertyValue(DataPtr, NewValue);
			}

			return true;
		}
	}
	else if (Patch.Operation == TEXT("Multiply"))
	{
		// Multiply numeric value
		if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
		{
			double CurrentValue = NumericProperty->GetFloatingPointPropertyValue(DataPtr);
			double Multiplier = FCString::Atod(*Patch.NewValue);
			double NewValue = CurrentValue * Multiplier;

			if (NumericProperty->IsInteger())
			{
				NumericProperty->SetIntPropertyValue(DataPtr, static_cast<int64>(NewValue));
			}
			else
			{
				NumericProperty->SetFloatingPointPropertyValue(DataPtr, NewValue);
			}

			return true;
		}
	}
	else if (Patch.Operation == TEXT("Append"))
	{
		// Append to string value
		if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
		{
			FString CurrentValue = StrProperty->GetPropertyValue(DataPtr);
			FString NewValue = CurrentValue + Patch.NewValue;
			StrProperty->SetPropertyValue(DataPtr, NewValue);
			return true;
		}
	}

	OutResult.ErrorMessage = FString::Printf(TEXT("Unsupported operation or property type: %s"), *Patch.Operation);
	return false;
}

FString UHarmoniaDataTablePatcher::GetPropertyValueAsString(FProperty* Property, const void* DataPtr) const
{
	FString ValueString;

	if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		if (NumericProperty->IsInteger())
		{
			ValueString = FString::Printf(TEXT("%lld"), NumericProperty->GetSignedIntPropertyValue(DataPtr));
		}
		else
		{
			ValueString = FString::Printf(TEXT("%f"), NumericProperty->GetFloatingPointPropertyValue(DataPtr));
		}
	}
	else if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
	{
		ValueString = StrProperty->GetPropertyValue(DataPtr);
	}
	else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		ValueString = BoolProperty->GetPropertyValue(DataPtr) ? TEXT("true") : TEXT("false");
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		ValueString = NameProperty->GetPropertyValue(DataPtr).ToString();
	}
	else
	{
		Property->ExportTextItem_Direct(ValueString, DataPtr, nullptr, nullptr, PPF_None);
	}

	return ValueString;
}

bool UHarmoniaDataTablePatcher::SetPropertyValueFromString(FProperty* Property, void* DataPtr, const FString& Value) const
{
	if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		if (NumericProperty->IsInteger())
		{
			int64 IntValue = FCString::Atoi64(*Value);
			NumericProperty->SetIntPropertyValue(DataPtr, IntValue);
		}
		else
		{
			double FloatValue = FCString::Atod(*Value);
			NumericProperty->SetFloatingPointPropertyValue(DataPtr, FloatValue);
		}
		return true;
	}
	else if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
	{
		StrProperty->SetPropertyValue(DataPtr, Value);
		return true;
	}
	else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		bool BoolValue = Value.Equals(TEXT("true"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("1"));
		BoolProperty->SetPropertyValue(DataPtr, BoolValue);
		return true;
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		NameProperty->SetPropertyValue(DataPtr, FName(*Value));
		return true;
	}
	else
	{
		Property->ImportText_Direct(*Value, DataPtr, nullptr, PPF_None);
		return true;
	}
}

FString UHarmoniaDataTablePatcher::GeneratePatchKey(const FSoftObjectPath& TablePath, FName RowName, FName PropertyName) const
{
	return FString::Printf(TEXT("%s|%s|%s"),
		*TablePath.ToString(),
		*RowName.ToString(),
		*PropertyName.ToString());
}
