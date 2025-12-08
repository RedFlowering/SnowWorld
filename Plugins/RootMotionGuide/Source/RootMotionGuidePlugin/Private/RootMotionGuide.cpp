// Copyright 2017 Lee Ju Sik

#include "RootMotionGuide.h"
//#include "Engine.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "RootMotionMovement.h"

#define LOCTEXT_NAMESPACE "RootMotionGuide"

// Sets default values
ARootMotionGuide::ARootMotionGuide()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	CapsuleHalfHeight = 96.0f;
	CapsuleRadius = 42.0f;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Capsule->SetCollisionProfileName(TEXT("Trigger"));
	

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->AttachToComponent(Capsule, FAttachmentTransformRules::KeepRelativeTransform);

	MovementDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("MovementDirectionArrow"));
	MovementDirectionArrow->ArrowColor = FColor::Green;
	MovementDirectionArrow->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	MovementDirectionArrow->AttachToComponent(Capsule, FAttachmentTransformRules::KeepRelativeTransform);

	TriggerType = ERootMotionGuideTriggerType::AutoPlay;

	bCanPlayInPlayingOtherRootMotion = true;

	RootMotionBlendTime = 0.2f;
	MovementDirection = FVector(1.0f, 0.0f, 0.0f);
	MinimumDotValue = -1.0f;
	NextMovementMode = EMovementMode::MOVE_Walking;

	bIsSubTrigger = false;

	bEnableAdjustRootMotionScale = false;
	AdjustRootMotionScale = FVector::OneVector;

#if	WITH_EDITORONLY_DATA

	bShowArrows = false;
	bShowSnapshot = true;

	bShowSubTriggerSnapshot = false;

#endif
}

void ARootMotionGuide::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	if (World)
	{
		for (int i = 1; i <= SubTriggerCount; i++)
		{
			FTransform SubTriggerTransform = FTransform::Identity;
			SubTriggerTransform.SetLocation(SubTriggerOffset * i);
			SubTriggerTransform = SubTriggerTransform;// *GetActorTransform();

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParameters.bDeferConstruction = true;

			ARootMotionGuide* SubTrigger = World->SpawnActor<ARootMotionGuide>(GetClass(), SpawnParameters);
			SubTrigger->InitializeSubTrigger(this);
			SubTrigger->FinishSpawning(SubTriggerTransform);
			SubTrigger->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

			// Rename for multiplay
			SubTrigger->Rename(*(GetName() + TEXT("_Sub_") + FString::FromInt(i)), GetOuter());
		}
	}
	
}

void ARootMotionGuide::InitializeSubTrigger(const ARootMotionGuide * MainTrigger)
{
	AnimSequence = MainTrigger->AnimSequence;
	TriggerOffset = MainTrigger->TriggerOffset;
	bShowCapsuleInGame = MainTrigger->bShowCapsuleInGame;
	CapsuleHalfHeight = MainTrigger->CapsuleHalfHeight;
	CapsuleRadius = MainTrigger->CapsuleRadius;
	TriggerType = MainTrigger->TriggerType;
	TriggerRange = MainTrigger->TriggerRange;
	NonInterruptibleTime = MainTrigger->NonInterruptibleTime;
	bCanPlayInPlayingOtherRootMotion = MainTrigger->bCanPlayInPlayingOtherRootMotion;
	AnimMontage = MainTrigger->AnimMontage;
	RootMotionBlendTime = MainTrigger->RootMotionBlendTime;
	MovementDirection = MainTrigger->MovementDirection;
	MinimumDotValue = MainTrigger->MinimumDotValue;
	PrevRootMotionGuideTag = MainTrigger->PrevRootMotionGuideTag;
	NextMovementMode = MainTrigger->NextMovementMode;
	TotalRootMotion = MainTrigger->TotalRootMotion;
	TotalRootRotation = MainTrigger->TotalRootRotation;
	CharacterMeshRotation = MainTrigger->CharacterMeshRotation;

	bEnableAdjustRootMotionScale = MainTrigger->bEnableAdjustRootMotionScale;
	AdjustRootMotionScaleBeginTime = MainTrigger->AdjustRootMotionScaleBeginTime;
	AdjustRootMotionScaleEndTime = MainTrigger->AdjustRootMotionScaleEndTime;
	AdjustRootMotionScaleBlendInTime = MainTrigger->AdjustRootMotionScaleBlendInTime;
	AdjustRootMotionScaleBlendOutTime = MainTrigger->AdjustRootMotionScaleBlendOutTime;
	AdjustRootMotionScale = MainTrigger->AdjustRootMotionScale;
	AdjustRootMotionScaleBeginLocation = MainTrigger->AdjustRootMotionScaleBeginLocation;
	AdjustRootMotionScaleBlendInLocation = MainTrigger->AdjustRootMotionScaleBlendInLocation;
	AdjustRootMotionScaleBlendOutLocation = MainTrigger->AdjustRootMotionScaleBlendOutLocation;
	AdjustRootMotionScaleEndLocation = MainTrigger->AdjustRootMotionScaleEndLocation;

	Tags = MainTrigger->Tags;

	bIsSubTrigger = true;

	ReceiveInitializeSubTrigger(MainTrigger);
}

