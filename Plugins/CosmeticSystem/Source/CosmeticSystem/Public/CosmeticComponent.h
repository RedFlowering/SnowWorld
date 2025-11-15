// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/CoreNet.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "CosmeticItemID.h"
#include "CosmeticStructure.h"
#include "CosmeticActor.h"
#include "CosmeticSkeletalMeshComponent.h"
#include "CosmeticGroomComponent.h"
#include "CosmeticComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FChangedMeshPartSignature, UCosmeticComponent, OnChangedMeshPart, UCosmeticComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FCreatedCosmeticActorSignature, UCosmeticComponent, OnCreatedCosmeticActor, ACosmeticActor*, CosmeticActor);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FChangedRetargetMeshPresetSignature, UCosmeticComponent, OnChangedRetargetMeshPreset, FCosmeticItemID, RetargetMeshPresetID);

/**  
 * UCosmeticComponent 
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Cosmetic), meta = (BlueprintSpawnableComponent))
class COSMETICSYSTEM_API UCosmeticComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;
		
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	USkeletalMeshComponent* GetVisualMesh();

	ACosmeticActor* GetCosmeticActorInstance() { return CosmeticActorInstance; }	

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	void SetRetargetMeshPreset(FCosmeticItemID RetargetMeshPresetID);

	UFUNCTION(Server, Reliable)
	void Server_SetRetargetMeshPreset(FCosmeticItemID RetargetMeshPresetID);

	void ApplyRetargetMeshPreset(FCosmeticItemID RetargetMeshPresetID);

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	void ResetMeshParts();

	UFUNCTION(Server, Reliable)
	void Server_ResetMeshParts();	

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CosmeticSystem")
	bool GetRetargetMeshData(FCosmeticItemID RetargetMeshPresetID, FRetargetMeshData& RetargetMeshData) const;

 	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CosmeticSystem")
 	bool GetBodyMeshData(FGameplayTag CharacterType, UPARAM(meta = (Categories = "CosmeticSystem")) FGameplayTagContainer EquippedMeshPartTags, TArray<FBodyMeshPartData>& Body) const;

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	USkeletalMeshComponent* GetChildMeshComponent(FGameplayTag MeshType);

	//	MeshPartData
	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	bool AddMeshPart(FCosmeticItemID MeshPartID);

	UFUNCTION(Server, Reliable)
	void Server_AddMeshPart(FCosmeticItemID MeshPartID);

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	bool AddMeshParts(const TArray<FCosmeticItemID> MeshPartIDs);

	UFUNCTION(Server, Reliable)
	void Server_AddMeshParts(const TArray<FCosmeticItemID>& MeshPartIDs);

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	bool RemoveMeshPart(FCosmeticItemID MeshPartID);

	UFUNCTION(Server, Reliable)
	void Server_RemoveMeshPart(FCosmeticItemID MeshPartID);

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	bool RemoveMeshParts(const TArray<FCosmeticItemID> MeshPartIDs);

	UFUNCTION(Server, Reliable)
	void Server_RemoveMeshParts(const TArray<FCosmeticItemID>& MeshPartIDs);


	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CosmeticSystem")
	bool GetMeshPartData(FGameplayTag CharacterType, FCosmeticItemID MeshPartID, FMeshPartData& MeshPartData) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CosmeticData(FCosmeticData OldData);
	
	UPROPERTY(BlueprintAssignable, Category = "CosmeticSystem")
	FChangedMeshPartSignature OnChangedMeshPart;

	UPROPERTY(BlueprintAssignable, Category = "CosmeticSystem")
	FCreatedCosmeticActorSignature OnCreatedCosmeticActor;

	UPROPERTY(BlueprintAssignable, Category = "CosmeticSystem")
	FChangedRetargetMeshPresetSignature OnChangedRetargetMeshPreset;

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	FCosmeticData GetCosmeticData();

	/**
	 * get equipment change information for inventory system
	 * @return parts to be unequipped
	 */
	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	FChangedCosmeticMeshes GetInformationToBeRemoved(FCosmeticItemID AddMeshPartID) const;

	/**	 
	 * get equipment change information for inventory system
	 * @return default parts to be equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem")
	TArray<FCosmeticItemID> GetInformationToBeAdded(FCosmeticItemID RemoveMeshPartID) const;

	bool IsDefaultPartItem(FCosmeticItemID MeshPartID) const;

protected:
	void ApplyVisualMesh(ACharacter* Character, const FRetargetMeshData& RetargetMeshData);
		
	bool ProcessAddMeshPartID(FCosmeticItemID MeshPartID);

	bool ProcessRemoveMeshPartID(FCosmeticItemID MeshPartID);
		
	virtual void CheckCosmeticData(const FCosmeticData& NewCosmeticData, bool DrawOnlyBody = false);

	UFUNCTION(BlueprintCallable, Category = "CosmeticSystem Debug")
	void Debug_CheckCosmeticData(bool DrawOnlyBody);

	UCosmeticSkeletalMeshComponent* AddNewInstanceSkeletalMesh(FGameplayTag PartTag, FName SocketName = NAME_None, 
						bool bUseMasterPoseComponent = true, TSubclassOf<UCosmeticSkeletalMeshComponent> OverrideSkeletalMeshComponent = nullptr);

	UCosmeticGroomComponent* AddNewInstanceGroomAsset(FGameplayTag PartTag, FName SocketName = NAME_None, 
						TSubclassOf<UCosmeticGroomComponent> OverrideGroomComponent = nullptr);

	// return RemoveItemIDs
	TArray<FCosmeticItemID> AppendMeshPart(TMap<FGameplayTag, FCachedMeshPartData>& NewEquippedMeshes, FCosmeticItemID MeshPartID);

	bool EquipFaceMeshToVisualMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe);
	
	bool EquipBodyMeshToVisualMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe);

	void EquipGroomMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe);	

	void EquipSkeletalMesh(FGameplayTag EquippedMesheTag, const FCachedMeshPartData& EquippedMeshe);

	void ClearEquippedMeshComponents();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialize|Cosmetic", meta = (AllowPrivateAccess = "true"))
	FCosmeticItemID InitializedRetargetMeshPresetID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialize|Cosmetic", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACosmeticActor> CosmeticActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cosmetic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ACosmeticActor> CosmeticActorInstance = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_CosmeticData)
	FCosmeticData CosmeticData;
	
	UPROPERTY()
	FRetargetMeshData CachedRetargetMeshData;

	UPROPERTY()
	TMap<FGameplayTag, FCachedMeshPartData> EquippedMeshes;
		
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UCosmeticSkeletalMeshComponent>> EquippedMeshComponents;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UCosmeticGroomComponent>> EquippedGroomComponents;

	UPROPERTY(Transient)
	FName CachedMeshCollisionProfileName;
};