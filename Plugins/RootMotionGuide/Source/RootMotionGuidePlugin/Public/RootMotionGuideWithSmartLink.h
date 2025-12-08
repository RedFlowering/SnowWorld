// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "RootMotionGuide.h"
#include "NavLinkCustomComponentWithFilter.h"
#include "RootMotionGuideSmartLinkFilter.h"
#include "RootMotionGuideWithSmartLink.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGIN_API ARootMotionGuideWithSmartLink : public ARootMotionGuide, public IRootMotionGuideSmartLinkFilter
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

	ARootMotionGuideWithSmartLink();


	UFUNCTION(CallInEditor, Category = "SmartLink")
	void UpdateSmartLink();


	virtual void InitializeSubTrigger(const ARootMotionGuide* MainTrigger) override;


	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveSmartLinkReached(AActor* Agent, const FVector& Destination);
	

private:

	UPROPERTY()
	TObjectPtr<UNavLinkCustomComponentWithFilter> SmartLinkComp;
	

	void NotifySmartLinkReached(UNavLinkCustomComponent* LinkComp, UObject* PathingAgent, const FVector& DestPoint);
};
