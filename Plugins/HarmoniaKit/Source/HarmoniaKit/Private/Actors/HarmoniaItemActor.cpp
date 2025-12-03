// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

// ������
AHarmoniaItemActor::AHarmoniaItemActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // ���忡 ǥ���� ������ �޽�
    PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    RootComponent = PreviewMesh;
    PreviewMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    PreviewMesh->SetGenerateOverlapEvents(true);
    PreviewMesh->SetIsReplicated(true);
}

void AHarmoniaItemActor::BeginPlay()
{
    Super::BeginPlay();
    // �޽�/���� �� ���� ����
}

// ��Ʈ��ũ ����ȭ ������Ƽ ����
void AHarmoniaItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AHarmoniaItemActor, ItemID);
    DOREPLIFETIME(AHarmoniaItemActor, Count);
    DOREPLIFETIME(AHarmoniaItemActor, Durability);
}

void AHarmoniaItemActor::InitItem(FHarmoniaID InitID, int32 InitCount, float InitDurability)
{
    ItemID = InitID;
    Count = InitCount;
    Durability = InitDurability;
}