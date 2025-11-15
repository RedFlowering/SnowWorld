// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Engine/DataTable.h"
#include "NumberRendererDefine.generated.h"

DECLARE_STATS_GROUP(TEXT("Number Renderer"), STATGROUP_Number_Renderer, STATCAT_Advanced);

/**
 * Class representing an handle to a NumberHandle
 */
class FNumberHandle
{
public:
	enum EGenerateNewHandleType
	{
		GenerateNewHandle
	};

	FNumberHandle()
		: ID(0)
	{
	}

	explicit FNumberHandle(EGenerateNewHandleType)
		: ID(GenerateNewID())
	{
	}

	explicit FNumberHandle(uint64 NewID)
		: ID(NewID)
	{
	}

	bool IsValid() const
	{
		return ID != 0;
	}

	void Reset()
	{
		ID = 0;
	}
	uint64 GetID() const { return ID; }

private:
	friend bool operator==(const FNumberHandle& Lhs, const FNumberHandle& Rhs)
	{
		return Lhs.ID == Rhs.ID;
	}

	friend bool operator!=(const FNumberHandle& Lhs, const FNumberHandle& Rhs)
	{
		return Lhs.ID != Rhs.ID;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FNumberHandle& Key)
	{
		return GetTypeHash(Key.ID);
	}

	/**
	 * Generates a new ID for use the delegate handle.
	 *
	 * @return A unique ID for the delegate.
	 */
	static NUMBERRENDERER_API uint64 GenerateNewID();

	uint64 ID = 0;
};

USTRUCT(BlueprintType)
struct FNumberInstance
{
	GENERATED_BODY()

	FNumberInstance() {}

	bool bActive = false;
	bool bUseCurveAnim = false;
	bool bLockZAxis = false;
	float Number = 999;
	float NumberCount = 3;
	float Duration = 1.0f;
	float FadeTime = 0.5f;
	float StartTime = 0.0f;
	float Alpha = 1.0f;
	FLinearColor Color = FLinearColor::White;
	FNumberHandle Handle;
	FTransform StartTranform = FTransform::Identity;
	FTransform CurrentTranform = FTransform::Identity;
	FName RowName;
};

USTRUCT(BlueprintType)
struct FNumberAnimation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "NumberAnimation")
	TObjectPtr<UCurveVector> Location = nullptr;

	UPROPERTY(EditAnywhere, Category = "NumberAnimation")
	TObjectPtr<UCurveFloat> RotationX = nullptr;

	UPROPERTY(EditAnywhere, Category = "NumberAnimation")
	TObjectPtr<UCurveVector> Scale = nullptr;

	UPROPERTY(EditAnywhere, Category = "NumberAnimation")
	TObjectPtr<UCurveLinearColor> Color = nullptr;
};

USTRUCT(BlueprintType)
struct FNumberAnimationTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "NumberAnimation")
	FNumberAnimation NumberAnimation;
};

UENUM(BlueprintType)
enum ENumberFontType
{
	NumberFontType_None UMETA(DisplayName = "None"),
	NumberFontType1 UMETA(Hidden),
	NumberFontType2 UMETA(Hidden),
	NumberFontType3 UMETA(Hidden),
	NumberFontType4 UMETA(Hidden),
	NumberFontType5 UMETA(Hidden),
	NumberFontType6 UMETA(Hidden),
	NumberFontType7 UMETA(Hidden),
	NumberFontType8 UMETA(Hidden),
	NumberFontType9 UMETA(Hidden),
	NumberFontType10 UMETA(Hidden),
	NumberFontType_Max UMETA(Hidden)
};