// Copyright 2025 Snow Game Studio.

#include "HarmoniaOnlineSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UHarmoniaOnlineSubsystem::UHarmoniaOnlineSubsystem()
	: bIsConnected(false)
	, bIsMicrophoneMuted(false)
	, VoiceChatStatus(EHarmoniaVoiceChatStatus::Disconnected)
{
}

void UHarmoniaOnlineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Initialized"));

	// 초기화 작업
	bIsConnected = false;
	CachedFriendList.Empty();
	PendingFriendRequests.Empty();
	PendingInvites.Empty();
	VoiceChannels.Empty();
}

void UHarmoniaOnlineSubsystem::Deinitialize()
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		if (FriendListUpdateTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(FriendListUpdateTimerHandle);
		}
	}

	// 음성 채널 정리
	if (!CurrentVoiceChannelId.IsEmpty())
	{
		LeaveVoiceChannel(CurrentVoiceChannelId);
	}

	// 연결 해제
	if (bIsConnected)
	{
		Disconnect();
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Deinitialized"));

	Super::Deinitialize();
}

//~=============================================================================
// 친구 관리 기능
//~=============================================================================

void UHarmoniaOnlineSubsystem::SearchUsers(const FString& SearchQuery, int32 MaxResults)
{
	if (SearchQuery.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Search query is empty"));
		OnUserSearchCompleted.Broadcast(false, TArray<FHarmoniaUserSearchResult>());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Searching for users: %s (Max: %d)"), *SearchQuery, MaxResults);

	// TODO: 실제 서버 API 호출 구현
	// 현재는 시뮬레이션된 결과 반환
	TArray<FHarmoniaUserSearchResult> Results;

	// 시뮬레이션: 검색어에 기반한 더미 데이터 생성
	for (int32 i = 0; i < FMath::Min(MaxResults, 5); ++i)
	{
		FHarmoniaUserSearchResult Result;
		Result.UserId = FString::Printf(TEXT("User_%s_%d"), *SearchQuery, i);
		Result.DisplayName = FString::Printf(TEXT("%s_%d"), *SearchQuery, i);
		Result.Status = (i % 2 == 0) ? EHarmoniaFriendStatus::Online : EHarmoniaFriendStatus::Offline;
		Result.bIsFriend = false;
		Result.bHasPendingRequest = false;

		Results.Add(Result);
	}

	OnUserSearchCompleted.Broadcast(true, Results);
}

void UHarmoniaOnlineSubsystem::SendFriendRequest(const FString& UserId, const FString& Message)
{
	if (UserId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Cannot send friend request to empty UserId"));
		OnFriendRequestResult.Broadcast(false, TEXT("Invalid User ID"));
		return;
	}

	// 이미 친구인지 확인
	FHarmoniaFriendInfo ExistingFriend;
	if (GetFriendInfo(UserId, ExistingFriend))
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: User %s is already a friend"), *UserId);
		OnFriendRequestResult.Broadcast(false, TEXT("Already friends"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Sending friend request to %s"), *UserId);

	// TODO: 실제 서버 API 호출 구현
	// 현재는 시뮬레이션
	SimulateFriendRequestResponse(UserId, true);
}

void UHarmoniaOnlineSubsystem::AcceptFriendRequest(const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Accepting friend request from %s"), *UserId);

	// 요청 찾기
	int32 RequestIndex = PendingFriendRequests.IndexOfByPredicate([&UserId](const FHarmoniaFriendRequest& Request)
	{
		return Request.UserId == UserId;
	});

	if (RequestIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Friend request from %s not found"), *UserId);
		return;
	}

	// TODO: 실제 서버 API 호출 구현

	// 친구 목록에 추가
	FHarmoniaFriendInfo NewFriend;
	NewFriend.UserId = UserId;
	NewFriend.DisplayName = PendingFriendRequests[RequestIndex].DisplayName;
	NewFriend.Status = EHarmoniaFriendStatus::Online;
	NewFriend.FriendSince = FDateTime::UtcNow();

	CachedFriendList.Add(NewFriend);

	// 요청 제거
	PendingFriendRequests.RemoveAt(RequestIndex);

	// 이벤트 브로드캐스트
	OnFriendListUpdated.Broadcast(CachedFriendList);
}

void UHarmoniaOnlineSubsystem::RejectFriendRequest(const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Rejecting friend request from %s"), *UserId);

	// 요청 찾기 및 제거
	int32 RequestIndex = PendingFriendRequests.IndexOfByPredicate([&UserId](const FHarmoniaFriendRequest& Request)
	{
		return Request.UserId == UserId;
	});

	if (RequestIndex != INDEX_NONE)
	{
		PendingFriendRequests.RemoveAt(RequestIndex);
		// TODO: 실제 서버 API 호출 구현
	}
}

void UHarmoniaOnlineSubsystem::RemoveFriend(const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Removing friend %s"), *UserId);

	// 친구 목록에서 제거
	int32 FriendIndex = CachedFriendList.IndexOfByPredicate([&UserId](const FHarmoniaFriendInfo& Friend)
	{
		return Friend.UserId == UserId;
	});

	if (FriendIndex != INDEX_NONE)
	{
		CachedFriendList.RemoveAt(FriendIndex);

		// TODO: 실제 서버 API 호출 구현

		// 이벤트 브로드캐스트
		OnFriendListUpdated.Broadcast(CachedFriendList);
	}
}

void UHarmoniaOnlineSubsystem::RefreshFriendList()
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Refreshing friend list"));

	// TODO: 실제 서버 API 호출 구현
	// 현재는 캐시 업데이트만 수행
	UpdateFriendListCache();
}

bool UHarmoniaOnlineSubsystem::GetFriendInfo(const FString& UserId, FHarmoniaFriendInfo& OutFriendInfo) const
{
	const FHarmoniaFriendInfo* FoundFriend = CachedFriendList.FindByPredicate([&UserId](const FHarmoniaFriendInfo& Friend)
	{
		return Friend.UserId == UserId;
	});

	if (FoundFriend)
	{
		OutFriendInfo = *FoundFriend;
		return true;
	}

	return false;
}

TArray<FHarmoniaFriendInfo> UHarmoniaOnlineSubsystem::GetFriendsByStatus(EHarmoniaFriendStatus Status) const
{
	TArray<FHarmoniaFriendInfo> FilteredFriends;

	for (const FHarmoniaFriendInfo& Friend : CachedFriendList)
	{
		if (Friend.Status == Status)
		{
			FilteredFriends.Add(Friend);
		}
	}

	return FilteredFriends;
}

//~=============================================================================
// 초대 시스템
//~=============================================================================

void UHarmoniaOnlineSubsystem::InviteToGame(const FString& UserId, const FString& SessionId, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Inviting %s to game session %s"), *UserId, *SessionId);
	SendInviteInternal(UserId, EHarmoniaInviteType::Game, Message, SessionId);
}

void UHarmoniaOnlineSubsystem::InviteToParty(const FString& UserId, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Inviting %s to party"), *UserId);
	SendInviteInternal(UserId, EHarmoniaInviteType::Party, Message, TEXT(""));
}

void UHarmoniaOnlineSubsystem::InviteToVoiceChat(const FString& UserId, const FString& ChannelId, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Inviting %s to voice channel %s"), *UserId, *ChannelId);
	SendInviteInternal(UserId, EHarmoniaInviteType::VoiceChat, Message, ChannelId);
}

void UHarmoniaOnlineSubsystem::AcceptInvite(const FString& InviteId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Accepting invite %s"), *InviteId);

	// 초대 찾기
	int32 InviteIndex = PendingInvites.IndexOfByPredicate([&InviteId](const FHarmoniaInviteInfo& Invite)
	{
		return Invite.InviteId == InviteId;
	});

	if (InviteIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Invite %s not found"), *InviteId);
		return;
	}

	FHarmoniaInviteInfo& Invite = PendingInvites[InviteIndex];

	// 초대 타입에 따라 처리
	switch (Invite.InviteType)
	{
	case EHarmoniaInviteType::Game:
		// TODO: 게임 세션 참가 로직
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Joining game session %s"), *Invite.SessionId);
		break;

	case EHarmoniaInviteType::Party:
		// TODO: 파티 참가 로직
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Joining party"));
		break;

	case EHarmoniaInviteType::VoiceChat:
		// 음성 채널 참가
		JoinVoiceChannel(Invite.ChannelId);
		break;
	}

	// 초대 제거
	PendingInvites.RemoveAt(InviteIndex);
}

void UHarmoniaOnlineSubsystem::RejectInvite(const FString& InviteId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Rejecting invite %s"), *InviteId);

	// 초대 제거
	int32 InviteIndex = PendingInvites.IndexOfByPredicate([&InviteId](const FHarmoniaInviteInfo& Invite)
	{
		return Invite.InviteId == InviteId;
	});

	if (InviteIndex != INDEX_NONE)
	{
		PendingInvites.RemoveAt(InviteIndex);
		// TODO: 서버에 거절 알림
	}
}

//~=============================================================================
// 음성 대화 기능
//~=============================================================================

FString UHarmoniaOnlineSubsystem::CreateVoiceChannel(const FString& ChannelName, int32 MaxParticipants, bool bIsPrivate)
{
	FString ChannelId = FGuid::NewGuid().ToString();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Creating voice channel '%s' (ID: %s)"), *ChannelName, *ChannelId);

	FHarmoniaVoiceChannelInfo ChannelInfo;
	ChannelInfo.ChannelId = ChannelId;
	ChannelInfo.ChannelName = ChannelName;
	ChannelInfo.MaxParticipants = MaxParticipants;
	ChannelInfo.bIsPrivate = bIsPrivate;

	VoiceChannels.Add(ChannelId, ChannelInfo);

	// TODO: 실제 음성 채널 생성 로직 (서버 연동)

	return ChannelId;
}

void UHarmoniaOnlineSubsystem::JoinVoiceChannel(const FString& ChannelId)
{
	if (ChannelId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Cannot join empty channel ID"));
		return;
	}

	// 이미 다른 채널에 있다면 먼저 나가기
	if (!CurrentVoiceChannelId.IsEmpty() && CurrentVoiceChannelId != ChannelId)
	{
		LeaveVoiceChannel(CurrentVoiceChannelId);
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Joining voice channel %s"), *ChannelId);

	CurrentVoiceChannelId = ChannelId;
	UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Connecting);

	// TODO: 실제 음성 채널 연결 로직

	// 시뮬레이션: 연결 성공
	UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Connected);

	// 채널 참가자 목록에 자신 추가
	if (FHarmoniaVoiceChannelInfo* ChannelInfo = VoiceChannels.Find(ChannelId))
	{
		if (!ChannelInfo->ParticipantIds.Contains(CurrentUserId))
		{
			ChannelInfo->ParticipantIds.Add(CurrentUserId);
		}
	}
}

void UHarmoniaOnlineSubsystem::LeaveVoiceChannel(const FString& ChannelId)
{
	if (ChannelId.IsEmpty())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Leaving voice channel %s"), *ChannelId);

	// TODO: 실제 음성 채널 연결 해제 로직

	// 채널 참가자 목록에서 제거
	if (FHarmoniaVoiceChannelInfo* ChannelInfo = VoiceChannels.Find(ChannelId))
	{
		ChannelInfo->ParticipantIds.Remove(CurrentUserId);
	}

	if (CurrentVoiceChannelId == ChannelId)
	{
		CurrentVoiceChannelId.Empty();
		UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Disconnected);
	}
}

