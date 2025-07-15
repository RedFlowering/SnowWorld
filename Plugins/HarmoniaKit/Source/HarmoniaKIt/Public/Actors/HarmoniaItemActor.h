// Copyright 2025 Snow Game Studio.

#pragma once

#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaItemSystemDefinitions.h"
#include "HarmoniaItemActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class HARMONIAKIT_API AHarmoniaItemActor : public AActor
{
    GENERATED_BODY()

public:
    AHarmoniaItemActor();

protected:
    virtual void BeginPlay() override;

    // 네트워크 동기화 선언
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    UFUNCTION(BlueprintCallable, Category="Item")
    void InitItem(FHarmoniaID InitID, int32 InitCount, float InitDurability);

public:
    // Replicated Item Data
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    FHarmoniaID ItemID;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    int32 Count = 1;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Item")
    float Durability = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
    EItemCategory ItemCategory = EItemCategory::Resource;

    // 아이템 미리보기(에디터용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
    TObjectPtr<UStaticMeshComponent> PreviewMesh = nullptr;
};