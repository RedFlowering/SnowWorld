// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaRaidComponent.generated.h"

/**
 * 레이드 멤버 정보
 */
USTRUCT(BlueprintType)
struct FRaidMemberInfo
{
	GENERATED_BODY()

	/** 플레이어 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerID;

	/** 플레이어 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerName;

	/** 역할 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	ERaidRole Role = ERaidRole::DPS;

	/** 준비 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsReady = false;

	/** 생존 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsAlive = true;

	/** 남은 부활 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 RemainingRevives = 0;
};

/**
 * 레이드 페이즈 정보
 */
USTRUCT(BlueprintType)
struct FRaidPhaseInfo
{
	GENERATED_BODY()

	/** 페이즈 번호 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 PhaseNumber = 1;

	/** 페이즈 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseName;

	/** 보스 체력 임계값 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	float HealthThreshold = 100.0f;

	/** 페이즈 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseDescription;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberJoined, const FRaidMemberInfo&, MemberInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberLeft, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidPhaseChanged, int32, OldPhase, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidMemberDied, const FString&, PlayerID, int32, RemainingRevives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidWipe, int32, CurrentPhase);

/**
 * 레이드 컴포넌트
 * 레이드 전투를 관리하는 컴포넌트
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
	//~ 델리게이트
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

	//~ 멤버 관리

	/** 멤버 추가 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool AddMember(const FRaidMemberInfo& MemberInfo);

	/** 멤버 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool RemoveMember(const FString& PlayerID);

	/** 멤버 정보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidMemberInfo GetMemberInfo(const FString& PlayerID) const;

	/** 모든 멤버 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetAllMembers() const;

	/** 역할별 멤버 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetMembersByRole(ERaidRole Role) const;

	/** 공격대 크기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetRaidSize() const { return RaidMembers.Num(); }

	//~ 준비 상태

	/** 준비 완료 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void SetMemberReady(const FString& PlayerID, bool bReady);

	/** 모든 멤버 준비 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool AreAllMembersReady() const;

	/** 준비된 멤버 수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetReadyMemberCount() const;

	//~ 페이즈 관리

	/** 현재 페이즈 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** 페이즈 전환 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void AdvanceToNextPhase();

	/** 페이즈 정보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidPhaseInfo GetPhaseInfo(int32 PhaseNumber) const;

	//~ 전투 관리

	/** 멤버 사망 처리 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void OnMemberDeath(const FString& PlayerID);

	/** 멤버 부활 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ReviveMember(const FString& PlayerID);

	/** 생존 멤버 수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetAliveMemberCount() const;

	/** 전멸 체크 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool IsWiped() const;

	//~ 역할 구성 검증

	/** 역할 구성 확인 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ValidateRoleComposition(const URaidDataAsset* RaidData) const;

	/** 역할별 인원 수 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TMap<ERaidRole, int32> GetRoleDistribution() const;

protected:
	/** 레이드 멤버 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TMap<FString, FRaidMemberInfo> RaidMembers;

	/** 현재 페이즈 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 CurrentPhase;

	/** 페이즈 정보 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TArray<FRaidPhaseInfo> PhaseInfos;

	/** 전역 부활 제한 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 GlobalReviveLimit;

	/** 사용된 부활 횟수 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 UsedRevives;

	/** 레이드 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	const URaidDataAsset* CurrentRaidData;

private:
	/** 전멸 처리 */
	void ProcessWipe();
};
