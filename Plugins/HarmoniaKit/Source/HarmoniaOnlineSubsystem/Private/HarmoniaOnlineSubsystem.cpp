// Copyright 2025 Snow Game Studio.

#include "HarmoniaOnlineSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "VoiceChatUser.h"
#include "EOSVoiceChatFactory.h"

UHarmoniaOnlineSubsystem::UHarmoniaOnlineSubsystem()
	: OnlineSubsystem(nullptr)
	, bIsConnected(false)
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

	// OnlineSubsystem 초기화
	// 기본적으로 DefaultPlatformService를 사용 (EOS 또는 Steam)
	OnlineSubsystem = IOnlineSubsystem::Get();

	if (OnlineSubsystem)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Using OnlineSubsystem: %s"), *OnlineSubsystem->GetSubsystemName().ToString());

		// 인터페이스 가져오기
		FriendsInterface = OnlineSubsystem->GetFriendsInterface();
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		UserInterface = OnlineSubsystem->GetUserInterface();
		PresenceInterface = OnlineSubsystem->GetPresenceInterface();

		// 델리게이트 바인딩
		if (FriendsInterface.IsValid())
		{
			OnReadFriendsListCompleteHandle = FriendsInterface->AddOnReadFriendsListCompleteDelegate_Handle(
				0, FOnReadFriendsListCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnReadFriendsListComplete));

			OnSendInviteCompleteHandle = FriendsInterface->AddOnSendInviteCompleteDelegate_Handle(
				0, FOnSendInviteCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnSendInviteComplete));

			OnAcceptInviteCompleteHandle = FriendsInterface->AddOnAcceptInviteCompleteDelegate_Handle(
				0, FOnAcceptInviteCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnAcceptInviteComplete));

			OnDeleteFriendCompleteHandle = FriendsInterface->AddOnDeleteFriendCompleteDelegate_Handle(
				0, FOnDeleteFriendCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnDeleteFriendComplete));
		}

		if (SessionInterface.IsValid())
		{
			OnCreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
				FOnCreateSessionCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnCreateSessionComplete));

			OnJoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
				FOnJoinSessionCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnJoinSessionComplete));
		}

		// EOS Voice Chat 초기화 (크로스플랫폼 음성)
		FName SubsystemName = OnlineSubsystem->GetSubsystemName();
		if (SubsystemName == FName(TEXT("EOS")) || SubsystemName == FName(TEXT("EOSPlus")))
		{
			VoiceChat = OnlineSubsystem->GetVoiceChatUserInterface();
			if (VoiceChat.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: EOS Voice Chat initialized"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to get OnlineSubsystem!"));
	}
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

	// 델리게이트 정리
	if (FriendsInterface.IsValid())
	{
		FriendsInterface->ClearOnReadFriendsListCompleteDelegate_Handle(0, OnReadFriendsListCompleteHandle);
		FriendsInterface->ClearOnSendInviteCompleteDelegate_Handle(0, OnSendInviteCompleteHandle);
		FriendsInterface->ClearOnAcceptInviteCompleteDelegate_Handle(0, OnAcceptInviteCompleteHandle);
		FriendsInterface->ClearOnDeleteFriendCompleteDelegate_Handle(0, OnDeleteFriendCompleteHandle);
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteHandle);
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

	// 인터페이스 포인터 정리
	FriendsInterface = nullptr;
	SessionInterface = nullptr;
	UserInterface = nullptr;
	PresenceInterface = nullptr;
	VoiceChat = nullptr;
	OnlineSubsystem = nullptr;

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

	// Note: Steam/EOS는 기본적으로 사용자 검색 API를 제공하지 않습니다.
	// 대신 친구 목록을 검색하거나, 백엔드 서버를 통한 검색이 필요합니다.
	// 여기서는 현재 친구 목록에서 검색하는 방식으로 구현합니다.

	TArray<FHarmoniaUserSearchResult> Results;

	// 친구 목록에서 검색어와 일치하는 친구 찾기
	for (const FHarmoniaFriendInfo& Friend : CachedFriendList)
	{
		if (Friend.DisplayName.Contains(SearchQuery) || Friend.UserId.Contains(SearchQuery))
		{
			FHarmoniaUserSearchResult Result;
			Result.UserId = Friend.UserId;
			Result.DisplayName = Friend.DisplayName;
			Result.Status = Friend.Status;
			Result.bIsFriend = true;
			Result.bHasPendingRequest = false;

			Results.Add(Result);

			if (Results.Num() >= MaxResults)
			{
				break;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Found %d users matching '%s'"), Results.Num(), *SearchQuery);
	OnUserSearchCompleted.Broadcast(true, Results);

	// TODO: 백엔드 서버가 있다면 HTTP 요청으로 전체 사용자 검색 구현 가능
	// 예: GET /api/users/search?query={SearchQuery}&limit={MaxResults}
}

void UHarmoniaOnlineSubsystem::SendFriendRequest(const FString& UserId, const FString& Message)
{
	if (UserId.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Cannot send friend request to empty UserId"));
		OnFriendRequestResult.Broadcast(false, TEXT("Invalid User ID"));
		return;
	}

	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: FriendsInterface is not valid"));
		OnFriendRequestResult.Broadcast(false, TEXT("Friends interface not available"));
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

	// FString을 FUniqueNetId로 변환
	TSharedPtr<const FUniqueNetId> FriendId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(UserId);

	if (!FriendId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to create UniqueNetId from UserId: %s"), *UserId);
		OnFriendRequestResult.Broadcast(false, TEXT("Invalid User ID format"));
		return;
	}

	// Steam/EOS API를 통해 친구 요청 전송
	bool bResult = FriendsInterface->SendInvite(0, *FriendId, EFriendsLists::ToString(EFriendsLists::Default), FOnSendInviteComplete());

	if (!bResult)
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to send friend invite to %s"), *UserId);
		OnFriendRequestResult.Broadcast(false, TEXT("Failed to send invite"));
	}
	// 결과는 OnSendInviteComplete 델리게이트에서 처리됨
}

void UHarmoniaOnlineSubsystem::AcceptFriendRequest(const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Accepting friend request from %s"), *UserId);

	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: FriendsInterface is not valid"));
		return;
	}

	// FString을 FUniqueNetId로 변환
	TSharedPtr<const FUniqueNetId> FriendId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(UserId);

	if (!FriendId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to create UniqueNetId from UserId: %s"), *UserId);
		return;
	}

	// Steam/EOS API를 통해 친구 요청 수락
	bool bResult = FriendsInterface->AcceptInvite(0, *FriendId, EFriendsLists::ToString(EFriendsLists::Default), FOnAcceptInviteComplete());

	if (bResult)
	{
		// 요청 목록에서 제거
		int32 RequestIndex = PendingFriendRequests.IndexOfByPredicate([&UserId](const FHarmoniaFriendRequest& Request)
		{
			return Request.UserId == UserId;
		});

		if (RequestIndex != INDEX_NONE)
		{
			PendingFriendRequests.RemoveAt(RequestIndex);
		}

		// 친구 목록 갱신
		RefreshFriendList();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to accept friend request from %s"), *UserId);
	}
}

