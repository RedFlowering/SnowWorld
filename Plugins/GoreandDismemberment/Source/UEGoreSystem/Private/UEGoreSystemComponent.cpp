// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#include "UEGoreSystemComponent.h"
#include "UEGoreSystemAnimInstance.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DecalActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GoreSkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

#include "GroomComponent.h"
#include "GroomBindingAsset.h"

#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UEGoreSystem.h"

// Sets default values for this component's properties
UUEGoreSystemComponent::UUEGoreSystemComponent(const FObjectInitializer& ObjInit)
	:Super(ObjInit)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

// Replicated variables
void UUEGoreSystemComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate libms health values
	//DOREPLIFETIME(UUEGoreSystemComponent, variable...);
}

bool UUEGoreSystemComponent::GetBoneHealth(FName BoneName, float& Health) const
{
	if (!IsValid(GetOwner()))
		return false;
	if (!GetOwner()->HasAuthority())
		return false;

	if (GoreSettings.BodyMap.Contains(BoneName))
	{
		if (HealthValues.BoneName.Contains(BoneName)) {
			Health = HealthValues.Health[HealthValues.BoneName.IndexOfByKey(BoneName)];
		}
		else {
			if (GoreSettings.HealthMap.Contains(BoneName)) {
				Health = GoreSettings.HealthMap.FindRef(BoneName);
			}
			else {
				Health = GoreSettings.GlobalLimbsHealth;
			}
		}
		return true;
	}
	return false;
}

bool UUEGoreSystemComponent::GetDetachedMeshFromBoneName(FName BoneName, USkeletalMeshComponent*& MeshComponent) const
{
	if (!GoreSettings.BodyMap.Find(BoneName))
		return false;

	MeshComponent = GoreSettings.BodyMap.Find(BoneName)->DetachedMesh;
	return IsValid(MeshComponent);
}

void UUEGoreSystemComponent::DestroyAllAttachedGoreFXs()
{
	DestroyAllAttachedGoreFXs_Multi();
}

void UUEGoreSystemComponent::DestroyAllAttachedGoreFXs_Multi_Implementation()
{
	const uint32 FXNum = AttachedGoreFXs.Num();
	
	for (uint32 i = 0; i < FXNum; i++) {
		AttachedGoreFXs[i]->DestroyComponent();
	}
}

// Called when the game starts
void UUEGoreSystemComponent::BeginPlay()
{
	Super::BeginPlay();	

	// Try to initialize on begin play
	Init(nullptr); 
}

// Called every frame
void UUEGoreSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// Initializer function
void UUEGoreSystemComponent::Init(USkeletalMeshComponent* NewRootMesh)
{
	// Setup the target mesh
	if (NewRootMesh) {
		MeshRoot = NewRootMesh;
	}
	else {
		MeshRoot = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	}
}

void UUEGoreSystemComponent::SpawnDecalAt(const TSubclassOf<ADecalActor> DecalActorClass,const FTransform DecalTransform, const float InLifespan)
{
	//Set spawn parameters to No Fail (Always Spawn)
	FActorSpawnParameters params;
	params.bNoFail = true;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//Spawn Decal Actor
	ADecalActor* DecalAct = GetWorld()->SpawnActor<ADecalActor>(DecalActorClass, DecalTransform, params);

	if (DecalAct) {
		//If Actor is valid, set new random rotation
		DecalAct->AddActorLocalRotation(FQuat(FRotator(0.0f, 0.0f, UKismetMathLibrary::RandomFloatInRange(0.0f, 360.0f))));
		DecalAct->SetLifeSpan(InLifespan);
		DecalsList.Add(DecalAct);
	}
}

// Hit functions
void UUEGoreSystemComponent::HitBones(const TArray<FUEGoreSystemHit> Bones)
{
	if (!MeshRoot)
		return;

	HitBones_Serv(Bones);
}

void UUEGoreSystemComponent::HitBones_Serv_Implementation(const TArray<FUEGoreSystemHit>& Bones)
{
	HitBones_Multi(Bones);
}

void UUEGoreSystemComponent::HitBones_Multi_Implementation(const TArray<FUEGoreSystemHit>& Bones)
{
	Int_HitBones(Bones);
}

