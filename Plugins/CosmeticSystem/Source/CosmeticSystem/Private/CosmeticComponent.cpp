// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "CosmeticChildActorComponent.h"
#include "NativeGameplayTags.h"
#include "CosmeticItemID.h"
#include "CosmeticSkeletalMeshComponent.h"
#include "CosmeticSystemSettings.h"
#include "Engine/SkinnedAssetCommon.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "CosmeticSystem.h"
#include "GroomAsset.h"
#include "GroomComponent.h"
#include "SkeletalMergingLibrary.h"
#include "Materials/MaterialInterface.h"
#include "CosmeticMeshStorage.h"
#include "CosmeticBFL.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_CosmeticSystem_BodyType_Tag, "CosmeticSystem.Body");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_CosmeticSystem_FaceType_Tag, "CosmeticSystem.Face");

UCosmeticComponent::UCosmeticComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);	
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	
}

void UCosmeticComponent::BeginPlay()
{
	Super::BeginPlay();	

	ACharacter* Character = GetOwner<ACharacter>();
	if (Character && Character->HasAuthority())
	{
		if (CosmeticData.RetargetMeshPresetID == -1)
		{
			if (InitializedRetargetMeshPresetID != -1)
			{
				SetRetargetMeshPreset(InitializedRetargetMeshPresetID);			
			}
		}
	}

}

void UCosmeticComponent::SetRetargetMeshPreset(FCosmeticItemID RetargetMeshPresetID)
{
	if (CosmeticData.RetargetMeshPresetID != RetargetMeshPresetID)
	{
		if (ACharacter* Character = GetOwner<ACharacter>())
		{
			if (Character->HasAuthority())
			{
				if ((Character->GetNetMode() == ENetMode::NM_Standalone) || (Character->GetNetMode() == ENetMode::NM_ListenServer))
				{
					if (CosmeticData.RetargetMeshPresetID != RetargetMeshPresetID)
					{
						CosmeticData.RetargetMeshPresetID = RetargetMeshPresetID;
						ApplyRetargetMeshPreset(RetargetMeshPresetID);
					}

					CheckCosmeticData(CosmeticData);
					return;
				}

				// DedicatedServer
				ApplyRetargetMeshPreset(RetargetMeshPresetID);
			}
			else if (Character->IsLocallyControlled())
			{
				Server_SetRetargetMeshPreset(RetargetMeshPresetID);
				return;
			}
		}
	}
}

void UCosmeticComponent::Server_SetRetargetMeshPreset_Implementation(FCosmeticItemID RetargetMeshPresetID)
 {
	 SetRetargetMeshPreset(RetargetMeshPresetID);
 }

void UCosmeticComponent::ApplyRetargetMeshPreset(FCosmeticItemID RetargetMeshPresetID)
 {	
	 CosmeticData.RetargetMeshPresetID = RetargetMeshPresetID;
	 FRetargetMeshData RetargetMeshData;
	 if (GetRetargetMeshData(CosmeticData.RetargetMeshPresetID, RetargetMeshData))
	 {
		 if (CosmeticActorInstance)
			 CosmeticActorInstance->ClearCreatedMergeTargetMeshes();

		 ApplyVisualMesh(GetOwner<ACharacter>(), RetargetMeshData);

		 CachedRetargetMeshData = RetargetMeshData;
		 CosmeticData.CosmeticMeshPartIDs.Empty(RetargetMeshData.DefaultPartTags.Num());
		 for (auto& Ele : RetargetMeshData.DefaultPartTags)
		 {
			 CosmeticData.CosmeticMeshPartIDs.Add(Ele.Value);
		 }
	 }
 }

void UCosmeticComponent::ResetMeshParts()
{
	if (ACharacter* Character = GetOwner<ACharacter>())
	{
		if (Character->HasAuthority())
		{			
			CosmeticData.RetargetMeshPresetID = -1;			
			SetRetargetMeshPreset(InitializedRetargetMeshPresetID);
			return;
		}
		else if (Character->IsLocallyControlled())
		{
			Server_ResetMeshParts();
			return;
		}
	}
}

void UCosmeticComponent::Server_ResetMeshParts_Implementation()
{
	ResetMeshParts();
}

USkeletalMeshComponent* UCosmeticComponent::GetChildMeshComponent(FGameplayTag MeshType)
{
	for (auto& Componet : EquippedMeshComponents)
	{		
		if (Componet.Key.MatchesTag(MeshType))
		{
			return Componet.Value;
		}
	}

	return nullptr;	
}

