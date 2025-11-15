// Copyright 2022 HGsofts, Ltd. All Rights Reserved.


#include "NumberRenderActor.h"
#include "NRBlueprintFunctionLibrary.h"
#include "NumberISMComponent.h"
#include "NumberRenderer.h"
#include "NumberRendererDefine.h"
#include "Curves/CurveLinearColor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"



DECLARE_CYCLE_STAT(TEXT("Number Renderer - Number Render Actor tick"), STAT_NumberRenderActorTick, STATGROUP_Number_Renderer);

DECLARE_DWORD_COUNTER_STAT(TEXT("Number count"), STAT_Number_Count, STATGROUP_Number_Renderer);


ANumberRenderActor::ANumberRenderActor()
{
	PrimaryActorTick.bCanEverTick = true;

#if !UE_SERVER	
	Meshes = CreateDefaultSubobject<UNumberISMComponent>(FName("Meshes"));
	Meshes->SetSimulatePhysics(false);
	Meshes->SetCastShadow(false);
	Meshes->SetGenerateOverlapEvents(false);
	Meshes->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Meshes->PreAllocateInstancesMemory(InitialMaxCount);
	Meshes->NumCustomDataFloats = 6;
	Instances.Reserve(InitialMaxCount);
	PendingDelete.Reserve(InitialMaxCount);
	SetRootComponent(Meshes);
#endif

}

void ANumberRenderActor::BeginPlay()
{
	Super::BeginPlay();

#if !UE_SERVER
	if (NumberAnimationTable)
		RowNames = NumberAnimationTable->GetRowNames();

	TempFontXYSizeRate = FontXYSizeRate * 0.333f;

	bInitialized = true;
#endif

}

void ANumberRenderActor::Tick(float DeltaTime)
{
#if !UE_SERVER
	SCOPE_CYCLE_COUNTER(STAT_NumberRenderActorTick);

	Super::Tick(DeltaTime);

	float CurrentTime = GetWorld()->GetTimeSeconds();

	PendingDelete.Empty(InitialMaxCount);

	FQuat BillobrdRotate(FQuat::Identity);
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		if (APlayerController* PlayerController = GetWorld()->GetGameInstance()->GetFirstLocalPlayerController())
		{
			FVector X, Y, Z;
			FRotationMatrix R(PlayerController->PlayerCameraManager->GetCameraRotation());
			R.GetScaledAxes(X, Y, Z);
			X = FVector::CrossProduct(Z, Y);
			FMatrix RotMatrix(X, -Y, Z, FVector::ZeroVector);
			BillobrdRotate = RotMatrix.ToQuat();


			for (int32 Index = 0; Index < Instances.Num(); ++Index)
			{
				FNumberInstance& Instance = Instances[Index];

				float LastTime = CurrentTime - Instance.StartTime;
				if (LastTime > Instance.Duration)
				{
					PendingDelete.Add(Instance.Handle);
					continue;
				}

				Instance.Alpha = FMath::Clamp((Instance.Duration - LastTime) / Instance.FadeTime, 0.0f, 1.0f);
				if (FMath::IsNearlyZero(Instance.Alpha))
				{
					PendingDelete.Add(Instance.Handle);
					continue;
				}

				Instance.CurrentTranform.SetLocation(Instance.StartTranform.GetLocation());
				Instance.CurrentTranform.SetScale3D(Instance.StartTranform.GetScale3D());

				if (NumberAnimationTable && Instance.bUseCurveAnim)
				{
					FNumberAnimationTable* Anim = NumberAnimationTable->FindRow<FNumberAnimationTable>(Instance.RowName, FString(), true);
					if (Anim)
					{
						if (Anim->NumberAnimation.Location)
						{
							FVector AnimLocation = Anim->NumberAnimation.Location->GetVectorValue(LastTime);
							Instance.CurrentTranform.SetLocation(Instance.CurrentTranform.GetLocation() + AnimLocation);
						}

						if (Anim->NumberAnimation.RotationX)
						{
							float AnimRotation = Anim->NumberAnimation.RotationX->GetFloatValue(LastTime);
							FQuat NewRot = FRotator(0, 0, AnimRotation).Quaternion();

							if (!Instance.bLockZAxis)
							{
								NewRot = BillobrdRotate * NewRot;
								NewRot.Normalize();
								Instance.CurrentTranform.SetRotation(NewRot);
							}
							else
							{
								FVector DirToCamera = PlayerController->PlayerCameraManager->GetCameraLocation() - Instance.CurrentTranform.GetLocation();
								DirToCamera.Normalize();

								NewRot = FRotationMatrix::MakeFromZX(FVector(0, 0, 1), DirToCamera).ToQuat() * NewRot;
								NewRot.Normalize();
								Instance.CurrentTranform.SetRotation(NewRot);
							}
						}
						else
						{
							if (!Instance.bLockZAxis)
							{
								Instance.CurrentTranform.SetRotation(BillobrdRotate); // BillBoard Transform
							}
							else
							{
								FVector DirToCamera = PlayerController->PlayerCameraManager->GetCameraLocation() - Instance.CurrentTranform.GetLocation();
								DirToCamera.Normalize();
								Instance.CurrentTranform.SetRotation(FRotationMatrix::MakeFromZX(FVector(0, 0, 1), DirToCamera).ToQuat());
							}
						}

						if (Anim->NumberAnimation.Scale)
						{
							FVector AnimScale = Anim->NumberAnimation.Scale->GetVectorValue(LastTime);
							Instance.CurrentTranform.SetScale3D(Instance.CurrentTranform.GetScale3D() * AnimScale);
						}

						if (Anim->NumberAnimation.Color)
						{
							FLinearColor Color = Anim->NumberAnimation.Color->GetLinearColorValue(LastTime);
							Instance.Color = Color;
							Instance.Alpha = Color.A;
						}
					}
				}
				else
				{
					if (!Instance.bLockZAxis)
					{
						Instance.CurrentTranform.SetRotation(BillobrdRotate); // BillBoard Transform
					}
					else
					{
						FVector DirToCamera = PlayerController->PlayerCameraManager->GetCameraLocation() - Instance.CurrentTranform.GetLocation();
						DirToCamera.Normalize();
						Instance.CurrentTranform.SetRotation(FRotationMatrix::MakeFromZX(FVector(0, 0, 1), DirToCamera).ToQuat());
					}
				}
			}
			// Delete
			for (int32 i = 0; i < PendingDelete.Num(); ++i)
			{
				for (int32 Index = 0; Index < Instances.Num(); ++Index)
				{
					if (Instances[Index].Handle == PendingDelete[i])
					{
						Instances.RemoveAtSwap(Index, 1, EAllowShrinking::No);
						break;
					}
				}
			}

			SET_DWORD_STAT(STAT_Number_Count, Instances.Num());

			Meshes->ResetInstancesTransforms(Instances, InitialMaxCount);
		}
	}

