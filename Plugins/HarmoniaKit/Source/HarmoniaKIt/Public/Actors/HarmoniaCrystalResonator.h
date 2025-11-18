// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/HarmoniaInteractableInterface.h"
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
 * 크리스탈 공명기 - 체크포인트 액터
 *
 * 모닥불 대신 마법 크리스탈이 공명하며 에너지를 방출하는 독창적인 체크포인트 시스템
 *
 * 주요 기능:
 * - 상호작용으로 체크포인트 활성화
 * - 공명(Resonance) 시스템 - 휴식하여 회복
 * - 각 크리스탈마다 고유한 "공명 주파수" (색상/음향)
 * - 체크포인트 강화 시스템
 * - 텔레포트 기능 (공명 네트워크)
 *
 * 독창적 요소:
 * - 비활성화 상태: 어둡게 빛나는 잠든 크리스탈
 * - 활성화 상태: 밝게 공명하며 에너지를 방출
 * - 공명 주파수별 고유한 색상/음향/효과
 * - 가까운 크리스탈들끼리 공명 효과 발생
 * - 공명 잔향 - 마지막으로 공명한 위치 표시
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

	/** 루트 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 크리스탈 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CrystalMesh;

	/** 받침대 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	/** 포인트 라이트 (공명 효과) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> ResonanceLight;

	/** 파티클 시스템 (공명 효과) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ResonanceEffect;

	/** 상호작용 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	/** 오디오 컴포넌트 (공명 소리) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> ResonanceAudio;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** 체크포인트 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	/** 체크포인트 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	/** 체크포인트 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (MultiLine = true))
	FText CheckpointDescription;

	/** 공명 주파수 (색상/특성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	/** 시작 시 자동 활성화 여부 (디버그용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Debug")
	bool bStartActivated = false;

	/** 상호작용 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float InteractionRange = 200.0f;

	// ============================================================================
	// Visual Configuration
	// ============================================================================

	/** 비활성화 상태 발광 세기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float InactiveLightIntensity = 500.0f;

	/** 활성화 상태 발광 세기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ActiveLightIntensity = 2000.0f;

	/** 공명 상태 발광 세기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonatingLightIntensity = 5000.0f;

	/** 공명 주파수별 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	TMap<EHarmoniaResonanceFrequency, FLinearColor> FrequencyColors;

	/** 공명 펄스 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Visual")
	float ResonancePulseSpeed = 2.0f;

	// ============================================================================
	// Audio Configuration
	// ============================================================================

	/** 활성화 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ActivationSound;

	/** 공명 시작 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceStartSound;

	/** 공명 루프 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceLoopSound;

	/** 공명 완료 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> ResonanceCompleteSound;

	/** 텔레포트 도착 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Audio")
	TObjectPtr<USoundBase> TeleportArrivalSound;

	// ============================================================================
	// State
	// ============================================================================

	/** 현재 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State", Replicated)
	EHarmoniaCheckpointState CurrentState = EHarmoniaCheckpointState::Inactive;

	/** 현재 공명 중인 플레이어들 */
	UPROPERTY(BlueprintReadOnly, Category = "Checkpoint|State")
	TArray<TObjectPtr<APlayerController>> ResonatingPlayers;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * 체크포인트 활성화
	 * @param Player 활성화하는 플레이어
	 * @return 활성화 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool Activate(APlayerController* Player);

	/**
	 * 공명 시작 (휴식/회복)
	 * @param Player 공명하는 플레이어
	 * @return 공명 시작 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	bool StartResonance(APlayerController* Player);

	/**
	 * 공명 종료
	 * @param Player 공명 중인 플레이어
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void EndResonance(APlayerController* Player);

	/**
	 * 상태 변경
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void SetCheckpointState(EHarmoniaCheckpointState NewState);

	/**
	 * 체크포인트 데이터 가져오기
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FHarmoniaCheckpointData GetCheckpointData() const;

	/**
	 * 체크포인트 데이터 적용
	 */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void ApplyCheckpointData(const FHarmoniaCheckpointData& Data);

	/**
	 * 활성화 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsActivated() const { return CurrentState != EHarmoniaCheckpointState::Inactive; }

	/**
	 * 공명 중인지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsResonating() const { return CurrentState == EHarmoniaCheckpointState::Resonating; }

	/**
	 * 플레이어가 공명 중인지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	bool IsPlayerResonating(APlayerController* Player) const;

	/**
	 * 공명 주파수 색상 가져오기
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
	 * 체크포인트 서브시스템 등록
	 */
	void RegisterToSubsystem();

	/**
	 * 비주얼 업데이트
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateVisuals();
	virtual void UpdateVisuals_Implementation();

	/**
	 * 공명 펄스 효과
	 */
	void UpdateResonancePulse(float DeltaTime);

	/**
	 * 가까운 크리스탈과 공명 효과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void UpdateNearbyResonance();
	virtual void UpdateNearbyResonance_Implementation();

	/**
	 * 공명 주파수별 특수 효과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void ApplyFrequencyEffects();
	virtual void ApplyFrequencyEffects_Implementation();

	/**
	 * 활성화 비주얼 효과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayActivationEffects();
	virtual void PlayActivationEffects_Implementation();

	/**
	 * 공명 시작 비주얼 효과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceStartEffects();
	virtual void PlayResonanceStartEffects_Implementation();

	/**
	 * 공명 완료 비주얼 효과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Checkpoint")
	void PlayResonanceCompleteEffects();
	virtual void PlayResonanceCompleteEffects_Implementation();

	/**
	 * 초기 색상 설정
	 */
	void InitializeFrequencyColors();

private:
	/** 공명 펄스 타이머 */
	float ResonancePulseTimer = 0.0f;

	/** 마지막 공명 시간 */
	FDateTime LastResonanceTime;

	/** 강화 레벨 */
	TMap<EHarmoniaCheckpointUpgradeType, int32> UpgradeLevels;

#if WITH_EDITORONLY_DATA
	/** 에디터에서 체크포인트 ID 자동 생성 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
