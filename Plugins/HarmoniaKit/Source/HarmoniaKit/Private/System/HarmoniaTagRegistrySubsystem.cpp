// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaTagRegistrySubsystem.h"
#include "Settings/HarmoniaTagSettings.h"
#include "GameplayTagsManager.h"
#include "Engine/DataTable.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HarmoniaLogCategories.h"

void UHarmoniaTagRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UHarmoniaTagSettings* Settings = UHarmoniaTagSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("HarmoniaTagRegistrySubsystem: Could not get tag settings"));
		return;
	}

	// Register tags from INI if enabled
	if (Settings->bAutoRegisterIniTags && !Settings->CustomTagsIniPath.IsEmpty())
	{
		int32 TagCount = RegisterTagsFromIni();
		UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaTagRegistrySubsystem: Registered %d tags from INI"), TagCount);
	}

	// Register tags from DataTable if enabled
	if (Settings->bAutoRegisterDataTableTags && !Settings->TagDefinitionsTable.IsNull())
	{
		UDataTable* TagTable = Settings->TagDefinitionsTable.LoadSynchronous();
		if (TagTable)
		{
			int32 TagCount = RegisterTagsFromDataTable(TagTable);
			UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaTagRegistrySubsystem: Registered %d tags from DataTable"), TagCount);
		}
	}

	// Register attributes from DataTable if available
	if (!Settings->AttributeDefinitionsTable.IsNull())
	{
		UDataTable* AttributeTable = Settings->AttributeDefinitionsTable.LoadSynchronous();
		if (AttributeTable)
		{
			int32 AttributeCount = RegisterAttributesFromDataTable(AttributeTable);
			UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaTagRegistrySubsystem: Registered %d attributes from DataTable"), AttributeCount);
		}
	}

	// Log if debug is enabled
	if (Settings->bLogRegisteredTags)
	{
		LogRegisteredTags();
	}
	if (Settings->bLogRegisteredAttributes)
	{
		LogRegisteredAttributes();
	}

	bInitialized = true;
}

void UHarmoniaTagRegistrySubsystem::Deinitialize()
{
	RegisteredTags.Empty();
	AttributeToTagMap.Empty();
	TagToAttributeMap.Empty();
	bInitialized = false;

	Super::Deinitialize();
}

UHarmoniaTagRegistrySubsystem* UHarmoniaTagRegistrySubsystem::Get()
{
	return GEngine ? GEngine->GetEngineSubsystem<UHarmoniaTagRegistrySubsystem>() : nullptr;
}

int32 UHarmoniaTagRegistrySubsystem::RegisterTagsFromIni()
{
	UHarmoniaTagSettings* Settings = UHarmoniaTagSettings::Get();
	if (!Settings || Settings->CustomTagsIniPath.IsEmpty())
	{
		return 0;
	}

	return RegisterTagsFromIniPath(Settings->CustomTagsIniPath);
}

int32 UHarmoniaTagRegistrySubsystem::RegisterTagsFromIniPath(const FString& IniPath)
{
	FString FullPath = FPaths::ProjectDir() / IniPath;
	
	FString IniContent;
	if (!FFileHelper::LoadFileToString(IniContent, *FullPath))
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("HarmoniaTagRegistrySubsystem: Could not load INI file: %s"), *FullPath);
		return 0;
	}

	TArray<TPair<FString, FString>> ParsedTags = ParseTagsFromIniContent(IniContent);
	
	int32 RegisteredCount = 0;
	for (const auto& TagPair : ParsedTags)
	{
		if (RegisterTag(TagPair.Key, TagPair.Value))
		{
			RegisteredCount++;
		}
	}

	return RegisteredCount;
}

int32 UHarmoniaTagRegistrySubsystem::RegisterTagsFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return 0;
	}

	// Verify row struct type
	if (!DataTable->GetRowStruct()->IsChildOf(FHarmoniaTagDefinition::StaticStruct()))
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("HarmoniaTagRegistrySubsystem: DataTable does not use FHarmoniaTagDefinition row struct"));
		return 0;
	}

	int32 RegisteredCount = 0;
	FString ContextString(TEXT("RegisterTagsFromDataTable"));

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FHarmoniaTagDefinition* TagDef = DataTable->FindRow<FHarmoniaTagDefinition>(RowName, ContextString);
		if (TagDef && !TagDef->TagName.IsEmpty())
		{
			if (RegisterTag(TagDef->TagName, TagDef->Description))
			{
				RegisteredCount++;
			}
		}
	}

	return RegisteredCount;
}

