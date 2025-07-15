// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"

// 생성자
AHarmoniaItemActor::AHarmoniaItemActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // 월드에 표시할 프리뷰 메시
    PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    RootComponent = PreviewMesh;
    PreviewMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    PreviewMesh->SetGenerateOverlapEvents(true);
    PreviewMesh->SetIsReplicated(true);
}

void AHarmoniaItemActor::BeginPlay()
{
    Super::BeginPlay();
    // (필요시: 메시/재질 등 동적 세팅)
}

// 네트워크 동기화 프로퍼티 선언
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