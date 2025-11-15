// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NumberRendererDefine.h"
#include "GameFramework/Actor.h"
#include "NumberRenderActor.generated.h"

class UNumberISMComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NUMBERRENDERER_API ANumberRenderActor : public AActor
{
	GENERATED_BODY()

public:
	ANumberRenderActor();

	UFUNCTION(BlueprintCallable, Category = NumberRender)
	int32 GetCurrentDrawCount() const { return Instances.Num(); }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	FNumberHandle AddNumber(int32 Number, const FTransform& Trans, FLinearColor Color, float Duration, float FadeTime, bool LockZAxis);

	FNumberHandle AddDrawNumberByTable(int32 Number, const FTransform& Trans, float Duration, FName TableRowName, bool LockZAxis);

	FNumberHandle AddDrawNumberByTable(int32 Number, const FTransform& Trans, float Duration, int32 RowNameIndex, bool LockZAxis);

	void Preload(const FTransform& Trans, float Duration);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NumberRender, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNumberISMComponent> Meshes = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NumberRender, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> NumberAnimationTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NumberRender, meta = (AllowPrivateAccess = "true"))
	float FontXYSizeRate = 0.65f;
	float TempFontXYSizeRate = 0.65f;

	TArray<FNumberInstance> Instances;
	TArray<FNumberHandle> PendingDelete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NumberRender, meta = (AllowPrivateAccess = "true"))
	int32 InitialMaxCount = 100;

	TArray<FName> RowNames;

	bool bInitialized = false;

};
