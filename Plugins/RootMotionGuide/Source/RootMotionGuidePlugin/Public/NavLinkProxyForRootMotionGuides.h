// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "RootMotionGuideSmartLinkFilter.h"
#include "NavLinkProxyForRootMotionGuides.generated.h"


class ARootMotionGuide;

/**
 * 
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGIN_API ANavLinkProxyForRootMotionGuides : public ANavLinkProxy, public IRootMotionGuideSmartLinkFilter
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category = "SmartLink")
	FVector LinkStartOffset;

	UPROPERTY(EditAnywhere, Category = "SmartLink")
	FVector LinkEndOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SmartLink", Meta = (MakeEditWidget))
	FVector LinkStart;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SmartLink", Meta = (MakeEditWidget))
	FVector LinkEnd;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide")
	TObjectPtr<ARootMotionGuide> RootMotionGuideStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide")
	TObjectPtr<ARootMotionGuide> RootMotionGuideEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RootMotionGuide")
	FVector MovementInput;
	

	ANavLinkProxyForRootMotionGuides(const FObjectInitializer& ObjectInitializer);


	UFUNCTION(CallInEditor, Category = "SmartLink")
	void UpdateSmartLink();


};
