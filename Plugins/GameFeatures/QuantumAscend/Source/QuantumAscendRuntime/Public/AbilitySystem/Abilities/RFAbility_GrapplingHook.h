// Copyright 2025 RedFlowering.

#pragma once 

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "RFAbility_GrapplingHook.generated.h"

class ARFCharacter;
class URFCharacterMovementComponent;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrappleInitEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHookedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveReadyEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveStartEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovingEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMovedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveEndEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrappleEndEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnedHookEvent, FVector, TargetLocation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTickDelegate, float, DeltaTime);

UENUM()
enum class EGrapplingHookMoveMode : uint8
{
	// None = 0,
	Teleport,	// Blink Effect + Teleport
	Immersive,	// Lerp
	Blink,	// Blink Effect + Teleport + Immersive Move
};

UENUM()
enum class EGrapplingTraceShape : uint8
{
	None = 0,
	LineTrace,
	CapsuleTrace,
	BoxTrace,
};

// Since there is no delay applied to the grappling skill, the current grappleend and idle states can be considered the same.
UENUM()
enum class EGrappleStep : uint8
{
	None = 0,
	Idle,
	Hooked,
	MoveReady,	// Hooked to Target
	MoveStart,	// Move Init
	Moving,		// Moving
	Moved,		// Moved
	MoveEnd,	// MoveEnd
	GrappleEnd,	// Grapple Init
};

USTRUCT(BlueprintType)
struct FGrapplingHookSetup
{
	GENERATED_BODY()

	// Setup
	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Setup")
	TSubclassOf<AActor> RopeActorClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	TObjectPtr<AActor> CachedRopeActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Setup")
	TSubclassOf<AActor> HookActorClass = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	TObjectPtr<AActor> CachedHookActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Setup")
	TObjectPtr<AActor> CachedHookShooter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float GrappleMinDistance = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float GrappleMaxDistance = 3725.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float AutoHookDelay = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float GrapplingHookSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float GrapplingHookDecelerationFactor = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float BlinkPreTeleportTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float BlinkTeleportHoldTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	float BlinkPostTeleportTime = 0.2f;

	// Movement Method (Blink, Immersive..)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Setup")
	EGrapplingHookMoveMode GrapplingMovementMode = EGrapplingHookMoveMode::Immersive;
};

USTRUCT(BlueprintType)
struct FGrapplingHookTrace
{
	GENERATED_BODY()

	// Trace
	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Trace")
	EGrapplingTraceShape CollisionShape = EGrapplingTraceShape::LineTrace;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Trace|Debug")
	bool DebugMode = false;

	UPROPERTY(EditAnywhere, Category = "GrapplingHook|Trace|Debug")
	float DebugTraceLifeTime = 5.0f;
};

USTRUCT()
struct FGrapplingHookMove
{
	GENERATED_BODY()

	UPROPERTY()
	EGrappleStep Step = EGrappleStep::Idle;

	UPROPERTY()
	FVector LastDistance = FVector::ZeroVector;

	UPROPERTY()
	float TeleportStartTime = -1.0f;
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
    UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HideFromLevelInfos, DefaultToSelf="OwningAbility"))
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

	void TickAbility(float DeltaTime);

	// Function
public:
	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	void ChangeGrapplingHookMode(EGrapplingHookMoveMode Mode);

	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	bool IsGrapplingIdle();

	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	void SetGrapplingReady();

	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	bool GetGrapplingReady();

	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	FVector GetTargetLocation();

	// Movement
	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	void ShootGrapplingHook(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	void CancelGrapplingHook();

	UFUNCTION(BlueprintCallable, Category = "GrapplingHook")
	const FGrapplingHookSetup GetHookSetup();

protected:
	UFUNCTION()
	bool PerformTrace();

protected:
	UFUNCTION()
	void SpawnGrapplingHookActor();

	UFUNCTION(Server, Reliable)
	void ServerSpawnGrapplingHookActor(FVector TargetLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnGrapplingHookActor(FVector TargetLocation);

	UFUNCTION()
	void ReleaseGrapplingHook();

	UFUNCTION(Server, Reliable)
	void ServerReleaseGrapplingHook();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReleaseGrapplingHook();

	UFUNCTION()
	void InitGrapplingHook();

	// Teleport Movement
protected:
	UFUNCTION()
	void TeleportMovement();

	UFUNCTION()
	void TeleportToTarget();

	UFUNCTION(Server, Reliable)
	void ServerTeleportToTarget(FVector TargetLocation);

	// Immersive Movement
protected:
	UFUNCTION()
	void ImmersiveMovement(float DeltaTime);

	UFUNCTION()
	void ImmersiveMoveToTarget(float DeltaTime);

	// Blink Movement
protected:
	UFUNCTION()
	void BlinkMovement(float DeltaTime);

	UFUNCTION()
	void PreMovement(float DeltaTime);

	UFUNCTION()
	void BlinkTeleportMovement();

	UFUNCTION()
	void PostMovement(float DeltaTime);

protected:
	UPROPERTY()
	TObjectPtr<ARFCharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<URFCharacterMovementComponent> OwnerMovementComponent = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> Target = nullptr;

protected:
	UPROPERTY()
	FGrapplingHookMove HookMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook", meta = (AllowPrivateAccess = "true"))
	FGrapplingHookSetup HookSetup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrapplingHook|Trace", meta = (AllowPrivateAccess = "true"))
	FGrapplingHookTrace HookTrace;

public:
	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnGrappleInitEvent OnGrappleInit;

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnHookedEvent OnHooked;

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnMoveReadyEvent OnMoveReady;	// MoveReady

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnMoveStartEvent OnMoveStart;	// MoveStart

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnMovingEvent OnMoving;		// Moving

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnMovedEvent OnMoved;		// MoveEnd

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnMoveEndEvent OnMoveEnd;				// Init

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnGrappleEndEvent OnGrappleEnd;

	UPROPERTY(BlueprintAssignable, Category = "GrapplingHook")
	FOnSpawnedHookEvent OnSpawnedHook;
};

