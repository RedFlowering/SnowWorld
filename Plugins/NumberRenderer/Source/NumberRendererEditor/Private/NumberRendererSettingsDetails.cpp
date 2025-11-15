// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#include "NumberRendererSettingsDetails.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/SToolTip.h"
#include "IDocumentation.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "NumberFontTypeDetails"

void SNumberFontTypeEditBox::Construct(const FArguments& InArgs)
{
	NumberFontType = InArgs._NumberFontType;
	NumberFontTypeEnum = InArgs._NumberFontTypeEnum;
	OnCommitChange = InArgs._OnCommitChange;
	check(NumberFontType.IsValid() && NumberFontTypeEnum);

	ChildSlot
		[
			SAssignNew(NameEditBox, SEditableTextBox)
				.Text(this, &SNumberFontTypeEditBox::GetName)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OnTextCommitted(this, &SNumberFontTypeEditBox::NewNameEntered)
				.OnTextChanged(this, &SNumberFontTypeEditBox::OnTextChanged)
				.IsReadOnly(NumberFontType->Type == ENumberFontType::NumberFontType_None)
				.SelectAllTextWhenFocused(true)
		];
}



void SNumberFontTypeEditBox::OnTextChanged(const FText& NewText)
{
	FString NewName = NewText.ToString();

	if (NewName.Find(TEXT(" ")) != INDEX_NONE)
	{
		// no white space
		NameEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		NameEditBox->SetError(TEXT(""));
	}
}

void SNumberFontTypeEditBox::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo)
{
	// Don't digest the number if we just clicked away from the pop-up
	if ((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		FString NewName = NewText.ToString();
		if (NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			FName NewNumberFontTypeName(*NewName);
			if (NumberFontType->Name != NAME_None && NewNumberFontTypeName == NAME_None)
			{
				if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SNumberFontTypeEditBox_DeleteConfirm", "Would you like to delete the name? If this type is used, it will invalidate the usage.")) == EAppReturnType::No)
				{
					return;
				}
			}
			if (NewNumberFontTypeName != NumberFontType->Name)
			{
				NumberFontType->Name = NewNumberFontTypeName;
				OnCommitChange.ExecuteIfBound();
			}
		}
		else
		{
			// clear error
			NameEditBox->SetError(TEXT(""));
		}
	}
}

FText SNumberFontTypeEditBox::GetName() const
{
	return FText::FromName(NumberFontType->Name);
}

class FNumberFontTypeList : public IDetailCustomNodeBuilder, public TSharedFromThis<FNumberFontTypeList>
{
public:
	FNumberFontTypeList(UNumberRendererSettings* InNumberRendererSettings, UEnum* InNumberFontTypeEnum, TSharedPtr<IPropertyHandle>& InNumberFontTypeProperty)
		: NumberRendererSettings(InNumberRendererSettings)
		, NumberFontTypeEnum(InNumberFontTypeEnum)
		, NumberFontTypeProperty(InNumberFontTypeProperty)
	{
		NumberFontTypeProperty->MarkHiddenByCustomization();
	}

	void RefreshNumberFontTypeList()
	{
		// make sure no duplicate exists
		// if exists, use the last one
		for (auto Iter = NumberRendererSettings->NumberFontTypes.CreateIterator(); Iter; ++Iter)
		{
			for (auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
			{
				// see if same type
				if (Iter->Type == InnerIter->Type)
				{
					// remove the current one
					NumberRendererSettings->NumberFontTypes.RemoveAt(Iter.GetIndex());
					--Iter;
					break;
				}
			}
		}

		bool bCreatedItem[ENumberFontType::NumberFontType_Max];
		FGenericPlatformMemory::Memzero(bCreatedItem, sizeof(bCreatedItem));

		NumberFontTypeList.Empty();

		{
			bCreatedItem[0] = true;
			NumberFontTypeList.Add(MakeShareable(new FNumberFontTypeListItem(MakeShareable(new FNumberFontTypeName(ENumberFontType::NumberFontType_None, TEXT("None"))))));
		}

		// we don't create the first one. First one is always default. 
		for (auto Iter = NumberRendererSettings->NumberFontTypes.CreateIterator(); Iter; ++Iter)
		{
			bCreatedItem[Iter->Type] = true;
			NumberFontTypeList.Add(MakeShareable(new FNumberFontTypeListItem(MakeShareable(new FNumberFontTypeName(*Iter)))));
		}

		for (int32 Index = (int32)ENumberFontType::NumberFontType1; Index < ENumberFontType::NumberFontType_Max; ++Index)
		{
			if (bCreatedItem[Index] == false)
			{
				FNumberFontTypeName NeweElement((ENumberFontType)Index, TEXT(""));
				NumberFontTypeList.Add(MakeShareable(new FNumberFontTypeListItem(MakeShareable(new FNumberFontTypeName(NeweElement)))));
			}
		}

		// sort NumberFontTypeListItem by Type

		struct FCompareNumberFontType
		{
			FORCEINLINE bool operator()(const TSharedPtr<FNumberFontTypeListItem> A, const TSharedPtr<FNumberFontTypeListItem> B) const
			{
				check(A.IsValid());
				check(B.IsValid());
				return A->NumberFontType->Type < B->NumberFontType->Type;
			}
		};

		NumberFontTypeList.Sort(FCompareNumberFontType());

		NumberRendererSettings->LoadTypes();

		RegenerateChildren.ExecuteIfBound();
	}

	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		RegenerateChildren = InOnRegenerateChildren;
	}

	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		// no header row
	}

	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		FText SearchString = LOCTEXT("FNumberRendererSettingsDetails_NumberFontTypeName", "NumberFontTypeName");

		for (TSharedPtr<FNumberFontTypeListItem>& Item : NumberFontTypeList)
		{
			FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(SearchString);

			FString TypeString = NumberFontTypeEnum->GetNameStringByValue((int64)Item->NumberFontType->Type);

			Row.NameContent()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TypeString))
						.Font(IDetailLayoutBuilder::GetDetailFont())
				];

			Row.ValueContent()
				[
					SNew(SNumberFontTypeEditBox)
						.NumberFontType(Item->NumberFontType)
						.NumberFontTypeEnum(NumberFontTypeEnum)
						.OnCommitChange(this, &FNumberFontTypeList::OnCommitChange)
				];
		}
	}

	virtual void Tick(float DeltaTime) override {}
	virtual bool RequiresTick() const override { return false; }
	virtual bool InitiallyCollapsed() const { return false; }
	virtual FName GetName() const override
	{
		static const FName Name(TEXT("NumberFontTypeList"));
		return Name;
	}