bool UCosmeticComponent::AddMeshPart(FCosmeticItemID MeshPartID)
{
	ACharacter* Character = GetOwner<ACharacter>();
	if (Character)
	{	
		if (!CosmeticData.CosmeticMeshPartIDs.Contains(MeshPartID))
		{
			if (Character->HasAuthority())
			{			
				// without networking
				if ((Character->GetNetMode() == ENetMode::NM_Standalone) || (Character->GetNetMode() == ENetMode::NM_ListenServer))
				{					
					ProcessAddMeshPartID(MeshPartID);
					CheckCosmeticData(CosmeticData);
					return true;
				}

				// Dedicate server
				ProcessAddMeshPartID(MeshPartID);				
				return true;
									
			}
			else if (Character->IsLocallyControlled())
			{			
				FMeshPartData MeshPartData;
				if (GetMeshPartData(CachedRetargetMeshData.CharacterTypeTag, MeshPartID, MeshPartData))
				{	
					Server_AddMeshPart(MeshPartID);

					return true;
				}
				else
				{
					UE_LOG(LogCosmeticSystem, Warning, TEXT("Can't get MeshPartData [%s]"), *UCosmeticBFL::Conv_CosmeticItemIDToString(MeshPartID));
				}
			}
		}
		else
		{		
			UE_LOG(LogCosmeticSystem, Warning, TEXT("Already equipped [%s]"), *UCosmeticBFL::Conv_CosmeticItemIDToString(MeshPartID));
		}
	}
	return false;
}

void UCosmeticComponent::Server_AddMeshPart_Implementation(FCosmeticItemID MeshPartID)
{
	AddMeshPart(MeshPartID);
}

bool UCosmeticComponent::AddMeshParts(const TArray<FCosmeticItemID> MeshPartIDs)
{
	ACharacter* Character = GetOwner<ACharacter>();
	if (Character)
	{
		if (Character->GetNetMode() == NM_Client && Character->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_AddMeshParts(MeshPartIDs);
			return true;
		}		

		bool Added = false;
		for (FCosmeticItemID MeshPartID : MeshPartIDs)
		{
			if (!CosmeticData.CosmeticMeshPartIDs.Contains(MeshPartID))
			{
				if (Character->HasAuthority())
				{
					ProcessAddMeshPartID(MeshPartID);	
					Added = true;
				}				
			}
			else
			{
				UE_LOG(LogCosmeticSystem, Warning, TEXT("Already equipped [%s]"), *UCosmeticBFL::Conv_CosmeticItemIDToString(MeshPartID));
			}
		}

		if (Added)
		{
			if ((Character->GetNetMode() == ENetMode::NM_Standalone) || (Character->GetNetMode() == ENetMode::NM_ListenServer))
			{
				CheckCosmeticData(CosmeticData);
			}

			return true;
		}
	}

	return false;
}

void UCosmeticComponent::Server_AddMeshParts_Implementation(const TArray<FCosmeticItemID>& MeshPartIDs)
{
	AddMeshParts(MeshPartIDs);
}

bool UCosmeticComponent::RemoveMeshPart(FCosmeticItemID MeshPartID)
{
	ACharacter* Character = GetOwner<ACharacter>();
	if (Character)
	{
		if (CosmeticData.CosmeticMeshPartIDs.Contains(MeshPartID))
		{	
			for (auto& Elem : CachedRetargetMeshData.DefaultPartTags)
			{
				if (Elem.Value == MeshPartID)
				{
					UE_LOG(LogCosmeticSystem, Warning, TEXT("RemoveMeshPart: [%s] is a default part tag."), *UCosmeticBFL::Conv_CosmeticItemIDToString(MeshPartID));
					return false;
				}
			}

			if (Character->HasAuthority())
			{			
				// without networking
				if ((Character->GetNetMode() == ENetMode::NM_Standalone) || (Character->GetNetMode() == ENetMode::NM_ListenServer))
				{					
					ProcessRemoveMeshPartID(MeshPartID);
					CheckCosmeticData(CosmeticData);
					return true;
				}

				// Dedicate server
				ProcessRemoveMeshPartID(MeshPartID);
				return true;
			
			}
			else if (Character->IsLocallyControlled())
			{			
				FMeshPartData MeshPartData;
				if (GetMeshPartData(CachedRetargetMeshData.CharacterTypeTag, MeshPartID, MeshPartData))
				{
					Server_RemoveMeshPart(MeshPartID);

					return true;
				}			
			}
		}
	}
	return false;
}

void UCosmeticComponent::Server_RemoveMeshPart_Implementation(FCosmeticItemID MeshPartID)
{
	RemoveMeshPart(MeshPartID);
}

bool UCosmeticComponent::RemoveMeshParts(const TArray<FCosmeticItemID> MeshPartIDs)
{
	ACharacter* Character = GetOwner<ACharacter>();
	if (Character)
	{		
		if (Character->GetNetMode() == NM_Client && Character->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_RemoveMeshParts(MeshPartIDs);
			return true;
		}

		bool bRemoved = false;
		for (FCosmeticItemID MeshPartID : MeshPartIDs)
		{
			if (CosmeticData.CosmeticMeshPartIDs.Contains(MeshPartID))
			{
				for (auto& Elem : CachedRetargetMeshData.DefaultPartTags)
				{
					if (Elem.Value == MeshPartID)
					{
						UE_LOG(LogCosmeticSystem, Warning, TEXT("RemoveMeshPart: [%s] is a default part tag."), *UCosmeticBFL::Conv_CosmeticItemIDToString(MeshPartID));
					}
				}

				if (Character->HasAuthority())
				{					
					ProcessRemoveMeshPartID(MeshPartID);
					bRemoved = true;
				}
			}
		}

		if (bRemoved)
		{
			if ((Character->GetNetMode() == ENetMode::NM_Standalone) || (Character->GetNetMode() == ENetMode::NM_ListenServer))
			{			
				CheckCosmeticData(CosmeticData);
				
			}
			return true;
		}
	}
	return false;
}