void UHarmoniaOnlineSubsystem::RejectFriendRequest(const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Rejecting friend request from %s"), *UserId);

	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: FriendsInterface is not valid"));
		return;
	}

	// FString을 FUniqueNetId로 변환
	TSharedPtr<const FUniqueNetId> FriendId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(UserId);

	if (!FriendId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to create UniqueNetId from UserId: %s"), *UserId);
		return;
	}

	// Steam/EOS API를 통해 친구 요청 거절
	bool bResult = FriendsInterface->RejectInvite(0, *FriendId, EFriendsLists::ToString(EFriendsLists::Default));

	if (bResult)
	{
		// 요청 목록에서 제거
		int32 RequestIndex = PendingFriendRequests.IndexOfByPredicate([&UserId](const FHarmoniaFriendRequest& Request)
		{
			return Request.UserId == UserId;
		});

		if (RequestIndex != INDEX_NONE)
		{
			PendingFriendRequests.RemoveAt(RequestIndex);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to reject friend request from %s"), *UserId);
	}
}

void UHarmoniaOnlineSubsystem::RemoveFriend(const FString& UserId)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Removing friend %s"), *UserId);

	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: FriendsInterface is not valid"));
		return;
	}

	// FString을 FUniqueNetId로 변환
	TSharedPtr<const FUniqueNetId> FriendId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(UserId);

	if (!FriendId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to create UniqueNetId from UserId: %s"), *UserId);
		return;
	}

	// Steam/EOS API를 통해 친구 삭제
	bool bResult = FriendsInterface->DeleteFriend(0, *FriendId, EFriendsLists::ToString(EFriendsLists::Default));

	if (bResult)
	{
		// 친구 목록에서 제거
		int32 FriendIndex = CachedFriendList.IndexOfByPredicate([&UserId](const FHarmoniaFriendInfo& Friend)
		{
			return Friend.UserId == UserId;
		});

		if (FriendIndex != INDEX_NONE)
		{
			CachedFriendList.RemoveAt(FriendIndex);
		}

		// 이벤트 브로드캐스트
		OnFriendListUpdated.Broadcast(CachedFriendList);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to remove friend %s"), *UserId);
	}
}