void UUEGoreSystemComponent::Int_HitBones(const TArray<FUEGoreSystemHit> Bones)
{
	ParallelFor(Bones.Num(), [&](int32 Idx)
		{
			FUEGoreSystemEffects EffectsToUse = FUEGoreSystemEffects();			
			USoundCue* SoundToUse = nullptr;

			// Get correct VFXs and SFXs from data structure
			if (GoreSettings.BodyMap.Contains(Bones[Idx].BoneName)) { //!MeshRoot->IsBoneHiddenByName(Bones[Idx].BoneName
				FUEGoreSystemStruct* CStru = GoreSettings.BodyMap.Find(Bones[Idx].BoneName);

				if (CStru->OverrideEffects)	{
					EffectsToUse = CStru->Effects;
				}
				else{
					EffectsToUse = GoreSettings.GlobalEffects;
				}

				if (CStru->OverrideSounds) {
					SoundToUse = CStru->Sounds.Hit;
				}
				else {
					SoundToUse = GoreSettings.GlobalSounds.Hit;
				}
			}
			else{
				EffectsToUse = GoreSettings.GlobalEffects;
				SoundToUse = GoreSettings.GlobalSounds.Hit;
			}

			// Spawn Niagara hit effect
			if (EffectsToUse.Hit) {
				UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetOwner(), EffectsToUse.Hit, Bones[Idx].Location, Bones[Idx].Normal.Rotation(), FVector(1.0f), true, true);
			}

			if (EffectsToUse.HitLegacy) {
				UParticleSystemComponent* CParticleSyst = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EffectsToUse.HitLegacy, FTransform(Bones[Idx].Normal.Rotation(), Bones[Idx].Location, FVector(1.0f)));

				if (CParticleSyst) {
					FScriptDelegate Delegate;
					Delegate.BindUFunction(this, "SpawnDecalOnParticleHit");
					CParticleSyst->OnParticleCollide.Add(Delegate);
				}
			}

			// Spawn sound hit effect
			if(SoundToUse)
				UGameplayStatics::SpawnSoundAtLocation(GetOwner(), Cast<USoundBase>(SoundToUse), Bones[Idx].Location, Bones[Idx].Normal.Rotation());

			// Spawn decal
			if (EffectsToUse.Decal) {
				FTransform DecalTransf = FTransform();
				FHitResult FeetResult;
				FCollisionQueryParams QParams;
				QParams.AddIgnoredActor(GetOwner()); // Ignore self and other components
				FCollisionObjectQueryParams NewColParam;

				for (int32 iC = 0.0f; iC < GoreSettings.DecalCollisionChannels.Num(); iC++) {
					NewColParam.AddObjectTypesToQuery(GoreSettings.DecalCollisionChannels[iC]);
				}

				const FVector FirstTraceEnd = (Bones[Idx].Location + (Bones[Idx].Normal * 500.0f)) * FVector(1.0f, 1.0f, 0.01f);

				//DrawDebugLine(GetWorld(), Bones[Idx].Location, FirstTraceEnd, FColor::Emerald, false, 10.0f);

				// Trace from Hit Location using Normal direction
				if (GetWorld()->LineTraceSingleByObjectType(FeetResult, Bones[Idx].Location, FirstTraceEnd, NewColParam, QParams))
				{
					//Store decal transform
					DecalTransf = FTransform(
						FQuat(UKismetMathLibrary::MakeRotFromZ(FeetResult.Normal)), //Rotation from hit normal
						FeetResult.Location, //Given location
						FVector(UKismetMathLibrary::RandomFloatInRange(1.0f, 1.5f))); //Random scale	

					FTimerHandle DecalHandle;
					FTimerDelegate TimerDel;
					TimerDel.BindUFunction(this, FName(TEXT("SpawnDecalAt")), EffectsToUse.Decal, DecalTransf, EffectsToUse.DecalLifespan);
					GetOwner()->GetWorldTimerManager().SetTimer(DecalHandle, TimerDel, FeetResult.Distance / 500.0f, false);
				}
			}

		});
}

void UUEGoreSystemComponent::HitBone(const FUEGoreSystemHit BoneHit)
{
	TArray<FUEGoreSystemHit> SingleBone;
	SingleBone.Add(BoneHit);

	HitBones(SingleBone);
}

// Destroy functions
bool UUEGoreSystemComponent::DestroyBones(const TArray<FUEGoreSystemHit> Bones, const float Force)
{
	TArray<FName> FilteredBoneName;

	// Filter invalids
	for (int32 i = 0; i < Bones.Num(); i++) {
			if (GoreSettings.BodyMap.Contains(Bones[i].BoneName))
				FilteredBoneName.Add(Bones[i].BoneName);
		}

	if (FilteredBoneName.Num() == 0)
		return false;

	DestroyBones_Serv(Bones, Force);
	return true;
}

void UUEGoreSystemComponent::DestroyBones_Serv_Implementation(const TArray<FUEGoreSystemHit>& Bones, const float Force)
{
	DestroyBones_Multi(Bones, Force);
}

void UUEGoreSystemComponent::DestroyBones_Multi_Implementation(const TArray<FUEGoreSystemHit>& Bones, const float Force)
{
	Int_DestroyBones(Bones, Force);
}

