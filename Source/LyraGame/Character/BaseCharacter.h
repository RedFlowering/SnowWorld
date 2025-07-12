// Copyright 2024 Snow Game Studio.

#pragma once 

#include "LyraCharacterWithAbilities.h"
#include "NativeGameplayTags.h"
#include "Utility/AlsGameplayTags.h"

#include "BaseCharacter.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_Status_Aiming);

class UBaseCharacterMovementComponent;
class USenseReceiverComponent;

USTRUCT()
struct FHandIK
{
	GENERATED_BODY()

public:
	bool bUseHandIK = false;

	FTransform TargetTransform = FTransform::Identity;
};

UCLASS(Blueprintable)
class LYRAGAME_API ABaseCharacter : public ALyraCharacterWithAbilities
{
	GENERATED_BODY()

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable, Category = "Base Character")
	const FGameplayTag GetDefaultOverlayMode() const { return OverrideDefaultOverlayMode; }

	UFUNCTION(BlueprintCallable, Category = "Base Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter  = "Als.OverlayMode"))
	void SetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast = true);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Base Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter = "Als.OverlayMode"))
	void ServerSetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "Base Character", Meta = (AutoCreateRefTerm = "NewModeTag", GameplayTagFilter = "Als.OverlayMode"))
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

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	void SetRightHandIK(FTransform IKTransform);

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	FTransform GetRightHandIK();

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	void SetUseRightHandIK(bool bUseIK);

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	bool GetUseRightHandIK();

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	void SetLeftHandIK(FTransform IKTransform);

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	FTransform GetLeftHandIK();

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	void SetUseLefttHandIK(bool bUseIK);

	UFUNCTION(BlueprintCallable, Category = "Base Character|Hand IK")
	bool GetUseLeftHandIK();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USenseReceiverComponent> WallDetector = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Base Character|Desired State", meta=(EditCondition="bOverrideDefaultOverlay"))
	FGameplayTag OverrideDefaultOverlayMode{ AlsOverlayModeTags::Default };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Base Character|Desired State")
	FGameplayTag DesiredOverlayMode = AlsOverlayModeTags::Default;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Settings|Base Character|Desired State")
	FGameplayTag PreviousOverlayMode = AlsOverlayModeTags::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Base Character|Desired State")
	bool bOverrideDefaultOverlay = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Base Character")
	bool bChangingOverlayMode = false;

	FHandIK RightHandIK;

	FHandIK LeftHandIK;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Character")
	TObjectPtr<UBaseCharacterMovementComponent> BaseCharacterMovement = nullptr;
};

