// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#include "NumberRendererDefine.h"

namespace NumberHandle_Private
{
	TAtomic<uint64> GNextID(1);
}

uint64 FNumberHandle::GenerateNewID()
{
	// Just increment a counter to generate an ID.
	uint64 Result = ++NumberHandle_Private::GNextID;

	// Check for the next-to-impossible event that we wrap round to 0, because we reserve 0 for null delegates.
	if (Result == 0)
	{
		// Increment it again - it might not be zero, so don't just assign it to 1.
		Result = ++NumberHandle_Private::GNextID;
	}

	return Result;
}

