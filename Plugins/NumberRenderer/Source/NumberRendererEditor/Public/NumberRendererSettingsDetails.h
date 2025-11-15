// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "IDetailCustomization.h"
#include "NumberRendererSettings.h"

class IDetailLayoutBuilder;
class SEditableTextBox;
template <typename ItemType> class SListView;

DECLARE_DELEGATE(FOnCommitChange)

class FNumberFontTypeListItem
{
public:

	/**
	* Constructor takes the required details
	*/
	FNumberFontTypeListItem(TSharedPtr<FNumberFontTypeName> InNumberFontType)
		: NumberFontType(InNumberFontType)
	{
	}

	TSharedPtr<FNumberFontTypeName> NumberFontType;
};

/**
* Implements the FriendsList
*/
class SNumberFontTypeEditBox : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SNumberFontTypeEditBox) {}
		SLATE_ARGUMENT(TSharedPtr<FNumberFontTypeName>, NumberFontType)
		SLATE_ARGUMENT(UEnum*, NumberFontTypeEnum)
		SLATE_EVENT(FOnCommitChange, OnCommitChange)
	SLATE_END_ARGS()

public:

	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs);

	FString GetTypeString() const;

	FText GetName() const;
	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo);
	void OnTextChanged(const FText& NewText);

private:
	TSharedPtr<FNumberFontTypeName> NumberFontType;
	UEnum* NumberFontTypeEnum;
	FOnCommitChange OnCommitChange;
	TSharedPtr<SEditableTextBox> NameEditBox;
};

typedef  SListView< TSharedPtr< FNumberFontTypeListItem > > SNumberFontTypeListView;

class FNumberRendererSettingsDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:

	/**
	* Generates a widget for a channel item.
	* @param InItem - the ChannelListItem
	* @param OwnerTable - the owning table
	* @return The table row widget
	*/
	TSharedRef<ITableRow> HandleGenerateListWidget(TSharedPtr< FNumberFontTypeListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);


private:
	TArray< TSharedPtr< FNumberFontTypeListItem > > NumberFontTypeList;
	UNumberRendererSettings* NumberRendererSettings;
	UEnum* NumberFontTypeEnum;
	// functions
	void OnCommitChange();
};

