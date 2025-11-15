// Copyright 2022 HGsofts, Ltd. All Rights Reserved.


#include "NRBlueprintFunctionLibrary.h"
#include "NumberRenderer.h"
#include "WorldNumberManager.h"
#include "Engine/Engine.h"

UNRBlueprintFunctionLibrary::UNRBlueprintFunctionLibrary()
{

}

int32 UNRBlueprintFunctionLibrary::GetDrawNumberCount(int32 Number)
{
	int32 Count = 1;
	int32 Mask = Number >> 31;

	Count += (Number & 0x80000000) != 0;

	Number = Number ^ Mask;
	Number = Number - Mask;

	Count += Number >= 10;
	Count += Number >= 100;
	Count += Number >= 1000;
	Count += Number >= 10000;
	Count += Number >= 100000;
	Count += Number >= 1000000;
	Count += Number >= 10000000;
	Count += Number >= 100000000;
	Count += Number >= 1000000000;
	Count += Number >= 10000000000;

	return Count;
}

void UNRBlueprintFunctionLibrary::DrawAnimatedNumber(UObject* WorldContextObject, int32 Number, FTransform Trans, FName TableRowName, ENumberFontType FontType, float Duration, bool LockZAxis)
{
#if !UE_SERVER
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UWorldNumberManager* Manager = FNumberRendererModule::Get(World))
		{
			if (ANumberRenderActor* Actor = Manager->GetRenderActor(FontType))
			{
				Actor->AddDrawNumberByTable(Number, Trans, Duration, TableRowName, LockZAxis);
			}

		}
	}
#endif
}

void UNRBlueprintFunctionLibrary::DrawNumber(UObject* WorldContextObject, int32 Number, FTransform Trans, ENumberFontType FontType, FLinearColor Color, float Duration, float FadeTime, bool LockZAxis)
{
#if !UE_SERVER
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UWorldNumberManager* Manager = FNumberRendererModule::Get(World))
		{
			if (ANumberRenderActor* Actor = Manager->GetRenderActor(FontType))
			{
				Actor->AddNumber(Number, Trans, Color, Duration, FadeTime, LockZAxis);
			}

		}
	}
#endif
}

void UNRBlueprintFunctionLibrary::DrawAnimatedNumberByIndex(UObject* WorldContextObject, int32 Number, FTransform Trans, int32 RowNameIndex, ENumberFontType FontType, float Duration, bool LockZAxis)
{
#if !UE_SERVER
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UWorldNumberManager* Manager = FNumberRendererModule::Get(World))
		{
			if (ANumberRenderActor* Actor = Manager->GetRenderActor(FontType))
			{
				Actor->AddDrawNumberByTable(Number, Trans, Duration, RowNameIndex, LockZAxis);
			}

		}
	}
#endif
}

void UNRBlueprintFunctionLibrary::PreloadNumberRenderer(UObject* WorldContextObject, FVector PreSpawnLocation)
{
#if !UE_SERVER
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UWorldNumberManager* Manager = FNumberRendererModule::Get(World))
		{
			Manager->Preload(PreSpawnLocation);
		}
	}
#endif
}
