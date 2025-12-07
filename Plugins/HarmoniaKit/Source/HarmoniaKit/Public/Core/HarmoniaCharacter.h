// Copyright 2025 Snow Game Studio.

#pragma once 

#include "Character/LyraCharacter.h"
#include "NativeGameplayTags.h"
#include "Utility/AlsGameplayTags.h"
#include "HarmoniaCharacter.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_Status_Aiming);

class UHarmoniaCharacterMovementComponent;
class USenseReceiverComponent;
class UHarmoniaLockOnTargetingComponent;
class UHarmoniaHealthComponent;

USTRUCT()
struct FHandIK
{
	GENERATED_BODY()

public:
	bool bUseHandIK = false;

	FTransform TargetTransform = FTransform::Identity;
};

UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaCharacter : public ALyraCharacter
{
	GENERATED_BODY()

public:
	AHarmoniaCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter  = "Als.OverlayMode"))
	void SetDesiredOverlayMode(const FGameplayTag& NewModeTag);

	UFUNCTION(Server, Reliable, Category = "Harmonia Character")
	void ServerSetDesiredOverlayMode(const FGameplayTag& NewModeTag);

protected:
	void AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnRep_DesiredOverlayMode(const FGameplayTag& PreviousMode);

	void PreOverlayModeChanged(const FGameplayTag& PreviousMode);
	void PostOverlayModeChanged();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnEquipAnimation(bool bStart);
	void OnUnEquipAnimation(bool bStart);
	 
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	void SetRightHandIK(FTransform IKTransform);

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	FTransform GetRightHandIK();

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	void SetUseRightHandIK(bool bUseIK);

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	bool GetUseRightHandIK();

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	void SetLeftHandIK(FTransform IKTransform);

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	FTransform GetLeftHandIK();

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	void SetUseLeftHandIK(bool bUseIK);

	UFUNCTION(BlueprintCallable, Category = "Harmonia Character|Hand IK")
	bool GetUseLeftHandIK();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Harmonia Character|Desired State", meta=(EditCondition="bOverrideDefaultOverlay"))
	FGameplayTag OverrideDefaultOverlayMode{ AlsOverlayModeTags::Default };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_DesiredOverlayMode, Category = "Settings|Harmonia Character|Desired State")
	FGameplayTag DesiredOverlayMode = AlsOverlayModeTags::Default;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Harmonia Character|Desired State")
	FGameplayTag PreviousOverlayMode = AlsOverlayModeTags::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Harmonia Character|Desired State")
	bool bOverrideDefaultOverlay = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Harmonia Character")
	bool bChangingOverlayMode = false;

	FHandIK RightHandIK;

	FHandIK LeftHandIK;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia Character")
	TObjectPtr<UHarmoniaCharacterMovementComponent> HarmoniaCharacterMovement = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harmonia Character|Combat")
	TObjectPtr<UHarmoniaLockOnTargetingComponent> LockOnComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harmonia Character|Health")
	TObjectPtr<UHarmoniaHealthComponent> HarmoniaHealthComponent = nullptr;

public:
	UFUNCTION(BlueprintPure, Category = "Harmonia Character|Combat")
	UHarmoniaLockOnTargetingComponent* GetLockOnComponent() const { return LockOnComponent; }

	UFUNCTION(BlueprintPure, Category = "Harmonia Character|Health")
	UHarmoniaHealthComponent* GetHarmoniaHealthComponent() const { return HarmoniaHealthComponent; }
};