void UHarmoniaOnlineSubsystem::RefreshFriendList()
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Refreshing friend list"));

	if (!FriendsInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: FriendsInterface is not valid"));
		return;
	}

	// Steam/EOS API를 통해 친구 목록 읽기
	// 결과는 OnReadFriendsListComplete 델리게이트에서 처리됨
	bool bResult = FriendsInterface->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::Default), FOnReadFriendsListComplete());

	if (!bResult)
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to read friends list"));
	}
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
		// 게임 세션 참가 로직
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Joining game session %s"), *Invite.SessionId);
		if (SessionInterface.IsValid())
		{
			// SessionId를 FName으로 변환하여 세션 참가
			FName SessionName(*Invite.SessionId);

			// TODO: SearchResult를 통해 세션을 찾아야 하므로
			// 실제로는 세션 검색 후 참가하는 로직이 필요합니다
			// SessionInterface->JoinSession(0, SessionName, SearchResult);

			UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Session join requires search result - implement session search first"));
		}
		break;

	case EHarmoniaInviteType::Party:
		// 파티 참가 로직 (보통 파티도 세션을 사용)
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Joining party"));
		// Party 시스템은 게임마다 다르므로 프로젝트에 맞게 구현 필요
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

		// Note: Steam/EOS는 초대 거절을 명시적으로 알리는 API가 없습니다
		// 초대를 무시하면 자동으로 만료됩니다
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Invite rejected (removed from pending list)"));
	}
}

//~=============================================================================
// 음성 대화 기능
//~=============================================================================

FString UHarmoniaOnlineSubsystem::CreateVoiceChannel(const FString& ChannelName, int32 MaxParticipants, bool bIsPrivate)
{
	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Creating voice channel '%s'"), *ChannelName);

	FHarmoniaVoiceChannelInfo ChannelInfo;
	ChannelInfo.ChannelId = ChannelName; // EOS는 채널 이름을 ID로 사용
	ChannelInfo.ChannelName = ChannelName;
	ChannelInfo.MaxParticipants = MaxParticipants;
	ChannelInfo.bIsPrivate = bIsPrivate;

	VoiceChannels.Add(ChannelName, ChannelInfo);

	// Note: EOS Voice Chat은 채널을 자동으로 생성합니다
	// 참가할 때 채널이 없으면 생성됩니다

	return ChannelName;
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

	if (!VoiceChat.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: VoiceChat interface not available, using fallback"));

		// VoiceChat이 없으면 기본 동작만 수행 (시뮬레이션)
		CurrentVoiceChannelId = ChannelId;
		UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Connected);

		if (FHarmoniaVoiceChannelInfo* ChannelInfo = VoiceChannels.Find(ChannelId))
		{
			if (!ChannelInfo->ParticipantIds.Contains(CurrentUserId))
			{
				ChannelInfo->ParticipantIds.Add(CurrentUserId);
			}
		}
		return;
	}

	CurrentVoiceChannelId = ChannelId;
	UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Connecting);

	// EOS Voice Chat으로 채널 참가
	VoiceChat->JoinChannel(ChannelId, TEXT(""), EVoiceChatChannelType::NonPositional,
		FOnVoiceChatChannelJoinCompleteDelegate::CreateLambda([this, ChannelId](const FString& JoinedChannelName, const FVoiceChatResult& Result)
		{
			if (Result.IsSuccess())
			{
				UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Connected);

				// 채널 참가자 목록에 자신 추가
				if (FHarmoniaVoiceChannelInfo* ChannelInfo = VoiceChannels.Find(ChannelId))
				{
					if (!ChannelInfo->ParticipantIds.Contains(CurrentUserId))
					{
						ChannelInfo->ParticipantIds.Add(CurrentUserId);
					}
				}

				UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Successfully joined voice channel %s"), *ChannelId);
			}
			else
			{
				UpdateVoiceChannelStatus(ChannelId, EHarmoniaVoiceChatStatus::Disconnected);
				CurrentVoiceChannelId.Empty();
				UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to join voice channel %s: %s"), *ChannelId, *Result.ErrorDesc);
			}
		}),
		FOnVoiceChatChannelLeaveCompleteDelegate());
}

