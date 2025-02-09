// Copyright 2024 RedFlowering.

#pragma once 

#include "LyraCharacterWithAbilities.h"
#include "NativeGameplayTags.h"
#include "RFCharacter.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_Status_Aiming);

class URFCharacterMovementComponent;

USTRUCT()
struct FHandIK
{
	GENERATED_BODY()

public:
	bool bUseHandIK = false;

	FTransform TargetTransform = FTransform::Identity;
};

UCLASS(Blueprintable)
class LYRAGAME_API ARFCharacter : public ALyraCharacterWithAbilities
{
	GENERATED_BODY()

public:
	ARFCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "RF Character")
	const FGameplayTag GetDefaultOverlayMode() const { return OverrideDefaultOverlayMode; }

	UFUNCTION(BlueprintCallable, Category = "RF Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter  = "Als.OverlayMode"))
	void SetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast = true);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Als Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter = "Als.OverlayMode"))
	void ServerSetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Als Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter = "Als.OverlayMode"))
	void MulticastSetDesiredOverlayMode(const FGameplayTag& NewModeTag);

protected:
	void AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	bool TrySetDesiredOverlayMode(const FGameplayTag& NewModeTag);

	void PreOverlayModeChanged(const FGameplayTag& PreviousMode);
	void PostOverlayModeChanged();

public:
	void OnEquipAnimation(bool bStart);
	void OnUnEquipAnimation(bool bStart);
	 
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	void SetRightHandIK(FTransform IKTransform);

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	FTransform GetRightHandIK();

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	void SetUseRightHandIK(bool bUseIK);

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	bool GetUseRightHandIK();

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	void SetLeftHandIK(FTransform IKTransform);

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	FTransform GetLeftHandIK();

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	void SetUseLefttHandIK(bool bUseIK);

	UFUNCTION(BlueprintCallable, Category = "RF Character|Hand IK")
	bool GetUseLeftHandIK();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|RF Character|Desired State", meta=(EditCondition="bOverrideDefaultOverlay"))
	FGameplayTag OverrideDefaultOverlayMode{ AlsOverlayModeTags::Default };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings|RF Character|Desired State")
	FGameplayTag DesiredOverlayMode = AlsOverlayModeTags::Default;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Settings|RF Character|Desired State")
	FGameplayTag PreviousOverlayMode = AlsOverlayModeTags::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|RF Character|Desired State")
	bool bOverrideDefaultOverlay = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "RF Character")
	bool bChangingOverlayMode = false;

	FHandIK RightHandIK;

	FHandIK LeftHandIK;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RF Character")
	TObjectPtr<URFCharacterMovementComponent> RFCharacterMovement = nullptr;
};