void UCosmeticComponent::Server_RemoveMeshParts_Implementation(const TArray<FCosmeticItemID>& MeshPartIDs)
{
	RemoveMeshParts(MeshPartIDs);
}

void UCosmeticComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CosmeticData);
}

void UCosmeticComponent::OnRep_CosmeticData(FCosmeticData OldData)
{
	if (OldData.RetargetMeshPresetID != CosmeticData.RetargetMeshPresetID)
	{
		ApplyRetargetMeshPreset(CosmeticData.RetargetMeshPresetID);
	}

	CheckCosmeticData(CosmeticData);
}

FCosmeticData UCosmeticComponent::GetCosmeticData()
{
	return CosmeticData;
}

FChangedCosmeticMeshes UCosmeticComponent::GetInformationToBeRemoved(FCosmeticItemID AddMeshPartID) const
{
	FChangedCosmeticMeshes RemovedMeshes;

	if (!CosmeticData.CosmeticMeshPartIDs.Contains(AddMeshPartID))
	{
		TSet<FCosmeticItemID> RemoveItemIDs;

		FMeshPartData MeshPartData;
		if (GetMeshPartData(CachedRetargetMeshData.CharacterTypeTag, AddMeshPartID, MeshPartData))
		{
			for (auto& Elem : MeshPartData.RemovePartIDs)
			{
				for (auto& Elem2 : CosmeticData.CosmeticMeshPartIDs)
				{
					if (Elem.GetPartAB() == Elem2.GetPartAB())
					{
						if (AddMeshPartID != Elem)
							RemoveItemIDs.Add(Elem2);
					}
				}
			}
		}

		for (auto& Elem : RemoveItemIDs)
		{
			if (IsDefaultPartItem(Elem))
			{
				RemovedMeshes.DefaultMeshPartIDs.Add(Elem);
			}
			else
			{
				RemovedMeshes.MeshPartIDs.Add(Elem);;
			}
		}		
	}

	return RemovedMeshes;
}

TArray<FCosmeticItemID> UCosmeticComponent::GetInformationToBeAdded(FCosmeticItemID RemoveMeshPartID) const
{
	TArray<FCosmeticItemID> AddDefaultParts;
	
	if (CosmeticData.CosmeticMeshPartIDs.Contains(RemoveMeshPartID))
	{
		for (auto& Elem : CachedRetargetMeshData.DefaultPartTags)
		{
			if (Elem.Value == RemoveMeshPartID)
			{
				UE_LOG(LogCosmeticSystem, Warning, TEXT("GetAddDefaultMesheInformation: [%s] is a default part tag."), *UCosmeticBFL::Conv_CosmeticItemIDToString(RemoveMeshPartID));
				return AddDefaultParts;
			}
		}
				
		TMap<FGameplayTag, FCosmeticItemID> NewEquippedMeshes;

		TArray<FCosmeticItemID> IgnoreItemIDs;
		for (FCosmeticItemID MeshPartID : CosmeticData.CosmeticMeshPartIDs)
		{			
			if (MeshPartID != RemoveMeshPartID)
			{
				FMeshPartData MeshPartData;				
				if (GetMeshPartData(CachedRetargetMeshData.CharacterTypeTag, MeshPartID, MeshPartData))
				{					
					IgnoreItemIDs.Append(MeshPartData.RemovePartIDs);
					TArray<FGameplayTag>TagArray;
					MeshPartData.CharacterPartTag.GetGameplayTagArray(TagArray);

					if (!TagArray.IsEmpty())
					{
						NewEquippedMeshes.Add(TagArray[0], MeshPartID);
					}					
				}
			}
		}

		for (auto& Elem : CachedRetargetMeshData.DefaultPartTags)
		{
			bool bFind = false;
			for (auto& NewEle : NewEquippedMeshes)
			{
				if (NewEle.Key.MatchesTag(Elem.Key))
				{
					bFind = true;
					break;
				}
			}

			if (!bFind)
			{
				bool bSkip = false;
				for (FCosmeticItemID& ID : IgnoreItemIDs)
				{
					if (ID.GetPartAB() == Elem.Value.GetPartAB())
					{
						bSkip = true;
						break;
					}
				}

				if (!bSkip)
					AddDefaultParts.Add(Elem.Value);					
			}
		}
	}
	
	return AddDefaultParts;
}

bool UCosmeticComponent::IsDefaultPartItem(FCosmeticItemID MeshPartID) const
{
	for (auto& Elem : CachedRetargetMeshData.DefaultPartTags)
	{
		if (MeshPartID == Elem.Value)
			return true;
	}

	return false;
}

USkeletalMeshComponent* UCosmeticComponent::GetVisualMesh()
{
	if (CosmeticActorInstance)
	{
		return CosmeticActorInstance->GetVisualMesh();
	}

	return nullptr;
}

void UCosmeticComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	CosmeticActorInstance = nullptr;
}