void UUEGoreSystemComponent::Int_DestroyBones(const TArray<FUEGoreSystemHit> Bones, const float Force)
{
	UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("[DestroyBones] with %i bone input(s)"), Bones.Num());

	FCriticalSection Mutex;
	// ParallelFor(Bones.Num(), [&](int32 Idx)
	for (int32 Idx = 0; Idx < Bones.Num(); Idx++) {

		if (DestroyedBones.Contains(Bones[Idx].BoneName))
			continue;		

		UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Destroying %s"), *Bones[Idx].BoneName.ToString());

		const FUEGoreSystemStruct* SelectStruct = GoreSettings.BodyMap.Find(Bones[Idx].BoneName);
		const FUEGoreSystemEffects SelectEffects = SelectStruct->OverrideEffects ? SelectStruct->Effects : GoreSettings.GlobalEffects;
		const FUEGoreSystemSounds SelectSounds = SelectStruct->OverrideEffects ? SelectStruct->Sounds : GoreSettings.GlobalSounds;
		FVector BloodDirection = UKismetMathLibrary::GetDirectionUnitVector(MeshRoot->GetSocketLocation(MeshRoot->GetParentBone(Bones[Idx].BoneName)), MeshRoot->GetSocketLocation(Bones[Idx].BoneName));
		UNiagaraComponent* NiagaraComp = nullptr;
		UParticleSystemComponent* CParticleSyst = nullptr;

		// Find correct skeletal mesh component
		USkeletalMeshComponent* CSKComp = nullptr;

		bool bBoneHasDestroyedChildren = false;
		if (bEnableStretchingFix) {
			for (auto& DBone : DestroyedBones) {
				if (MeshRoot->BoneIsChildOf(Bones[Idx].BoneName, DBone)) {
					bBoneHasDestroyedChildren = true;
					break;
				}
			}
		}

		if (!MeshRoot->IsBoneHidden(MeshRoot->GetBoneIndex(Bones[Idx].BoneName)) && !bBoneHasDestroyedChildren) {
			CSKComp = MeshRoot;		
		}
		else {
			// Start with nearest parent
			FName ParentName = Bones[Idx].BoneName;

			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Damaged component is a detached limb"));

			for (int32 i = 0; i < MeshRoot->GetNumBones(); i++) {
				FUEGoreSystemStruct* CGoreStruct = GoreSettings.BodyMap.Find(MeshRoot->GetParentBone(ParentName));

				if (CGoreStruct)
					if (CGoreStruct->DetachedMesh)
						if (!CGoreStruct->DetachedMesh->IsBoneHiddenByName(Bones[Idx].BoneName) && !bBoneHasDestroyedChildren) {
							CSKComp = CGoreStruct->DetachedMesh;

							UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Damaged component is %s"), *CGoreStruct->DetachedMesh->GetFName().ToString());

							break;
						}

				// If "break" doesn't happen, get next parent bone
				ParentName = MeshRoot->GetParentBone(ParentName);
			}
		}

		// Make sure Skeletal Mesh Component has been found
		if (!CSKComp || CSKComp->IsBeingDestroyed()) {
			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Warning, TEXT("Damaged component isn't valid for this GoreSystem Component"));

			continue;
		}		

		if (Cast<UGoreSkeletalMeshComponent>(CSKComp) || bBoneHasDestroyedChildren) {
			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Warning, TEXT("Deep dismemberment is not supported with auto dismemberment enabled"));

			continue;
		}

		// Spawn spill effects
		// Looking for custom sockets (suffix BoneName + "_spill")
		FName SocketToUse = Bones[Idx].BoneName;
		if (MeshRoot->DoesSocketExist(FName(SocketToUse.ToString().Append("_spill")))) {
			SocketToUse = FName(SocketToUse.ToString().Append("_spill"));
			BloodDirection = CSKComp->GetSocketRotation(SocketToUse).Vector();
		}

		if (SelectEffects.Spill && CSKComp) {
			NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(SelectEffects.Spill, CSKComp, CSKComp->GetParentBone(CSKComp->GetSocketBoneName(SocketToUse)), CSKComp->GetSocketLocation(SocketToUse), BloodDirection.Rotation(), EAttachLocation::KeepWorldPosition, true);

			if (NiagaraComp) {
				NiagaraComp->SetWorldScale3D(CSKComp->GetComponentScale());
				AttachedGoreFXs.Add(NiagaraComp);
			}
		}

		if (SelectEffects.SpillLegacy) {
			CParticleSyst = UGameplayStatics::SpawnEmitterAttached(SelectEffects.SpillLegacy, CSKComp, CSKComp->GetParentBone(CSKComp->GetSocketBoneName(SocketToUse)), CSKComp->GetSocketLocation(SocketToUse), BloodDirection.Rotation(), EAttachLocation::KeepWorldPosition, true);

			if (CParticleSyst) {
				CParticleSyst->SetWorldScale3D(CSKComp->GetComponentScale());
				FScriptDelegate Delegate;
				Delegate.BindUFunction(this, "SpawnDecalOnParticleHit");
				CParticleSyst->OnParticleCollide.Add(Delegate);
				AttachedGoreFXs.Add(CParticleSyst);
			}
		}		

		// Make sure the target socket exists
		if (!CSKComp->DoesSocketExist(Bones[Idx].BoneName)) {
			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Warning, TEXT("Socket '%s' doesn't exist or it has been destroyed already!"), *Bones[Idx].BoneName.ToString());

			continue;
		}

		// Save a reference to all the components attached to the destroyed bone or "children"
		const TArray<USceneComponent*> AllComp = CSKComp->GetAttachChildren();
		FUEGoreSystemStruct NewStruct = GoreSettings.BodyMap.FindRef(Bones[Idx].BoneName);

		// Spawn decal
		if (SelectEffects.Decal) {

			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Decal class is valid and will be spawned if trace has success"));

			FTransform DecalTransf = FTransform();
			FHitResult FeetResult;

			FCollisionQueryParams QParams;
			QParams.AddIgnoredActor(GetOwner());
			FCollisionObjectQueryParams NewColParam;

			for (int32 iC = 0.0f; iC < GoreSettings.DecalCollisionChannels.Num(); iC++) {
				NewColParam.AddObjectTypesToQuery(GoreSettings.DecalCollisionChannels[iC]);
			}

			const FVector FirstTraceEnd = (CSKComp->GetSocketLocation(Bones[Idx].BoneName) + (BloodDirection * 500.0f)) * FVector(1.0f, 1.0f, 0.025f);
			//DrawDebugLine(GetWorld(), CSKComp->GetSocketLocation(Bones[Idx].BoneName), FirstTraceEnd, FColor::Red, false, 10.0f);

			if (GetWorld()->LineTraceSingleByObjectType(FeetResult, CSKComp->GetSocketLocation(Bones[Idx].BoneName), FirstTraceEnd, NewColParam, QParams)) {
				//Store decal transform
				DecalTransf = FTransform(
					FQuat(UKismetMathLibrary::MakeRotFromZ(FeetResult.Normal)), //Rotation from hit normal
					FeetResult.Location, //Given location
					FVector(UKismetMathLibrary::RandomFloatInRange(1.0f, 1.5f))); //Random scale				
			}
			else {
				if (GetWorld()->LineTraceSingleByObjectType(FeetResult, CSKComp->GetComponentLocation() + (FVector(0.0f, 0.0f, 50.0f)), CSKComp->GetComponentLocation() - (FVector(0.0f, 0.0f, 200.0f) * GetOwner()->GetActorScale()), NewColParam, QParams)) {
					//Store decal transform
					DecalTransf = FTransform(
						FQuat(UKismetMathLibrary::MakeRotFromZ(FeetResult.Normal)), //Rotation from hit normal
						FeetResult.Location, //Given location
						FVector(UKismetMathLibrary::RandomFloatInRange(1.0f, 1.5f))); //Random scale	
				}
			}

			if (FeetResult.bBlockingHit) {
				UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Trace for decal found a spot"));

				FTimerHandle DecalHandle;
				FTimerDelegate TimerDel;
				TimerDel.BindUFunction(this, FName(TEXT("SpawnDecalAt")), SelectEffects.Decal, DecalTransf, SelectEffects.DecalLifespan);
				GetOwner()->GetWorldTimerManager().SetTimer(DecalHandle, TimerDel, FeetResult.Distance / 500.0f, false);
			}
		}

		// Spawn break sound
		if(SelectSounds.Break)
			UGameplayStatics::SpawnSoundAtLocation(GetOwner(), Cast<USoundBase>(SelectSounds.Break), CSKComp->GetSocketLocation(Bones[Idx].BoneName));

		// Save snapshot pose for later
		FPoseSnapshot SnapPose;
		if (CSKComp->IsVisible()) {
			CSKComp->SnapshotPose(SnapPose);

			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Snapshot pose saved from %s"), *CSKComp->GetFName().ToString());
		}
		else {
			MeshRoot->SnapshotPose(SnapPose);

			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Snapshot pose saved from RootMesh"));
		}

		if (bEnableAutoDismemberment && !NewStruct.Mesh) {
			NewStruct.DetachedMesh = NewObject<USkeletalMeshComponent>(CSKComp->GetOwner(), UGoreSkeletalMeshComponent::StaticClass(), MakeUniqueObjectName(CSKComp->GetOwner(), UGoreSkeletalMeshComponent::StaticClass(), TEXT("AutoLimbSkeletalComponent_")));

			USkeletalMesh* NewMesh = nullptr;
			if (VisualMeshComponent) {
				NewMesh = VisualMeshComponent->GetSkeletalMeshAsset();
			}
			else {
				if (LimbsMesh) {
					NewMesh = LimbsMesh;
				}
				else {
					NewMesh = CSKComp->GetSkeletalMeshAsset();
				}
			}

			NewStruct.DetachedMesh->SetSkeletalMesh(NewMesh);
		}
		else {
			if (NewStruct.Mesh)	{
				NewStruct.DetachedMesh = NewObject<USkeletalMeshComponent>(
					CSKComp->GetOwner(),
					USkeletalMeshComponent::StaticClass(),
					MakeUniqueObjectName(CSKComp->GetOwner(), USkeletalMeshComponent::StaticClass(), TEXT("LimbSkeletalComponent_")));

				//Set new skeletal mesh parameters before registration
				
				NewStruct.DetachedMesh->SetSkeletalMesh(NewStruct.Mesh, false);				
			}
		}				

		if (NewStruct.DetachedMesh)	{
			NewStruct.DetachedMesh->bDisableClothSimulation = true;
			NewStruct.DetachedMesh->SetVisibility(false);
			NewStruct.DetachedMesh->SetSimulatePhysics(false);
			NewStruct.DetachedMesh->SetWorldTransform(CSKComp->GetComponentTransform());			
			NewStruct.DetachedMesh->RegisterComponent(); //DuplicateObject(CSKComp, CSKComp->GetOuter(), MakeUniqueObjectName(CSKComp->GetOuter(), UPhysicsAsset::StaticClass(), TEXT("ClonedModel")));		
			

			const USkeletalMeshComponent* SKMatRef = VisualMeshComponent ? VisualMeshComponent : CSKComp;
			for (int32 i = 0; i < NewStruct.DetachedMesh->GetMaterials().Num(); i++)
			{
				if (NewStruct.MaterialsOverride.Num() > i) {
					NewStruct.DetachedMesh->SetMaterial(i, NewStruct.MaterialsOverride[i]);
				}
				else {
					if(SKMatRef->GetMaterials().Num() > i)
						if (SKMatRef->GetMaterials()[i]) {
							NewStruct.DetachedMesh->SetMaterial(i, SKMatRef->GetMaterials()[i]);
						}
				}				
			}
		}
		
		// Sync hidden sockets
		TArray<FName> BonesToHide;
		TArray<FName> BoneKeys;

		GoreSettings.BodyMap.GetKeys(BoneKeys);

		for (int32 iB = 0; iB < BoneKeys.Num(); iB++) {
			if (CSKComp->BoneIsChildOf(BoneKeys[iB], Bones[Idx].BoneName) && BoneKeys[iB] != Bones[Idx].BoneName && GoreSettings.BodyMap.Find(BoneKeys[iB])->DetachedMesh) {
				BonesToHide.Add(BoneKeys[iB]);
			}
		}

		// Make sure new component is spawned correctly
		if (NewStruct.DetachedMesh && GoreSettings.GlobalAnimInstance) {
			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Detach Mesh pointer is valid, proceeding to spawn the new skeletal mesh"));

			SpawnedComponents.Add(NewStruct.DetachedMesh);

			if (!NewStruct.Mesh || !NewStruct.DetachedMesh->GetPhysicsAsset()) {
				UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Cloning PhAt"));
				if (DefaultPhysicsAsset || CSKComp->GetPhysicsAsset()) {
					NewStruct.DetachedMesh->SetPhysicsAsset(ClonePhysicsAsset(DefaultPhysicsAsset ? DefaultPhysicsAsset : CSKComp->GetPhysicsAsset(), Bones[Idx].BoneName), true);
				}
				else {
					UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Warning, TEXT("No Physics Asset applied!"));
				}
			}

			NewStruct.DetachedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("New Component registered and collisions disabled"));

			// Transfer attached components
			for (int32 i = 0; i < AllComp.Num(); i++) {				
				if (CSKComp->BoneIsChildOf(CSKComp->GetSocketBoneName(AllComp[i]->GetAttachSocketName()), CSKComp->GetParentBone(Bones[Idx].BoneName))) {
					UGroomComponent* GroomComp = Cast<UGroomComponent>(AllComp[i]);
					if (GroomComp) {
						GroomComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
						GroomComp->AttachmentName = Bones[Idx].BoneName.ToString();
						GroomComp->PhysicsAsset = NewStruct.DetachedMesh->GetPhysicsAsset();
						GroomComp->AttachToComponent(NewStruct.DetachedMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					}
					else if (AllComp[i] && NiagaraComp != AllComp[i] && AllComp[i] != CParticleSyst) {
						if (CSKComp->BoneIsChildOf(CSKComp->GetSocketBoneName(AllComp[i]->GetAttachSocketName()), Bones[Idx].BoneName) || (CSKComp->GetSocketBoneName(AllComp[i]->GetAttachSocketName()) == Bones[Idx].BoneName)) {
							AllComp[i]->AttachToComponent(NewStruct.DetachedMesh, FAttachmentTransformRules::KeepRelativeTransform, AllComp[i]->GetAttachSocketName());
						}
					}
				}
			};	

			// Set correct animation instance
			NewStruct.DetachedMesh->SetAnimInstanceClass(GoreSettings.GlobalAnimInstance);
			UUEGoreSystemAnimInstance* CAnimInst = Cast<UUEGoreSystemAnimInstance>(NewStruct.DetachedMesh->GetAnimInstance());

			// Apply correct snapshot pose and animation variables
			if (CAnimInst) {
				CAnimInst->Detached_Snapshot = SnapPose;
				CAnimInst->ImpulseOnDetach = Bones[Idx].Normal.GetSafeNormal() * Force * -1.0f;
				CAnimInst->Rootbone = Bones[Idx].BoneName;
				CAnimInst->CollisionProfile = GoreSettings.LimbsCollisionProfile;
				CAnimInst->BonesToHide = BonesToHide;
				CAnimInst->StretchFixEnabled = bEnableStretchingFix;
			}

			// Save reference to new component
			GoreSettings.BodyMap.Emplace(Bones[Idx].BoneName, NewStruct);
		}
		else {
			if (bEnableDebugLogging)
				UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("No Detach Mesh pointer"));

			for (int32 i = 0; i < AllComp.Num(); i++) {
				if (AllComp[i] && NiagaraComp != AllComp[i] && (CSKComp->BoneIsChildOf(CSKComp->GetSocketBoneName(AllComp[i]->GetAttachSocketName()), CSKComp->GetParentBone(Bones[Idx].BoneName)))) {
					if (CSKComp->BoneIsChildOf(CSKComp->GetSocketBoneName(AllComp[i]->GetAttachSocketName()), Bones[Idx].BoneName) || CSKComp->GetSocketBoneName(AllComp[i]->GetAttachSocketName()) == Bones[Idx].BoneName)
						AllComp[i]->DestroyComponent();
				}
			};
		}

		// Deco spawning
		const TMap<FName, FUEGoreSystemDecoArray> CDecoMeshes = GoreSettings.BodyMap.Find(Bones[Idx].BoneName)->DecoMesh;
		TArray<FName> DecoSockets;
		CDecoMeshes.GetKeys(DecoSockets);

		ParallelFor(DecoSockets.Num(), [&](int32 ISockets) {

			if (!CSKComp->DoesSocketExist(DecoSockets[ISockets]))
				return;

			const FUEGoreSystemDecoArray CDecoArray = CDecoMeshes.FindRef(DecoSockets[ISockets]);

			for (int32 IDecos = 0; IDecos < CDecoArray.Decos.Num(); IDecos++) {
				// Make sure the asset is a static mesh
				UStaticMesh* SMRef = Cast<UStaticMesh>(CDecoArray.Decos[IDecos]);

				// Static Mesh deco spawn
				if (SMRef) {

					UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Spawning Static Mesh Decoration: %s"), *SMRef->GetFName().ToString());

					UStaticMeshComponent* NewSMComp = NewObject<UStaticMeshComponent>(
						CSKComp->GetOwner(),
						UStaticMeshComponent::StaticClass(),
						MakeUniqueObjectName(CSKComp->GetOwner(), UStaticMeshComponent::StaticClass(), TEXT("DecoStaticComponent_")));

					NewSMComp->RegisterComponent();

					// Setup deco component settings
					if (NewSMComp) {
						NewSMComp->SetStaticMesh(SMRef);
						NewSMComp->SetCollisionProfileName(GoreSettings.DecoCollisionProfile == FName() ? FName(TEXT("NoCollision")) : GoreSettings.DecoCollisionProfile); // Set "NoCollision" profile if "DecoCollisionProfile" is null
						NewSMComp->AttachToComponent(CSKComp, FAttachmentTransformRules::SnapToTargetIncludingScale, DecoSockets[ISockets]);
						NewSMComp->SetWorldTransform(CSKComp->GetSocketTransform(DecoSockets[ISockets]));
					}
				}
				else {
					// Make sure the asset is a skeletal mesh
					USkeletalMesh* SKRef = Cast<USkeletalMesh>(CDecoArray.Decos[IDecos]);

					// Skeletal Mesh deco spawn
					if (SKRef) {

						UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Spawning Skeletal Mesh Decoration: %s"), *SKRef->GetFName().ToString());

						USkeletalMeshComponent* NewSKComp = NewObject<USkeletalMeshComponent>(
							CSKComp->GetOwner(),
							USkeletalMeshComponent::StaticClass(),
							MakeUniqueObjectName(CSKComp->GetOwner(), USkeletalMeshComponent::StaticClass(), TEXT("DecoSkeletalComponent_")));

						NewSKComp->RegisterComponent();

						// Setup deco component settings
						if (NewSKComp) {
							NewSKComp->SetSkeletalMesh(SKRef);
							NewSKComp->SetCollisionProfileName(GoreSettings.DecoCollisionProfile == FName() ? FName(TEXT("NoCollision")) : GoreSettings.DecoCollisionProfile); // Set "NoCollision" profile if "DecoCollisionProfile" is null
							NewSKComp->AttachToComponent(CSKComp, FAttachmentTransformRules::SnapToTargetIncludingScale, DecoSockets[ISockets]);
							NewSKComp->SetWorldTransform(CSKComp->GetSocketTransform(DecoSockets[ISockets]));
						}
					}
				}
			}
			});

		DestroyedBones.Add(Bones[Idx].BoneName);

		if (!bControlRigProcedural) {
			CSKComp->HideBoneByName(Bones[Idx].BoneName, EPhysBodyOp::PBO_Term);
		}

		/*if (!bEnableAutoDismemberment) {
			Int_BoneHide(Bones[Idx].BoneName, CSKComp);
		}
		else {
			GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UUEGoreSystemComponent::Int_BoneHide, Bones[Idx].BoneName, CSKComp));
		}*/

		UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Limb hidden"));

		On_BoneBroken.Broadcast(Bones[Idx].BoneName, NewStruct.DetachedMesh);
	};
}

