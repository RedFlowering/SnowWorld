// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaCrystalResonator.h"
#include "System/HarmoniaCheckpointSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AHarmoniaCrystalResonator::AHarmoniaCrystalResonator()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 루트 컴포넌트
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 받침대 메시
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// 크리스탈 메시
	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(SceneRoot);
	CrystalMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// 포인트 라이트
	ResonanceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ResonanceLight"));
	ResonanceLight->SetupAttachment(CrystalMesh);
	ResonanceLight->SetIntensity(InactiveLightIntensity);
	ResonanceLight->SetAttenuationRadius(1000.0f);
	ResonanceLight->SetCastShadows(false);

	// 나이아가라 이펙트
	ResonanceEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ResonanceEffect"));
	ResonanceEffect->SetupAttachment(CrystalMesh);
	ResonanceEffect->SetAutoActivate(false);

	// 상호작용 범위
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 오디오 컴포넌트
	ResonanceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ResonanceAudio"));
	ResonanceAudio->SetupAttachment(CrystalMesh);
	ResonanceAudio->SetAutoActivate(false);

	// 기본 값 설정
	CheckpointID = NAME_None;
	CheckpointName = FText::FromString(TEXT("Crystal Resonator"));
	CheckpointDescription = FText::FromString(TEXT("A mystical crystal that resonates with ancient energies."));
	ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// 초기 색상 설정
	InitializeFrequencyColors();
}

void AHarmoniaCrystalResonator::BeginPlay()
{
	Super::BeginPlay();

	// 서브시스템에 등록
	RegisterToSubsystem();

	// 시작 시 자동 활성화
	if (bStartActivated && HasAuthority())
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
	}

	// 초기 비주얼 설정
	UpdateVisuals();
}

void AHarmoniaCrystalResonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 공명 펄스 효과
	if (CurrentState == EHarmoniaCheckpointState::Resonating)
	{
		UpdateResonancePulse(DeltaTime);
	}

	// 주변 크리스탈과 공명
	if (CurrentState != EHarmoniaCheckpointState::Inactive)
	{
		UpdateNearbyResonance();
	}
}

void AHarmoniaCrystalResonator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 서브시스템에서 등록 해제
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			CheckpointSubsystem->UnregisterCheckpoint(CheckpointID);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AHarmoniaCrystalResonator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHarmoniaCrystalResonator, CurrentState);
}

// ============================================================================
// Public Functions
// ============================================================================

bool AHarmoniaCrystalResonator::Activate(APlayerController* Player)
{
	if (!Player || !HasAuthority())
	{
		return false;
	}

	if (CurrentState != EHarmoniaCheckpointState::Inactive)
	{
		// 이미 활성화됨
		return true;
	}

	SetCheckpointState(EHarmoniaCheckpointState::Activated);
	PlayActivationEffects();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Checkpoint %s activated by player"), *CheckpointID.ToString());
	return true;
}

bool AHarmoniaCrystalResonator::StartResonance(APlayerController* Player)
{
	if (!Player || !HasAuthority())
	{
		return false;
	}

	if (CurrentState == EHarmoniaCheckpointState::Inactive)
	{
		return false;
	}

	if (!ResonatingPlayers.Contains(Player))
	{
		ResonatingPlayers.Add(Player);
	}

	SetCheckpointState(EHarmoniaCheckpointState::Resonating);
	PlayResonanceStartEffects();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Player started resonance at checkpoint %s"), *CheckpointID.ToString());
	return true;
}

void AHarmoniaCrystalResonator::EndResonance(APlayerController* Player)
{
	if (!Player || !HasAuthority())
	{
		return;
	}

	ResonatingPlayers.Remove(Player);

	if (ResonatingPlayers.Num() == 0)
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
		PlayResonanceCompleteEffects();
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Player ended resonance at checkpoint %s"), *CheckpointID.ToString());
}

void AHarmoniaCrystalResonator::SetCheckpointState(EHarmoniaCheckpointState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	UpdateVisuals();
}

