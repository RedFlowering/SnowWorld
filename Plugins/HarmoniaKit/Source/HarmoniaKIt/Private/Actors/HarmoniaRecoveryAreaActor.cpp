// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaRecoveryAreaActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Character/LyraHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

AHarmoniaRecoveryAreaActor::AHarmoniaRecoveryAreaActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Sphere Component 생성
	RecoveryAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RecoveryAreaSphere"));
	SetRootComponent(RecoveryAreaSphere);
	RecoveryAreaSphere->SetSphereRadius(300.0f);
	RecoveryAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RecoveryAreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	RecoveryAreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// VFX Component 생성
	AreaVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AreaVFXComponent"));
	AreaVFXComponent->SetupAttachment(RootComponent);
	AreaVFXComponent->SetAutoActivate(false);

	// Audio Component 생성
	AreaAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AreaAudioComponent"));
	AreaAudioComponent->SetupAttachment(RootComponent);
	AreaAudioComponent->SetAutoActivate(false);

	// 기본 설정
	RecoveryConfig.RecoveryRadius = 300.0f;
	RecoveryConfig.Duration = 60.0f;
	RecoveryConfig.HealthPerTick = 5.0f;
	RecoveryConfig.TickInterval = 1.0f;
}

void AHarmoniaRecoveryAreaActor::BeginPlay()
{
	Super::BeginPlay();

	// Overlap 이벤트 바인딩
	RecoveryAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaRecoveryAreaActor::OnActorEnterRecoveryArea);
	RecoveryAreaSphere->OnComponentEndOverlap.AddDynamic(this, &AHarmoniaRecoveryAreaActor::OnActorLeaveRecoveryArea);

	// 자동 활성화
	ActivateRecoveryArea();
}

void AHarmoniaRecoveryAreaActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecoveryTickTimerHandle);
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AHarmoniaRecoveryAreaActor::InitializeRecoveryArea(const FHarmoniaDeployableRecoveryConfig& Config)
{
	RecoveryConfig = Config;

	// Sphere 반경 업데이트
	if (RecoveryAreaSphere)
	{
		RecoveryAreaSphere->SetSphereRadius(Config.RecoveryRadius);
	}

	// VFX 설정
	if (AreaVFXComponent && Config.AreaVFX)
	{
		AreaVFXComponent->SetAsset(Config.AreaVFX);
	}

	// SFX 설정
	if (AreaAudioComponent && Config.AreaSound)
	{
		AreaAudioComponent->SetSound(Config.AreaSound);
	}
}

void AHarmoniaRecoveryAreaActor::ActivateRecoveryArea()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;

	// VFX 활성화
	if (AreaVFXComponent)
	{
		AreaVFXComponent->Activate();
	}

	// SFX 활성화
	if (AreaAudioComponent)
	{
		AreaAudioComponent->Play();
	}

	// 회복 틱 타이머 시작
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RecoveryTickTimerHandle,
			this,
			&AHarmoniaRecoveryAreaActor::PerformRecoveryTick,
			RecoveryConfig.TickInterval,
			true
		);

		// 만료 타이머 설정
		if (RecoveryConfig.Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				ExpirationTimerHandle,
				this,
				&AHarmoniaRecoveryAreaActor::ExpireRecoveryArea,
				RecoveryConfig.Duration,
				false
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Recovery Area activated with radius %f, duration %f, tick %f"),
		RecoveryConfig.RecoveryRadius, RecoveryConfig.Duration, RecoveryConfig.TickInterval);
}

void AHarmoniaRecoveryAreaActor::DeactivateRecoveryArea()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	// VFX 비활성화
	if (AreaVFXComponent && AreaVFXComponent->IsActive())
	{
		AreaVFXComponent->Deactivate();
	}

	// SFX 정지
	if (AreaAudioComponent && AreaAudioComponent->IsPlaying())
	{
		AreaAudioComponent->Stop();
	}

	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecoveryTickTimerHandle);
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("Recovery Area deactivated"));
}

void AHarmoniaRecoveryAreaActor::PerformRecoveryTick()
{
	if (!bIsActive)
	{
		return;
	}

	int32 HealedCount = 0;

	// 범위 내 모든 액터에게 회복 적용
	for (AActor* Actor : ActorsInArea)
	{
		if (!Actor || !Actor->IsValidLowLevel())
		{
			continue;
		}

		// Health Component 찾기
		if (ULyraHealthComponent* HealthComp = Actor->FindComponentByClass<ULyraHealthComponent>())
		{
			if (HealthComp->GetHealth() < HealthComp->GetMaxHealth())
			{
				// TODO: Apply healing through HealthComponent or GameplayEffect
				// For now, we'll just log
				UE_LOG(LogTemp, Log, TEXT("Healing %s for %f HP"), *Actor->GetName(), RecoveryConfig.HealthPerTick);
				HealedCount++;

				// Gameplay Effect로 회복 적용하는 것이 더 좋음
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
				{
					// TODO: Apply healing Gameplay Effect
				}
			}
		}
	}

	if (HealedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Recovery Area healed %d actors"), HealedCount);
	}
}

TArray<AActor*> AHarmoniaRecoveryAreaActor::GetActorsInRecoveryArea() const
{
	return ActorsInArea.Array();
}

void AHarmoniaRecoveryAreaActor::ExpireRecoveryArea()
{
	UE_LOG(LogTemp, Log, TEXT("Recovery Area expired"));

	DeactivateRecoveryArea();

	// Actor 파괴
	Destroy();
}

void AHarmoniaRecoveryAreaActor::OnActorEnterRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !bIsActive)
	{
		return;
	}

	// Character만 회복 (또는 HealthComponent가 있는 Actor)
	if (OtherActor->IsA<ACharacter>() || OtherActor->FindComponentByClass<ULyraHealthComponent>())
	{
		ActorsInArea.Add(OtherActor);
		UE_LOG(LogTemp, Log, TEXT("Actor %s entered recovery area"), *OtherActor->GetName());
	}
}

void AHarmoniaRecoveryAreaActor::OnActorLeaveRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	ActorsInArea.Remove(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("Actor %s left recovery area"), *OtherActor->GetName());
}