void UHarmoniaOnlineSubsystem::LeaveVoiceChannel(const FString& ChannelId)
{
	if (ChannelId.IsEmpty())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Leaving voice channel %s"), *ChannelId);

	// EOS Voice Chat으로 채널 나가기
	if (VoiceChat.IsValid())
	{
		VoiceChat->LeaveChannel(ChannelId, FOnVoiceChatChannelLeaveCompleteDelegate::CreateLambda(
			[this, ChannelId](const FString& LeftChannelName, const FVoiceChatResult& Result)
			{
				if (Result.IsSuccess())
				{
					UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Successfully left voice channel %s"), *ChannelId);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Failed to leave voice channel %s: %s"),
						*ChannelId, *Result.ErrorDesc);
				}
			}));
	}

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

	// EOS Voice Chat으로 마이크 음소거 설정
	if (VoiceChat.IsValid())
	{
		VoiceChat->SetAudioInputDeviceMuted(bMuted);
	}

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

	// EOS Voice Chat으로 특정 사용자 음소거 설정
	if (VoiceChat.IsValid() && !CurrentVoiceChannelId.IsEmpty())
	{
		VoiceChat->SetPlayerMuted(UserId, bMuted);
	}
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

	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: OnlineSubsystem is not valid"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Connecting to online services..."));

	IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: IdentityInterface is not valid"));
		return;
	}

	// 로그인 델리게이트 바인딩
	OnLoginCompleteHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnLoginComplete));

	// Steam/EOS는 자동 로그인을 시도합니다
	// 수동 로그인이 필요한 경우 FOnlineAccountCredentials를 전달해야 합니다
	FOnlineAccountCredentials Credentials;
	Credentials.Type = TEXT("accountportal"); // EOS의 경우
	// Steam의 경우 자동으로 Steam 계정으로 로그인됩니다

	bool bResult = IdentityInterface->Login(0, Credentials);

	if (!bResult)
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to initiate login"));
		IdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, OnLoginCompleteHandle);
	}
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

	// Steam/EOS 로그아웃
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			IdentityInterface->Logout(0);
		}
	}

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

	// 친구 목록 갱신 (비동기)
	RefreshFriendList();

	// Note: 실제 상태 변경은 OnReadFriendsListComplete 델리게이트에서 처리됩니다
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

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Sending invite to %s (Type: %d)"), *UserId, static_cast<int32>(InviteType));

	// Note: Steam/EOS의 초대 시스템은 게임 세션과 연동됩니다
	// 세션 기반 초대의 경우 SessionInterface->SendSessionInviteToFriend()를 사용합니다

	if (InviteType == EHarmoniaInviteType::Game && SessionInterface.IsValid())
	{
		// 게임 세션 초대
		TSharedPtr<const FUniqueNetId> FriendId = OnlineSubsystem->GetIdentityInterface()->CreateUniquePlayerId(UserId);
		if (FriendId.IsValid())
		{
			// 현재 활성 세션에 초대 (세션 이름은 AdditionalData로 전달됨)
			FName SessionName = AdditionalData.IsEmpty() ? NAME_GameSession : FName(*AdditionalData);
			bool bResult = SessionInterface->SendSessionInviteToFriend(0, SessionName, *FriendId);

			if (bResult)
			{
				UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Game session invite sent to %s"), *UserId);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to send game session invite to %s"), *UserId);
			}
		}
	}
	else
	{
		// 음성 채팅 또는 파티 초대는 커스텀 메시지 시스템이 필요합니다
		// 백엔드 서버가 있다면 HTTP 요청으로 전송하거나,
		// P2P 메시징 시스템을 구현해야 합니다

		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Non-session invites require custom backend or messaging system"));
		// TODO: 백엔드 API를 통한 초대 전송 구현
	}
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

