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

    // ��Ʈ��ũ ����ȭ ����
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

    // ������ �̸�����(�����Ϳ�)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
    TObjectPtr<UStaticMeshComponent> PreviewMesh = nullptr;
};