// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "ResetRootBoneTransform.generated.h"

/**
 * Resets the Skeleton's root bone transform to identity.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UResetRootBoneTransform : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:

	/**
	* Skeleton's SkeletalMesh to initialize the root bone
	*/
	UPROPERTY(EditAnywhere, Category = "ResetRootBoneTransform")
 	TObjectPtr<class USkeletalMesh> SkeletalMesh;
	
	/**
	* Resets the Skeleton's root bone transform to identity.
	*/
	FReply ResetRootBoneTransform();
	
};