//~=============================================================================
// 음성 효과 프리셋 (정적 함수)
//~=============================================================================

FHarmoniaVoiceEffectSettings FHarmoniaVoiceEffectSettings::FromPreset(EHarmoniaEnvironmentPreset Preset)
{
	FHarmoniaVoiceEffectSettings Settings;

	switch (Preset)
	{
	case EHarmoniaEnvironmentPreset::Default:
		Settings.EffectType = EHarmoniaVoiceEffectType::None;
		Settings.bEnabled = false;
		break;

	case EHarmoniaEnvironmentPreset::SmallRoom:
		Settings.EffectType = EHarmoniaVoiceEffectType::Reverb;
		Settings.Intensity = 0.3f;
		Settings.DecayTime = 0.5f;
		Settings.DelayTime = 0.02f;
		Settings.Density = 0.7f;
		Settings.Diffusion = 0.5f;
		Settings.DryWetMix = 0.2f;
		break;

	case EHarmoniaEnvironmentPreset::LargeRoom:
		Settings.EffectType = EHarmoniaVoiceEffectType::Reverb;
		Settings.Intensity = 0.5f;
		Settings.DecayTime = 1.2f;
		Settings.DelayTime = 0.05f;
		Settings.Density = 0.6f;
		Settings.Diffusion = 0.7f;
		Settings.DryWetMix = 0.35f;
		break;

	case EHarmoniaEnvironmentPreset::Hall:
		Settings.EffectType = EHarmoniaVoiceEffectType::Reverb;
		Settings.Intensity = 0.7f;
		Settings.DecayTime = 2.5f;
		Settings.DelayTime = 0.1f;
		Settings.Density = 0.5f;
		Settings.Diffusion = 0.9f;
		Settings.DryWetMix = 0.5f;
		break;

	case EHarmoniaEnvironmentPreset::Cave:
		Settings.EffectType = EHarmoniaVoiceEffectType::Cave;
		Settings.Intensity = 0.8f;
		Settings.DecayTime = 3.0f;
		Settings.DelayTime = 0.15f;
		Settings.Density = 0.8f;
		Settings.Diffusion = 0.6f;
		Settings.LowPassCutoff = 3000.0f; // 동굴은 고음이 감쇠됨
		Settings.DryWetMix = 0.6f;
		break;

	case EHarmoniaEnvironmentPreset::Underwater:
		Settings.EffectType = EHarmoniaVoiceEffectType::Underwater;
		Settings.Intensity = 0.9f;
		Settings.DecayTime = 1.5f;
		Settings.DelayTime = 0.08f;
		Settings.Density = 1.0f;
		Settings.Diffusion = 0.8f;
		Settings.LowPassCutoff = 1000.0f; // 물 속에서는 고음이 많이 감쇠됨
		Settings.DryWetMix = 0.8f;
		break;

	case EHarmoniaEnvironmentPreset::Outdoor:
		Settings.EffectType = EHarmoniaVoiceEffectType::None;
		Settings.Intensity = 0.1f;
		Settings.DecayTime = 0.3f;
		Settings.DelayTime = 0.01f;
		Settings.Density = 0.2f;
		Settings.Diffusion = 0.3f;
		Settings.DryWetMix = 0.1f;
		break;

	case EHarmoniaEnvironmentPreset::Forest:
		Settings.EffectType = EHarmoniaVoiceEffectType::Reverb;
		Settings.Intensity = 0.4f;
		Settings.DecayTime = 0.8f;
		Settings.DelayTime = 0.03f;
		Settings.Density = 0.6f;
		Settings.Diffusion = 0.7f;
		Settings.HighPassCutoff = 150.0f; // 숲에서는 저음이 약간 감쇠됨
		Settings.DryWetMix = 0.25f;
		break;

	case EHarmoniaEnvironmentPreset::Mountain:
		Settings.EffectType = EHarmoniaVoiceEffectType::Echo;
		Settings.Intensity = 0.6f;
		Settings.DecayTime = 2.0f;
		Settings.DelayTime = 0.3f;
		Settings.Density = 0.3f;
		Settings.Diffusion = 0.4f;
		Settings.DryWetMix = 0.4f;
		break;

	case EHarmoniaEnvironmentPreset::Canyon:
		Settings.EffectType = EHarmoniaVoiceEffectType::Echo;
		Settings.Intensity = 0.85f;
		Settings.DecayTime = 4.0f;
		Settings.DelayTime = 0.5f;
		Settings.Density = 0.4f;
		Settings.Diffusion = 0.5f;
		Settings.DryWetMix = 0.7f;
		break;
	}

	return Settings;
}