FVector ARootMotionGuide::GetRootMotionLocation() const
{
	return Arrow->GetComponentLocation();
}


FVector ARootMotionGuide::GetRootMotionDirection() const
{
	return Arrow->GetForwardVector();
}

FRotator ARootMotionGuide::GetRootMotionRotation() const
{
	//return Arrow->GetForwardVector().ToOrientationRotator();
	return Arrow->GetComponentRotation();
}

UAnimMontage* ARootMotionGuide::GetRootMotionAnimMontage() const
{
	return AnimMontage;
}

FVector ARootMotionGuide::GetMovementDirectionInWorld() const
{
	return MovementDirectionArrow->GetForwardVector();
}

EMovementMode ARootMotionGuide::GetRootMotionNextMovementMode() const
{
	return NextMovementMode;
}

void ARootMotionGuide::BeginOverlap_Implementation(ACharacter * Character)
{
}

void ARootMotionGuide::EndOverlap_Implementation(class ACharacter* Character)
{
}

bool ARootMotionGuide::CanPlayRootMotionMontage_Implementation(class ACharacter* Character, const URootMotionMovement* RootMotionMovement) const
{
	if (IsValid(AnimMontage) == false)
	{
		UE_LOG(LogClass, Log, TEXT("No AnimMontage"));
		return false;
	}
	
	if (AnimMontage->HasRootMotion() == false)
	{
		UE_LOG(LogClass, Log, TEXT("No RootMotion"));
		return false;
	}

	if (bCanPlayInPlayingOtherRootMotion == false && RootMotionMovement->IsPlayingRootMotion())
	{
		return false;
	}

	if (PrevRootMotionGuideTag.IsNone() == false)
	{
		if (IsValid(RootMotionMovement->CurrentRootMotionGuide) == false || RootMotionMovement->CurrentRootMotionGuide->ActorHasTag(PrevRootMotionGuideTag) == false)
		{
			return false;
		}
	}

	if (TriggerRange > 0.0f)
	{
		float Distance = (GetRootMotionLocation() - Character->GetActorLocation()).Size();

		if (Distance > TriggerRange)
		{
			return false;
		}
	}
	
	return true;
}

void ARootMotionGuide::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Capsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	Capsule->SetRelativeLocation(FVector(0.0f, 0.0f, CapsuleHalfHeight) + TriggerOffset);
	Capsule->SetHiddenInGame(bShowCapsuleInGame == false, true);

	MovementDirectionArrow->SetRelativeRotation(MovementDirection.ToOrientationRotator());
	

	if (bIsSubTrigger)
	{
		return;
	}


	UpdateArrow();
	UpdateSnapshot();
}


