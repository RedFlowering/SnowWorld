// Copyright 2017 Lee Ju Sik

#include "RootMotionMovement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

// Sets default values for this component's properties
URootMotionMovement::URootMotionMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URootMotionMovement::BeginPlay()
{
	Super::BeginPlay();

	// ...
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character))
	{
		DefaultMeshTransform = Character->GetMesh()->GetRelativeTransform();

		bOrientRotationToMovementOrig = Character->GetCharacterMovement()->bOrientRotationToMovement;
		bIgnoreClientMovementErrorChecksAndCorrectionOrig = Character->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection;
		NetworkSmoothingModeOrig = Character->GetCharacterMovement()->NetworkSmoothingMode;
		PrimaryComponentTick.AddPrerequisite(Character->GetMovementComponent(), Character->GetMovementComponent()->PrimaryComponentTick);
	}
}

void URootMotionMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (WaitTimeForNextRootMotion > 0.0f)
	{
		WaitTimeForNextRootMotion -= DeltaTime;
	}

	if (IsValid(CurrentRootMotionGuide) && IsValid(AnimSequence))
	{
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (IsValid(Character) && Character->GetLocalRole() == ROLE_SimulatedProxy)
		{
			UpdateRootMotionInternal(Character, DeltaTime);
		}
	}
}

void URootMotionMovement::BeginRootMotion(bool& bResult, ARootMotionGuide*& RootMotionGuide)
{
	if (WaitTimeForNextRootMotion > 0.0f)
	{
		bResult = false;
		return;
	}

	if (IsValid(CurrentRootMotionGuide) && CurrentMontageTime < CurrentRootMotionGuide->NonInterruptibleTime)
	{
		bResult = false;

		if (IsValid(CurrentOverlappedRootMotionGuide) && CurrentOverlappedRootMotionGuide->TriggerType == ERootMotionGuideTriggerType::AutoPlay)
		{
			DelayedAutoPlayRootMotionGuideArray.AddUnique(CurrentOverlappedRootMotionGuide);
		}

		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character) == false || IsValid(CurrentOverlappedRootMotionGuide) == false || CurrentOverlappedRootMotionGuide->CanPlayRootMotionMontage(Character, this) == false)
	{
		bResult = false;

		if (IsValid(CurrentOverlappedRootMotionGuide) && CurrentOverlappedRootMotionGuide->TriggerType == ERootMotionGuideTriggerType::AutoPlay)
		{
			DelayedAutoPlayRootMotionGuideArray.AddUnique(CurrentOverlappedRootMotionGuide);
		}
		return;
	}

	if (CurrentRootMotionGuide == CurrentOverlappedRootMotionGuide)
	{
		bResult = false;
		return;
	}


	DelayedAutoPlayRootMotionGuideArray.Remove(CurrentOverlappedRootMotionGuide);

	CurrentRootMotionGuide = CurrentOverlappedRootMotionGuide;
	CurrentRootMotionGuide->OnBeginRootMotion(Character);


	FTransform WorldMeshTransform = Character->GetMesh()->GetComponentTransform();
	Character->SetActorLocationAndRotation(CurrentRootMotionGuide->GetRootMotionLocation(), CurrentRootMotionGuide->GetRootMotionRotation());
	Character->GetMesh()->SetWorldTransform(WorldMeshTransform);
	MeshTransformAtRootMotionStart = Character->GetMesh()->GetRelativeTransform();


	AnimSequence = CurrentRootMotionGuide->AnimSequence;
	AnimSequenceTime = CurrentRootMotionGuide->AnimSequence->GetPlayLength();
	RootMotionBlendTime = CurrentRootMotionGuide->RootMotionBlendTime;

	CurrentMontageTime = 0.0f;

	CharacterMeshRotation.SetRotation(CurrentRootMotionGuide->CharacterMeshRotation.Quaternion());

	RootMotionNextMovementMode = CurrentRootMotionGuide->GetRootMotionNextMovementMode();

	//TargetTransform.SetLocation(CurrentRootMotionGuide->GetRootMotionLocation());
	//TargetTransform.SetRotation(CurrentRootMotionGuide->GetRootMotionRotation().Quaternion());
	TargetTransform.SetIdentity();

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;

	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Custom);
	Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	bResult = true;
	RootMotionGuide = CurrentRootMotionGuide;
}

