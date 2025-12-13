// Copyright 2025 Snow Game Studio.

#include "Sensors/HarmoniaSocketSensor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "CosmeticComponent.h"
#include "SenseReceiverComponent.h"

#if WITH_EDITORONLY_DATA
#include "SceneManagement.h"
#include "DrawDebugHelpers.h"
#endif

UHarmoniaSocketSensor::UHarmoniaSocketSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default to Manual sensor type - triggered via TriggerManualSensors() from AnimNotify
	// This ensures we only sense during attack windows, not continuously
	SensorType = ESensorType::Manual;
	
	// Disable timer-based updates since we're Manual
	UpdateTimeRate = 0.0f;
}

void UHarmoniaSocketSensor::InitializeFromReceiver(USenseReceiverComponent* FromReceiver)
{
	Super::InitializeFromReceiver(FromReceiver);

	// Auto-find skeletal mesh from owner if enabled
	if (bAutoFindOwnerMesh && !TargetMeshComponent.IsValid())
	{
		TargetMeshComponent = FindOwnerSkeletalMesh();
		
		if (TargetMeshComponent.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[HARMONIA_SOCKET_SENSOR] Auto-found mesh '%s' on owner '%s'"),
				*TargetMeshComponent->GetName(),
				GetSensorOwner() ? *GetSensorOwner()->GetName() : TEXT("NULL"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SOCKET_SENSOR] Failed to auto-find skeletal mesh on owner '%s'"),
				GetSensorOwner() ? *GetSensorOwner()->GetName() : TEXT("NULL"));
		}
	}
}

EUpdateReady UHarmoniaSocketSensor::GetSensorReadyBP_Implementation()
{
	// Update sensor transform from socket before returning ready state
	UpdateSensorTransformFromSocket();

	// Check if we have a valid socket
	if (!IsSocketValid())
	{
		return EUpdateReady::Fail;
	}

	return EUpdateReady::Ready;
}

void UHarmoniaSocketSensor::SetTargetMesh(USkeletalMeshComponent* InMesh)
{
	TargetMeshComponent = InMesh;

	if (InMesh)
	{
		UE_LOG(LogTemp, Log, TEXT("[HARMONIA_SOCKET_SENSOR] SetTargetMesh: Mesh='%s', Socket='%s', Valid=%d"),
			*InMesh->GetName(),
			*SocketName.ToString(),
			InMesh->DoesSocketExist(SocketName));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_SOCKET_SENSOR] SetTargetMesh: Mesh is nullptr"));
	}
}

USkeletalMeshComponent* UHarmoniaSocketSensor::GetTargetMesh() const
{
	return TargetMeshComponent.Get();
}

bool UHarmoniaSocketSensor::IsSocketValid() const
{
	if (!TargetMeshComponent.IsValid())
	{
		return false;
	}

	if (SocketName.IsNone())
	{
		return false;
	}

	return TargetMeshComponent->DoesSocketExist(SocketName);
}

FTransform UHarmoniaSocketSensor::GetSocketTransform() const
{
	if (!IsSocketValid())
	{
		return FTransform::Identity;
	}

	// Get socket transform in world space
	FTransform SocketWorldTransform = TargetMeshComponent->GetSocketTransform(SocketName, ERelativeTransformSpace::RTS_World);

	// Apply local offset and rotation
	if (!LocalOffset.IsNearlyZero() || !LocalRotation.IsNearlyZero())
	{
		FTransform LocalTransform(LocalRotation, LocalOffset);
		SocketWorldTransform = LocalTransform * SocketWorldTransform;
	}

	return SocketWorldTransform;
}

USkeletalMeshComponent* UHarmoniaSocketSensor::FindOwnerSkeletalMesh() const
{
	AActor* Owner = GetSensorOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Priority 1: Try CosmeticComponent's visual mesh (for cosmetic actor system)
	if (UCosmeticComponent* CosmeticComp = Owner->FindComponentByClass<UCosmeticComponent>())
	{
		if (USkeletalMeshComponent* VisualMesh = CosmeticComp->GetVisualMesh())
		{
			UE_LOG(LogTemp, Log, TEXT("[HARMONIA_SOCKET_SENSOR] Using CosmeticComponent visual mesh '%s'"),
				*VisualMesh->GetName());
			return VisualMesh;
		}
	}

	// Priority 2: Character's mesh (fallback if no cosmetic system)
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		return Character->GetMesh();
	}

	// Priority 3: Find any skeletal mesh component
	return Owner->FindComponentByClass<USkeletalMeshComponent>();
}

void UHarmoniaSocketSensor::UpdateSensorTransformFromSocket()
{
	FTransform NewTransform = GetSocketTransform();
	
	// Update the sensor's transform (used by SensorTests for detection origin)
	SensorTransform = NewTransform;
}

#if WITH_EDITORONLY_DATA
void UHarmoniaSocketSensor::SyncSensorTestTransforms()
{
	// Update SensorTransform first
	UpdateSensorTransformFromSocket();
	
	// Force all SensorTests to update their cached transform
	// This is needed because SensorTest.SensorTransform is only copied in PreTest()
	for (USensorTestBase* Test : SensorTests)
	{
		if (Test)
		{
			// PreTest() copies Sensor->GetSensorTransform() to Test->SensorTransform
			Test->PreTest();
		}
	}
}

void UHarmoniaSocketSensor::DrawSensor(const FSceneView* View, FPrimitiveDrawInterface* PDI) const
{
	// Sync SensorTest transforms before drawing (const_cast needed for mutable operation)
	const_cast<UHarmoniaSocketSensor*>(this)->SyncSensorTestTransforms();
	
	Super::DrawSensor(View, PDI);

	// Draw socket location indicator
	if (IsSocketValid() && PDI)
	{
		const FTransform SocketTransformWorld = GetSocketTransform();
		const FVector Location = SocketTransformWorld.GetLocation();
		const FVector Forward = SocketTransformWorld.GetUnitAxis(EAxis::X) * 50.0f;
		const FVector Right = SocketTransformWorld.GetUnitAxis(EAxis::Y) * 30.0f;
		const FVector Up = SocketTransformWorld.GetUnitAxis(EAxis::Z) * 30.0f;

		// Draw coordinate axes at socket location
		PDI->DrawLine(Location, Location + Forward, FColor::Red, SDPG_Foreground, 2.0f);
		PDI->DrawLine(Location, Location + Right, FColor::Green, SDPG_Foreground, 2.0f);
		PDI->DrawLine(Location, Location + Up, FColor::Blue, SDPG_Foreground, 2.0f);

		// Draw socket name
		// Note: Text drawing would require HUD/Canvas which isn't available in PDI
	}
}

void UHarmoniaSocketSensor::DrawDebug(bool bTest, bool bCurrentSensed, bool bLostSensed, bool bBestSensed, bool bAge, float Duration) const
{
	// Sync SensorTest transforms before drawing (for PIE runtime)
	const_cast<UHarmoniaSocketSensor*>(this)->SyncSensorTestTransforms();
	
	// Call base implementation which draws SensorTests
	Super::DrawDebug(bTest, bCurrentSensed, bLostSensed, bBestSensed, bAge, Duration);
}
#endif