bool UHarmoniaTagRegistrySubsystem::RegisterTag(const FString& TagName, const FString& Comment)
{
	if (TagName.IsEmpty())
	{
		return false;
	}

	// Check if already registered
	if (RegisteredTags.Contains(TagName))
	{
		return true; // Already registered
	}

	// Add native gameplay tag
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	
	// Request the tag (this creates it if it doesn't exist)
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagName), false);
	
	if (!Tag.IsValid())
	{
		// Tag doesn't exist, try to add it
		Manager.AddNativeGameplayTag(FName(*TagName), Comment);
		Tag = Manager.RequestGameplayTag(FName(*TagName), false);
	}

	if (Tag.IsValid())
	{
		RegisteredTags.Add(TagName);
		return true;
	}

	UE_LOG(LogHarmoniaKit, Warning, TEXT("HarmoniaTagRegistrySubsystem: Failed to register tag: %s"), *TagName);
	return false;
}

int32 UHarmoniaTagRegistrySubsystem::RegisterAttributesFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return 0;
	}

	// Verify row struct type
	if (!DataTable->GetRowStruct()->IsChildOf(FHarmoniaAttributeDefinition::StaticStruct()))
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("HarmoniaTagRegistrySubsystem: DataTable does not use FHarmoniaAttributeDefinition row struct"));
		return 0;
	}

	int32 RegisteredCount = 0;
	FString ContextString(TEXT("RegisterAttributesFromDataTable"));

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FHarmoniaAttributeDefinition* AttrDef = DataTable->FindRow<FHarmoniaAttributeDefinition>(RowName, ContextString);
		if (AttrDef && !AttrDef->AttributeName.IsEmpty())
		{
			// Register the tag first
			if (AttrDef->AttributeTag.IsValid())
			{
				RegisterTag(AttrDef->AttributeTag.ToString(), AttrDef->Description.ToString());
			}

			// Register the mapping
			RegisterAttributeMapping(AttrDef->AttributeName, AttrDef->AttributeTag);
			RegisteredCount++;
		}
	}

	return RegisteredCount;
}

void UHarmoniaTagRegistrySubsystem::RegisterAttributeMapping(const FString& AttributeName, FGameplayTag Tag)
{
	if (AttributeName.IsEmpty())
	{
		return;
	}

	AttributeToTagMap.Add(AttributeName, Tag);
	
	if (Tag.IsValid())
	{
		TagToAttributeMap.Add(Tag, AttributeName);
	}
}

FGameplayTag UHarmoniaTagRegistrySubsystem::FindTagByName(const FString& TagString, bool bMatchPartial) const
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);
	
	if (!Tag.IsValid() && bMatchPartial)
	{
		// Try partial match
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);
		
		for (const FGameplayTag& ExistingTag : AllTags)
		{
			if (ExistingTag.ToString().Contains(TagString))
			{
				return ExistingTag;
			}
		}
	}
	
	return Tag;
}

FGameplayTagContainer UHarmoniaTagRegistrySubsystem::GetTagsByCategory(const FString& Category) const
{
	FGameplayTagContainer Result;
	
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTagContainer AllTags;
	Manager.RequestAllGameplayTags(AllTags, true);
	
	for (const FGameplayTag& Tag : AllTags)
	{
		FString TagString = Tag.ToString();
		if (TagString.StartsWith(Category + TEXT(".")))
		{
			Result.AddTag(Tag);
		}
	}
	
	return Result;
}

FGameplayTagContainer UHarmoniaTagRegistrySubsystem::GetAllRegisteredTags() const
{
	FGameplayTagContainer Result;
	
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	for (const FString& TagName : RegisteredTags)
	{
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagName), false);
		if (Tag.IsValid())
		{
			Result.AddTag(Tag);
		}
	}
	
	return Result;
}