FHarmoniaCheckpointData AHarmoniaCrystalResonator::GetCheckpointData() const
{
	FHarmoniaCheckpointData Data;
	Data.CheckpointID = CheckpointID;
	Data.CheckpointName = CheckpointName;
	Data.CheckpointDescription = CheckpointDescription;
	Data.ResonanceFrequency = ResonanceFrequency;
	Data.Location = GetActorLocation();
	Data.Rotation = GetActorRotation();
	Data.bActivated = IsActivated();
	Data.LastResonanceTime = LastResonanceTime;
	Data.UpgradeLevels = UpgradeLevels;

	return Data;
}

void AHarmoniaCrystalResonator::ApplyCheckpointData(const FHarmoniaCheckpointData& Data)
{
	CheckpointID = Data.CheckpointID;
	CheckpointName = Data.CheckpointName;
	CheckpointDescription = Data.CheckpointDescription;
	ResonanceFrequency = Data.ResonanceFrequency;
	LastResonanceTime = Data.LastResonanceTime;
	UpgradeLevels = Data.UpgradeLevels;

	if (Data.bActivated)
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
	}
	else
	{
		SetCheckpointState(EHarmoniaCheckpointState::Inactive);
	}

	UpdateVisuals();
}

bool AHarmoniaCrystalResonator::IsPlayerResonating(APlayerController* Player) const
{
	return ResonatingPlayers.Contains(Player);
}

FLinearColor AHarmoniaCrystalResonator::GetFrequencyColor() const
{
	const FLinearColor* Color = FrequencyColors.Find(ResonanceFrequency);
	return Color ? *Color : FLinearColor::White;
}

// ============================================================================
// Interaction Interface
// ============================================================================

void AHarmoniaCrystalResonator::OnInteract_Implementation(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult)
{
	OutResult.bSuccess = false;

	APlayerController* PlayerController = Cast<APlayerController>(Context.Instigator);
	if (!PlayerController)
	{
		OutResult.Message = FText::FromString(TEXT("Invalid player"));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>();
	if (!CheckpointSubsystem)
	{
		return;
	}

	// 비활성화 상태면 활성화
	if (CurrentState == EHarmoniaCheckpointState::Inactive)
	{
		bool bActivated = CheckpointSubsystem->ActivateCheckpoint(CheckpointID, PlayerController);
		if (bActivated)
		{
			OutResult.bSuccess = true;
			OutResult.Message = FText::Format(
				FText::FromString(TEXT("Activated {0}")),
				CheckpointName
			);
		}
		return;
	}

	// 활성화 상태면 공명 시작
	bool bResonanceStarted = CheckpointSubsystem->StartResonance(CheckpointID, PlayerController);
	if (bResonanceStarted)
	{
		OutResult.bSuccess = true;
		OutResult.Message = FText::Format(
			FText::FromString(TEXT("Resonating with {0}...")),
			CheckpointName
		);
	}
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaCrystalResonator::RegisterToSubsystem()
{
	if (!HasAuthority())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>();
	if (CheckpointSubsystem)
	{
		CheckpointSubsystem->RegisterCheckpoint(this);
		UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Registered checkpoint %s to subsystem"), *CheckpointID.ToString());
	}
}

void AHarmoniaCrystalResonator::UpdateVisuals_Implementation()
{
	if (!ResonanceLight)
	{
		return;
	}

	// 공명 주파수 색상 적용
	FLinearColor FrequencyColor = GetFrequencyColor();
	ResonanceLight->SetLightColor(FrequencyColor);

	// 상태별 발광 세기
	float TargetIntensity = InactiveLightIntensity;
	bool bShouldActivateEffect = false;

	switch (CurrentState)
	{
	case EHarmoniaCheckpointState::Inactive:
		TargetIntensity = InactiveLightIntensity;
		bShouldActivateEffect = false;
		break;

	case EHarmoniaCheckpointState::Activated:
		TargetIntensity = ActiveLightIntensity;
		bShouldActivateEffect = true;
		break;

	case EHarmoniaCheckpointState::Resonating:
		TargetIntensity = ResonatingLightIntensity;
		bShouldActivateEffect = true;
		break;
	}

	ResonanceLight->SetIntensity(TargetIntensity);

	// 파티클 효과
	if (ResonanceEffect)
	{
		if (bShouldActivateEffect && !ResonanceEffect->IsActive())
		{
			ResonanceEffect->Activate(true);
		}
		else if (!bShouldActivateEffect && ResonanceEffect->IsActive())
		{
			ResonanceEffect->Deactivate();
		}
	}

	// 오디오
	if (ResonanceAudio)
	{
		if (CurrentState == EHarmoniaCheckpointState::Resonating)
		{
			if (!ResonanceAudio->IsPlaying() && ResonanceLoopSound)
			{
				ResonanceAudio->SetSound(ResonanceLoopSound);
				ResonanceAudio->Play();
			}
		}
		else
		{
			if (ResonanceAudio->IsPlaying())
			{
				ResonanceAudio->Stop();
			}
		}
	}
}

void AHarmoniaCrystalResonator::UpdateResonancePulse(float DeltaTime)
{
	if (!ResonanceLight)
	{
		return;
	}

	ResonancePulseTimer += DeltaTime * ResonancePulseSpeed;

	// 사인파 펄스
	float PulseValue = FMath::Sin(ResonancePulseTimer);
	PulseValue = (PulseValue + 1.0f) * 0.5f; // 0~1 범위로 정규화

	float CurrentIntensity = FMath::Lerp(
		ResonatingLightIntensity * 0.8f,
		ResonatingLightIntensity * 1.2f,
		PulseValue
	);

	ResonanceLight->SetIntensity(CurrentIntensity);
}

void AHarmoniaCrystalResonator::UpdateNearbyResonance_Implementation()
{
	// Blueprint에서 구현 가능
	// 가까운 다른 활성화된 크리스탈 찾아서 연결 효과 표시
}

void AHarmoniaCrystalResonator::ApplyFrequencyEffects_Implementation()
{
	// Blueprint에서 구현 가능
	// 각 공명 주파수별 특수 효과
}

void AHarmoniaCrystalResonator::PlayActivationEffects_Implementation()
{
	// 활성화 사운드
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, GetActorLocation());
	}

	// Blueprint에서 추가 효과 구현 가능
}

