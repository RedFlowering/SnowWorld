// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RootMotionGuideSmartLinkFilter.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URootMotionGuideSmartLinkFilter : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROOTMOTIONGUIDEPLUGIN_API IRootMotionGuideSmartLinkFilter
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="SmartLink")
	bool IsLinkPathfindingAllowed(const UObject* Querier) const;

	virtual bool IsLinkPathfindingAllowed_Implementation(const UObject* Querier) const;
	
};
