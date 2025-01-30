// Copyright 2024 RedFlowering.
#pragma once

#include "GameFramework/SpringArmComponent.h"
#include "RFSpringArmComponentBase.generated.h"

UCLASS(ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class LYRAGAME_API URFSpringArmComponentBase : public USpringArmComponent
{
	GENERATED_UCLASS_BODY()

public:
	void SetViewPitchMinMax(float PitchMin, float PitchMax) { ViewPitchMin = PitchMin; ViewPitchMax = PitchMax; }

	void SetUseTargetArmLagSubstepping(bool bUse) { bUseTargetArmLagSubstepping = bUse; }
	void SetTargetArmLagMaxTimeStep(float TimeStep) { TargetArmLagMaxTimeStep  = TimeStep; }

protected:
	virtual void OnRegister() override;

	virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay)
	uint32 bUseTargetArmLagSubstepping : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float TargetArmLagSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lag, AdvancedDisplay, meta = (editcondition = "bUseTargetArmLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float TargetArmLagMaxTimeStep = 0.016667f;	
	
	float PreviousTargetArmLength = 0.0f;

protected:
	// Minimum view pitch (in degrees).
	float ViewPitchMin = -89.0f;

	// Maximum view pitch (in degrees).
	float ViewPitchMax = 89.0f;
};