void UCosmeticComponent::ApplyVisualMesh(ACharacter* Character, const FRetargetMeshData& RetargetMeshData)
{
	if (Character)
	{	
		// Crouch function use GetDefaultObject<ACharacter>(), So unwanted bugs can happen
		//Character->GetCapsuleComponent()->SetCapsuleSize(RetargetMeshData.CapsuleRadius, RetargetMeshData.CapsuleHalfHeight, false);
		Character->GetMesh()->SetWorldScale3D(RetargetMeshData.ParentMeshScale);

		UCosmeticChildActorComponent* ChildActor = Character->FindComponentByClass<UCosmeticChildActorComponent>();
		if (!ChildActor)
		{
			ChildActor = NewObject<UCosmeticChildActorComponent>(Character);						
			ChildActor->SetupAttachment(Character->GetMesh());
			ChildActor->SetChildActorClass(CosmeticActor);
			ChildActor->RegisterComponent();

			if (AActor* SpawnedActor = ChildActor->GetChildActor())
			{				
				SpawnedActor->SetActorEnableCollision(RetargetMeshData.bEnableCollision);

				// Set up a direct tick dependency to work around the child actor component not providing one
				if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
				{
					SpawnedRootComponent->AddTickPrerequisiteComponent(Character->GetMesh());
				}

				CosmeticActorInstance = Cast<ACosmeticActor>(SpawnedActor);
			}			
		}

		ChildActor->SetWorldScale3D(RetargetMeshData.VisualMeshScale);

		if (CosmeticActorInstance)
		{
			USkeletalMesh* VisualMesh = RetargetMeshData.VisualMeshObject.LoadSynchronous();

			USkeletalMeshComponent* MeshComp = CosmeticActorInstance->GetVisualMesh();			
			MeshComp->SetSkeletalMesh(VisualMesh);
			MeshComp->EmptyOverrideMaterials();
			MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			MeshComp->SetAnimInstanceClass(RetargetMeshData.AnimClass);
		
			UPhysicsAsset* NewPhysicsAsset = RetargetMeshData.NewPhysicsAssetClass.LoadSynchronous();
			if (NewPhysicsAsset)
			{
				MeshComp->SetPhysicsAsset(NewPhysicsAsset);
			}

			if (RetargetMeshData.bEnableCollision)
			{			
				MeshComp->SetCollisionProfileName(RetargetMeshData.CollisionProfileName);
			}
			else
			{
				MeshComp->SetPhysicsAsset(nullptr);			
				MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			}
									
		}
		
	}
}

bool UCosmeticComponent::ProcessAddMeshPartID(FCosmeticItemID MeshPartID)
{
	if (!CosmeticData.CosmeticMeshPartIDs.Contains(MeshPartID))
	{		
		TArray<FCosmeticItemID> RemoveItemIDs;
		// remove same part meshes
		for (auto& Elem : CosmeticData.CosmeticMeshPartIDs)
		{
			if (Elem.GetPartAB() == MeshPartID.GetPartAB())
			{
				RemoveItemIDs.Push(Elem);				
			}
		}

		FMeshPartData MeshPartData;
		if (GetMeshPartData(CachedRetargetMeshData.CharacterTypeTag, MeshPartID, MeshPartData))
		{
			for (auto& Elem : MeshPartData.RemovePartIDs)
			{
				for (auto& Elem2 : CosmeticData.CosmeticMeshPartIDs)
				{
					if (Elem.GetPartAB() == Elem2.GetPartAB())
					{
						RemoveItemIDs.Push(Elem2);
					}
				}
			}			
		}

		for (auto& Elem : RemoveItemIDs)
		{
			CosmeticData.CosmeticMeshPartIDs.Remove(Elem);
		}

		CosmeticData.CosmeticMeshPartIDs.Add(MeshPartID);

		return true;
	}

	return false;
}

bool UCosmeticComponent::ProcessRemoveMeshPartID(FCosmeticItemID MeshPartID)
{
	if (CosmeticData.CosmeticMeshPartIDs.Contains(MeshPartID))
	{		
		CosmeticData.CosmeticMeshPartIDs.Remove(MeshPartID);
		return true;
	}

	return false;
}

