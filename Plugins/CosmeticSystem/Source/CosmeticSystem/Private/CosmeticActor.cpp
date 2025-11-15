// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticActor.h"
#include "CosmeticSystem.h"
#include "GameplayTagsManager.h"
#include "CosmeticBFL.h"
#include "Components/GameFrameworkComponentManager.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/RigidBodyIndexPair.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Runtime/Engine/Classes/PhysicsEngine/SkeletalBodySetup.h"

ACosmeticActor::ACosmeticActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateOptionalDefaultSubobject<UCosmeticSkeletalMeshComponent>(TEXT("RetargetMesh"));
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;				
		Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
		SetRootComponent(Mesh);
	}	
}

USkeletalMeshComponent* ACosmeticActor::GetMergeTargetSkeletalMesh(FName ComponentTag)
{
	if (auto Ele = CreatedMergeTargetMeshes.Find(ComponentTag))
	{
		return *Ele;
	}

	UE_LOG(LogCosmeticSystem, Error, TEXT("Can't find MergeTargetSkeletalMesh [%s]"), *ComponentTag.ToString())

	return nullptr;
}

UCosmeticSkeletalMeshComponent* ACosmeticActor::CreateInstanceSkeletalMesh(FGameplayTag PartTag, FName SocketName, bool bUseMasterPoseComponent, TSubclassOf<UCosmeticSkeletalMeshComponent> OverrideSkeletalMeshComponent)
{
	TSubclassOf<UCosmeticSkeletalMeshComponent> SkeletalMeshCompType = 
				(OverrideSkeletalMeshComponent != nullptr) ? OverrideSkeletalMeshComponent : CosmeticSkeletalMeshComponentType;

	UCosmeticSkeletalMeshComponent* MeshComp = NewObject<UCosmeticSkeletalMeshComponent>(this, SkeletalMeshCompType, PartTag.GetTagName());
	MeshComp->AttachToComponent(GetVisualMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	MeshComp->RegisterComponent();

	if (bUseMasterPoseComponent)
		MeshComp->SetLeaderPoseComponent(GetVisualMesh());
	return MeshComp;
}

UCosmeticGroomComponent* ACosmeticActor::CreateInstanceGroomAsset(FGameplayTag PartTag, FName SocketName, TSubclassOf<UCosmeticGroomComponent> OverrideGroomComponent)
{
	TSubclassOf<UCosmeticGroomComponent> GroomCompType =
			(OverrideGroomComponent != nullptr) ? OverrideGroomComponent : CosmeticGroomComponentType;
	UCosmeticGroomComponent* GroomComp = NewObject<UCosmeticGroomComponent>(this, GroomCompType, PartTag.GetTagName());
	GroomComp->AttachToComponent(GetVisualMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	GroomComp->RegisterComponent();
	return GroomComp;
}

bool ACosmeticActor::IsEnableMerge_Implementation()
{
	return true;
}

bool ACosmeticActor::MergeCosmeticMeshes(FSkeletalMeshMergeParams MergeParams, UPhysicsAsset* DefaultPhysicsAsset, USkeletalMeshComponent* Target)
{	
	USkeletalMesh* BaseMesh = USkeletalMergingLibrary::MergeMeshes(MergeParams);
	if (BaseMesh)
	{	
		Target->SetSkeletalMesh(BaseMesh);
		Target->SetPhysicsAsset(DefaultPhysicsAsset);		
	}

	return true;
}

void ACosmeticActor::ClearCreatedMergeTargetMeshes()
{
	for (auto& Ele : CreatedMergeTargetMeshes)
	{	
		Ele.Value->SetSkeletalMesh(nullptr, false);
	}
}

void ACosmeticActor::OnChangedSkeletalMesh_Implementation()
{

}

void ACosmeticActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ACosmeticActor::BeginPlay()
{
	Super::BeginPlay();

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
}

void ACosmeticActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Mesh = nullptr;
	CreatedMergeTargetMeshes.Empty();

	Super::EndPlay(EndPlayReason);		
}

void ACosmeticActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
		
	int32 Index = 0;
	TArray<UActorComponent*> TempComponents = K2_GetComponentsByClass(UCosmeticSkeletalMeshComponent::StaticClass());
	for (UActorComponent* Elem : TempComponents)
	{
		if (!Elem->ComponentTags.IsEmpty())
		{
			CreatedMergeTargetMeshes.Add(Elem->ComponentTags[0], Cast<UCosmeticSkeletalMeshComponent>(Elem));
		}
	}
}