void UUEGoreSystemComponent::Int_BoneHide(const FName Bone, USkeletalMeshComponent* MeshComp)
{
	DestroyedBones.Add(Bone);

	if (!bControlRigProcedural) {
		MeshComp->HideBoneByName(Bone, EPhysBodyOp::PBO_Term);
	}
}

bool UUEGoreSystemComponent::DestroyBone(const FUEGoreSystemHit InBone, const float InForce)
{
	TArray<FUEGoreSystemHit> SingleBone;
	SingleBone.Add(InBone);

	return DestroyBones(SingleBone, InForce);
}

// Clone Physics Asset utility
UPhysicsAsset* UUEGoreSystemComponent::ClonePhysicsAsset(const UPhysicsAsset* PhysicsAssetToClone, const FName RemoveBefore)
{

	//Duplicate Physics Asset
	UPhysicsAsset* PhAt = DuplicateObject(PhysicsAssetToClone, GetOwner(), MakeUniqueObjectName(PhysicsAssetToClone->GetOuter(), UPhysicsAsset::StaticClass(), TEXT("ClonedPhAt_")));

	if (!PhAt)
	{
		UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Can't clone physics asset, return nullptr"));

		return nullptr;
	}

	if (RemoveBefore != FName())
	{
		UE_CLOG(bEnableDebugLogging, LogUEGoreSystem, Log, TEXT("Removing physics bodies before %s"), *RemoveBefore.ToString());

		TArray<int32> InstBelow;

		//Gather bodies before
		PhAt->GetBodyIndicesBelow(InstBelow, RemoveBefore, MeshRoot->GetSkeletalMeshAsset(), true);

		for (int32 Idx = 0; Idx < PhAt->SkeletalBodySetups.Num(); Idx++)
		{
			if (!InstBelow.Contains(Idx))
			{
				PhAt ->SkeletalBodySetups[Idx]->AggGeom.EmptyElements();
				PhAt->SkeletalBodySetups[Idx]->bConsiderForBounds = false;
			}
		}		
	}

	//Return new Physics Asset
	return PhAt;
}

