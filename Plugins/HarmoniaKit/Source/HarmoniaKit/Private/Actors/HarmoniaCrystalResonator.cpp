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

	// 루트 컴포?�트
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 받침?� 메시
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// ?�리?�탈 메시
	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(SceneRoot);
	CrystalMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// ?�인???�이??
	ResonanceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ResonanceLight"));
	ResonanceLight->SetupAttachment(CrystalMesh);
	ResonanceLight->SetIntensity(InactiveLightIntensity);
	ResonanceLight->SetAttenuationRadius(1000.0f);
	ResonanceLight->SetCastShadows(false);

	// ?�이?��????�펙??
	ResonanceEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ResonanceEffect"));
	ResonanceEffect->SetupAttachment(CrystalMesh);
	ResonanceEffect->SetAutoActivate(false);

	// ?�호?�용 범위
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// ?�디??컴포?�트
	ResonanceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ResonanceAudio"));
	ResonanceAudio->SetupAttachment(CrystalMesh);
	ResonanceAudio->SetAutoActivate(false);

	// 기본 �??�정
	CheckpointID = NAME_None;
	CheckpointName = FText::FromString(TEXT("Crystal Resonator"));
	CheckpointDescription = FText::FromString(TEXT("A mystical crystal that resonates with ancient energies."));
	ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// 초기 ?�상 ?�정
	InitializeFrequencyColors();
}

void AHarmoniaCrystalResonator::BeginPlay()
{
	Super::BeginPlay();

	// ?�브?�스?�에 ?�록
	RegisterToSubsystem();

	// ?�작 ???�동 ?�성??
	if (bStartActivated && HasAuthority())
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
	}

	// 초기 비주???�정
	UpdateVisuals();
}

void AHarmoniaCrystalResonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 공명 ?�스 ?�과
	if (CurrentState == EHarmoniaCheckpointState::Resonating)
	{
		UpdateResonancePulse(DeltaTime);
	}

	// 주�? ?�리?�탈�?공명
	if (CurrentState != EHarmoniaCheckpointState::Inactive)
	{
		UpdateNearbyResonance();
	}
}

void AHarmoniaCrystalResonator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ?�브?�스?�에???�록 ?�제
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
		// ?��? ?�성?�됨
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

	// Context.Interactor?�서 PlayerController 가?�오�?
	APlayerController* PlayerController = nullptr;
	if (APawn* InteractorPawn = Cast<APawn>(Context.Interactor))
	{
		PlayerController = Cast<APlayerController>(InteractorPawn->GetController());
	}
	else
	{
		PlayerController = Cast<APlayerController>(Context.Interactor);
	}

	if (!PlayerController)
	{
		OutResult.Message = TEXT("Invalid player");
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

	// 비활?�화 ?�태�??�성??
	if (CurrentState == EHarmoniaCheckpointState::Inactive)
	{
		bool bActivated = CheckpointSubsystem->ActivateCheckpoint(CheckpointID, PlayerController);
		if (bActivated)
		{
			OutResult.bSuccess = true;
			OutResult.Message = FString::Printf(TEXT("Activated %s"), *CheckpointName.ToString());
		}
		return;
	}

	// ?�성???�태�?공명 ?�작
	bool bResonanceStarted = CheckpointSubsystem->StartResonance(CheckpointID, PlayerController);
	if (bResonanceStarted)
	{
		OutResult.bSuccess = true;
		OutResult.Message = FString::Printf(TEXT("Resonating with %s..."), *CheckpointName.ToString());
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

	// 공명 주파???�상 ?�용
	FLinearColor FrequencyColor = GetFrequencyColor();
	ResonanceLight->SetLightColor(FrequencyColor);

	// ?�태�?발광 ?�기
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

	// ?�티???�과
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

	// ?�디??
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

	// ?�인???�스
	float PulseValue = FMath::Sin(ResonancePulseTimer);
	PulseValue = (PulseValue + 1.0f) * 0.5f; // 0~1 범위�??�규??

	float CurrentIntensity = FMath::Lerp(
		ResonatingLightIntensity * 0.8f,
		ResonatingLightIntensity * 1.2f,
		PulseValue
	);

	ResonanceLight->SetIntensity(CurrentIntensity);
}

void AHarmoniaCrystalResonator::UpdateNearbyResonance_Implementation()
{
	// Blueprint?�서 구현 가??
	// 가까운 ?�른 ?�성?�된 ?�리?�탈 찾아???�결 ?�과 ?�시
}

void AHarmoniaCrystalResonator::ApplyFrequencyEffects_Implementation()
{
	// Blueprint?�서 구현 가??
	// �?공명 주파?�별 ?�수 ?�과
}

void AHarmoniaCrystalResonator::PlayActivationEffects_Implementation()
{
	// ?�성???�운??
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, GetActorLocation());
	}

	// Blueprint?�서 추�? ?�과 구현 가??
}

void AHarmoniaCrystalResonator::PlayResonanceStartEffects_Implementation()
{
	// 공명 ?�작 ?�운??
	if (ResonanceStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceStartSound, GetActorLocation());
	}

	// Blueprint?�서 추�? ?�과 구현 가??
}

void AHarmoniaCrystalResonator::PlayResonanceCompleteEffects_Implementation()
{
	// 공명 ?�료 ?�운??
	if (ResonanceCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceCompleteSound, GetActorLocation());
	}

	// Blueprint?�서 추�? ?�과 구현 가??
}

void AHarmoniaCrystalResonator::InitializeFrequencyColors()
{
	// 공명 주파?�별 기본 ?�상 ?�정
	FrequencyColors.Empty();
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Azure, FLinearColor(0.0f, 0.5f, 1.0f)); // ?�른??
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Crimson, FLinearColor(1.0f, 0.0f, 0.2f)); // 붉�???
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Verdant, FLinearColor(0.0f, 1.0f, 0.3f)); // ?�색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Aurum, FLinearColor(1.0f, 0.8f, 0.0f)); // 금색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Violet, FLinearColor(0.8f, 0.0f, 1.0f)); // 보라??
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Luminous, FLinearColor(1.0f, 1.0f, 1.0f)); // ?�색
}

#if WITH_EDITOR
void AHarmoniaCrystalResonator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// CheckpointName??변경되�??�동?�로 ID ?�성
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, CheckpointName))
	{
		if (CheckpointID.IsNone() && !CheckpointName.IsEmpty())
		{
			FString NameString = CheckpointName.ToString();
			NameString.ReplaceInline(TEXT(" "), TEXT("_"));
			CheckpointID = FName(*NameString);
		}
	}

	// ?�호?�용 범위 변�???구체 ?�기 ?�데?�트
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, InteractionRange))
	{
		if (InteractionSphere)
		{
			InteractionSphere->SetSphereRadius(InteractionRange);
		}
	}

	// 공명 주파??변�???비주???�데?�트
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, ResonanceFrequency))
	{
		UpdateVisuals();
	}
}
#endif
