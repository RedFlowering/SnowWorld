// Copyright 2024 RedFlowering.

#include "LyraCameraMode_RF.h"
#include "GameFramework/Character.h"
#include "RFSpringArmComponentBase.h"
#include "Character/LyraCharacter.h"
#include "Components/CapsuleComponent.h"

ULyraCameraMode_RF::ULyraCameraMode_RF()
{

}

void ULyraCameraMode_RF::OnActivation()
{
	SpringArm = GetTargetActor()->FindComponentByClass<URFSpringArmComponentBase>();

	if (SpringArm)
	{
		SpringArm->SetViewPitchMinMax(ViewPitchMin, ViewPitchMax);
		SpringArm->SetRelativeLocation(FVector::ZeroVector);			
		SpringArm->TargetArmLength = SpringArmLength;
		SpringArm->bDoCollisionTest = true;
		SpringArm->bEnableCameraLag = bEnableCameraLag;
		SpringArm->bEnableCameraRotationLag = bEnableCameraRotationLag;
		SpringArm->CameraLagSpeed = CameraLagSpeed;
		SpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
		SpringArm->SetUseTargetArmLagSubstepping(bUseSpringArmLagSubstepping);
		SpringArm->SetTargetArmLagMaxTimeStep(SpringArmLagMaxTimeStep);
		SpringArm->bUseCameraLagSubstepping = bUseCameraLagSubstepping;
		SpringArm->CameraLagMaxTimeStep = CameraLagMaxTimeStep;
		SpringArm->bUsePawnControlRotation = bUsePawnControlRotation;
	}	
}

void ULyraCameraMode_RF::OnDeactivation()
{
	SpringArm = nullptr;
}

void ULyraCameraMode_RF::UpdateView(float DeltaTime)
{
	if (SpringArm)
	{		
		FTransform Trans = SpringArm->GetSocketTransform(USpringArmComponent::SocketName, ERelativeTransformSpace::RTS_World);
		
		// Camera
		View.Location = Trans.GetLocation();
		View.Rotation = Trans.GetRotation().Rotator();
		View.ControlRotation = View.Rotation;
		View.FieldOfView = FieldOfView;
		
		// Spring Arm
		Arm.Location = GetPivotLocation();
		Arm.Rotation = GetPivotRotation();
		Arm.Length = SpringArmLength;
	}
}

FVector ULyraCameraMode_RF::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight + AdjustPivotHeight;

			const FVector ActualCapsuleRight = CapsuleComp->GetRightVector();

			return (FVector::UpVector * HeightAdjustment) + (ActualCapsuleRight * AdjustPivotRight);
		}

		return FVector::ZeroVector;
	}

	return FVector::ZeroVector;
}

FRotator ULyraCameraMode_RF::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return FRotator3d::ZeroRotator + AdjustPivotRotate;
	}

	return FRotator3d::ZeroRotator;
}
