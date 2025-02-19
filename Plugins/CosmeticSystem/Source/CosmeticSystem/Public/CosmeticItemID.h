// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CosmeticItemID.generated.h"

UENUM(BlueprintType)
enum class ECosmeticItemIDCompareType : uint8
{		
	A = 0,	// 99 000000	// 9 is compare, 0 is ignore
	B,		// 00 99 0000 
	C,		// 0000 99 00 
	D,		// 000000 99 
	AB,		// 9999 0000
	BC,		// 00 9999 00
	CD,		// 0000 9999
	ABC,	// 999999 00
	BCD,	// 00 999999
	All,	// 99999999
};

/*
* CosmeticsItemID has 8 digits.
* The first four digits are the category of CosmeticsItemID of MeshPart.
* Categories are used to remove the same category items.
* The last four digits are the unique ID of the item.
* CosmeticsItemID of BodyType and RetargetMeshes is just a unique ID.
*/
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FCosmeticItemID
{
	GENERATED_BODY()

public:	
	FCosmeticItemID() = default;

	FCosmeticItemID(int32 ID);

	FCosmeticItemID(const FCosmeticItemID& ID) : Identifier(ID.Identifier) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic Item Identifier")
	int32 Identifier = -1; // must be 8 Digits

	bool IsValid() const { return Identifier >= 0 && Identifier < 100000000; }

	FORCEINLINE int32 GetIntID() const { return Identifier; }

	FORCEINLINE int32 GetPartA() const { return Identifier - (Identifier % 1000000); }

	FORCEINLINE int32 GetPartB() const { return (Identifier % 1000000) - (Identifier % 10000); }

	FORCEINLINE int32 GetPartC() const { return (Identifier % 10000) - (Identifier % 100); }

	FORCEINLINE int32 GetPartD() const { return Identifier % 100; }

	FORCEINLINE int32 GetPartAB() const { return Identifier - (Identifier % 10000); }

	FORCEINLINE int32 GetPartBC() const { return (Identifier - (Identifier % 100)) % 1000000; }

	FORCEINLINE int32 GetPartCD() const { return Identifier % 10000; }

	FORCEINLINE int32 GetPartABC() const { return Identifier - (Identifier % 100); }

	FORCEINLINE int32 GetPartBCD() const { return Identifier % 1000000; }
};

FORCEINLINE bool operator==(const FCosmeticItemID& Lhs, const FCosmeticItemID& Rhs) { return Lhs.Identifier == Rhs.Identifier; }
FORCEINLINE bool operator!=(const FCosmeticItemID& Lhs, const FCosmeticItemID& Rhs) { return Lhs.Identifier != Rhs.Identifier; }
FORCEINLINE uint32 GetTypeHash(const FCosmeticItemID& ID) {	return ID.Identifier; }