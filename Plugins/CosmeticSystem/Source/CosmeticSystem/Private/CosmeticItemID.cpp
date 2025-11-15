// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticItemID.h"
#include "CosmeticSystem.h"

FCosmeticItemID::FCosmeticItemID(int32 ID)
{
	if (ID > 99999999)
	{
		UE_LOG(LogCosmeticSystem, Error, TEXT("[%d] ID must be 8 Digits"), ID);
		ID = ID % 100000000;
	}

	Identifier = ID;
}