#endif
}

FNumberHandle ANumberRenderActor::AddNumber(int32 Number, const FTransform& Trans, FLinearColor Color, float Duration, float FadeTime, bool LockZAxis)
{
	FNumberInstance Instance;
	if (bInitialized)
	{
#if !UE_SERVER

		FVector NewScale = Trans.GetScale3D();
		int32 Count = UNRBlueprintFunctionLibrary::GetDrawNumberCount(Number);
		NewScale.Y *= (TempFontXYSizeRate * Count);

		Instance.bActive = true;
		Instance.bLockZAxis = LockZAxis;
		Instance.Number = Number;
		Instance.NumberCount = Count;
		Instance.Handle = FNumberHandle(FNumberHandle::EGenerateNewHandleType::GenerateNewHandle);
		Instance.Duration = Duration;
		Instance.FadeTime = FadeTime;
		Instance.StartTime = GetWorld()->GetTimeSeconds();
		Instance.Color = Color;
		Instance.StartTranform = Trans;
		Instance.StartTranform.SetScale3D(NewScale);
		Instances.Add(Instance);

#if WITH_EDITOR
		if (FMath::IsNearlyZero(Duration))
		{
			UE_LOG(LogNumberRenderer, Error, TEXT("Duration is 0.0"));
		}

		if (Instances.Num() >= InitialMaxCount)
		{
			UE_LOG(LogNumberRenderer, Warning, TEXT("Max Count = %d"), Instances.Num());
		}
#endif // WITH_EDITOR

#endif // !UE_SERVER
	}
	return Instance.Handle;
}

FNumberHandle ANumberRenderActor::AddDrawNumberByTable(int32 Number, const FTransform& Trans, float Duration, FName TableRowName, bool LockZAxis)
{
	FNumberInstance Instance;
	if (bInitialized)
	{
#if !UE_SERVER

		FVector NewScale = Trans.GetScale3D();
		int32 Count = UNRBlueprintFunctionLibrary::GetDrawNumberCount(Number);
		NewScale.Y *= (TempFontXYSizeRate * (float)Count);

		Instance.bActive = true;
		Instance.bUseCurveAnim = true;
		Instance.bLockZAxis = LockZAxis;
		Instance.RowName = TableRowName;
		Instance.Number = Number;
		Instance.NumberCount = Count;
		Instance.Handle = FNumberHandle(FNumberHandle::EGenerateNewHandleType::GenerateNewHandle);
		Instance.Duration = Duration;
		Instance.FadeTime = 0.5f;
		Instance.StartTime = GetWorld()->GetTimeSeconds();
		Instance.Color = FLinearColor::White;
		Instance.StartTranform = Trans;
		Instance.StartTranform.SetScale3D(NewScale);
		Instances.Add(Instance);

#if WITH_EDITOR
		if (FMath::IsNearlyZero(Duration))
		{
			UE_LOG(LogNumberRenderer, Error, TEXT("Duration is 0.0"));
		}

		if (Instances.Num() >= InitialMaxCount)
		{
			UE_LOG(LogNumberRenderer, Warning, TEXT("Max Count = %d"), Instances.Num());
		}
#endif // WITH_EDITOR

#endif // !UE_SERVER
	}
	return Instance.Handle;
}

FNumberHandle ANumberRenderActor::AddDrawNumberByTable(int32 Number, const FTransform& Trans, float Duration, int32 RowNameIndex, bool LockZAxis)
{
	if (bInitialized)
	{
#if !UE_SERVER
		if (RowNameIndex < RowNames.Num())
		{
			return AddDrawNumberByTable(Number, Trans, Duration, RowNames[RowNameIndex], LockZAxis);
		}
#endif // !UE_SERVER
	}
	return FNumberHandle();
}

void ANumberRenderActor::Preload(const FTransform& Trans, float Duration)
{
	if (bInitialized)
	{
#if !UE_SERVER
		if (Meshes && Meshes->OverlayMaterial)
		{
			for (auto& MeterialInstance : Meshes->GetMaterials())
			{
				MeterialInstance->SetForceMipLevelsToBeResident(true, true, 600.0f);
			}
		}

		if (RowNames.Num() > 0)
		{
			for (const auto& Name : RowNames)
			{
				AddDrawNumberByTable(0, Trans, Duration, Name, false);
			}
		}
		else
		{
			AddNumber(0, Trans, FLinearColor::Black, Duration, 0.1f, false);
		}

#endif // !UE_SERVER
	}
}