//Damage functions
void UUEGoreSystemComponent::DamageBonesMultipleDamages(const FUEGoreSystemHealthValues BonesDamage)
{
	DamageBones_Serv(BonesDamage);
}

void UUEGoreSystemComponent::DamageBonesSingleDamage(const TArray<FName> InBones, const float InDamage)
{
	FUEGoreSystemHealthValues ValueToPass;
	
	ParallelFor(InBones.Num(), [&](int32 Idx) {
		TTuple<FName, float> AddValue;
		AddValue.Key = InBones[Idx];
		AddValue.Value = InDamage;
		ValueToPass.Add(AddValue);
		});	

	DamageBonesMultipleDamages(ValueToPass);
}

void UUEGoreSystemComponent::DamageBones_Serv_Implementation(const FUEGoreSystemHealthValues BonesDamage)
{
	const TArray<FName> CKeys = BonesDamage.BoneName;

	FCriticalSection Mutex;

	// Multiple bones
	ParallelFor(BonesDamage.BoneName.Num(), [&](int32 Idx)	{
		// Find the correct bone inside TMap "BodyMap". If not found, return
		if (!GoreSettings.BodyMap.Contains(CKeys[Idx]))
			return;

		// Initialize total damage using the input value
		float TotalBoneDamage = BonesDamage.Health[BonesDamage.BoneName.IndexOfByKey(CKeys[Idx])];

		// Multiply the damage for its factor if found in the list
		if (GoreSettings.DamageMultiplierMap.Contains(CKeys[Idx]))
			TotalBoneDamage = TotalBoneDamage * GoreSettings.DamageMultiplierMap.FindRef(CKeys[Idx]);

		// Initialize a reference for new values
		TTuple<FName, float> NewHealthValue;
		NewHealthValue.Key = CKeys[Idx];

		// Check if the bone has been damage already (therefore it's stored in the limbs health list)
		if (HealthValues.Contains(NewHealthValue.Key)) {

			// If so, replace the float value and keep the same entry in the array
			HealthValues.GetHealth(NewHealthValue.Key, NewHealthValue.Value);
			NewHealthValue.Value = NewHealthValue.Value - TotalBoneDamage;

			//Mutex.Lock();
			// Replace health value
			HealthValues.SetHealth(NewHealthValue);
			//Mutex.Unlock();
		}
		else {
			// If not, add a new entry to the stored limbs health variable
			// Check if initial limb health has been overridden using the data structure TMap "HealthMap"
			if (GoreSettings.HealthMap.Contains(NewHealthValue.Key)) {
				NewHealthValue.Value = GoreSettings.HealthMap.FindRef(NewHealthValue.Key) - TotalBoneDamage;
			}
			else {
				NewHealthValue.Value = GoreSettings.GlobalLimbsHealth - TotalBoneDamage;
			}

			//Mutex.Lock();
			// Add to health values
			HealthValues.Add(NewHealthValue);
			//Mutex.Unlock();
		}
	});
}