//~=============================================================================
// 음성 효과 함수 구현
//~=============================================================================

void UHarmoniaOnlineSubsystem::ApplyVoiceEffect(const FHarmoniaVoiceEffectSettings& Settings)
{
	CurrentVoiceEffectSettings = Settings;

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Applying voice effect (Type: %d, Intensity: %.2f)"),
		static_cast<int32>(Settings.EffectType), Settings.Intensity);

	// TODO: 실제 음성 SDK에 효과 적용
	// 예: Vivox, EOS Voice, Agora 등의 SDK API 호출
	// SDK->SetVoiceEffect(Settings);
}

void UHarmoniaOnlineSubsystem::ApplyEnvironmentPreset(EHarmoniaEnvironmentPreset Preset)
{
	FHarmoniaVoiceEffectSettings Settings = FHarmoniaVoiceEffectSettings::FromPreset(Preset);
	ApplyVoiceEffect(Settings);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Applied environment preset: %d"), static_cast<int32>(Preset));
}

void UHarmoniaOnlineSubsystem::ApplyVoiceEffectToUser(const FString& UserId, const FHarmoniaVoiceEffectSettings& Settings)
{
	UserVoiceEffects.Add(UserId, Settings);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Applying voice effect to user %s"), *UserId);

	// TODO: 실제 음성 SDK에 사용자별 효과 적용
	// SDK->SetUserVoiceEffect(UserId, Settings);
}

void UHarmoniaOnlineSubsystem::ClearVoiceEffect()
{
	CurrentVoiceEffectSettings = FHarmoniaVoiceEffectSettings();
	CurrentVoiceEffectSettings.bEnabled = false;

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Cleared voice effect"));

	// TODO: 실제 음성 SDK에서 효과 제거
	// SDK->ClearVoiceEffect();
}

void UHarmoniaOnlineSubsystem::ClearVoiceEffectFromUser(const FString& UserId)
{
	UserVoiceEffects.Remove(UserId);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Cleared voice effect from user %s"), *UserId);

	// TODO: 실제 음성 SDK에서 사용자별 효과 제거
	// SDK->ClearUserVoiceEffect(UserId);
}

void UHarmoniaOnlineSubsystem::SetVoiceEffectIntensity(float Intensity)
{
	CurrentVoiceEffectSettings.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Set voice effect intensity to %.2f"), Intensity);

	// Note: 음성 효과는 언리얼 엔진의 Audio Modulation 시스템이나
	// Submix Effects를 사용하여 구현할 수 있습니다
	// EOS Voice Chat 자체는 음성 효과를 제공하지 않습니다
}

//~=============================================================================
// OnlineSubsystem 델리게이트 핸들러
//~=============================================================================