void ARootMotionGuide::UpdateArrow()
{
#if WITH_EDITOR

	for (UArrowComponent* ArrowComponent : ArrowArray)
	{
		if (IsValid(ArrowComponent))
		{
			ArrowComponent->DestroyComponent();
		}
	}

	ArrowArray.Empty();

	if (IsValid(AnimSequence) == false)
	{
		//UE_LOG(LogClass, Log, TEXT("No AnimSequence"));
		return;
	}

	if (AnimSequence != AnimSequenceForAnimationData)
	{
		UE_LOG(LogClass, Log, TEXT("Invalid AnimationData"));
		return;
	}


	if (bShowArrows)
	{
		TArray<FName> BoneNameArray;
		TArray<FColor> ColorArray;

		BoneNameArray.Empty(ExtractBoneArray.Num());
		ColorArray.Empty(ExtractBoneArray.Num());

		for (int i = 0; i < ExtractBoneArray.Num(); i++)
		{
			BoneNameArray.Add(ExtractBoneArray[i].BoneName);
			ColorArray.Add(ExtractBoneArray[i].ArrowColor);
		}

		FVector FirstRootPosition = FVector::ZeroVector;
		if (AnimSequence->bEnableRootMotion)
		{
			if (AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() > 0)
			{
				FirstRootPosition = AnimationData[0].ComponentSpaceAnimationTrack[0].GetLocation();
			}
		}


		FVector PrevPosition = FVector::ZeroVector;

		bool bRootTrack = true;

		for (const FRootMotionGuideAnimationTrack& Track : AnimationData)
		{
			FColor ArrowColor;
			if (bRootTrack)
			{
				bRootTrack = false;

				ArrowColor = FColor::Yellow;
			}
			else
			{
				int32 ColorIndex = BoneNameArray.Find(Track.BoneName);
				if (ColorIndex != INDEX_NONE)
				{
					ArrowColor = ColorArray[ColorIndex];
				}
				else
				{
					ArrowColor = FColor::White;
				}
			}


			bool bFirstPosition = true;

			if (ArrowInterval > 0)
			{
				for (int32 Index = 0; Index < Track.ComponentSpaceAnimationTrack.Num(); Index++)
				{
					if (Index % ArrowInterval == 0 || Index == Track.ComponentSpaceAnimationTrack.Num() - 1)
					{
						const FTransform& Key = Track.ComponentSpaceAnimationTrack[Index];

						if (bFirstPosition == false)
						{
							FVector ArrowDirection = CharacterMeshRotation.RotateVector(Key.GetLocation() - FirstRootPosition) - PrevPosition;
							FRotator ArrowRotation = ArrowDirection.ToOrientationRotator();

							UArrowComponent* ArrowComponent = NewObject<UArrowComponent>(this);
							ArrowComponent->ArrowColor = ArrowColor;

							ArrowComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
							ArrowComponent->OnComponentCreated();


							ArrowComponent->SetRelativeLocationAndRotation(PrevPosition + TriggerOffset, ArrowRotation);
							ArrowComponent->SetRelativeScale3D(FVector(ArrowDirection.Size() * 0.01f, 0.2f, 0.2f));


							ArrowComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

							ArrowComponent->RegisterComponent();

							ArrowArray.Add(ArrowComponent);
						}

						PrevPosition = CharacterMeshRotation.RotateVector(Key.GetLocation() - FirstRootPosition);
						bFirstPosition = false;
					}
				}
			}
			else
			{
				if (Track.ComponentSpaceAnimationTrack.Num() >= 2)
				{
					const FTransform& FirstKey = Track.ComponentSpaceAnimationTrack[0];
					PrevPosition = CharacterMeshRotation.RotateVector(FirstKey.GetLocation() - FirstRootPosition);

					const FTransform& LastKey = Track.ComponentSpaceAnimationTrack.Last();

					FVector ArrowDirection = CharacterMeshRotation.RotateVector(LastKey.GetLocation() - FirstRootPosition) - PrevPosition;
					FRotator ArrowRotation = ArrowDirection.ToOrientationRotator();

					UArrowComponent* ArrowComponent = NewObject<UArrowComponent>(this);
					ArrowComponent->ArrowColor = ArrowColor;

					ArrowComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
					ArrowComponent->OnComponentCreated();


					ArrowComponent->SetRelativeLocationAndRotation(PrevPosition + TriggerOffset, ArrowRotation);
					ArrowComponent->SetRelativeScale3D(FVector(ArrowDirection.Size() * 0.01f, 0.2f, 0.2f));


					ArrowComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

					ArrowComponent->RegisterComponent();

					ArrowArray.Add(ArrowComponent);
				}
			}
		}
	}

	


	TotalRootMotion = FVector::ZeroVector;
	TotalRootRotation = FRotator::ZeroRotator;

	int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();//AnimSequence->GetRawNumberOfFrames();

	if (AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() == NumKeys)
	{
		TotalRootMotion = AnimationData[0].ComponentSpaceAnimationTrack[NumKeys - 1].GetLocation() - AnimationData[0].ComponentSpaceAnimationTrack[0].GetLocation();
		TotalRootMotion = CharacterMeshRotation.RotateVector(TotalRootMotion);

		TotalRootRotation = AnimationData[0].ComponentSpaceAnimationTrack[NumKeys - 1].Rotator() - AnimationData[0].ComponentSpaceAnimationTrack[0].Rotator();
	}

#endif
}

