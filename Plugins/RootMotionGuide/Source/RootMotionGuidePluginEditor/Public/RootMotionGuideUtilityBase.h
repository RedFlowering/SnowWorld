// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "RootMotionGuideUtilityBase.generated.h"

/**
 * Provides common functionality of the root motion utility
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API URootMotionGuideUtilityBase : public UObject
{
	GENERATED_BODY()

public:
	
	/**
	* Show notifications from the editor
	*/
	void Notification(const FText NotificationText, float ExpireDuration, SNotificationItem::ECompletionState State);

	
};