bool UUEGoreSystemComponent::DamageBone(const FName InBoneName, const float InDamage, float& NewBoneHealth, float& DamageApplied)
{
	if (!GoreSettings.BodyMap.Contains(InBoneName))
		return false;

	FUEGoreSystemHealthValues DamageInfo;
	TTuple<FName, float> TDamage;
	TDamage.Key = InBoneName;
	TDamage.Value = InDamage;
	DamageInfo.Add(TDamage);

	float oldbonehealth = 0.0f;
	GetBoneHealth(InBoneName, oldbonehealth);
	DamageBonesMultipleDamages(DamageInfo);
	GetBoneHealth(InBoneName, NewBoneHealth);
	DamageApplied = oldbonehealth - NewBoneHealth;

	return true;
}

bool UUEGoreSystemComponent::ApplyGoreDamage(const float Damage, const bool IsDead, const float DetachmentForce, const FUEGoreSystemHit BoneHit, float& NewBoneHealth, float& DamageApplied)
{
	if (!IsActive()) {
		return false;
	}

	// Return if there is no bone to damage or the damage is less or equal to 0
	if (BoneHit.BoneName == FName() || Damage <= 0.0f)
		return false;

	// Hit event, this will spawn the Niagara, Sound adn Decal hit effect
	HitBone(BoneHit);

	// Damage the target bone and detach it if...
	if (DamageBone(BoneHit.BoneName, Damage, NewBoneHealth, DamageApplied)) {
		if (NewBoneHealth <= 0.0f && (IsDead || GoreSettings.IgnoreIsDead.Contains(BoneHit.BoneName)))  // Rules for detachment (default must be dead or contained in the list (TArray<FName>) "IgnoreIsDead"
			DestroyBone(BoneHit);

		return true;
	}

	// If not damaged return false
	return false;
}