void AHarmoniaCrystalResonator::PlayResonanceStartEffects_Implementation()
{
	// 공명 시작 사운드
	if (ResonanceStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceStartSound, GetActorLocation());
	}

	// Blueprint에서 추가 효과 구현 가능
}

void AHarmoniaCrystalResonator::PlayResonanceCompleteEffects_Implementation()
{
	// 공명 완료 사운드
	if (ResonanceCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceCompleteSound, GetActorLocation());
	}

	// Blueprint에서 추가 효과 구현 가능
}

void AHarmoniaCrystalResonator::InitializeFrequencyColors()
{
	// 공명 주파수별 기본 색상 설정
	FrequencyColors.Empty();
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Azure, FLinearColor(0.0f, 0.5f, 1.0f)); // 푸른색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Crimson, FLinearColor(1.0f, 0.0f, 0.2f)); // 붉은색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Verdant, FLinearColor(0.0f, 1.0f, 0.3f)); // 녹색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Aurum, FLinearColor(1.0f, 0.8f, 0.0f)); // 금색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Violet, FLinearColor(0.8f, 0.0f, 1.0f)); // 보라색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Luminous, FLinearColor(1.0f, 1.0f, 1.0f)); // 흰색
}

#if WITH_EDITOR
void AHarmoniaCrystalResonator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// CheckpointName이 변경되면 자동으로 ID 생성
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, CheckpointName))
	{
		if (CheckpointID.IsNone() && !CheckpointName.IsEmpty())
		{
			FString NameString = CheckpointName.ToString();
			NameString.ReplaceInline(TEXT(" "), TEXT("_"));
			CheckpointID = FName(*NameString);
		}
	}

	// 상호작용 범위 변경 시 구체 크기 업데이트
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, InteractionRange))
	{
		if (InteractionSphere)
		{
			InteractionSphere->SetSphereRadius(InteractionRange);
		}
	}

	// 공명 주파수 변경 시 비주얼 업데이트
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, ResonanceFrequency))
	{
		UpdateVisuals();
	}
}
#endif
