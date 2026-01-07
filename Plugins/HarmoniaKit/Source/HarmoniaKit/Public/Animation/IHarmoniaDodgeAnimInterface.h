// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IHarmoniaDodgeAnimInterface.generated.h"

/**
 * Interface for AnimInstances that support Harmonia dodge BlendSpace
 * 
 * Implement this interface in your AnimInstance (ALS, Lyra, custom) to receive
 * dodge direction values from HarmoniaMeleeCombatComponent.
 * 
 * The combat component will check for this interface first, then fall back to
 * direct property access if not implemented.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UHarmoniaDodgeAnimInterface : public UInterface
{
	GENERATED_BODY()
};

class HARMONIAKIT_API IHarmoniaDodgeAnimInterface
{
	GENERATED_BODY()

public:
	/**
	 * Set dodge direction for BlendSpace
	 * @param DirectionX Left(-1) to Right(+1)
	 * @param DirectionY Backward(-1) to Forward(+1)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Harmonia|Dodge")
	void SetDodgeDirection(float DirectionX, float DirectionY);

	/** Clear dodge direction and reset dodge state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Harmonia|Dodge")
	void ClearDodgeDirection();
};