void UHarmoniaOnlineSubsystem::SetMicrophoneMuted(bool bMuted)
{
	if (bIsMicrophoneMuted == bMuted)
	{
		return;
	}

	bIsMicrophoneMuted = bMuted;

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Microphone %s"), bMuted ? TEXT("muted") : TEXT("unmuted"));

	// TODO: 실제 마이크 음소거 로직

	// 상태 업데이트
	if (!CurrentVoiceChannelId.IsEmpty())
	{
		EHarmoniaVoiceChatStatus NewStatus = bMuted ? EHarmoniaVoiceChatStatus::Muted : EHarmoniaVoiceChatStatus::Connected;
		UpdateVoiceChannelStatus(CurrentVoiceChannelId, NewStatus);
	}
}

void UHarmoniaOnlineSubsystem::SetUserMuted(const FString& UserId, bool bMuted)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: User %s %s"), *UserId, bMuted ? TEXT("muted") : TEXT("unmuted"));

	if (bMuted)
	{
		MutedUsers.Add(UserId);
	}
	else
	{
		MutedUsers.Remove(UserId);
	}

	// TODO: 실제 사용자 음소거 로직
}

bool UHarmoniaOnlineSubsystem::GetVoiceChannelInfo(const FString& ChannelId, FHarmoniaVoiceChannelInfo& OutChannelInfo) const
{
	const FHarmoniaVoiceChannelInfo* FoundChannel = VoiceChannels.Find(ChannelId);

	if (FoundChannel)
	{
		OutChannelInfo = *FoundChannel;
		return true;
	}

	return false;
}

