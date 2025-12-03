// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaRaidComponent.generated.h"

/**
 * ?�이??멤버 ?�보
 */
USTRUCT(BlueprintType)
struct FRaidMemberInfo
{
	GENERATED_BODY()

	/** ?�레?�어 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerID;

	/** ?�레?�어 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerName;

	/** ??�� */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	ERaidRole Role = ERaidRole::DPS;

	/** 준�??�태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsReady = false;

	/** ?�존 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsAlive = true;

	/** ?��? 부???�수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 RemainingRevives = 0;
};

/**
 * ?�이???�이�??�보
 */
USTRUCT(BlueprintType)
struct FRaidPhaseInfo
{
	GENERATED_BODY()

	/** ?�이�?번호 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 PhaseNumber = 1;

	/** ?�이�??�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseName;

	/** 보스 체력 ?�계�?(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	float HealthThreshold = 100.0f;

	/** ?�이�??�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseDescription;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberJoined, const FRaidMemberInfo&, MemberInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberLeft, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidPhaseChanged, int32, OldPhase, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidMemberDied, const FString&, PlayerID, int32, RemainingRevives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidWipe, int32, CurrentPhase);

/**
 * ?�이??컴포?�트
 * ?�이???�투�?관리하??컴포?�트
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRaidComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRaidComponent();

protected:
	virtual void BeginPlay() override;

public:
	//~ ?�리게이??
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidMemberJoined OnRaidMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidMemberLeft OnRaidMemberLeft;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidPhaseChanged OnRaidPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidMemberDied OnRaidMemberDied;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Raid")
	FOnRaidWipe OnRaidWipe;

	//~ 멤버 관�?

	/** 멤버 추�? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool AddMember(const FRaidMemberInfo& MemberInfo);

	/** 멤버 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool RemoveMember(const FString& PlayerID);

	/** 멤버 ?�보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidMemberInfo GetMemberInfo(const FString& PlayerID) const;

	/** 모든 멤버 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetAllMembers() const;

	/** ??���?멤버 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetMembersByRole(ERaidRole Role) const;

	/** 공격?� ?�기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetRaidSize() const { return RaidMembers.Num(); }

	//~ 준�??�태

	/** 준�??�료 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void SetMemberReady(const FString& PlayerID, bool bReady);

	/** 모든 멤버 준�??�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool AreAllMembersReady() const;

	/** 준비된 멤버 ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetReadyMemberCount() const;

	//~ ?�이�?관�?

	/** ?�재 ?�이�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** ?�이�??�환 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void AdvanceToNextPhase();

	/** ?�이�??�보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidPhaseInfo GetPhaseInfo(int32 PhaseNumber) const;

	//~ ?�투 관�?

	/** 멤버 ?�망 처리 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void OnMemberDeath(const FString& PlayerID);

	/** 멤버 부??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ReviveMember(const FString& PlayerID);

	/** ?�존 멤버 ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetAliveMemberCount() const;

	/** ?�멸 체크 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool IsWiped() const;

	//~ ??�� 구성 검�?

	/** ??�� 구성 ?�인 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ValidateRoleComposition(const URaidDataAsset* RaidData) const;

	/** ??���??�원 ??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TMap<ERaidRole, int32> GetRoleDistribution() const;

protected:
	/** ?�이??멤버 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TMap<FString, FRaidMemberInfo> RaidMembers;

	/** ?�재 ?�이�?*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 CurrentPhase;

	/** ?�이�??�보 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TArray<FRaidPhaseInfo> PhaseInfos;

	/** ?�역 부???�한 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 GlobalReviveLimit;

	/** ?�용??부???�수 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 UsedRevives;

	/** ?�이???�이??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	const URaidDataAsset* CurrentRaidData;

private:
	/** ?�멸 처리 */
	void ProcessWipe();
};
