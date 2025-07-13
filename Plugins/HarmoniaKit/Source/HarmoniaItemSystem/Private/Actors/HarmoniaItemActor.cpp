// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HarmoniaInventoryComponent.h"

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

    // 오버랩 이벤트 바인딩
    PreviewMesh->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaItemActor::OnOverlapBegin);
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
    DOREPLIFETIME(AHarmoniaItemActor, ItemCategory);
}

// 줍기 함수: 인벤토리 컴포넌트 찾아서 AddItem 시도 (서버 권한!)
void AHarmoniaItemActor::Pickup(AActor* Picker)
{
    if (!HasAuthority() || !Picker) return;

    // 예시: Picker에 인벤토리 컴포넌트가 있다면 추가
    UActorComponent* Comp = Picker->GetComponentByClass(UHarmoniaInventoryComponent::StaticClass());
    if (Comp)
    {
        auto* Inventory = Cast<UHarmoniaInventoryComponent>(Comp);
        if (Inventory && Inventory->AddItem( /* ItemData 찾기 필요시 추가 ,*/ ItemID, Count, Durability)) // 실전에서는 FItemID, Count로 충분
        {
            // 성공 시 아이템 액터 제거(Replicated, 모든 클라에서 삭제됨)
            Destroy();
        }
    }
}

// 오버랩 시 자동 줍기(필요 없으면 삭제 가능)
void AHarmoniaItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || !OtherActor) return;

    // 조건: 플레이어만 줍기 허용 등 체크 가능
    Pickup(OtherActor);
}

void AHarmoniaItemActor::RequestPickup(class AActor* Picker)
{
    if (HasAuthority())
        Pickup(Picker);
    else
        ServerPickup(Picker);
}

void AHarmoniaItemActor::ServerPickup_Implementation(class AActor* Picker)
{
    Pickup(Picker);
}