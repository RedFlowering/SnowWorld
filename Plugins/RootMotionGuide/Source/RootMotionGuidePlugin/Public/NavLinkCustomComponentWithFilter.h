// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "NavLinkCustomComponent.h"
#include "NavLinkCustomComponentWithFilter.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGIN_API UNavLinkCustomComponentWithFilter : public UNavLinkCustomComponent
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmartLink")
	TObjectPtr<UObject> SmartLinkFilter;
	
	
	virtual bool IsLinkPathfindingAllowed(const UObject* Querier) const override;
};
