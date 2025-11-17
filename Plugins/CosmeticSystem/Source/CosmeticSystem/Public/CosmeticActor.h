// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "CosmeticSkeletalMeshComponent.h"
#include "CosmeticGroomComponent.h"
#include "SkeletalMergingLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "CosmeticActor.generated.h"

class USkeletalMeshComponent;

UCLASS(Blueprintable, ClassGroup = (Cosmetic))
class COSMETICSYSTEM_API ACosmeticActor : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ACosmeticActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	USkeletalMeshComponent* GetVisualMesh() const { return Mesh; }

	USkeletalMeshComponent* GetMergeTargetSkeletalMesh(FName ComponentTag);

	UCosmeticSkeletalMeshComponent* CreateInstanceSkeletalMesh(FGameplayTag PartTag, FName SocketName,
		bool bUseMasterPoseComponent, TSubclassOf<UCosmeticSkeletalMeshComponent> OverrideSkeletalMeshComponent = nullptr);

	UCosmeticGroomComponent* CreateInstanceGroomAsset(FGameplayTag PartTag, FName SocketName,
		TSubclassOf<UCosmeticGroomComponent> OverrideGroomComponent = nullptr);

	// force the merge function to be turned off
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "CosmeticSystem Actor")
	bool IsEnableMerge();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "CosmeticSystem Actor")
	void OnChangedSkeletalMesh();

	bool MergeCosmeticMeshes(FSkeletalMeshMergeParams MergeParams, UPhysicsAsset* DefaultPhysicsAsset, USkeletalMeshComponent* Target);

	void ClearCreatedMergeTargetMeshes();

	UPhysicsAsset* MergePhysicsAssets(TArray<UPhysicsAsset*> PhysicsAssets, USkeletalMeshComponent* Target);

	virtual FGenericTeamId GetGenericTeamId() const override;

protected:
	virtual void PreInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostInitializeComponents() override;

	bool CreateSkeletalBodySetup(UPhysicsAsset* PhysAsset, USkeletalBodySetup* SKBSetup);

public:
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCosmeticSkeletalMeshComponent> Mesh;

	UPROPERTY()
	TMap<FName, TObjectPtr<UCosmeticSkeletalMeshComponent>> CreatedMergeTargetMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialize|CosmeticActor", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCosmeticSkeletalMeshComponent> CosmeticSkeletalMeshComponentType = UCosmeticSkeletalMeshComponent::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initialize|CosmeticActor", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCosmeticGroomComponent> CosmeticGroomComponentType = UCosmeticGroomComponent::StaticClass();
};
