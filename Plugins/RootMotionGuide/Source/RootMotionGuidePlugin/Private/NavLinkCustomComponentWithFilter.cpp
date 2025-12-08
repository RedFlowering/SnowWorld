// Copyright 2017 Lee Ju Sik

#include "NavLinkCustomComponentWithFilter.h"
#include "RootMotionGuideSmartLinkFilter.h"

bool UNavLinkCustomComponentWithFilter::IsLinkPathfindingAllowed(const UObject * Querier) const
{
	if (IsValid(SmartLinkFilter) && SmartLinkFilter->GetClass()->ImplementsInterface(URootMotionGuideSmartLinkFilter::StaticClass()))
	{
		return IRootMotionGuideSmartLinkFilter::Execute_IsLinkPathfindingAllowed(SmartLinkFilter, Querier);
	}

	return true;
}
