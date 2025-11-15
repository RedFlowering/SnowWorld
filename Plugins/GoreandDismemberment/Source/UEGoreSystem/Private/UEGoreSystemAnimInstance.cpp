// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.


#include "UEGoreSystemAnimInstance.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "GoreSkeletalMeshComponent.h"
#include "TimerManager.h"

UUEGoreSystemAnimInstance::UUEGoreSystemAnimInstance(const FObjectInitializer& Init)
	:Super(Init)
{
	//Set defaults
	
}

void UUEGoreSystemAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// On update check if the reference Snapshot is valid, if it's valid apply it to the skeletal mesh
	if (Detached_Snapshot.bIsValid)
		if (GetOwningComponent())
			if (!GetOwningComponent()->IsAnySimulatingPhysics() && !PhysTimerHandle.IsValid())
				GetOwningActor()->GetWorldTimerManager().SetTimerForNextTick(this, &UUEGoreSystemAnimInstance::EnablePhys);
}

void UUEGoreSystemAnimInstance::EnablePhys()
{	
	// Setting up the new detached component after using the Snapshot pose to replicate the correct position 
	// This will happen after the Snapshot has been applied and will enable physics/collision/visibility
	// A custom impulse will be applied after physics has been enabled, the direction and amount is set by parameters passed to this class by the GoreSystemComponent
	// This function will have a short delay after the detach happen, this will sync the correct pose	

	UGoreSkeletalMeshComponent* GoreMesh = Cast<UGoreSkeletalMeshComponent>(GetOwningComponent());

	if (GoreMesh) {
		GoreMesh->HideBoneBeforeNameGore(Rootbone, EPhysBodyOp::PBO_Term);

		ParallelFor(BonesToHide.Num(), [&](int32 i) {
			if (!GetOwningComponent()->IsBoneHiddenByName(BonesToHide[i]))
				GoreMesh->HideBoneByNameGore(BonesToHide[i], EPhysBodyOp::PBO_Term, false);
			});

		GoreMesh->NewRootBone = Rootbone;
		GoreMesh->StretchFixEnabled = StretchFixEnabled;
		GoreMesh->DetachImpulse = ImpulseOnDetach;
	}
	else {		
		ParallelFor(BonesToHide.Num(), [&](int32 i) {
			if(!GetOwningComponent()->IsBoneHiddenByName(BonesToHide[i]) && Rootbone != BonesToHide[i])
				GetOwningComponent()->HideBoneByName(BonesToHide[i], EPhysBodyOp::PBO_Term);
			});
	}		

	GetOwningComponent()->SetCollisionProfileName(CollisionProfile == FName() ? FName(TEXT("Ragdoll")) : CollisionProfile, true); // If the "CollisionProfile" variable is empty, use "Ragdoll" instead		

	if (!StretchFixEnabled || !Cast<UGoreSkeletalMeshComponent>(GetOwningComponent())) {
		GetOwningComponent()->SetVisibility(true);
		GetOwningComponent()->SetAllBodiesBelowSimulatePhysics(Rootbone, true, true);
		GetOwningComponent()->AddImpulse(ImpulseOnDetach, Rootbone);
	}
}