void ARootMotionGuide::UpdateSnapshot()
{
#if WITH_EDITOR

	for (USkinnedMeshComponent* SkinnedMeshComponent : SnapshotArray)
	{
		if (IsValid(SkinnedMeshComponent))
		{
			SkinnedMeshComponent->DestroyComponent();
		}
	}

	SnapshotArray.Empty();

	if (bShowSnapshot == false)
	{
		return;
	}

	if (IsValid(AnimSequence) == false)
	{
		//UE_LOG(LogClass, Log, TEXT("No AnimSequence"));
		return;
	}

	if (AnimSequence != AnimSequenceForAnimationData)
	{
		UE_LOG(LogClass, Log, TEXT("Invalid AnimationData"));
		return;
	}

	if (IsValid(AnimSequence->GetSkeleton()->GetPreviewMesh()) == false)
	{
		UE_LOG(LogClass, Log, TEXT("No PreviewMesh"));
		return;
	}

	USkeletalMesh* PreviewMesh = AnimSequence->GetSkeleton()->GetPreviewMesh();

	int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();//AnimSequence->GetRawNumberOfFrames();
	//float interval = (NumKeys > 1) ? (AnimSequence->SequenceLength / (NumKeys - 1)) : MINIMUM_ANIMATION_LENGTH;
	double interval = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

	TArray<float> SnapshotTimeArray;
	TArray<FTransform> SnapshotTransformArray;

	if (NumKeys > 0)
	{
		bool bUseSnapshotFrameArray = false;

		if (SnapshotFrameArray.Num() > 0)
		{
			for (int32 i = 0; i < SnapshotFrameArray.Num(); i++)
			{
				int frame = SnapshotFrameArray[i];
				if (frame >= 0 && frame < NumKeys)
				{
					bUseSnapshotFrameArray = true;
					break;
				}
			}
		}


		if (bUseSnapshotFrameArray)
		{
			SnapshotTimeArray.Empty(SnapshotFrameArray.Num());
			SnapshotTransformArray.Empty(SnapshotTimeArray.Num());

			for (int32 Key = 0; Key < NumKeys; Key++)
			{
				if (SnapshotFrameArray.Contains(Key))
				{
					SnapshotTimeArray.Add(Key * interval);

					if (AnimSequence->bEnableRootMotion)
					{
						if (AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() > Key)
						{
							SnapshotTransformArray.Add(AnimationData[0].ComponentSpaceAnimationTrack[Key]);
						}
						else
						{
							SnapshotTransformArray.Add(FTransform::Identity);
						}
					}
					else
					{
						SnapshotTransformArray.Add(FTransform::Identity);
					}

				}
			}

			if (bShowSubTriggerSnapshot && SubTriggerCount > 0)
			{
				FVector Offset = CharacterMeshRotation.UnrotateVector(SubTriggerOffset);

				float MinTime = FLT_MAX;
				float MaxTime = 0.0f;

				int FirstFrameIndex = INDEX_NONE;
				int LastFrameIndex = INDEX_NONE;

				for (int32 i = 0; i < SnapshotTimeArray.Num(); i++)
				{
					float Time = SnapshotTimeArray[i];
					if (Time < MinTime)
					{
						MinTime = Time;
						FirstFrameIndex = i;
					}

					if (Time > MaxTime)
					{
						MaxTime = Time;
						LastFrameIndex = i;
					}
				}

				if (FirstFrameIndex == LastFrameIndex)
				{
					LastFrameIndex = INDEX_NONE;
				}

				if (FirstFrameIndex != INDEX_NONE)
				{
					float FirstTime = SnapshotTimeArray[FirstFrameIndex];
					FTransform FirstTransform = SnapshotTransformArray[FirstFrameIndex];

					for (int i = 1; i <= SubTriggerCount; i++)
					{
						SnapshotTimeArray.Add(FirstTime);
						FirstTransform.AddToTranslation(Offset);
						SnapshotTransformArray.Add(FirstTransform);
					}
				}

				if (LastFrameIndex != INDEX_NONE)
				{
					float LastTime = SnapshotTimeArray[LastFrameIndex];
					FTransform LastTransform = SnapshotTransformArray[LastFrameIndex];

					for (int i = 1; i <= SubTriggerCount; i++)
					{
						SnapshotTimeArray.Add(LastTime);
						LastTransform.AddToTranslation(Offset);
						SnapshotTransformArray.Add(LastTransform);
					}
				}
			}
		}
		else
		{
			if (SnapshotInterval > 0)
			{
				SnapshotTimeArray.Empty((NumKeys / SnapshotInterval) + 2);
				SnapshotTransformArray.Empty(SnapshotTimeArray.Num());

				for (int32 Key = 0; Key < NumKeys; Key++)
				{
					if (Key % SnapshotInterval == 0 || Key == NumKeys - 1)
					{
						SnapshotTimeArray.Add(Key * interval);

						if (AnimSequence->bEnableRootMotion)
						{
							if (AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() > Key)
							{
								SnapshotTransformArray.Add(AnimationData[0].ComponentSpaceAnimationTrack[Key]);
							}
							else
							{
								SnapshotTransformArray.Add(FTransform::Identity);
							}
						}
						else
						{
							SnapshotTransformArray.Add(FTransform::Identity);
						}

					}
				}
			}
			else
			{
				SnapshotTimeArray.Empty(2);

				SnapshotTimeArray.Add(0.0f);
				SnapshotTimeArray.Add((NumKeys - 1) * interval);

				SnapshotTransformArray.Empty(SnapshotTimeArray.Num());

				if (AnimSequence->bEnableRootMotion)
				{
					if (AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() == NumKeys)
					{
						SnapshotTransformArray.Add(AnimationData[0].ComponentSpaceAnimationTrack[0]);
						SnapshotTransformArray.Add(AnimationData[0].ComponentSpaceAnimationTrack[NumKeys - 1]);
					}
					else
					{
						SnapshotTransformArray.Add(FTransform::Identity);
						SnapshotTransformArray.Add(FTransform::Identity);
					}
				}
				else
				{
					SnapshotTransformArray.Add(FTransform::Identity);
					SnapshotTransformArray.Add(FTransform::Identity);
				}

			}

			if (bShowSubTriggerSnapshot && SubTriggerCount > 0)
			{
				float FirstTime = SnapshotTimeArray[0];
				float LastTime = SnapshotTimeArray.Last(0);

				FTransform FirstTransform = SnapshotTransformArray[0];
				FTransform LastTransform = SnapshotTransformArray.Last(0);

				FVector Offset = CharacterMeshRotation.UnrotateVector(SubTriggerOffset);

				for (int i = 1; i <= SubTriggerCount; i++)
				{
					SnapshotTimeArray.Add(FirstTime);
					SnapshotTimeArray.Add(LastTime);

					FirstTransform.AddToTranslation(Offset);
					LastTransform.AddToTranslation(Offset);

					SnapshotTransformArray.Add(FirstTransform);
					SnapshotTransformArray.Add(LastTransform);
				}
			}
		}



		
	}

	FVector FirstRootPosition = FVector::ZeroVector;
	if (AnimSequence->bEnableRootMotion)
	{
		if (AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() > 0)
		{
			FirstRootPosition = AnimationData[0].ComponentSpaceAnimationTrack[0].GetLocation();
		}
	}
	

	for (int SnapshotIndex = 0; SnapshotIndex < SnapshotTimeArray.Num(); SnapshotIndex++)
	{
		float SnapshotTime = SnapshotTimeArray[SnapshotIndex];

		USkeletalMeshComponent* SkeletalMeshComponent = NewObject<USkeletalMeshComponent>(this);

		SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);

		FSingleAnimationPlayData SingleAnimationPlayData;
		SingleAnimationPlayData.AnimToPlay = AnimSequence;
		SingleAnimationPlayData.bSavedLooping = false;
		SingleAnimationPlayData.bSavedPlaying = false;
		SingleAnimationPlayData.SavedPosition = SnapshotTime;

		SkeletalMeshComponent->AnimationData = SingleAnimationPlayData;


		SkeletalMeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SkeletalMeshComponent->OnComponentCreated();


		FTransform SkeletalMeshTransform = FTransform::Identity;
		SkeletalMeshTransform = SnapshotTransformArray[SnapshotIndex] * FTransform(CharacterMeshRotation.Quaternion(), -FirstRootPosition);
		
		SkeletalMeshTransform.AddToTranslation(TriggerOffset);
		SkeletalMeshComponent->SetRelativeTransform(SkeletalMeshTransform);

		SkeletalMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		SkeletalMeshComponent->RegisterComponent();
		SkeletalMeshComponent->SetHiddenInGame(true);
		SkeletalMeshComponent->SetSkeletalMesh(PreviewMesh);

		SnapshotArray.Add(SkeletalMeshComponent);
	}

