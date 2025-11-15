// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CosmeticComponent.h"
#include "CosmeticBFL.generated.h"

class UPhysicsAsset;

/**
 * 
 */
UCLASS()
class COSMETICSYSTEM_API UCosmeticBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Cosmetic System")
	static ACosmeticActor* GetCosmeticActor(const ACharacter* Target);

	UFUNCTION(BlueprintPure, Category = "Cosmetic System")
	static USkeletalMeshComponent* GetVisualMesh(const ACharacter* Target);

	UFUNCTION(BlueprintPure, Category = "Cosmetic System")
	static USkeletalMeshComponent* GetCosmeticMegedSkeletalMeshComponent(const ACharacter* Target, FName ComponentTag);

	UFUNCTION(BlueprintPure, Category = "Cosmetic System")
	static USkeletalMesh* GetCosmeticMegedSkeletalMesh(const ACharacter* Target, FName ComponentTag);
	
	UFUNCTION(BlueprintPure, Category = "Cosmetic System")
	static UCosmeticComponent* GetCosmeticComponent(const ACharacter* Target);

	// Cosmetic Item ID
	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartA(FCosmeticItemID ItemID) { return ItemID.GetPartA(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartB(FCosmeticItemID ItemID) { return ItemID.GetPartB(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartC(FCosmeticItemID ItemID) { return ItemID.GetPartC(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartD(FCosmeticItemID ItemID) { return ItemID.GetPartD(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartAB(FCosmeticItemID ItemID) { return ItemID.GetPartAB(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartBC(FCosmeticItemID ItemID) { return ItemID.GetPartBC(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartCD(FCosmeticItemID ItemID) { return ItemID.GetPartCD(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartABC(FCosmeticItemID ItemID) { return ItemID.GetPartABC(); }

	UFUNCTION(BlueprintPure, Category = "Cosmetic Item ID")
	static int32 GetCosmeticItemIDPartBCD(FCosmeticItemID ItemID) { return ItemID.GetPartBCD(); }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToInt (CosmeticItemID)", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Cosmetic Item ID")
	static int32 Conv_CosmeticItemIDToInt(FCosmeticItemID CosmeticItemID);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (CosmeticItemID)", CompactNodeTitle = "->", Keywords = "cast convert", BlueprintAutocast), Category = "Cosmetic Item ID")
	static FString Conv_CosmeticItemIDToString(FCosmeticItemID CosmeticItemID);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (CosmeticItemID)", CompactNodeTitle = "==", BlueprintThreadSafe), Category = "Cosmetic Item ID")
	static bool Equal_CosmeticItemID(const FCosmeticItemID& A, const FCosmeticItemID& B);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (CosmeticItemID)", CompactNodeTitle = "!=", BlueprintThreadSafe), Category = "Cosmetic Item ID")
	static bool NotEqual_CosmeticItemID(const FCosmeticItemID& A, const FCosmeticItemID& B);

};
