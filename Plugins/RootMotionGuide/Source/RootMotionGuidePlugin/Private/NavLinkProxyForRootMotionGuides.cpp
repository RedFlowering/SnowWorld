// Copyright 2017 Lee Ju Sik

#include "NavLinkProxyForRootMotionGuides.h"
#include "NavLinkCustomComponent.h"
#include "RootMotionGuide.h"
#include "NavLinkCustomComponentWithFilter.h"

ANavLinkProxyForRootMotionGuides::ANavLinkProxyForRootMotionGuides(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UNavLinkCustomComponentWithFilter>("SmartLinkComp"))
{
	PointLinks.Empty(0);

	UNavLinkCustomComponentWithFilter* SmartLinkCompWithFilter = Cast<UNavLinkCustomComponentWithFilter>(GetSmartLinkComp());
	if (SmartLinkCompWithFilter)
	{
		SmartLinkCompWithFilter->SmartLinkFilter = this;
	}
}


void ANavLinkProxyForRootMotionGuides::UpdateSmartLink()
{
	if (IsValid(RootMotionGuideStart) == false || IsValid(RootMotionGuideEnd) == false)
	{
		bSmartLinkIsRelevant = false;
		return;
	}


	SetActorTransform(RootMotionGuideStart->GetActorTransform());

	LinkStart = LinkStartOffset;

	LinkEnd = GetActorTransform().InverseTransformPosition(RootMotionGuideEnd->GetActorTransform().TransformPosition(RootMotionGuideEnd->TotalRootMotion)) + LinkEndOffset;
	
	GetSmartLinkComp()->SetLinkData(LinkStart, LinkEnd, ENavLinkDirection::LeftToRight);

	bSmartLinkIsRelevant = true;

	MovementInput = RootMotionGuideStart->GetMovementDirectionInWorld();

	MarkPackageDirty();
}