void UUEGoreSystemComponent::RemoveAllSpawnedComponents(const bool KeepDecals)
{
	RemoveAllSpawnedComponents_Server(KeepDecals);
}

void UUEGoreSystemComponent::RemoveAllSpawnedComponents_Server_Implementation(const bool KeepDecals)
{
	RemoveAllSpawnedComponents_Multi(KeepDecals);
}

void UUEGoreSystemComponent::RemoveAllSpawnedComponents_Multi_Implementation(const bool KeepDecals)
{
	TArray<FName> BodyMapKeys = TArray<FName>();
	GoreSettings.BodyMap.GetKeys(BodyMapKeys);

	for (int32 Idx = 0; Idx < BodyMapKeys.Num(); Idx++) {

		USkeletalMeshComponent* TargetMesh = GoreSettings.BodyMap.Find(BodyMapKeys[Idx])->DetachedMesh;

		if (!TargetMesh)
			continue;

		TArray<USceneComponent*> AttachedComps = TargetMesh->GetAttachChildren();
		for (int32 IdxC = 0; IdxC < AttachedComps.Num(); IdxC++) {
			AttachedComps[IdxC]->DestroyComponent();
		}

		FUEGoreSystemStruct StructToEmplace = GoreSettings.BodyMap.FindRef(BodyMapKeys[Idx]);
		StructToEmplace.DetachedMesh = nullptr;
		TargetMesh->DestroyComponent();

		GoreSettings.BodyMap.Emplace(BodyMapKeys[Idx], StructToEmplace);
	}

	SpawnedComponents.Empty();

	if (!KeepDecals) {
		for (int32 Idx = 0; Idx < DecalsList.Num(); Idx++) {
			if (DecalsList[Idx]) {
				DecalsList[Idx]->Destroy();
			}
		};

		DecalsList.Empty();
	}
}

void UUEGoreSystemComponent::SpawnDecalOnParticleHit(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat)
{
	if (GoreSettings.GlobalEffects.Decal)
	{
		//Store decal transform
		const FTransform DecalTransf = FTransform(
			FQuat(UKismetMathLibrary::MakeRotFromZ(Normal)), //Rotation from hit normal
			Location, //Given location
			FVector(1.0f)); //[ADD] Random scale

		SpawnDecalAt(GoreSettings.GlobalEffects.Decal, DecalTransf, GoreSettings.GlobalEffects.DecalLifespan);
	}
}