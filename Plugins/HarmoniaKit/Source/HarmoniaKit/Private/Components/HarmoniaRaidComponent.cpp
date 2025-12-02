// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#include "Components/HarmoniaRaidComponent.h"

UHarmoniaRaidComponent::UHarmoniaRaidComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentPhase = 0;
	GlobalReviveLimit = 3;
	UsedRevives = 0;
	CurrentRaidData = nullptr;
}

void UHarmoniaRaidComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UHarmoniaRaidComponent::AddMember(const FRaidMemberInfo& MemberInfo)
{
	if (RaidMembers.Contains(MemberInfo.PlayerID))
	{
		return false;
	}

	// ?ˆì´???¸ì› ?œí•œ ?•ì¸
	if (CurrentRaidData)
	{
		if (RaidMembers.Num() >= CurrentRaidData->MaxRaidSize)
		{
			return false;
		}
	}

	RaidMembers.Add(MemberInfo.PlayerID, MemberInfo);
	OnRaidMemberJoined.Broadcast(MemberInfo);

	return true;
}

bool UHarmoniaRaidComponent::RemoveMember(const FString& PlayerID)
{
	if (RaidMembers.Remove(PlayerID) > 0)
	{
		OnRaidMemberLeft.Broadcast(PlayerID);
		return true;
	}

	return false;
}

FRaidMemberInfo UHarmoniaRaidComponent::GetMemberInfo(const FString& PlayerID) const
{
	if (const FRaidMemberInfo* Found = RaidMembers.Find(PlayerID))
	{
		return *Found;
	}

	return FRaidMemberInfo();
}

TArray<FRaidMemberInfo> UHarmoniaRaidComponent::GetAllMembers() const
{
	TArray<FRaidMemberInfo> Result;
	RaidMembers.GenerateValueArray(Result);
	return Result;
}

TArray<FRaidMemberInfo> UHarmoniaRaidComponent::GetMembersByRole(ERaidRole Role) const
{
	TArray<FRaidMemberInfo> Result;

	for (const auto& Pair : RaidMembers)
	{
		if (Pair.Value.Role == Role)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

void UHarmoniaRaidComponent::SetMemberReady(const FString& PlayerID, bool bReady)
{
	if (FRaidMemberInfo* Member = RaidMembers.Find(PlayerID))
	{
		Member->bIsReady = bReady;
	}
}

bool UHarmoniaRaidComponent::AreAllMembersReady() const
{
	if (RaidMembers.Num() == 0)
	{
		return false;
	}

	for (const auto& Pair : RaidMembers)
	{
		if (!Pair.Value.bIsReady)
		{
			return false;
		}
	}

	return true;
}

int32 UHarmoniaRaidComponent::GetReadyMemberCount() const
{
	int32 Count = 0;

	for (const auto& Pair : RaidMembers)
	{
		if (Pair.Value.bIsReady)
		{
			Count++;
		}
	}

	return Count;
}

void UHarmoniaRaidComponent::AdvanceToNextPhase()
{
	int32 OldPhase = CurrentPhase;
	CurrentPhase++;

	OnRaidPhaseChanged.Broadcast(OldPhase, CurrentPhase);

	UE_LOG(LogTemp, Log, TEXT("Raid phase advanced: %d -> %d"), OldPhase, CurrentPhase);
}

FRaidPhaseInfo UHarmoniaRaidComponent::GetPhaseInfo(int32 PhaseNumber) const
{
	if (PhaseInfos.IsValidIndex(PhaseNumber - 1))
	{
		return PhaseInfos[PhaseNumber - 1];
	}

	return FRaidPhaseInfo();
}

void UHarmoniaRaidComponent::OnMemberDeath(const FString& PlayerID)
{
	if (FRaidMemberInfo* Member = RaidMembers.Find(PlayerID))
	{
		Member->bIsAlive = false;
		OnRaidMemberDied.Broadcast(PlayerID, Member->RemainingRevives);

		// ?„ë©¸ ì²´í¬
		if (IsWiped())
		{
			ProcessWipe();
		}
	}
}

bool UHarmoniaRaidComponent::ReviveMember(const FString& PlayerID)
{
	FRaidMemberInfo* Member = RaidMembers.Find(PlayerID);
	if (!Member)
	{
		return false;
	}

	// ë¶€??ê°€???¬ë? ?•ì¸
	if (Member->RemainingRevives <= 0 && UsedRevives >= GlobalReviveLimit)
	{
		return false;
	}

	Member->bIsAlive = true;

	// ë¶€???Ÿìˆ˜ ì°¨ê°
	if (Member->RemainingRevives > 0)
	{
		Member->RemainingRevives--;
	}
	else
	{
		UsedRevives++;
	}

	return true;
}

int32 UHarmoniaRaidComponent::GetAliveMemberCount() const
{
	int32 Count = 0;

	for (const auto& Pair : RaidMembers)
	{
		if (Pair.Value.bIsAlive)
		{
			Count++;
		}
	}

	return Count;
}

bool UHarmoniaRaidComponent::IsWiped() const
{
	return GetAliveMemberCount() == 0;
}

bool UHarmoniaRaidComponent::ValidateRoleComposition(const URaidDataAsset* RaidData) const
{
	if (!RaidData)
	{
		return false;
	}

	// ìµœì†Œ/ìµœë? ?¸ì› ?•ì¸
	int32 RaidSize = GetRaidSize();
	if (RaidSize < RaidData->MinRaidSize || RaidSize > RaidData->MaxRaidSize)
	{
		return false;
	}

	// ??•  êµ¬ì„± ?•ì¸
	TMap<ERaidRole, int32> CurrentDistribution = GetRoleDistribution();

	for (const auto& Pair : RaidData->RecommendedRoleComposition)
	{
		const int32* CurrentCount = CurrentDistribution.Find(Pair.Key);
		if (!CurrentCount || *CurrentCount < Pair.Value)
		{
			// ê¶Œì¥ ??•  ë¶€ì¡?
			UE_LOG(LogTemp, Warning, TEXT("Insufficient role: %d"), static_cast<int32>(Pair.Key));
		}
	}

	return true;
}

TMap<ERaidRole, int32> UHarmoniaRaidComponent::GetRoleDistribution() const
{
	TMap<ERaidRole, int32> Distribution;

	for (const auto& Pair : RaidMembers)
	{
		int32& Count = Distribution.FindOrAdd(Pair.Value.Role);
		Count++;
	}

	return Distribution;
}

void UHarmoniaRaidComponent::ProcessWipe()
{
	OnRaidWipe.Broadcast(CurrentPhase);

	UE_LOG(LogTemp, Log, TEXT("Raid wiped at phase %d"), CurrentPhase);

	// ëª¨ë“  ë©¤ë²„ ë¶€???íƒœ ì´ˆê¸°??
	for (auto& Pair : RaidMembers)
	{
		Pair.Value.bIsAlive = true;
	}

	// ?˜ì´ì¦?ì´ˆê¸°??
	CurrentPhase = 0;
	UsedRevives = 0;
}
