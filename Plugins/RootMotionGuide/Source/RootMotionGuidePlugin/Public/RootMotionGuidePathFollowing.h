// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RootMotionGuidePathFollowing.generated.h"

class ACharacter;
class URootMotionMovement;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOTMOTIONGUIDEPLUGIN_API URootMotionGuidePathFollowing : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "RootMotionGuide")
	FVector MovementInput;

private:

	UPROPERTY()
	TObjectPtr<ACharacter> Character;

	UPROPERTY()
	TObjectPtr<URootMotionMovement> RootMotionMovement;



public:	
	// Sets default values for this component's properties
	URootMotionGuidePathFollowing();

	UFUNCTION(BlueprintCallable, Category = "RootMotionGuide")
	void StartPathFollowing();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