void UCosmeticComponent::CheckCosmeticData(const FCosmeticData& NewCosmeticData, bool DrawOnlyBody)
{		
	if (ACharacter* Character = GetOwner<ACharacter>())
	{			
		if (CosmeticActorInstance)
		{
			// Make list of new Mesh Parts
			TMap<FGameplayTag, FCachedMeshPartData> NewEquippedMeshes;			
			
			TArray<FCosmeticItemID> IgnoreItemIDs;
			for (FCosmeticItemID MeshPartID : NewCosmeticData.CosmeticMeshPartIDs)
			{
				TArray<FCosmeticItemID> RemoveItemIDs = AppendMeshPart(NewEquippedMeshes, MeshPartID);
				IgnoreItemIDs.Append(RemoveItemIDs);
			}			

			for (auto& Elem : CachedRetargetMeshData.DefaultPartTags)
			{
				bool bFind = false;
				for (auto& NewEle : NewEquippedMeshes)
				{
					if (NewEle.Key.MatchesTag(Elem.Key))
					{
						bFind = true;
						break;
					}
				}

				if (!bFind)
				{
					bool bSkip = false;
					for (FCosmeticItemID& ID : IgnoreItemIDs)
					{
						if (ID.GetPartAB() == Elem.Value.GetPartAB())
						{
							bSkip = true;
							break;
						}
					}

					if (!bSkip)
						AppendMeshPart(NewEquippedMeshes, Elem.Value);
				}
			}
						
			// Find Body Parts from blueprint
			FGameplayTagContainer EquippedMeshPartTags;

			for (auto& Elem : NewEquippedMeshes)
				EquippedMeshPartTags.AddTag(Elem.Key);

			if (DrawOnlyBody)
				NewEquippedMeshes.Empty();			
			
			//if (EquippedMeshPartTags.Num() > 0)
			{
				TArray<FBodyMeshPartData> BodyTypeData;
				if (GetBodyMeshData(CachedRetargetMeshData.CharacterTypeTag, EquippedMeshPartTags, BodyTypeData))
				{					
					for(FBodyMeshPartData& Elem : BodyTypeData)
					{						
						USkeletalMesh* BodyMesh = Elem.BodyMeshObject.LoadSynchronous();
						FCachedMeshPartData CachedData;
						CachedData.MeshPartID = -1;
						CachedData.Mesh = BodyMesh;
						CachedData.MergeTargetMeshTag = Elem.MergeTargetMeshTag;
						CachedData.bMergeable = Elem.bMergeable;
						
						NewEquippedMeshes.Add(Elem.CharacterPartTag, CachedData);
					}
				}
			}

			ClearEquippedMeshComponents();						
			
			if (CosmeticActorInstance->IsEnableMerge())
			{
				// merge and add				
				TMap<FName, TArray<FMeshesToMergeData>> MeshesToMerge;
				for (auto& Elem : NewEquippedMeshes)
				{
					if (Elem.Value.bFakeData)
						continue;

					if (Elem.Value.Groom != nullptr)
					{
						EquipGroomMesh(Elem.Key, Elem.Value);						
					}					
					else if (Elem.Value.Mesh != nullptr)
					{
						if (CachedRetargetMeshData.bUseSeparateFaceMesh)
						{
							if (EquipFaceMeshToVisualMesh(Elem.Key, Elem.Value))
								continue;

							if (EquipBodyMeshToVisualMesh(Elem.Key, Elem.Value))
								continue;						
						}
						
						if (Elem.Value.bMergeable && Elem.Value.Mesh->GetMeshClothingAssets().IsEmpty())
						{
							if (USkeletalMeshComponent* TargetMesh = CosmeticActorInstance->GetMergeTargetSkeletalMesh(Elem.Value.MergeTargetMeshTag))
							{
								FMeshesToMergeData Data;
								Data.PartTag = Elem.Key;
								Data.MeshPartData = Elem.Value;

								TArray<FMeshesToMergeData>& Meshes = MeshesToMerge.FindOrAdd(Elem.Value.MergeTargetMeshTag);
								Meshes.Add(Data);

								continue;
							}
						}

						EquipSkeletalMesh(Elem.Key, Elem.Value);
					}					
				}

				TMap<USkeletalMeshComponent*, FMeshesToMergeSkeletalMeshes> MergeMeshes;
				for (auto& Elem : MeshesToMerge)
				{
					if (Elem.Value.Num() == 1)
					{
						EquipSkeletalMesh(Elem.Value[0].PartTag, Elem.Value[0].MeshPartData);
					}
					else
					{
						USkeletalMeshComponent* TargetMesh = CosmeticActorInstance->GetMergeTargetSkeletalMesh(Elem.Key);						
						FMeshesToMergeSkeletalMeshes& MergeSkeletalMeshes = MergeMeshes.FindOrAdd(TargetMesh);

						for (int32 Index = 0; Index < Elem.Value.Num(); ++Index)
						{
							if (Elem.Value[Index].MeshPartData.MeshPartID.IsValid())
							{
								MergeSkeletalMeshes.MeshPartIDs.Add(Elem.Value[Index].MeshPartData.MeshPartID);
							}

							MergeSkeletalMeshes.Meshes.Add(Elem.Value[Index].MeshPartData.Mesh);

							for (auto& MaterialElem : Elem.Value[Index].MeshPartData.Materials)
							{
								MergeSkeletalMeshes.OverrideMaterials.Add(MaterialElem.Key, MaterialElem.Value);
							}
						}					
					}
				}

				if (!MergeMeshes.IsEmpty())
				{
					FSkeletalMeshMergeParams MergeParams;					

					UPhysicsAsset* DefaultPhysAsset = CachedRetargetMeshData.NewPhysicsAssetClass.LoadSynchronous();

					for (auto& Elem : MergeMeshes)
					{					
						Elem.Key->EmptyOverrideMaterials();
						MergeParams.MeshesToMerge = Elem.Value.Meshes;
						if (!CachedRetargetMeshData.bMergeSkeleton)
						{
							MergeParams.Skeleton = CachedRetargetMeshData.VisualMeshObject->GetSkeleton();
						}
						else
						{
							FSkeletonMergeParams SkeletonMergeParams;

							for (USkeletalMesh* EleMesh : Elem.Value.Meshes)
							{
								if (!SkeletonMergeParams.SkeletonsToMerge.Contains(EleMesh->GetSkeleton()))
								{
									SkeletonMergeParams.SkeletonsToMerge.Add(EleMesh->GetSkeleton());
								}
							}

							MergeParams.Skeleton = USkeletalMergingLibrary::MergeSkeletons(SkeletonMergeParams);
							if (!MergeParams.Skeleton)
							{
								FString Msg = FString::Printf(TEXT("MergeSkeletons failed, Instance [%s]. Skeletons involved: "), *GetOuter()->GetName());

								const int32 SkeletonCount = SkeletonMergeParams.SkeletonsToMerge.Num();
								for (int32 SkeletonIndex = 0; SkeletonIndex < SkeletonCount; ++SkeletonIndex)
								{
									Msg += FString::Printf(TEXT(" [%s]"), *SkeletonMergeParams.SkeletonsToMerge[SkeletonIndex].GetName());
								}

								UE_LOG(LogCosmeticSystem, Warning, TEXT("%s"), *Msg);

								MergeParams.Skeleton = CachedRetargetMeshData.VisualMeshObject->GetSkeleton();
							}
						}

						USkeletalMesh* MergedMesh = nullptr;
						if (UCosmeticSystemSettings::Get()->bUseMergedMeshStorage)
						{							
							UCosmeticMeshStorage* Storage = UCosmeticMeshStorage::Get(Character->GetWorld());
							MergedMesh = Storage->GetMergedMesh(Elem.Value.MeshPartIDs);
							if (!MergedMesh)
							{
								MergedMesh = USkeletalMergingLibrary::MergeMeshes(MergeParams);
								if (MergedMesh)
								{
									if (UCosmeticSystemSettings::Get()->bCheckClear)
									{
										if (UCosmeticSystemSettings::Get()->CheckClearCount <= Storage->GetStoredMergedMeshCount() + 1)
										{
											Storage->Clear();
										}
									}

									FMergedMeshHandle Data;
									Data.MergedMesh = MergedMesh;
									Data.MeshPartIDs = Elem.Value.MeshPartIDs;
									Storage->RegisterMergedMesh(Data);
								}
							}
						}
						else
						{
							MergedMesh = USkeletalMergingLibrary::MergeMeshes(MergeParams);							
						}
						
						if (MergedMesh)
						{
							Elem.Key->SetSkeletalMesh(MergedMesh);
							Elem.Key->SetPhysicsAsset(DefaultPhysAsset);

							// bUseSeparatelyMasterMesh 
							if (CosmeticActorInstance->GetVisualMesh() != Elem.Key)
							{							
								Elem.Key->SetLeaderPoseComponent(CosmeticActorInstance->GetVisualMesh(), true);
							}

							for (auto& MateirlaElem : Elem.Value.OverrideMaterials)
							{
								Elem.Key->SetMaterialByName(MateirlaElem.Key, MateirlaElem.Value);
							}

							if (CachedRetargetMeshData.bMergePhysAsset)
							{
								TArray<UPhysicsAsset*> PhysicsAssets;

								for (USkeletalMesh* SkeletalMesh : MergeParams.MeshesToMerge)
								{
									if (UPhysicsAsset* PhysicAsset = SkeletalMesh->GetPhysicsAsset())
									{
										PhysicsAssets.Add(PhysicAsset);
									}
								}

								if (PhysicsAssets.Num() > 1)
								{
									CosmeticActorInstance->MergePhysicsAssets(PhysicsAssets, Elem.Key);									
								}
								else
								{
									UE_LOG(LogCosmeticSystem, Warning, TEXT("Error: Merge PhysAsset. There must be at least two meshes to which PhysAsset is assigned."));
								}
							}
						}
					}
				}
			}
			else // Not Merge
			{	
				for (auto& Elem : NewEquippedMeshes)
				{
					if (Elem.Value.Groom != nullptr)
					{
						EquipGroomMesh(Elem.Key, Elem.Value);
					}
					else if (Elem.Value.Mesh != nullptr)
					{
						if (EquipFaceMeshToVisualMesh(Elem.Key, Elem.Value))
							continue;	
							
						if (EquipBodyMeshToVisualMesh(Elem.Key, Elem.Value))
							continue;

						EquipSkeletalMesh(Elem.Key, Elem.Value);						
					}
					
				}
			}

			EquippedMeshes = NewEquippedMeshes;
			
			// remove Components
			TArray<FGameplayTag> RemoveTags;
			for (auto& Elem : EquippedMeshComponents)
			{
				if (!NewEquippedMeshes.Contains(Elem.Key))
				{					
					RemoveTags.Add(Elem.Key);			
				}
			}

			for (FGameplayTag& Elem : RemoveTags)
			{
				EquippedMeshComponents[Elem]->DestroyComponent();
				EquippedMeshComponents.Remove(Elem);
			}

		} // if (CosmeticActorInstance)

		OnChangedMeshPart.Broadcast(this);

		if (CosmeticActorInstance)
		{
			CosmeticActorInstance->OnChangedSkeletalMesh();
		}
	} // if (ACharacter* Character = GetOwner<ACharacter>())
}