private:

	void OnCommitChange()
	{
		bool bDoCommit = true;
		// make sure it verifies all data is correct
		// skip the first one
		for (auto Iter = NumberFontTypeList.CreateConstIterator() + 1; Iter; ++Iter)
		{
			TSharedPtr<FNumberFontTypeListItem> ListItem = *Iter;
			if (ListItem->NumberFontType->Name != NAME_None)
			{
				// make sure no same name exists
				for (auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
				{
					TSharedPtr<FNumberFontTypeListItem> InnerItem = *InnerIter;
					if (ListItem->NumberFontType->Name == InnerItem->NumberFontType->Name)
					{
						// duplicate name, warn user and get out
						FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FNumberRendererSettingsDetails_InvalidName", "Duplicate name found."));
						bDoCommit = false;
						break;
					}
				}
			}
		}

		if (bDoCommit)
		{
			NumberFontTypeProperty->NotifyPreChange();

			NumberRendererSettings->NumberFontTypes.Empty();
			for (auto Iter = NumberFontTypeList.CreateConstIterator() + 1; Iter; ++Iter)
			{
				TSharedPtr<FNumberFontTypeListItem> ListItem = *Iter;
				if (ListItem->NumberFontType->Name != NAME_None)
				{
					NumberRendererSettings->NumberFontTypes.Add(FNumberFontTypeName(ListItem->NumberFontType->Type, ListItem->NumberFontType->Name));
				}
			}

			NumberRendererSettings->TryUpdateDefaultConfigFile();
			EPropertyChangeType::Type ChangeType = NumberRendererSettings->LoadTypes();

			NumberFontTypeProperty->NotifyPostChange(ChangeType);
		}
	}
private:
	FSimpleDelegate RegenerateChildren;
	TArray< TSharedPtr< FNumberFontTypeListItem > > NumberFontTypeList;
	UNumberRendererSettings* NumberRendererSettings;
	UEnum* NumberFontTypeEnum;
	TSharedPtr<IPropertyHandle> NumberFontTypeProperty;
};


//====================================================================================
// FNumberRendererSettingsDetails
//=====================================================================================
TSharedRef<IDetailCustomization> FNumberRendererSettingsDetails::MakeInstance()
{
	return MakeShareable(new FNumberRendererSettingsDetails);
}

void FNumberRendererSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& NumberFontTypeCategory = DetailBuilder.EditCategory("NumberFontType", FText::GetEmpty(), ECategoryPriority::Uncommon);

	NumberRendererSettings = UNumberRendererSettings::Get();
	check(NumberRendererSettings);

	NumberFontTypeEnum = StaticEnum<ENumberFontType>();
	check(NumberFontTypeEnum);

	TSharedPtr<IPropertyHandle> NumberFontTypeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UNumberRendererSettings, NumberFontTypes));

	TSharedRef<FNumberFontTypeList> NumberFontTypeListCustomization = MakeShareable(new FNumberFontTypeList(NumberRendererSettings, NumberFontTypeEnum, NumberFontTypeProperty));
	NumberFontTypeListCustomization->RefreshNumberFontTypeList();

	const FString NumberFontTypeDocLink = TEXT("Shared/NumberFont");
	TSharedPtr<SToolTip> NumberFontTypeTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("NumberFontType", "Edit Number Font Type Name"), NULL, NumberFontTypeDocLink, TEXT("NumberFontType"));


	// Customize collision section
	NumberFontTypeCategory.AddCustomRow(LOCTEXT("FNumberRendererSettingsDetails_NumberFontType", "NumberFontType"))
		[
			SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.ToolTip(NumberFontTypeTooltip)
				.AutoWrapText(true)
				.Text(LOCTEXT("NumberFontType_Menu_Description", " You can have up to 9 custom NumberFont Types for your project. \nOnce you name each type"))
		];


	NumberFontTypeCategory.AddCustomBuilder(NumberFontTypeListCustomization);
}



#undef LOCTEXT_NAMESPACE

