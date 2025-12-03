// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "HarmoniaCrystalResonator.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UParticleSystemComponent;
class UNiagaraComponent;
class USphereComponent;
class UAudioComponent;

/**
 * AHarmoniaCrystalResonator
 *
 * ?�리?�탈 공명�?- 체크?�인???�터
 *
 * 모닥�??�??마법 ?�리?�탈??공명?�며 ?�너지�?방출?�는 ?�창?�인 체크?�인???�스??
 *
 * 주요 기능:
 * - ?�호?�용?�로 체크?�인???�성??
 * - 공명(Resonance) ?�스??- ?�식?�여 ?�복
 * - �??�리?�탈마다 고유??"공명 주파?? (?�상/?�향)
 * - 체크?�인??강화 ?�스??
 * - ?�레?�트 기능 (공명 ?�트?�크)
 *
 * ?�창???�소:
 * - 비활?�화 ?�태: ?�둡�?빛나???�든 ?�리?�탈
 * - ?�성???�태: 밝게 공명?�며 ?�너지�?방출
 * - 공명 주파?�별 고유???�상/?�향/?�과
 * - 가까운 ?�리?�탈?�끼�?공명 ?�과 발생
 * - 공명 ?�향 - 마�?막으�?공명???�치 ?�시
 */
UCLASS(Blueprintable, ClassGroup = "Harmonia")
class HARMONIAKIT_API AHarmoniaCrystalResonator : public AActor, public IHarmoniaInteractableInterface
{
	GENERATED_BODY()

public:
	AHarmoniaCrystalResonator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ============================================================================
	// Components
	// ============================================================================

	/** 루트 컴포?�트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** ?�리?�탈 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CrystalMesh;

	/** 받침?� 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	/** ?�인???�이??(공명 ?�과) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> ResonanceLight;

	/** ?�티???�스??(공명 ?�과) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ResonanceEffect;

	/** ?�호?�용 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	/** ?�디??컴포?�트 (공명 ?�리) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> ResonanceAudio;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** 체크?�인??고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	/** 체크?�인???�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	/** 체크?�인???�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (MultiLine = true))
	FText CheckpointDescription;

	/** 공명 주파??(?�상/?�성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	/** ?�작 ???�동 ?�성???��? (?�버그용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Debug")
	bool bStartActivated = false;

	/** ?�호?�용 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float InteractionRange = 200.0f;

	// ============================================================================
	// Visual Configuration
	// ============================================================================

	/** 비활?�화 ?�태 발광 ?�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float InactiveLightIntensity = 500.0f;

	/** ?�성???�태 발광 ?�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ActiveLightIntensity = 2000.0f;

	/** 공명 ?�태 발광 ?�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonatingLightIntensity = 5000.0f;

	/** 공명 주파?�별 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	TMap<EHarmoniaResonanceFrequency, FLinearColor> FrequencyColors;

	/** 공명 ?�스 ?�도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonancePulseSpeed = 2.0f;

	// ============================================================================
	// Audio Configuration
	// ============================================================================

	/** ?�성???�운??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ActivationSound;

	/** 공명 ?�작 ?�운??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceStartSound;

	/** 공명 루프 ?�운??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceLoopSound;

	/** 공명 ?�료 ?�운??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceCompleteSound;

	/** ?�레?�트 ?�착 ?�운??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> TeleportArrivalSound;

	// ============================================================================
	// State
	// ============================================================================

	/** ?�재 ?�태 */
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State", Replicated)
	EHarmoniaCheckpointState CurrentState = EHarmoniaCheckpointState::Inactive;

	/** ?�재 공명 중인 ?�레?�어??*/
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State")
	TArray<TObjectPtr<APlayerController>> ResonatingPlayers;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * 체크?�인???�성??
	 * @param Player ?�성?�하???�레?�어
	 * @return ?�성???�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool Activate(APlayerController* Player);

	/**
	 * 공명 ?�작 (?�식/?�복)
	 * @param Player 공명?�는 ?�레?�어
	 * @return 공명 ?�작 ?�공 ?��?
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool StartResonance(APlayerController* Player);

	/**
	 * 공명 종료
	 * @param Player 공명 중인 ?�레?�어
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void EndResonance(APlayerController* Player);

	/**
	 * ?�태 변�?
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void SetCheckpointState(EHarmoniaCheckpointState NewState);

	/**
	 * 체크?�인???�이??가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FHarmoniaCheckpointData GetCheckpointData() const;

	/**
	 * 체크?�인???�이???�용
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void ApplyCheckpointData(const FHarmoniaCheckpointData& Data);

	/**
	 * ?�성???��?
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsActivated() const { return CurrentState != EHarmoniaCheckpointState::Inactive; }

	/**
	 * 공명 중인지 ?�인
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsResonating() const { return CurrentState == EHarmoniaCheckpointState::Resonating; }

	/**
	 * ?�레?�어가 공명 중인지 ?�인
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsPlayerResonating(APlayerController* Player) const;

	/**
	 * 공명 주파???�상 가?�오�?
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FLinearColor GetFrequencyColor() const;

	// ============================================================================
	// Interaction Interface
	// ============================================================================

	virtual void OnInteract_Implementation(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult) override;

	// ============================================================================
	// Replication
	// ============================================================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * 체크?�인???�브?�스???�록
	 */
	void RegisterToSubsystem();

	/**
	 * 비주???�데?�트
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateVisuals();
	virtual void UpdateVisuals_Implementation();

	/**
	 * 공명 ?�스 ?�과
	 */
	void UpdateResonancePulse(float DeltaTime);

	/**
	 * 가까운 ?�리?�탈�?공명 ?�과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateNearbyResonance();
	virtual void UpdateNearbyResonance_Implementation();

	/**
	 * 공명 주파?�별 ?�수 ?�과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void ApplyFrequencyEffects();
	virtual void ApplyFrequencyEffects_Implementation();

	/**
	 * ?�성??비주???�과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayActivationEffects();
	virtual void PlayActivationEffects_Implementation();

	/**
	 * 공명 ?�작 비주???�과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceStartEffects();
	virtual void PlayResonanceStartEffects_Implementation();

	/**
	 * 공명 ?�료 비주???�과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceCompleteEffects();
	virtual void PlayResonanceCompleteEffects_Implementation();

	/**
	 * 초기 ?�상 ?�정
	 */
	void InitializeFrequencyColors();

private:
	/** 공명 ?�스 ?�?�머 */
	float ResonancePulseTimer = 0.0f;

	/** 마�?�?공명 ?�간 */
	FDateTime LastResonanceTime;

	/** 강화 ?�벨 */
	TMap<EHarmoniaCheckpointUpgradeType, int32> UpgradeLevels;

#if WITH_EDITORONLY_DATA
	/** ?�디?�에??체크?�인??ID ?�동 ?�성 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
