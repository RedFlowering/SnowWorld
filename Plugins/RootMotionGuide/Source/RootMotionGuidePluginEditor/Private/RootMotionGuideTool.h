// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
//#include "PropertyEditing.h"
#include "Editor/PropertyEditor/Public/IDetailCustomization.h"
#include "Widgets/Notifications/SNotificationList.h"

/**
 * Provides tools for ARootMotionGuide
 */
class FRootMotionGuideTool : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	// CustomizeDetails Implementation of IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder & DetailBuilder) override;

private:
	TWeakObjectPtr< class ARootMotionGuide > RootMotionGuide;

	/**
	* Extract AnimationData
	*/
	FReply	RefreshAnimationData();

	/**
	* Hide Other Arrows
	*/
	FReply	HideOtherArrows();

	/**
	* Show notifications from the editor
	*/
	void Notification(const FText NotificationText, float ExpireDuration, SNotificationItem::ECompletionState State);
};
