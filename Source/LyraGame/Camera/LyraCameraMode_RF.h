// Copyright 2024 RedFlowering.

#pragma once

#include "CoreMinimal.h"
#include "LyraCameraMode.h"
#include "LyraCameraMode_RF.generated.h"

class UCurveVector;
class URFSpringArmComponentBase;

/**
 * ULyraCameraMode_RF
 *
 *	A basic third person camera mode.
 */

UCLASS(Abstract, Blueprintable)
class ULyraCameraMode_RF : public ULyraCameraMode
{
	GENERATED_BODY()

public:
	ULyraCameraMode_RF();	

	virtual void OnActivation() override;

	virtual void OnDeactivation() override;

protected:
	virtual void UpdateView(float DeltaTime) override;

	virtual FVector GetPivotLocation() const;

	virtual FRotator GetPivotRotation() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float AdjustPivotHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float AdjustPivotRight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float SpringArmLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	FRotator AdjustPivotRotate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag")
	bool bEnableCameraLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag")
	bool bEnableCameraRotationLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", meta = (editcondition = "bEnableCameraLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", meta = (editcondition = "bEnableCameraRotationLag", ClampMin = "0.0", ClampMax = "1000.0", UIMin = "0.0", UIMax = "1000.0"))
	float CameraRotationLagSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", AdvancedDisplay)
	bool bUseSpringArmLagSubstepping = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", AdvancedDisplay, meta = (editcondition = "bUseSpringArmLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float SpringArmLagMaxTimeStep = 0.016667f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", AdvancedDisplay)
	bool bUseCameraLagSubstepping = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lag", AdvancedDisplay, meta = (editcondition = "bUseCameraLagSubstepping", ClampMin = "0.005", ClampMax = "0.5", UIMin = "0.005", UIMax = "0.5"))
	float CameraLagMaxTimeStep = 0.016667f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn")
	bool bUsePawnControlRotation = false;

protected:	
	TObjectPtr<URFSpringArmComponentBase> SpringArm = nullptr;
};
