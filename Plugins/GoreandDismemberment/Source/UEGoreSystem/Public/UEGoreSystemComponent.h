// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UEGoreSystemStruct.h"
#include "UEGoreSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBoneBroken, FName, Bone, class USkeletalMeshComponent*, DetachedLimb);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEGORESYSTEM_API UUEGoreSystemComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties
	UUEGoreSystemComponent();

	/** Should enable debuging?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem")
		uint8 bEnableDebugLogging : 1;
	/** Physics asset to use, will override any other*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem")
		UPhysicsAsset* DefaultPhysicsAsset;
	/** Should enable automatic dismemberment? (beta)*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem|Automation")
		uint8 bEnableAutoDismemberment : 1;
	/** Should enable stretching fix? (beta)*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem|Automation", meta = (EditCondition = "bEnableAutoDismemberment"))
		uint8 bEnableStretchingFix : 1;
	/** Should enable debuging?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem|Automation", meta = (EditCondition = "bEnableAutoDismemberment"))
		uint8 bControlRigProcedural : 1;	
	/** Replace your character mesh with another "wounded" version for your detached limbs*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem|Automation", meta = (EditCondition = "bEnableAutoDismemberment"))
		USkeletalMesh* LimbsMesh;

	/** Main data structure for this GoreSystem Component*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GoreSystem")
		FUEGoreSystemSettings GoreSettings;

	UPROPERTY(BlueprintReadOnly, Category = "GoreSystem|Runtime")
		FUEGoreSystemHealthValues HealthValues;

	/** [SERVER ONLY] Get Health of a specific bone
	* >BoneName - Target bone
	* @Health - Return Health value
	* @return true if bone has been found, false if not;
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
	bool GetBoneHealth(FName BoneName, float& Health) const;

	/** Get Detached Mesh Component from BoneName
		* >BoneName - Target bone
		* @MeshComponent - Returns pointer to mesh component
		* @return true if the mesh component exists;
		*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
	bool GetDetachedMeshFromBoneName(FName BoneName, USkeletalMeshComponent*& MeshComponent) const;

	UPROPERTY()
	TArray<class UFXSystemComponent*> AttachedGoreFXs;

	UFUNCTION(BlueprintCallable, Category = "GoreSystem", BlueprintAuthorityOnly)
		void DestroyAllAttachedGoreFXs();
	UFUNCTION(NetMulticast, Reliable)
		void DestroyAllAttachedGoreFXs_Multi();

	UPROPERTY()
		TArray<FName> DestroyedBones;
	UPROPERTY()
		TArray<ADecalActor*> DecalsList;
	UPROPERTY()
		TArray<USceneComponent*> SpawnedComponents;
	UPROPERTY()
		USkeletalMeshComponent* VisualMeshComponent;

	UFUNCTION(BlueprintPure, Category = "GoreSystem")
		USkeletalMeshComponent* GetVisualMeshComponent() const {
		return VisualMeshComponent;
	}

	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void SetVisualMeshComponent(USkeletalMeshComponent* InComp) {
		VisualMeshComponent = InComp;
	}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "GoreSystem|Runtime")
	class USkeletalMeshComponent* MeshRoot;

public:	
	UFUNCTION(BlueprintPure, Category = "GoreSystem")
		USkeletalMeshComponent* GetMeshRoot() const {
		return MeshRoot;
	}

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// Replication setup
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	//meta = (WorldContext = "WorldContextObject")

	/** [Optional] This function will initialize the GoreSystem, it's automatically called OnBeginPlay but you can use it to manually setup the target SkeletalMeshComponent you want to use
	* >NewRootMesh - SkeletalMeshComponent to use by the GoreSystem
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void Init(USkeletalMeshComponent* NewRootMesh);

	UFUNCTION()
		void SpawnDecalAt(const TSubclassOf<ADecalActor> DecalActorClass, const FTransform DecalTransform, const float InLifespan);

	/** Hit multiple bones, spawning VFXs and SFXs for each socket in the list [THIS WILL NOT AFFECT LIMBS HEALTH]
	* >Bones - BoneName, Location and Normal to use for these Hit events. Normally these informations are gathered from a trace or hit event depending on your damage system!
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void HitBones(const TArray<FUEGoreSystemHit> Bones);
	UFUNCTION(Server, Reliable)
		void HitBones_Serv(const TArray<FUEGoreSystemHit>& Bones);
	UFUNCTION(NetMulticast, Reliable)
		void HitBones_Multi(const TArray<FUEGoreSystemHit>& Bones);
	UFUNCTION()
		void Int_HitBones(const TArray<FUEGoreSystemHit> Bones);

	/** Hit a single bone, spawning VFXs and SFXs [THIS WILL NOT AFFECT LIMBS HEALTH]
	* >Bone - BoneName, Location and Normal to use for this Hit event. Normally these informations are gathered from a trace or hit event depending on your damage system!
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void HitBone(const FUEGoreSystemHit BoneHit);

	/** Destroy multiple bones, spawning VFXs and SFXs for each socket in the list [THIS WILL NOT AFFECT LIMBS HEALTH]
	* >Bones - BoneName, Location and Normal to use for these Destroy event. Normally these informations are gathered from a trace or hit event depending on your damage system!
	* >InForce - Force to apply on detachment
	* @return true if detach happened, false if not;
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		bool DestroyBones(const TArray<FUEGoreSystemHit> Bones, const float Force = 3000.0f);
	UFUNCTION(Server, Reliable)
		void DestroyBones_Serv(const TArray<FUEGoreSystemHit>& Bones, const float Force);
	UFUNCTION(NetMulticast, Reliable)
		void DestroyBones_Multi(const TArray<FUEGoreSystemHit>& Bones, const float Force);
	UFUNCTION()
		void Int_DestroyBones(const TArray<FUEGoreSystemHit> Bones, const float Force);
	UFUNCTION()
		void Int_BoneHide(const FName Bone, USkeletalMeshComponent* MeshComp);

	/** Destroy a single bone, spawning VFXs and SFXs [THIS WILL NOT AFFECT LIMBS HEALTH]
	* >Bone - BoneName, Location and Normal to use for this Destroy event. Normally these informations are gathered from a trace or hit event depending on your damage system!
	* >InForce - Force to apply on detachment
	* @return true if detach happened, false if not;
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		bool DestroyBone(const FUEGoreSystemHit InBone, const float InForce = 3000.0f);

	UFUNCTION()
		class UPhysicsAsset* ClonePhysicsAsset(const UPhysicsAsset* PhysicsAssetToClone, const FName RemoveBefore);

	/** Damage multiple bones with multiple damage values, this function will only modify the health value stored in the system
	* No VFXs or SFXs involved
	* >BonesDamage - Double Array pairing BoneName and Damage, you should use this variable as a TMap
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void DamageBonesMultipleDamages(const FUEGoreSystemHealthValues BonesDamage);
	/** Damage multiple bones, this function will only modify the health value stored in the system
	* No VFXs or SFXs involved
	* >InBones - List of bones to damage
	* >InDamage - Damage to apply to each bone
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void DamageBonesSingleDamage(const TArray<FName> InBones, const float InDamage);

	UFUNCTION(Server, Reliable)
		void DamageBones_Serv(const FUEGoreSystemHealthValues BonesDamage);
	/** Damage a single bone, this function will only modify the health value stored in the system
	* No VFXs or SFXs involved
	* >InBoneName - Bone to damage
	* >InDamage - Input damage to use
	* @NewBoneHealth - Return the new health of this bone after damage has been appied
	* @DamageApplied - Return the total amount of damage appied
	* @return true if damage has been applied, false if not
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		bool DamageBone(const FName InBoneName, const float InDamage, float& NewBoneHealth, float& DamageApplied);

	/** Damage a single bone
	* VFXs and SFXs involved
	* >Damage - Input damage to use
	* >IsDead - The character is dead and the system can proceed on detaching any limb on the list?
	* >DetachmentForce - Force to apply on detachment
	* >BoneHit - BoneName, Location and Normal to use for this Destroy event. Normally these informations are gathered from a trace or hit event depending on your damage system!
	* @NewBoneHealth - Return the new health of this bone after damage has been appied
	* @DamageApplied - Return the total amount of damage appied
	* @return true if damage has been applied, false if not
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem", BlueprintAuthorityOnly)
		bool ApplyGoreDamage(const float Damage, const bool IsDead, const float DetachmentForce, const FUEGoreSystemHit BoneHit, float& NewBoneHealth, float& DamageApplied);

	/** Destroy all the additional skeletal components spawned from this system
	* >KeepDecals - Should skip blood decals?
	*/
	UFUNCTION(BlueprintCallable, Category = "GoreSystem")
		void RemoveAllSpawnedComponents(const bool KeepDecals);
	UFUNCTION(Server, Reliable)
		void RemoveAllSpawnedComponents_Server(const bool KeepDecals);
	UFUNCTION(NetMulticast, Reliable)
		void RemoveAllSpawnedComponents_Multi(const bool KeepDecals);

	/**[Internal/Legacy] Funtion called to spawn decal when a particle hits something (NO NIAGARA SUPPORT!!!)*/
	UFUNCTION()
		void SpawnDecalOnParticleHit(
			FName EventName,
			float EmitterTime,
			int32 ParticleTime,
			FVector Location,
			FVector Velocity,
			FVector Direction,
			FVector Normal,
			FName BoneName,
			UPhysicalMaterial* PhysMat);

	/**[HINT] Called when a bone has been broken*/
	UPROPERTY(BlueprintAssignable, Category = "GoreSystem")
		FOnBoneBroken On_BoneBroken;
};
