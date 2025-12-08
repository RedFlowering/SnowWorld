// Copyright 2017 Lee Ju Sik

#include "RootMotionGuideSmartLinkFilter.h"


// Add default functionality here for any IRootMotionGuideSmartLinkFilter functions that are not pure virtual.
bool IRootMotionGuideSmartLinkFilter::IsLinkPathfindingAllowed_Implementation(const UObject* Querier) const
{
	return true;
}