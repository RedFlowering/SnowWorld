// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticAttachActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "CosmeticBFL.h"
#include "Net/UnrealNetwork.h"


ACosmeticAttachActor::ACosmeticAttachActor(const FObjectInitializer& ObjectInitializer)
{
	LinkSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("CustomSceneRoot"));

	SetRootComponent(LinkSceneComponent);
}

void ACosmeticAttachActor::SetAttachSocket(FName Socket, FTransform AttachTransform)
{
	AttachInfo.AttachSocket = Socket;
	AttachInfo.AttachTransform = AttachTransform;

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if ((OwnerCharacter->GetNetMode() == ENetMode::NM_Standalone) || (OwnerCharacter->GetNetMode() == ENetMode::NM_ListenServer))
		{
			OnRep_AttachInfo(AttachInfo);
		}
	}
}

void ACosmeticAttachActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AttachInfo);
}

void ACosmeticAttachActor::OnRep_AttachInfo(FCosmeticAttachActorInfo OldValue)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (USceneComponent* AttachTarget = UCosmeticBFL::GetVisualMesh(OwnerCharacter))
		{
			LinkSceneComponent->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, AttachInfo.AttachSocket);
			LinkSceneComponent->SetRelativeTransform(AttachInfo.AttachTransform);
		}
	}
}
