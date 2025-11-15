// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#include "NumberRendererSettings.h"

#include "NumberRenderActor.h"

UNumberRendererSettings::UNumberRendererSettings()
{
	CategoryName = TEXT("Plugins");
}

UNumberRendererSettings* UNumberRendererSettings::Get()
{
	return UNumberRendererSettings::StaticClass()->GetDefaultObject<UNumberRendererSettings>();
}

void UNumberRendererSettings::UpdateSettings()
{
#if WITH_EDITOR
	LoadTypes();
#endif
}

void UNumberRendererSettings::PostInitProperties()
{
	Super::PostInitProperties();

	UpdateSettings();
}

#if WITH_EDITOR
bool UNumberRendererSettings::CanEditChange(const FProperty* Property) const
{
	bool bIsEditable = Super::CanEditChange(Property);
	return bIsEditable;
}

void UNumberRendererSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UNumberRendererSettings, NumberFontTypes))
	{
		UpdateSettings();
	}
}

EPropertyChangeType::Type UNumberRendererSettings::LoadTypes()
{
	EPropertyChangeType::Type ChangeType = EPropertyChangeType::Unspecified;

	UEnum* Enum = StaticEnum<ENumberFontType>();

	check(Enum);

	const FString KeyName = TEXT("DisplayName");
	const FString HiddenMeta = TEXT("Hidden");
	const FString UnusedDisplayName = TEXT("Unused");

	for (int32 EnumIndex = 1; EnumIndex < Enum->NumEnums(); ++EnumIndex)
	{
		if (!Enum->HasMetaData(*HiddenMeta, EnumIndex))
		{
			Enum->SetMetaData(*HiddenMeta, TEXT(""), EnumIndex);
			Enum->SetMetaData(*KeyName, *UnusedDisplayName, EnumIndex);
		}
	}

	TArray<TEnumAsByte<enum ENumberFontType>> DeleteList;
	for (auto& RenderIter : NumberRenderActorClasses)
	{
		bool bFind = false;
		for (auto Iter = NumberFontTypes.CreateConstIterator(); Iter; ++Iter)
		{
			if (RenderIter.Key == Iter->Type)
			{
				bFind = true;
				break;
			}
		}

		if (!bFind)
		{
			DeleteList.Add(RenderIter.Key);
		}
	}

	for (auto& Iter : DeleteList)
	{
		NumberRenderActorClasses.Remove(Iter);
		ChangeType = EPropertyChangeType::ArrayRemove;
	}

	for (auto Iter = NumberFontTypes.CreateConstIterator(); Iter; ++Iter)
	{
		const FString ShadowTypeName = Iter->Name.ToString();
		Enum->SetMetaData(*KeyName, *ShadowTypeName, Iter->Type);
		Enum->RemoveMetaData(*HiddenMeta, Iter->Type);

		if (!NumberRenderActorClasses.Contains(Iter->Type))
		{
			NumberRenderActorClasses.Add(Iter->Type, nullptr);
			ChangeType = EPropertyChangeType::ArrayAdd;
		}
	}

	return ChangeType;
}
#endif
