// Copyright 2025 RedFlowering.

#pragma once 

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "RFAbility_GrapplingHook.generated.h"

class ARFCharacter;
class ARopeActor;
class AHookActor;
class URFCharacterMovementComponent;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTickDelegate, float, DeltaTime);

UENUM()
enum class EGrapplingTraceShape : uint8
{
	None = 0,
	Line,
	Box,
	Capsule,
};

UENUM()
enum class EGrappleStep : uint8
{
	None = 0,
	Idle,
	Ready,
	Start,
	Finish,
};

UCLASS()
class QUANTUMASCENDRUNTIME_API URFAbilityTask_WaitTick : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 매 틱마다 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FWaitTickDelegate OnTick;

	// AbilityTask 생성 함수 (Ability에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HideFromLevelInfos, DefaultToSelf = "OwningAbility"))
	static URFAbilityTask_WaitTick* WaitTick(UGameplayAbility* OwningAbility);

	// AbilityTask 활성화 시 호출
	virtual void Activate() override;

	// 매 프레임 호출되는 함수 (Ability가 활성화된 동안)
	virtual void TickTask(float DeltaTime) override;

protected:
	// Ability 종료 시 호출 (Tick 중지 등)
	virtual void OnDestroy(bool AbilityEnded) override;
};

UCLASS(Abstract)
class QUANTUMASCENDRUNTIME_API URFAbility_GrapplingHook : public ULyraGameplayAbility
{
	GENERATED_BODY()
public:
	URFAbility_GrapplingHook(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void TickAbility(float DeltaTime);

protected:
	UPROPERTY()
	TObjectPtr<ARFCharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<URFCharacterMovementComponent> OwnerMovementComponent = nullptr;

public:
	UFUNCTION()
	bool PerformTrace(FVector TargetPos);

	UFUNCTION()
	void ShootGrapplingHook(FVector TargetPos);

	UFUNCTION(Server, Reliable)
	void ServerShootGrpplingHook(FVector TargetPos);

	UFUNCTION()
	void ReleaseGrpplingHook();

	UFUNCTION(Server, Reliable)
	void ServerReleaseGrapplingHook();

	UFUNCTION()
	void CancelGrapplingHook();

	UFUNCTION()
	void OnHookArrivedHandler();

	UFUNCTION()
	void SwingMovement(FVector TargetPos);

	UFUNCTION()
	void StartMoveToTarget();

public:
	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Setup")
	TSubclassOf<ARopeActor> RopeActorClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	TObjectPtr<ARopeActor> CachedRopeActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Setup")
	TSubclassOf<AHookActor> HookActorClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	TObjectPtr<AHookActor> CachedHookActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Trace")
	EGrapplingTraceShape CollisionShape = EGrapplingTraceShape::Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Trace")
	float TraceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Trace|Debug")
	bool UseDebugMode = false;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Trace|Debug")
	float DebugTraceLifeTime = 0.5f;

	FVector GrapplingTargetLocation = FVector::ZeroVector;
	float GrapplingTargetDistance = 0.0f;


protected:
	UPROPERTY()
	EGrappleStep Step = EGrappleStep::Idle;
};