//~=============================================================================
// 연결 및 상태 관리
//~=============================================================================

void UHarmoniaOnlineSubsystem::Connect()
{
	if (bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Already connected"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Connecting to online services..."));

	// TODO: 실제 온라인 서비스 연결 로직
	// 현재는 시뮬레이션

	// 시뮬레이션: 더미 사용자 정보 설정
	CurrentUserId = FGuid::NewGuid().ToString();
	CurrentUserDisplayName = TEXT("TestUser");
	bIsConnected = true;

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Connected as %s (ID: %s)"), *CurrentUserDisplayName, *CurrentUserId);

	// 주기적인 친구 목록 업데이트 타이머 시작 (30초마다)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FriendListUpdateTimerHandle,
			this,
			&UHarmoniaOnlineSubsystem::UpdateFriendListCache,
			30.0f,
			true
		);
	}

	// 초기 친구 목록 로드
	RefreshFriendList();
}

void UHarmoniaOnlineSubsystem::Disconnect()
{
	if (!bIsConnected)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Disconnecting from online services..."));

	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		if (FriendListUpdateTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(FriendListUpdateTimerHandle);
		}
	}

	// 음성 채널 정리
	if (!CurrentVoiceChannelId.IsEmpty())
	{
		LeaveVoiceChannel(CurrentVoiceChannelId);
	}

	// TODO: 실제 온라인 서비스 연결 해제 로직

	bIsConnected = false;
	CurrentUserId.Empty();
	CurrentUserDisplayName.Empty();
	CachedFriendList.Empty();
	PendingFriendRequests.Empty();
	PendingInvites.Empty();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Disconnected"));
}