void URootMotionMovement::EndRootMotion()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character) == false)
	{
		return;
	}

	FRotator CharacterRotation = Character->GetActorRotation();
	CharacterRotation.Roll = 0.0f;
	CharacterRotation.Pitch = 0.0f;
	Character->SetActorRotation(CharacterRotation);


	Character->GetCharacterMovement()->bOrientRotationToMovement = bOrientRotationToMovementOrig;
	Character->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = bIgnoreClientMovementErrorChecksAndCorrectionOrig;
	Character->GetCharacterMovement()->NetworkSmoothingMode = NetworkSmoothingModeOrig;


	Character->GetCharacterMovement()->SetMovementMode(RootMotionNextMovementMode);

	if (IsValid(CurrentRootMotionGuide) && CurrentRootMotionGuide->NonInterruptibleTime > CurrentMontageTime)
	{
		WaitTimeForNextRootMotion = CurrentRootMotionGuide->NonInterruptibleTime - CurrentMontageTime;
	}
	else
	{
		WaitTimeForNextRootMotion = 0.0f;
	}

	CurrentRootMotionGuide = nullptr;
}

void URootMotionMovement::BeginRootMotionNetwork(FString RootMotionGuideName, bool & bResult, ARootMotionGuide *& RootMotionGuide)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character) == false)
	{
		bResult = false;
		RootMotionGuide = nullptr;
		return;
	}

	CurrentRootMotionGuide = nullptr;

	for (ARootMotionGuide* TestRootMotionGuide : OverlappedRootMotionGuideArray)
	{
		if (TestRootMotionGuide->GetName().Equals(RootMotionGuideName))
		{
			CurrentRootMotionGuide = TestRootMotionGuide;
			break;
		}
	}

	if (IsValid(CurrentRootMotionGuide) == false)
	{
		TArray<AActor*> RootMotionGuideArray;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARootMotionGuide::StaticClass(), RootMotionGuideArray);
		for (AActor* TestActor : RootMotionGuideArray)
		{
			if (TestActor->GetName().Equals(RootMotionGuideName))
			{
				CurrentRootMotionGuide = Cast<ARootMotionGuide>(TestActor);
				break;
			}
		}
	}


	if (IsValid(CurrentRootMotionGuide) == false)
	{
		bResult = false;
		RootMotionGuide = nullptr;
		return;
	}


	CurrentRootMotionGuide->OnBeginRootMotion(Character);


	FTransform WorldMeshTransform = Character->GetMesh()->GetComponentTransform();
	Character->SetActorLocationAndRotation(CurrentRootMotionGuide->GetRootMotionLocation(), CurrentRootMotionGuide->GetRootMotionRotation());
	Character->GetMesh()->SetWorldTransform(WorldMeshTransform);
	MeshTransformAtRootMotionStart = Character->GetMesh()->GetRelativeTransform();

	//
	AnimSequence = CurrentRootMotionGuide->AnimSequence;
	AnimSequenceTime = CurrentRootMotionGuide->AnimSequence->GetPlayLength();
	RootMotionBlendTime = CurrentRootMotionGuide->RootMotionBlendTime;

	CurrentMontageTime = 0.0f;

	CharacterMeshRotation.SetRotation(CurrentRootMotionGuide->CharacterMeshRotation.Quaternion());

	RootMotionNextMovementMode = CurrentRootMotionGuide->GetRootMotionNextMovementMode();

	//TargetTransform.SetLocation(CurrentRootMotionGuide->GetRootMotionLocation());
	//TargetTransform.SetRotation(CurrentRootMotionGuide->GetRootMotionRotation().Quaternion());
	TargetTransform.SetIdentity();


	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
	Character->GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;


	Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Custom);
	Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	bResult = true;
	RootMotionGuide = CurrentRootMotionGuide;
}


void URootMotionMovement::UpdateRootMotion(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character) == false)
	{
		return;
	}

	if (IsValid(AnimSequence) == false)
	{
		return;
	}


	if (Character->HasAuthority() || Character->IsLocallyControlled())
	{
		UpdateRootMotionInternal(Character, DeltaTime);
	}
}

bool URootMotionMovement::IsPlayingRootMotion() const
{
	return CurrentRootMotionGuide != nullptr;
}

bool URootMotionMovement::IsPlayingWith(ARootMotionGuide * RootMotionGuide) const
{
	return CurrentRootMotionGuide != nullptr && CurrentRootMotionGuide == RootMotionGuide;
}