#endif
}

void ARootMotionGuide::UpdateRootMotionScale()
{

#if WITH_EDITOR


	if (bEnableAdjustRootMotionScale == false)
	{
		return;
	}

	if (AdjustRootMotionScaleBeginTime >= AdjustRootMotionScaleEndTime)
	{
		return;
	}

	AdjustRootMotionScaleBlendInTime = FMath::Min(AdjustRootMotionScaleBlendInTime, (AdjustRootMotionScaleEndTime - AdjustRootMotionScaleBeginTime));
	AdjustRootMotionScaleBlendOutTime = FMath::Min(AdjustRootMotionScaleBlendOutTime, (AdjustRootMotionScaleEndTime - AdjustRootMotionScaleBeginTime) - AdjustRootMotionScaleBlendInTime);

	float AdjustBlendInTime = AdjustRootMotionScaleBeginTime + AdjustRootMotionScaleBlendInTime;
	float AdjustBlendOutTime = AdjustRootMotionScaleEndTime - AdjustRootMotionScaleBlendOutTime;

	
	//FTransform BeginTransform = AnimSequence->ExtractRootMotion(0.0f, AdjustRootMotionScaleBeginTime, false);
	//FTransform BlendInTransform = AnimSequence->ExtractRootMotion(0.0f, AdjustBlendInTime, false);
	//FTransform BlendOutTransform = AnimSequence->ExtractRootMotion(0.0f, AdjustBlendOutTime, false);
	//FTransform EndTransform = AnimSequence->ExtractRootMotion(0.0f, AdjustRootMotionScaleEndTime, false);

	FTransform BeginTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(AdjustRootMotionScaleBeginTime), false));
	FTransform BlendInTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(AdjustBlendInTime), false));
	FTransform BlendOutTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(AdjustBlendOutTime), false));
	FTransform EndTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(AdjustRootMotionScaleEndTime), false));

	AdjustRootMotionScaleBeginLocation = BeginTransform.GetLocation();
	AdjustRootMotionScaleBlendInLocation = BlendInTransform.GetLocation();
	AdjustRootMotionScaleBlendOutLocation = BlendOutTransform.GetLocation();
	AdjustRootMotionScaleEndLocation = EndTransform.GetLocation();

	int32 NumKeys = AnimSequence->GetDataModel()->GetNumberOfKeys();//AnimSequence->GetRawNumberOfFrames();
	//float interval = (NumKeys > 1) ? (AnimSequence->SequenceLength / (NumKeys - 1)) : MINIMUM_ANIMATION_LENGTH;
	double interval = AnimSequence->GetDataModel()->GetFrameRate().AsInterval();

	if (AnimSequence->bEnableRootMotion && AnimationData.Num() > 0 && AnimationData[0].ComponentSpaceAnimationTrack.Num() == NumKeys)
	{
		TArray<FVector> RootBoneLocationArray;
		RootBoneLocationArray.Empty(NumKeys);
		RootBoneLocationArray.Add(FVector::ZeroVector);

		FVector FirstRootBoneLocation = AnimationData[0].ComponentSpaceAnimationTrack[0].GetLocation();

		for (int32 Key = 1; Key < NumKeys; Key++)
		{
			float PrevMontageTime = (Key - 1) * interval;
			float CurrentMontageTime = Key * interval;

			FVector RootBoneLocation = AnimationData[0].ComponentSpaceAnimationTrack[Key].GetLocation() - FirstRootBoneLocation;

			if (CurrentMontageTime > AdjustRootMotionScaleBeginTime /*&& PrevMontageTime < AdjustRootMotionScaleEndTime*/)
			{
				if (CurrentMontageTime < AdjustBlendInTime)
				{
					//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
					FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

					FVector Move = FVector::ZeroVector;
					Move += BeginTransform.GetLocation();

					if (AdjustRootMotionScaleBlendInTime > 0.0f)
					{
						float Alpha = FMath::Clamp((CurrentMontageTime - AdjustRootMotionScaleBeginTime) / AdjustRootMotionScaleBlendInTime, 0.0f, 1.0f);

						FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, Alpha * 0.5f);

						FVector Delta = (CurrentTransform.GetLocation() - BeginTransform.GetLocation()) * Scale;
						Move += Delta;
					}

					RootBoneLocation = Move;
				}
				else if (CurrentMontageTime < AdjustBlendOutTime)
				{
					//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
					FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

					FVector Move = FVector::ZeroVector;
					Move += BeginTransform.GetLocation();

					if (AdjustRootMotionScaleBlendInTime > 0.0f)
					{
						FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

						FVector Delta = (BlendInTransform.GetLocation() - BeginTransform.GetLocation()) * Scale;
						Move += Delta;
					}

					Move += (CurrentTransform.GetLocation() - BlendInTransform.GetLocation()) * AdjustRootMotionScale;

					RootBoneLocation = Move;
				}
				else if (CurrentMontageTime <= AdjustRootMotionScaleEndTime)
				{
					//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
					FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

					FVector Move = FVector::ZeroVector;
					Move += BeginTransform.GetLocation();

					if (AdjustRootMotionScaleBlendInTime > 0.0f)
					{
						FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

						FVector Delta = (BlendInTransform.GetLocation() - BeginTransform.GetLocation()) * Scale;
						Move += Delta;
					}

					Move += (BlendOutTransform.GetLocation() - BlendInTransform.GetLocation()) * AdjustRootMotionScale;

					if (AdjustRootMotionScaleBlendOutTime > 0.0f)
					{
						float Alpha = FMath::Clamp((AdjustRootMotionScaleEndTime - CurrentMontageTime) / AdjustRootMotionScaleBlendOutTime, 0.0f, 1.0f);

						FVector Scale = FMath::Lerp(AdjustRootMotionScale, FVector::OneVector, (1.0f - Alpha) * 0.5f);

						FVector Delta = (CurrentTransform.GetLocation() - BlendOutTransform.GetLocation()) * Scale;
						Move += Delta;
					}

					RootBoneLocation = Move;
				}
				else
				{
					//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
					FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

					FVector Move = FVector::ZeroVector;
					Move += BeginTransform.GetLocation();

					if (AdjustRootMotionScaleBlendInTime > 0.0f)
					{
						FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

						FVector Delta = (BlendInTransform.GetLocation() - BeginTransform.GetLocation()) * Scale;
						Move += Delta;
					}

					Move += (BlendOutTransform.GetLocation() - BlendInTransform.GetLocation()) * AdjustRootMotionScale;

					if (AdjustRootMotionScaleBlendOutTime > 0.0f)
					{
						FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

						FVector Delta = (EndTransform.GetLocation() - BlendOutTransform.GetLocation()) * Scale;
						Move += Delta;
					}

					Move += (CurrentTransform.GetLocation() - EndTransform.GetLocation());

					RootBoneLocation = Move;
				}
			}

			RootBoneLocationArray.Add(RootBoneLocation);
		}

		for (int32 Key = 1; Key < NumKeys; Key++)
		{
			FVector AdjustMove = RootBoneLocationArray[Key] - AnimationData[0].ComponentSpaceAnimationTrack[Key].GetLocation();

			for (FRootMotionGuideAnimationTrack& Track : AnimationData)
			{
				FTransform Transform = Track.ComponentSpaceAnimationTrack[Key];
				Transform.SetLocation(Transform.GetLocation() + AdjustMove);
				Track.ComponentSpaceAnimationTrack[Key] = Transform;
			}
		}
	}

#endif

}

#undef LOCTEXT_NAMESPACE