//~=============================================================================
// 내부 헬퍼 함수
//~=============================================================================

void UHarmoniaOnlineSubsystem::UpdateFriendListCache()
{
	if (!bIsConnected)
	{
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("HarmoniaOnlineSubsystem: Updating friend list cache..."));

	// TODO: 실제 서버에서 친구 목록 가져오기
	// 현재는 시뮬레이션: 기존 친구들의 상태를 랜덤하게 변경

	bool bHasChanges = false;

	for (FHarmoniaFriendInfo& Friend : CachedFriendList)
	{
		// 랜덤하게 상태 변경 (10% 확률)
		if (FMath::RandRange(0, 100) < 10)
		{
			EHarmoniaFriendStatus OldStatus = Friend.Status;
			Friend.Status = static_cast<EHarmoniaFriendStatus>(FMath::RandRange(0, 4));

			if (OldStatus != Friend.Status)
			{
				bHasChanges = true;
				OnFriendStatusChanged.Broadcast(Friend);
			}
		}
	}

	if (bHasChanges)
	{
		OnFriendListUpdated.Broadcast(CachedFriendList);
	}
}

void UHarmoniaOnlineSubsystem::SimulateFriendRequestResponse(const FString& UserId, bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Friend request sent successfully to %s"), *UserId);
		OnFriendRequestResult.Broadcast(true, TEXT("Friend request sent"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Failed to send friend request to %s"), *UserId);
		OnFriendRequestResult.Broadcast(false, TEXT("Failed to send friend request"));
	}
}

void UHarmoniaOnlineSubsystem::SendInviteInternal(const FString& UserId, EHarmoniaInviteType InviteType, const FString& Message, const FString& AdditionalData)
{
	// 친구인지 확인
	FHarmoniaFriendInfo FriendInfo;
	if (!GetFriendInfo(UserId, FriendInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Cannot invite non-friend user %s"), *UserId);
		return;
	}

	// TODO: 실제 초대 전송 로직 (서버 연동)

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Invite sent to %s"), *UserId);
}

void UHarmoniaOnlineSubsystem::UpdateVoiceChannelStatus(const FString& ChannelId, EHarmoniaVoiceChatStatus NewStatus)
{
	if (VoiceChatStatus != NewStatus)
	{
		VoiceChatStatus = NewStatus;
		OnVoiceChatStatusChanged.Broadcast(ChannelId, NewStatus);

		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Voice chat status changed to %d"), static_cast<int32>(NewStatus));
	}
}