bool UHarmoniaTagRegistrySubsystem::IsTagRegistered(const FString& TagString) const
{
	return RegisteredTags.Contains(TagString);
}

FGameplayTag UHarmoniaTagRegistrySubsystem::GetTagForAttribute(const FString& AttributeName) const
{
	const FGameplayTag* Tag = AttributeToTagMap.Find(AttributeName);
	return Tag ? *Tag : FGameplayTag();
}

FString UHarmoniaTagRegistrySubsystem::GetAttributeForTag(const FGameplayTag& Tag) const
{
	const FString* AttributeName = TagToAttributeMap.Find(Tag);
	return AttributeName ? *AttributeName : FString();
}

TArray<FString> UHarmoniaTagRegistrySubsystem::GetAllAttributeNames() const
{
	TArray<FString> Result;
	AttributeToTagMap.GetKeys(Result);
	return Result;
}

TArray<FString> UHarmoniaTagRegistrySubsystem::ValidateRegisteredTags() const
{
	TArray<FString> Errors;
	
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	
	for (const FString& TagName : RegisteredTags)
	{
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagName), false);
		if (!Tag.IsValid())
		{
			Errors.Add(FString::Printf(TEXT("Tag not found in GameplayTags system: %s"), *TagName));
		}
	}
	
	return Errors;
}

TArray<TPair<FString, FString>> UHarmoniaTagRegistrySubsystem::ParseTagsFromIniContent(const FString& IniContent)
{
	TArray<TPair<FString, FString>> Result;
	
	TArray<FString> Lines;
	IniContent.ParseIntoArray(Lines, TEXT("\n"), true);
	
	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		
		// Skip comments and empty lines
		if (TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT(";")) || TrimmedLine.StartsWith(TEXT("[")))
		{
			continue;
		}
		
		// Look for +GameplayTags= lines
		if (TrimmedLine.StartsWith(TEXT("+GameplayTags=")))
		{
			// Extract (Tag="...",DevComment="...")
			int32 TagStart = TrimmedLine.Find(TEXT("Tag=\""));
			int32 CommentStart = TrimmedLine.Find(TEXT("DevComment=\""));
			
			if (TagStart != INDEX_NONE)
			{
				TagStart += 5; // Skip 'Tag="'
				int32 TagEnd = TrimmedLine.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, TagStart);
				
				if (TagEnd != INDEX_NONE)
				{
					FString TagName = TrimmedLine.Mid(TagStart, TagEnd - TagStart);
					FString Comment;
					
					if (CommentStart != INDEX_NONE)
					{
						CommentStart += 12; // Skip 'DevComment="'
						int32 CommentEnd = TrimmedLine.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, CommentStart);
						if (CommentEnd != INDEX_NONE)
						{
							Comment = TrimmedLine.Mid(CommentStart, CommentEnd - CommentStart);
						}
					}
					
					Result.Add(TPair<FString, FString>(TagName, Comment));
				}
			}
		}
	}
	
	return Result;
}

void UHarmoniaTagRegistrySubsystem::LogRegisteredTags() const
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("=== Registered Gameplay Tags ==="));
	
	TArray<FString> SortedTags = RegisteredTags.Array();
	SortedTags.Sort();
	
	for (const FString& TagName : SortedTags)
	{
		UE_LOG(LogHarmoniaKit, Log, TEXT("  %s"), *TagName);
	}
	
	UE_LOG(LogHarmoniaKit, Log, TEXT("=== Total: %d tags ==="), RegisteredTags.Num());
}

void UHarmoniaTagRegistrySubsystem::LogRegisteredAttributes() const
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("=== Registered Attribute Mappings ==="));
	
	for (const auto& Pair : AttributeToTagMap)
	{
		UE_LOG(LogHarmoniaKit, Log, TEXT("  %s -> %s"), *Pair.Key, *Pair.Value.ToString());
	}
	
	UE_LOG(LogHarmoniaKit, Log, TEXT("=== Total: %d mappings ==="), AttributeToTagMap.Num());
}