bool URootMotionMovement::IsPlayingWithCurrentOverlappedRootMotionGuide() const
{
	return IsPlayingWith(CurrentOverlappedRootMotionGuide);
}

void URootMotionMovement::BeginOverlap(ARootMotionGuide * RootMotionGuide, bool& bIsAutoTrigger)
{
	if (IsValid(RootMotionGuide) == false)
	{
		bIsAutoTrigger = false;
		return;
	}

	CurrentOverlappedRootMotionGuide = RootMotionGuide;
	OverlappedRootMotionGuideArray.AddUnique(RootMotionGuide);


	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character))
	{
		RootMotionGuide->BeginOverlap(Character);
	}

	bIsAutoTrigger = RootMotionGuide->TriggerType == ERootMotionGuideTriggerType::AutoPlay;

	return;
}

void URootMotionMovement::EndOverlap(ARootMotionGuide * RootMotionGuide)
{
	if (IsValid(RootMotionGuide) == false)
	{
		return;
	}

	OverlappedRootMotionGuideArray.Remove(RootMotionGuide);
	DelayedAutoPlayRootMotionGuideArray.Remove(RootMotionGuide);

	if (CurrentOverlappedRootMotionGuide == RootMotionGuide)
	{
		if (OverlappedRootMotionGuideArray.Num() > 0)
		{
			CurrentOverlappedRootMotionGuide = OverlappedRootMotionGuideArray.Last();
		}
		else
		{
			CurrentOverlappedRootMotionGuide = nullptr;
		}
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character))
	{
		RootMotionGuide->EndOverlap(Character);
	}

	return;
}

void URootMotionMovement::SetCurrentOverlappedRootMotionGuide(ARootMotionGuide * RootMotionGuide)
{
	if (IsValid(RootMotionGuide) == false)
	{
		return;
	}

	if (OverlappedRootMotionGuideArray.Contains(RootMotionGuide))
	{
		CurrentOverlappedRootMotionGuide = RootMotionGuide;
	}
}

bool URootMotionMovement::HasOverlappedRootMotionGuide() const
{
	return OverlappedRootMotionGuideArray.Num() > 0;
}

void URootMotionMovement::FindRootMotionGuide(FVector MovementDirection, ERootMotionGuideTriggerType TriggerType, float MinimumDotValue, bool & bFound, ARootMotionGuide *& RootMotionGuide) const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character) == false)
	{
		bFound = false;
		RootMotionGuide = nullptr;
		return;
	}


	if (TriggerType == ERootMotionGuideTriggerType::ActionEvent)
	{
		if (MovementDirection.Size() <= 0.0f)
		{
			MovementDirection = Character->GetActorForwardVector();
		}
	}
	else if (TriggerType == ERootMotionGuideTriggerType::AxisEvent)
	{
		float InputScale = MovementDirection.Size();

		//UE_LOG(LogClass, Log, TEXT("Input Scale %f"), InputScale);
		if (InputScale < MinimumDotValue)
		{
			bFound = false;
			RootMotionGuide = nullptr;
			return;
		}
	}

	

	MovementDirection.Normalize();



	float MaxDotValue = MinimumDotValue;
	float MinDistance = FLT_MAX;
	ARootMotionGuide* Found = nullptr;
	for (int i = 0; i < OverlappedRootMotionGuideArray.Num(); i++)
	{
		ARootMotionGuide* OverlappedGuide = OverlappedRootMotionGuideArray[i];

		if (OverlappedGuide->TriggerType == TriggerType && OverlappedGuide->CanPlayRootMotionMontage(Character, this))
		{
			float DotValue = FVector::DotProduct(OverlappedGuide->GetMovementDirectionInWorld(), MovementDirection);

			if (DotValue < OverlappedGuide->MinimumDotValue)
			{
				continue;
			}

			if (FMath::IsNearlyEqual(DotValue, MaxDotValue))
			{
				float Distance = (OverlappedGuide->GetRootMotionLocation() - Character->GetActorLocation()).Size();

				if (Distance < MinDistance)
				{
					MaxDotValue = DotValue;
					MinDistance = Distance;
					Found = OverlappedGuide;
				}
			}
			else if (DotValue > MaxDotValue)
			{
				MaxDotValue = DotValue;
				MinDistance = (OverlappedGuide->GetRootMotionLocation() - Character->GetActorLocation()).Size();
				Found = OverlappedGuide;
			}
		}
	}

	bFound = Found != nullptr;
	RootMotionGuide = Found;
}