void UCosmeticComponent::Debug_CheckCosmeticData(bool DrawOnlyBody)
{
	CheckCosmeticData(CosmeticData, DrawOnlyBody);
}

UCosmeticSkeletalMeshComponent* UCosmeticComponent::AddNewInstanceSkeletalMesh(FGameplayTag PartTag, FName SocketName, bool bUseMasterPoseComponent, TSubclassOf<UCosmeticSkeletalMeshComponent> OverrideSkeletalMeshComponent)
{
	if (CosmeticActorInstance)
	{
		if (!EquippedMeshComponents.Contains(PartTag))
		{			
			UCosmeticSkeletalMeshComponent* MeshComp = CosmeticActorInstance->CreateInstanceSkeletalMesh(PartTag, SocketName, bUseMasterPoseComponent, OverrideSkeletalMeshComponent);
			EquippedMeshComponents.Add(PartTag, MeshComp);			
			return MeshComp;
		}
	}

	return nullptr;
}

UCosmeticGroomComponent* UCosmeticComponent::AddNewInstanceGroomAsset(FGameplayTag PartTag, FName SocketName /*= NAME_None*/, TSubclassOf<UCosmeticGroomComponent> OverrideGroomComponent)
{
	if (CosmeticActorInstance)
	{
		if (!EquippedGroomComponents.Contains(PartTag))
		{
			UCosmeticGroomComponent* GroomComp = CosmeticActorInstance->CreateInstanceGroomAsset(PartTag, SocketName, OverrideGroomComponent);
			EquippedGroomComponents.Add(PartTag, GroomComp);

			return GroomComp;
		}
	}

	return nullptr;
}

