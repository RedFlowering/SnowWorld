// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NumberRendererDefine.h"
#include "NRBlueprintFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class NUMBERRENDERER_API UNRBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UNRBlueprintFunctionLibrary();

	UFUNCTION(BlueprintCallable, Category = "NumberRenderer")
	static int32 GetDrawNumberCount(int32 Number);

	UFUNCTION(BlueprintCallable, Category = "NumberRenderer", meta = (WorldContext = "WorldContextObject"))
	static void DrawNumber(UObject* WorldContextObject, int32 Number, FTransform Trans, ENumberFontType FontType, FLinearColor Color = FLinearColor::White, float Duration = 1.0f, float FadeTime = 0.0f, bool LockZAxis = false);

	UFUNCTION(BlueprintCallable, Category = "NumberRenderer", meta = (WorldContext = "WorldContextObject"))
	static void DrawAnimatedNumber(UObject* WorldContextObject, int32 Number, FTransform Trans, FName TableRowName, ENumberFontType FontType, float Duration = 1.0f, bool LockZAxis = false);

	static void DrawAnimatedNumberByIndex(UObject* WorldContextObject, int32 Number, FTransform Trans, int32 RowNameIndex, ENumberFontType FontType, float Duration = 1.0f, bool LockZAxis = false);

	// the number 0 rendered at the PreSpawnLocation to load material resources.
	UFUNCTION(BlueprintCallable, Category = "NumberRenderer", meta = (WorldContext = "WorldContextObject"))
	static void PreloadNumberRenderer(UObject* WorldContextObject, FVector PreSpawnLocation);
};
