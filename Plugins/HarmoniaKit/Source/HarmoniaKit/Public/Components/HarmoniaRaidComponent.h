// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaRaidComponent.generated.h"

/**
 * ?ˆì´??ë©¤ë²„ ?•ë³´
 */
USTRUCT(BlueprintType)
struct FRaidMemberInfo
{
	GENERATED_BODY()

	/** ?Œë ˆ?´ì–´ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerID;

	/** ?Œë ˆ?´ì–´ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FString PlayerName;

	/** ??•  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	ERaidRole Role = ERaidRole::DPS;

	/** ì¤€ë¹??íƒœ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsReady = false;

	/** ?ì¡´ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	bool bIsAlive = true;

	/** ?¨ì? ë¶€???Ÿìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 RemainingRevives = 0;
};

/**
 * ?ˆì´???˜ì´ì¦??•ë³´
 */
USTRUCT(BlueprintType)
struct FRaidPhaseInfo
{
	GENERATED_BODY()

	/** ?˜ì´ì¦?ë²ˆí˜¸ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	int32 PhaseNumber = 1;

	/** ?˜ì´ì¦??´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseName;

	/** ë³´ìŠ¤ ì²´ë ¥ ?„ê³„ê°?(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	float HealthThreshold = 100.0f;

	/** ?˜ì´ì¦??¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raid")
	FText PhaseDescription;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberJoined, const FRaidMemberInfo&, MemberInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidMemberLeft, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidPhaseChanged, int32, OldPhase, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaidMemberDied, const FString&, PlayerID, int32, RemainingRevives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaidWipe, int32, CurrentPhase);

/**
 * ?ˆì´??ì»´í¬?ŒíŠ¸
 * ?ˆì´???„íˆ¬ë¥?ê´€ë¦¬í•˜??ì»´í¬?ŒíŠ¸
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
	//~ ?¸ë¦¬ê²Œì´??
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

	//~ ë©¤ë²„ ê´€ë¦?

	/** ë©¤ë²„ ì¶”ê? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool AddMember(const FRaidMemberInfo& MemberInfo);

	/** ë©¤ë²„ ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool RemoveMember(const FString& PlayerID);

	/** ë©¤ë²„ ?•ë³´ ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidMemberInfo GetMemberInfo(const FString& PlayerID) const;

	/** ëª¨ë“  ë©¤ë²„ ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetAllMembers() const;

	/** ??• ë³?ë©¤ë²„ ì¡°íšŒ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TArray<FRaidMemberInfo> GetMembersByRole(ERaidRole Role) const;

	/** ê³µê²©?€ ?¬ê¸° */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetRaidSize() const { return RaidMembers.Num(); }

	//~ ì¤€ë¹??íƒœ

	/** ì¤€ë¹??„ë£Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void SetMemberReady(const FString& PlayerID, bool bReady);

	/** ëª¨ë“  ë©¤ë²„ ì¤€ë¹??•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool AreAllMembersReady() const;

	/** ì¤€ë¹„ëœ ë©¤ë²„ ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetReadyMemberCount() const;

	//~ ?˜ì´ì¦?ê´€ë¦?

	/** ?„ì¬ ?˜ì´ì¦?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** ?˜ì´ì¦??„í™˜ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void AdvanceToNextPhase();

	/** ?˜ì´ì¦??•ë³´ ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	FRaidPhaseInfo GetPhaseInfo(int32 PhaseNumber) const;

	//~ ?„íˆ¬ ê´€ë¦?

	/** ë©¤ë²„ ?¬ë§ ì²˜ë¦¬ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	void OnMemberDeath(const FString& PlayerID);

	/** ë©¤ë²„ ë¶€??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ReviveMember(const FString& PlayerID);

	/** ?ì¡´ ë©¤ë²„ ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	int32 GetAliveMemberCount() const;

	/** ?„ë©¸ ì²´í¬ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Raid")
	bool IsWiped() const;

	//~ ??•  êµ¬ì„± ê²€ì¦?

	/** ??•  êµ¬ì„± ?•ì¸ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	bool ValidateRoleComposition(const URaidDataAsset* RaidData) const;

	/** ??• ë³??¸ì› ??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Raid")
	TMap<ERaidRole, int32> GetRoleDistribution() const;

protected:
	/** ?ˆì´??ë©¤ë²„ ëª©ë¡ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TMap<FString, FRaidMemberInfo> RaidMembers;

	/** ?„ì¬ ?˜ì´ì¦?*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 CurrentPhase;

	/** ?˜ì´ì¦??•ë³´ ëª©ë¡ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	TArray<FRaidPhaseInfo> PhaseInfos;

	/** ?„ì—­ ë¶€???œí•œ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 GlobalReviveLimit;

	/** ?¬ìš©??ë¶€???Ÿìˆ˜ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	int32 UsedRevives;

	/** ?ˆì´???°ì´??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Raid")
	const URaidDataAsset* CurrentRaidData;

private:
	/** ?„ë©¸ ì²˜ë¦¬ */
	void ProcessWipe();
};
