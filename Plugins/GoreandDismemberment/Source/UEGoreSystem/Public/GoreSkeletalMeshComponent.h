// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GoreSkeletalMeshComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetachPhysicsEnabled, FName, Bone);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UEGORESYSTEM_API UGoreSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	UGoreSkeletalMeshComponent();	

	UPROPERTY()
		bool bDisableMultiThread;
	UPROPERTY()
		FName NewRootBone;
	UPROPERTY()
		uint8 StretchFixEnabled : 1;
	UPROPERTY()
		FVector DetachImpulse;

	UPROPERTY()
		uint8 InitialVisibilityFired : 1;

protected:
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**[HINT] Called when this detached limb starts simulating physics*/
	UPROPERTY(BlueprintAssignable, Category = "GoreSystem")
	FOnDetachPhysicsEnabled On_DetachPhysicsEnabled;

	/**
	* Will hide an individual bone by its name !!! THIS WILL FORCE SOME SKELETALMESHCOMPONENT MECHANICS !!! | Do not use this function with default "HideBoneByName" function
	* >BoneName - Name of bone to hide
	* >PhysBodyOption - Option for physics bodies that attach to the bones to be hidden
	* >bAvoidChildren - Exclude children (will not hide child bones)
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|SkinnedMesh")
		void HideBoneByNameGore(const FName BoneName, const  EPhysBodyOp PhysBodyOption, const bool bAvoidChildren);
	/**
	* Will hide all the bones before a certain bone name (excluded) !!! THIS WILL FORCE SOME SKELETALMESHCOMPONENT MECHANICS !!! | Do not use this function with default "HideBoneByName" function
	* >BoneName - Name of bone to hide
	* >PhysBodyOption - Option for physics bodies that attach to the bones to be hidden
	*/
	UFUNCTION(BlueprintCallable, Category = "Components|SkinnedMesh")
		void HideBoneBeforeNameGore(const FName BoneName, const EPhysBodyOp PhysBodyOption);

	/**[Internal] This function will hide a list of bones deciding if avoid hiding their children*/
	UFUNCTION()
		void HideBonesGore(const TArray<int32> BoneIndexes, const EPhysBodyOp PhysBodyOption, const bool bAvoidChildren);
	/**[Internal] This function will unhide a list of bones deciding if avoid hiding their children*/
	UFUNCTION()
		void UnhideBonesGore(const TArray<int32> BoneIndexes);

		/**
	 * Rebuild BoneVisibilityStates array. Mostly refresh information of bones for BVS_HiddenByParent
	 */
		void NewRebuildVisibilityArray(const bool bShouldAvoidChildren);
	
};