void URootMotionMovement::FindRootMotionGuideToAutoPlay(bool & bFound, ARootMotionGuide *& RootMotionGuide) const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character) == false)
	{
		bFound = false;
		RootMotionGuide = nullptr;
		return;
	}

	ARootMotionGuide* Found = nullptr;
	for (int i = 0; i < DelayedAutoPlayRootMotionGuideArray.Num(); i++)
	{
		ARootMotionGuide* DelayedAutoPlayGuide = DelayedAutoPlayRootMotionGuideArray[i];
		if (DelayedAutoPlayGuide->TriggerType == ERootMotionGuideTriggerType::AutoPlay && DelayedAutoPlayGuide->CanPlayRootMotionMontage(Character, this))
		{
			Found = DelayedAutoPlayGuide;
			break;
		}
	}

	bFound = Found != nullptr;
	RootMotionGuide = Found;
}

void URootMotionMovement::UpdateRootMotionInternal(ACharacter * Character, float DeltaTime)
{
	if (IsValid(CurrentRootMotionGuide) == false)
	{
		return;
	}

	FTransform RootMotionGuideTransform = FTransform::Identity;
	RootMotionGuideTransform.SetLocation(CurrentRootMotionGuide->GetRootMotionLocation());
	RootMotionGuideTransform.SetRotation(CurrentRootMotionGuide->GetRootMotionRotation().Quaternion());


	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

	float PrevMontageTime = FMath::Clamp(CurrentMontageTime, 0.0f, AnimSequenceTime);
	CurrentMontageTime += DeltaTime;

	//FTransform DeltaTransform = AnimSequence->ExtractRootMotion(PrevMontageTime, DeltaTime, false);
	FTransform DeltaTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(static_cast<double>(PrevMontageTime), true, FDeltaTimeRecord(DeltaTime), false));

	if (CurrentRootMotionGuide->bEnableAdjustRootMotionScale)
	{
		if (CurrentMontageTime > CurrentRootMotionGuide->AdjustRootMotionScaleBeginTime && PrevMontageTime < CurrentRootMotionGuide->AdjustRootMotionScaleEndTime)
		{
			const FVector& AdjustRootMotionScale = CurrentRootMotionGuide->AdjustRootMotionScale;
			const float AdjustRootMotionScaleBeginTime = CurrentRootMotionGuide->AdjustRootMotionScaleBeginTime;
			const float AdjustRootMotionScaleEndTime = CurrentRootMotionGuide->AdjustRootMotionScaleEndTime;
			const float AdjustRootMotionScaleBlendInTime = CurrentRootMotionGuide->AdjustRootMotionScaleBlendInTime;
			const float AdjustRootMotionScaleBlendOutTime = CurrentRootMotionGuide->AdjustRootMotionScaleBlendOutTime;

			const float AdjustBlendInTime = AdjustRootMotionScaleBeginTime + AdjustRootMotionScaleBlendInTime;
			const float AdjustBlendOutTime = AdjustRootMotionScaleEndTime - AdjustRootMotionScaleBlendOutTime;

			const FVector& BeginLocation = CurrentRootMotionGuide->AdjustRootMotionScaleBeginLocation;
			const FVector& BlendInLocation = CurrentRootMotionGuide->AdjustRootMotionScaleBlendInLocation;
			const FVector& BlendOutLocation = CurrentRootMotionGuide->AdjustRootMotionScaleBlendOutLocation;
			const FVector& EndLocation = CurrentRootMotionGuide->AdjustRootMotionScaleEndLocation;

			if (CurrentMontageTime < AdjustBlendInTime)
			{
				//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
				FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

				FVector Move = FVector::ZeroVector;
				Move += BeginLocation;

				if (AdjustRootMotionScaleBlendInTime > 0.0f)
				{
					float Alpha = FMath::Clamp((CurrentMontageTime - AdjustRootMotionScaleBeginTime) / AdjustRootMotionScaleBlendInTime, 0.0f, 1.0f);

					FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, Alpha * 0.5f);

					FVector Delta = (CurrentTransform.GetLocation() - BeginLocation) * Scale;
					Move += Delta;
				}

				DeltaTransform = CurrentTransform;
				DeltaTransform.SetLocation(Move);
				TargetTransform.SetIdentity();
			}
			else if (CurrentMontageTime < AdjustBlendOutTime)
			{
				//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
				FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

				FVector Move = FVector::ZeroVector;
				Move += BeginLocation;

				if (AdjustRootMotionScaleBlendInTime > 0.0f)
				{
					FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

					FVector Delta = (BlendInLocation - BeginLocation) * Scale;
					Move += Delta;
				}

				Move += (CurrentTransform.GetLocation() - BlendInLocation) * AdjustRootMotionScale;

				DeltaTransform = CurrentTransform;
				DeltaTransform.SetLocation(Move);
				TargetTransform.SetIdentity();
			}
			else if (CurrentMontageTime <= AdjustRootMotionScaleEndTime)
			{
				//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
				FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

				FVector Move = FVector::ZeroVector;
				Move += BeginLocation;

				if (AdjustRootMotionScaleBlendInTime > 0.0f)
				{
					FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

					FVector Delta = (BlendInLocation - BeginLocation) * Scale;
					Move += Delta;
				}

				Move += (BlendOutLocation - BlendInLocation) * AdjustRootMotionScale;

				if (AdjustRootMotionScaleBlendOutTime > 0.0f)
				{
					float Alpha = FMath::Clamp((AdjustRootMotionScaleEndTime - CurrentMontageTime) / AdjustRootMotionScaleBlendOutTime, 0.0f, 1.0f);

					FVector Scale = FMath::Lerp(AdjustRootMotionScale, FVector::OneVector, (1.0f - Alpha) * 0.5f);

					FVector Delta = (CurrentTransform.GetLocation() - BlendOutLocation) * Scale;
					Move += Delta;
				}

				DeltaTransform = CurrentTransform;
				DeltaTransform.SetLocation(Move);
				TargetTransform.SetIdentity();
			}
			else
			{
				//FTransform CurrentTransform = AnimSequence->ExtractRootMotion(0.0f, CurrentMontageTime, false);
				FTransform CurrentTransform = AnimSequence->ExtractRootMotion(FAnimExtractContext(0.0, true, FDeltaTimeRecord(CurrentMontageTime), false));

				FVector Move = FVector::ZeroVector;
				Move += BeginLocation;

				if (AdjustRootMotionScaleBlendInTime > 0.0f)
				{
					FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

					FVector Delta = (BlendInLocation - BeginLocation) * Scale;
					Move += Delta;
				}

				Move += (BlendOutLocation - BlendInLocation) * AdjustRootMotionScale;

				if (AdjustRootMotionScaleBlendOutTime > 0.0f)
				{
					FVector Scale = FMath::Lerp(FVector::OneVector, AdjustRootMotionScale, 0.5f);

					FVector Delta = (EndLocation - BlendOutLocation) * Scale;
					Move += Delta;
				}

				Move += (CurrentTransform.GetLocation() - EndLocation);

				DeltaTransform = CurrentTransform;
				DeltaTransform.SetLocation(Move);
				TargetTransform.SetIdentity();
			}
		}
	}


	DeltaTransform.SetLocation(CharacterMeshRotation.TransformVector(DeltaTransform.GetLocation()));

	TargetTransform = DeltaTransform * TargetTransform;

	//FVector MoveDelta = TargetTransform.GetLocation() - Character->GetActorLocation();
	//CharacterMovement->MoveUpdatedComponent(MoveDelta, TargetTransform.GetRotation(), false);

	FTransform FinalTransform = TargetTransform * RootMotionGuideTransform;
	FVector MoveDelta = FinalTransform.GetLocation() - Character->GetActorLocation();
	CharacterMovement->MoveUpdatedComponent(MoveDelta, FinalTransform.GetRotation(), false);


	float BlendTime = FMath::Min(AnimSequenceTime, RootMotionBlendTime);
	float Alpha = (BlendTime > 0.0f) ? FMath::Clamp(CurrentMontageTime / BlendTime, 0.0f, 1.0f) : 1.0f;

	FTransform MeshRelativeTransform = FTransform::Identity;
	MeshRelativeTransform.Blend(MeshTransformAtRootMotionStart, DefaultMeshTransform, Alpha);
	Character->GetMesh()->SetRelativeTransform(MeshRelativeTransform);

	CharacterMovement->bNetworkSmoothingComplete = true;
}