bool ACosmeticActor::CreateSkeletalBodySetup(UPhysicsAsset* PhysAsset, USkeletalBodySetup* SKBSetup)
{
	int32 BodyIndex = PhysAsset->FindBodyIndex(SKBSetup->BoneName);
	if (BodyIndex == INDEX_NONE)
	{
		USkeletalBodySetup* NewBodySetup = NewObject<USkeletalBodySetup>(PhysAsset, NAME_None, RF_Transactional);
		NewBodySetup->BoneName = SKBSetup->BoneName;
		NewBodySetup->CollisionTraceFlag = SKBSetup->CollisionTraceFlag;
		NewBodySetup->PhysicsType = SKBSetup->PhysicsType;		

		int32 BodySetupIndex = PhysAsset->SkeletalBodySetups.Add(NewBodySetup);
		PhysAsset->SkeletalBodySetups[BodySetupIndex]->AggGeom = SKBSetup->AggGeom;

		PhysAsset->UpdateBodySetupIndexMap();
		PhysAsset->UpdateBoundsBodiesArray();

		for (int i = 0; i < PhysAsset->SkeletalBodySetups.Num(); ++i)
		{
			PhysAsset->DisableCollision(i, BodySetupIndex);
		}
	}

	return false;
}

UPhysicsAsset* ACosmeticActor::MergePhysicsAssets(TArray<UPhysicsAsset*> PhysicsAssets, USkeletalMeshComponent* Target)
{	
    USkeletalMesh* SkeletalMesh = Target->GetSkeletalMeshAsset();
    UPhysicsAsset* MergedPhysicsAsset = NewObject<UPhysicsAsset>(SkeletalMesh, MakeUniqueObjectName(SkeletalMesh, UPhysicsAsset::StaticClass(), TEXT("MergedPhysicsAsset")), RF_Public | RF_Standalone | RF_Transient);

	for (UPhysicsAsset* PhysicsAsset : PhysicsAssets)
	{
		for (USkeletalBodySetup* SKBSetup : PhysicsAsset->SkeletalBodySetups)
		{
			CreateSkeletalBodySetup(MergedPhysicsAsset, SKBSetup);
		}
	}   

    const TArray<FTransform>& LocalPose = SkeletalMesh->GetRefSkeleton().GetRefBonePose();

    TArray<FName> AllBoneNames;
    MergedPhysicsAsset->BodySetupIndexMap.GenerateKeyArray(AllBoneNames);
	
	// create constraints
	for (int32 Index = 0; Index < AllBoneNames.Num(); ++Index)
    {
        const FName BoneName = AllBoneNames[Index];

        const int32 ConstraintIndex = MergedPhysicsAsset->FindConstraintIndex(BoneName);
        if (ConstraintIndex != INDEX_NONE)
        {
            continue;
        }

        const int32 BodyIndex = MergedPhysicsAsset->BodySetupIndexMap[BoneName];
        if (BodyIndex == INDEX_NONE)
        {
            continue;
        }

        const int32 BoneIndex = SkeletalMesh->GetRefSkeleton().FindRawBoneIndex(BoneName);

        int32 ParentBoneIndex = BoneIndex;
        int32 ParentBodyIndex = INDEX_NONE;
        FName ParentBoneName;

        FTransform RelTM = FTransform::Identity;
        do
        {
			// Transform of child from parent is just child ref-pose entry.
            RelTM = RelTM * LocalPose[ParentBoneIndex];
            
			//Travel up the hierarchy to find a parent which has a valid body
            ParentBoneIndex = SkeletalMesh->GetRefSkeleton().GetParentIndex(ParentBoneIndex);
            if (ParentBoneIndex != INDEX_NONE)
            {
                ParentBoneName = SkeletalMesh->GetRefSkeleton().GetBoneName(ParentBoneIndex);
                ParentBodyIndex = MergedPhysicsAsset->FindBodyIndex(ParentBoneName);
            }
            else
            { 
				//no more parents so just stop
                break;
            }

        } while (ParentBodyIndex == INDEX_NONE);

        if (ParentBodyIndex != INDEX_NONE)
        {
			// add constraint between BoneName and ParentBoneName

			// constraintClass must be a subclass of UPhysicsConstraintTemplate

            UPhysicsConstraintTemplate* NewConstraintSetup = NewObject<UPhysicsConstraintTemplate>(MergedPhysicsAsset, NAME_None, RF_Transient);
            const int32 NewConstraintSetupIndex = MergedPhysicsAsset->ConstraintSetup.Add(NewConstraintSetup);

            NewConstraintSetup->DefaultInstance.JointName = BoneName;

            UPhysicsConstraintTemplate* CS = MergedPhysicsAsset->ConstraintSetup[NewConstraintSetupIndex];

			// set angular constraint mode
            CS->DefaultInstance.SetAngularSwing1Motion(EAngularConstraintMotion::ACM_Limited);
            CS->DefaultInstance.SetAngularSwing2Motion(EAngularConstraintMotion::ACM_Limited);
            CS->DefaultInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Limited);

			// Place joint at origin of child
            CS->DefaultInstance.ConstraintBone1 = BoneName;
            CS->DefaultInstance.Pos1 = FVector::ZeroVector;
            CS->DefaultInstance.PriAxis1 = FVector(1, 0, 0);
            CS->DefaultInstance.SecAxis1 = FVector(0, 1, 0);

            CS->DefaultInstance.ConstraintBone2 = ParentBoneName;
            CS->DefaultInstance.Pos2 = RelTM.GetLocation();
            CS->DefaultInstance.PriAxis2 = RelTM.GetUnitAxis(EAxis::X);
            CS->DefaultInstance.SecAxis2 = RelTM.GetUnitAxis(EAxis::Y);

            MergedPhysicsAsset->DisableCollision(BodyIndex, ParentBodyIndex);
        }
    }

	Target->SetSkeletalMesh(SkeletalMesh);
	Target->SetPhysicsAsset(MergedPhysicsAsset);

	// update CollisionDisableTable
	const TArray<FBodyInstance*> Bodies = Target->Bodies;
	const int32 NumBodies = Bodies.Num();
	for (int32 BodyIdx = 0; BodyIdx < NumBodies; ++BodyIdx)
	{
		FBodyInstance* BodyInstance = Bodies[BodyIdx];
		if (BodyInstance && BodyInstance->BodySetup.IsValid())
		{
			FTransform BodyTM = BodyInstance->GetUnrealWorldTransform();

			for (int32 OtherBodyIdx = BodyIdx + 1; OtherBodyIdx < NumBodies; ++OtherBodyIdx)
			{
				FBodyInstance* OtherBodyInstance = Bodies[OtherBodyIdx];
				if (OtherBodyInstance && OtherBodyInstance->BodySetup.IsValid())
				{
					if (BodyInstance->OverlapTestForBody(BodyTM.GetLocation(), BodyTM.GetRotation(), OtherBodyInstance))
					{
						MergedPhysicsAsset->DisableCollision(BodyIdx, OtherBodyIdx);
					}
				}
			}
		}
	}

    return MergedPhysicsAsset;
}

void ACosmeticActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

