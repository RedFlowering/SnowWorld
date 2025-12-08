// Copyright 2017 Lee Ju Sik

#include "RootMotionGuideUtilityBase.h"
#include "Framework/Notifications/NotificationManager.h"



#define LOCTEXT_NAMESPACE "RootMotionGuide"


void URootMotionGuideUtilityBase::Notification(const FText NotificationText, float ExpireDuration, SNotificationItem::ECompletionState State)
{
	if (GIsEditor)
	{
		FNotificationInfo Info(NotificationText);
		Info.ExpireDuration = ExpireDuration;
		Info.bUseLargeFont = false;
		TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
		if (Notification.IsValid())
		{
			Notification->SetCompletionState(State);
		}
	}
}

#undef LOCTEXT_NAMESPACE