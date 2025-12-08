// Copyright 2017 Lee Ju Sik

#include "RootMotionGuideWithSmartLink.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

ARootMotionGuideWithSmartLink::ARootMotionGuideWithSmartLink()
{
	SmartLinkComp = CreateDefaultSubobject<UNavLinkCustomComponentWithFilter>(TEXT("SmartLinkComp"));
	SmartLinkComp->SetNavigationRelevancy(true);
	SmartLinkComp->SetMoveReachedLink(this, &ARootMotionGuideWithSmartLink::NotifySmartLinkReached);
	SmartLinkComp->SmartLinkFilter = this;
}

void ARootMotionGuideWithSmartLink::UpdateSmartLink()
{
	SmartLinkComp->SetLinkData(LinkStartOffset, TotalRootMotion + LinkEndOffset, ENavLinkDirection::LeftToRight);

	LinkStart = LinkStartOffset;
	LinkEnd = TotalRootMotion + LinkEndOffset;

	MarkPackageDirty();
}

void ARootMotionGuideWithSmartLink::InitializeSubTrigger(const ARootMotionGuide * MainTrigger)
{
	const ARootMotionGuideWithSmartLink* MainTriggerWithSmartLink = Cast<ARootMotionGuideWithSmartLink>(MainTrigger);
	if (MainTriggerWithSmartLink)
	{
		FVector Left, Right;
		ENavLinkDirection::Type Direction;
		MainTriggerWithSmartLink->SmartLinkComp->GetLinkData(Left, Right, Direction);

		SmartLinkComp->SetLinkData(Left, Right, Direction);
	}

	Super::InitializeSubTrigger(MainTrigger);
}

void ARootMotionGuideWithSmartLink::NotifySmartLinkReached(UNavLinkCustomComponent * LinkComp, UObject * PathingAgent, const FVector & DestPoint)
{
	UPathFollowingComponent* PathComp = Cast<UPathFollowingComponent>(PathingAgent);
	if (PathComp)
	{
		AActor* PathOwner = PathComp->GetOwner();
		AController* ControllerOwner = Cast<AController>(PathOwner);
		if (ControllerOwner)
		{
			PathOwner = ControllerOwner->GetPawn();
		}

		ReceiveSmartLinkReached(PathOwner, DestPoint);
	}
}