TArray<FCosmeticItemID> UCosmeticComponent::AppendMeshPart(TMap<FGameplayTag, FCachedMeshPartData>& NewEquippedMeshes, FCosmeticItemID MeshPartID)
{
	FMeshPartData MeshPartData;
	TArray<FCosmeticItemID> RemoveItemIDs;
	if (GetMeshPartData(CachedRetargetMeshData.CharacterTypeTag, MeshPartID, MeshPartData))
	{
		USkeletalMesh* MeshPart = MeshPartData.MeshPartClass.LoadSynchronous();
		UGroomAsset* Groom = MeshPartData.GroomAsset.LoadSynchronous();

		UPhysicsAsset* NewPhysicsAsset = nullptr;		
		RemoveItemIDs = MeshPartData.RemovePartIDs;
		TArray<FGameplayTag>TagArray;
		MeshPartData.CharacterPartTag.GetGameplayTagArray(TagArray);

		if (!TagArray.IsEmpty())
		{
			TMap<FName, TObjectPtr<UMaterialInterface>> Materials;
			if (MeshPartData.Materials.Num() > 0)
			{
				for (auto& Elem : MeshPartData.Materials)
				{
					UMaterialInterface* Inst = Elem.Value.LoadSynchronous();
					Materials.Add(Elem.Key, Inst);
				}
			}

			if (MeshPartData.PhysicsAssetClass.IsValid())
			{
				NewPhysicsAsset = MeshPartData.PhysicsAssetClass.LoadSynchronous();
			}
			
			FCachedMeshPartData CachedData;
			CachedData.MeshPartID = MeshPartID;
			CachedData.Mesh = MeshPart;
			CachedData.Groom = Groom;
			CachedData.AnimClass = MeshPartData.AnimClass;
			CachedData.PhysicsAsset = NewPhysicsAsset;
			CachedData.AttachSocketName = MeshPartData.AttachSocketName;
			CachedData.CollisionProfileName = MeshPartData.CollisionProfileName;
			CachedData.MergeTargetMeshTag = MeshPartData.MergeTargetMeshTag;
			CachedData.bMergeable = MeshPartData.bMergeable;
			CachedData.bUseMasterPoseComponent = MeshPartData.bUseMasterPoseComponent;			
			CachedData.bUseCustomTransform = MeshPartData.bUseCustomTransform;
			CachedData.AttachTransform = MeshPartData.AttachTransform;
			CachedData.Materials = Materials;
			CachedData.OverrideSkeletalMeshComponent = MeshPartData.OverrideCosmeticSkeletalMeshComponent;
			CachedData.OverrideGroomComponent = MeshPartData.OverrideCosmeticGroomComponent;
			
			if (Groom != nullptr)
			{
				CachedData.bMergeable = false;
			}

			NewEquippedMeshes.Add(TagArray[0], CachedData);
			
			FCachedMeshPartData CachedFakeData;
			CachedFakeData.MeshPartID = MeshPartID;
			CachedFakeData.bMergeable = false;
			CachedFakeData.bFakeData = true;
			for (int Index = 1; Index < TagArray.Num(); ++Index)
			{				
				NewEquippedMeshes.Add(TagArray[Index], CachedFakeData);
			}
		}
	}

	return RemoveItemIDs;
}

