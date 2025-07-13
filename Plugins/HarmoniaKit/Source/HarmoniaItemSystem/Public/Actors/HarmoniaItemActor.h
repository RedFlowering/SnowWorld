// Copyright 2025 Snow Game Studio.

#pragma once

#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaItemSystemDefinitions.h"
#include "HarmoniaItemActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class HARMONIAITEMSYSTEM_API AHarmoniaItemActor : public AActor
{
    GENERATED_BODY()

public:
    AHarmoniaItemActor();

protected:
    virtual void BeginPlay() override;

    // 네트워크 동기화 선언
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // 줍기 처리 함수
    UFUNCTION(BlueprintCallable, Category="Item")
    void Pickup(class AActor* Picker);

    // 액터가 Overlap되었을 때 자동으로 Pickup 시도
    UFUNCTION()
    void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void RequestPickup(class AActor* Picker);

	UFUNCTION(Server, Reliable)
	void ServerPickup(class AActor* Picker);

public:
    // Replicated Item Data
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    FHarmoniaID ItemID;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    int32 Count = 1;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    float Durability = 0.f;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    EItemCategory ItemCategory = EItemCategory::Resource;

    // 아이템 미리보기(에디터용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
    TObjectPtr<UStaticMeshComponent> PreviewMesh;
};