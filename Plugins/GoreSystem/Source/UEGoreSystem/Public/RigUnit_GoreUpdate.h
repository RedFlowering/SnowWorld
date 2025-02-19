// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#pragma once

//#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "Units/Highlevel/RigUnit_HighlevelBase.h"
#include "RigUnit_GoreUpdate.generated.h"

USTRUCT()
struct UEGORESYSTEM_API FRigUnit_GoreUpdate_WorkData
{
	GENERATED_BODY()

		UPROPERTY()
		TArray<FCachedRigElement> CachedItems;
};

/**
 * Update Gore Hidden Limbs
 */
USTRUCT(meta = (DisplayName = "Gore Update", Category = "Gore", DocumentationPolicy = "Strict", Keywords = "Gore"))
struct UEGORESYSTEM_API FRigUnit_GoreUpdate : public FRigUnit_HighlevelBaseMutable
{
	GENERATED_BODY()

		FRigUnit_GoreUpdate()
	{

	}

	RIGVM_METHOD()
		virtual void Execute() override;

	// Used to cache the internally used bone index
	UPROPERTY(transient)
		FRigUnit_GoreUpdate_WorkData WorkData;
};