bool UCosmeticComponent::EquipFaceMeshToVisualMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe)
{
	if (CachedRetargetMeshData.bIsFaceMeshMasterMesh)
	{
		if (EquippedMesheTag.MatchesTag(TAG_CosmeticSystem_FaceType_Tag))
		{
			CosmeticActorInstance->GetVisualMesh()->SetSkeletalMesh(EquippedMeshe.Mesh);
			CosmeticActorInstance->GetVisualMesh()->EmptyOverrideMaterials();
			for (auto& Elem : EquippedMeshe.Materials)
			{
				CosmeticActorInstance->GetVisualMesh()->SetMaterialByName(Elem.Key, Elem.Value);
			}
			
			CosmeticActorInstance->GetVisualMesh()->ClearAnimScriptInstance();
			CosmeticActorInstance->GetVisualMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			CosmeticActorInstance->GetVisualMesh()->SetAnimInstanceClass(CachedRetargetMeshData.AnimClass);
			CosmeticActorInstance->GetVisualMesh()->InitAnim(true);
			return true;
		}
	}

	return false;
}

bool UCosmeticComponent::EquipBodyMeshToVisualMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe)
{
	if (!CachedRetargetMeshData.bIsFaceMeshMasterMesh)
	{
		if (EquippedMesheTag.MatchesTag(TAG_CosmeticSystem_BodyType_Tag))
		{
			CosmeticActorInstance->GetVisualMesh()->SetSkeletalMesh(EquippedMeshe.Mesh);
			CosmeticActorInstance->GetVisualMesh()->EmptyOverrideMaterials();
			for (auto& Elem : EquippedMeshe.Materials)
			{
				CosmeticActorInstance->GetVisualMesh()->SetMaterialByName(Elem.Key, Elem.Value);
			}

			CosmeticActorInstance->GetVisualMesh()->ClearAnimScriptInstance();
			CosmeticActorInstance->GetVisualMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			CosmeticActorInstance->GetVisualMesh()->SetAnimInstanceClass(CachedRetargetMeshData.AnimClass);
			CosmeticActorInstance->GetVisualMesh()->InitAnim(true);
			return true;
		}
	}

	return false;
}

void UCosmeticComponent::EquipGroomMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe)
{
	UGroomComponent* GroomComponent = nullptr;
	if (EquippedGroomComponents.Contains(EquippedMesheTag))
	{
		GroomComponent = EquippedGroomComponents[EquippedMesheTag];
		GroomComponent->EmptyOverrideMaterials();
	}
	else
	{
		GroomComponent = AddNewInstanceGroomAsset(EquippedMesheTag, EquippedMeshe.AttachSocketName);
	}

	if (EquippedMeshe.bUseCustomTransform)
		GroomComponent->SetRelativeTransform(EquippedMeshe.AttachTransform);

	GroomComponent->SetGroomAsset(EquippedMeshe.Groom);

	for (auto& Elem : EquippedMeshe.Materials)
	{
		GroomComponent->SetMaterialByName(Elem.Key, Elem.Value);
	}	
}

void UCosmeticComponent::EquipSkeletalMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe)
{
	USkeletalMeshComponent* Skel = nullptr;
	if (EquippedMeshComponents.Contains(EquippedMesheTag))
	{
		Skel = EquippedMeshComponents[EquippedMesheTag];
		Skel->EmptyOverrideMaterials();
	}
	else
	{
		Skel = AddNewInstanceSkeletalMesh(EquippedMesheTag, EquippedMeshe.AttachSocketName, EquippedMeshe.bUseMasterPoseComponent, EquippedMeshe.OverrideSkeletalMeshComponent);
		Skel->LightingChannels = CosmeticActorInstance->GetVisualMesh()->LightingChannels;
	}

	if (EquippedMeshe.bUseCustomTransform)
		Skel->SetRelativeTransform(EquippedMeshe.AttachTransform);

	Skel->SetSkeletalMesh(EquippedMeshe.Mesh);

	for (auto& Elem : EquippedMeshe.Materials)
	{
		Skel->SetMaterialByName(Elem.Key, Elem.Value);
	}	

	if (EquippedMeshe.CollisionProfileName != NAME_None)
	{
		Skel->SetCollisionProfileName(EquippedMeshe.CollisionProfileName);
		//Skel->SetCollisionEnabled(Elem.Value.CollisionEnabled);
	}
	else
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (EquippedMeshe.PhysicsAsset)
	{
		Skel->SetPhysicsAsset(EquippedMeshe.PhysicsAsset, true);
		//Skel->SetSimulatePhysics(EquippedMeshe.bSimulatePhysics);
	}

	Skel->SetAnimInstanceClass(EquippedMeshe.AnimClass);

	if (EquippedMeshe.AnimClass && !EquippedMeshe.bUseMasterPoseComponent)
	{
		Skel->ClearAnimScriptInstance();
		Skel->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Skel->InitAnim(true);
	}

	// ClothSimulated is controlled by CosmeticSkeletalMeshComponent
	//Skel->bAllowClothActors = EquippedMeshe.bClothSimulation;
	//Skel->bDisableClothSimulation = !EquippedMeshe.bClothSimulation;
}

void UCosmeticComponent::ClearEquippedMeshComponents()
{
	// remove
	for (auto& Componet : EquippedMeshComponents)
	{
		Componet.Value->SetSkeletalMesh(nullptr);
	}

	for (auto& GroomComponet : EquippedGroomComponents)
	{
		GroomComponet.Value->SetGroomAsset(nullptr);
	}
}
