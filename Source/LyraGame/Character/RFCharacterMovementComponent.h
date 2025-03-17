// Copyright 2024 RedFlowering.

#pragma once 

#include "LyraCharacterMovementComponent.h"
#include "RFCharacterMovementComponent.generated.h"

class ARFCharacter;

UENUM()
enum class ERFMovementMode : uint8
{
	MOVE_None		UMETA(DisplayName = "None"),
	MOVE_Walking	UMETA(DisplayName = "Walking"),
	MOVE_NavWalking	UMETA(DisplayName = "Navmesh Walking"),
	MOVE_Falling	UMETA(DisplayName = "Falling"),
	MOVE_Swimming	UMETA(DisplayName = "Swimming"),
	MOVE_Flying		UMETA(DisplayName = "Flying"),
	MOVE_Custom		UMETA(DisplayName = "Custom"),
	MOVE_MAX		UMETA(Hidden),
};

UENUM()
enum class ERFCustomMovementMode : uint8
{
	MOVE_None				UMETA(DisplayName = "None"),
	MOVE_GrapplingHook		UMETA(DisplayName = "GrapplingHook"),
	MOVE_Climbing			UMETA(DisplayName = "Climbing"),
};

UCLASS(Config = Game)
class LYRAGAME_API URFCharacterMovementComponent : public ULyraCharacterMovementComponent
{
	GENERATED_BODY()

public:
	URFCharacterMovementComponent();

public:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;

	ERFMovementMode GetMovementMode();

	ERFCustomMovementMode GetCustomMovementMode();

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult = nullptr);

protected:
	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation) override;

	virtual void PhysicsRotation(float DeltaTime);

	virtual void PhysWalking(float DeltaTime, int32 IterationsCount) override;

	virtual void PhysNavWalking(float DeltaTime, int32 IterationsCount) override;

	virtual void PhysCustom(float DeltaTime, int32 IterationsCount) override;

	virtual void PhysGrpplingHook(float DeltaTime, int32 IterationCount);

	virtual void PhysClimbing(float DeltaTime, int32 IterationCount);

public:
	UFUNCTION(BlueprintCallable, Category = "RFMovementCompontnt|GrapplingHook")
	FVector GetGrapplingHookMovmentVector();

	UFUNCTION(BlueprintCallable, Category = "RFMovementCompontnt|GrapplingHook")
	void SetGrapplingHookMovementVector(FVector GrapplingVector);

	UFUNCTION(BlueprintCallable, Category = "RFMovementCompontnt|Climbing")
	FVector GetClimbingMovementVector();

	UFUNCTION(BlueprintCallable, Category = "RFMovementCompontnt|Climbing")
	void SetClimbingMovementVector(FVector ClimbVector);

protected:
	UPROPERTY()
	TObjectPtr<ARFCharacter> OwnerCharacter = nullptr;
	ERFMovementMode RFMovementMode = ERFMovementMode::MOVE_Walking;

	ERFCustomMovementMode RFCustomMovementMode = ERFCustomMovementMode::MOVE_None;

private:
	FVector GrapplingHookVector = FVector::ZeroVector;

	FVector ClimbingVector = FVector::ZeroVector;
};