void UHarmoniaOnlineSubsystem::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to read friends list: %s"), *ErrorStr);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Friends list read successfully"));

	if (!FriendsInterface.IsValid())
	{
		return;
	}

	// 친구 목록을 Harmonia 형식으로 변환
	TArray<FHarmoniaFriendInfo> NewFriendList;

	TArray<TSharedRef<FOnlineFriend>> Friends;
	FriendsInterface->GetFriendsList(LocalUserNum, ListName, Friends);

	for (const TSharedRef<FOnlineFriend>& Friend : Friends)
	{
		FHarmoniaFriendInfo FriendInfo;
		FriendInfo.UserId = Friend->GetUserId()->ToString();
		FriendInfo.DisplayName = Friend->GetDisplayName();

		// 온라인 상태 변환
		FOnlineUserPresence Presence = Friend->GetPresence();
		if (Presence.bIsOnline)
		{
			if (Presence.bIsPlaying)
			{
				FriendInfo.Status = EHarmoniaFriendStatus::InGame;
			}
			else
			{
				FriendInfo.Status = EHarmoniaFriendStatus::Online;
			}
		}
		else
		{
			FriendInfo.Status = EHarmoniaFriendStatus::Offline;
		}

		FriendInfo.FriendSince = FDateTime::UtcNow(); // TODO: 실제 친구 추가 날짜 가져오기

		NewFriendList.Add(FriendInfo);
	}

	CachedFriendList = NewFriendList;
	OnFriendListUpdated.Broadcast(CachedFriendList);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Cached %d friends"), CachedFriendList.Num());
}

void UHarmoniaOnlineSubsystem::OnSendInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Friend invite sent successfully to %s"), *FriendId.ToString());
		OnFriendRequestResult.Broadcast(true, TEXT("Friend request sent"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to send friend invite: %s"), *ErrorStr);
		OnFriendRequestResult.Broadcast(false, ErrorStr);
	}
}

void UHarmoniaOnlineSubsystem::OnAcceptInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Friend invite accepted successfully from %s"), *FriendId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to accept friend invite: %s"), *ErrorStr);
	}
}

void UHarmoniaOnlineSubsystem::OnDeleteFriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Friend deleted successfully: %s"), *FriendId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to delete friend: %s"), *ErrorStr);
	}
}

void UHarmoniaOnlineSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Session created successfully: %s"), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to create session: %s"), *SessionName.ToString());
	}
}

void UHarmoniaOnlineSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Session joined successfully: %s"), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to join session: %s (Result: %d)"), *SessionName.ToString(), static_cast<int32>(Result));
	}
}

void UHarmoniaOnlineSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		CurrentUserId = UserId.ToString();

		// 사용자 이름 가져오기
		if (OnlineSubsystem)
		{
			IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
			if (IdentityInterface.IsValid())
			{
				CurrentUserDisplayName = IdentityInterface->GetPlayerNickname(LocalUserNum);
			}
		}

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

		// 초기 친구 목록 로드 (비동기)
		RefreshFriendList();

		// EOS Voice Chat 연결
		if (VoiceChat.IsValid())
		{
			VoiceChat->Connect(FOnVoiceChatConnectCompleteDelegate::CreateUObject(this, &UHarmoniaOnlineSubsystem::OnVoiceChatConnectComplete));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HarmoniaOnlineSubsystem: Failed to login: %s"), *Error);
	}
}

void UHarmoniaOnlineSubsystem::OnVoiceChatConnectComplete(const FVoiceChatResult& Result)
{
	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("HarmoniaOnlineSubsystem: Voice chat connected successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HarmoniaOnlineSubsystem: Voice chat connection failed: %s"), *Result.ErrorDesc);
	}
}

void UHarmoniaOnlineSubsystem::OnVoiceChatChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
	// 이 핸들러는 JoinVoiceChannel에서 람다로 직접 처리됨
}

void UHarmoniaOnlineSubsystem::OnVoiceChatChannelLeaveComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
	// 이 핸들러는 LeaveVoiceChannel에서 람다로 직접 처리됨
}
