// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaOnlineTypes.h"
#include "HarmoniaOnlineAsyncTasks.h"
#include "HarmoniaOnlineSubsystem.generated.h"

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendListUpdated, const TArray<FHarmoniaFriendInfo>&, FriendList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendStatusChanged, const FHarmoniaFriendInfo&, FriendInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendRequestReceived, const FHarmoniaFriendRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendRequestResult, bool, bSuccess, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteReceived, const FHarmoniaInviteInfo&, InviteInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceChatStatusChanged, const FString&, ChannelId, EHarmoniaVoiceChatStatus, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUserSearchCompleted, bool, bSuccess, const TArray<FHarmoniaUserSearchResult>&, Results);

/**
 * Harmonia 온라인 서브시스템
 *
 * 친구 관리, 초대, 음성 대화 등의 커뮤니티 기능을 제공합니다.
 * UI에 필요한 데이터 캐싱과 기능 함수들을 제공하며, UI는 직접 생성하지 않습니다.
 */
UCLASS()
class HARMONIAONLINESUBSYSTEM_API UHarmoniaOnlineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaOnlineSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End of USubsystem interface

	//~=============================================================================
	// 친구 관리 기능
	//~=============================================================================

	/**
	 * 사용자 이름으로 검색
	 * @param SearchQuery 검색어 (사용자 이름)
	 * @param MaxResults 최대 결과 개수
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Friends")
	void SearchUsers(const FString& SearchQuery, int32 MaxResults = 20);

	/**
	 * 친구 요청 보내기
	 * @param UserId 친구 요청을 보낼 사용자 ID
	 * @param Message 요청과 함께 보낼 메시지 (선택사항)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Friends")
	void SendFriendRequest(const FString& UserId, const FString& Message = TEXT(""));

	/**
	 * 친구 요청 수락
	 * @param UserId 요청을 보낸 사용자 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Friends")
	void AcceptFriendRequest(const FString& UserId);

	/**
	 * 친구 요청 거절
	 * @param UserId 요청을 보낸 사용자 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Friends")
	void RejectFriendRequest(const FString& UserId);

	/**
	 * 친구 제거
	 * @param UserId 제거할 친구의 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Friends")
	void RemoveFriend(const FString& UserId);

	/**
	 * 친구 목록 새로고침
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Friends")
	void RefreshFriendList();

	/**
	 * 캐싱된 친구 목록 가져오기
	 * @return 현재 캐싱된 친구 목록
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Friends")
	const TArray<FHarmoniaFriendInfo>& GetCachedFriendList() const { return CachedFriendList; }

	/**
	 * 특정 친구 정보 가져오기
	 * @param UserId 친구 ID
	 * @param OutFriendInfo 친구 정보 출력
	 * @return 친구를 찾았는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Friends")
	bool GetFriendInfo(const FString& UserId, FHarmoniaFriendInfo& OutFriendInfo) const;

	/**
	 * 받은 친구 요청 목록 가져오기
	 * @return 받은 친구 요청 목록
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Friends")
	const TArray<FHarmoniaFriendRequest>& GetPendingFriendRequests() const { return PendingFriendRequests; }

	/**
	 * 상태별로 친구 목록 필터링
	 * @param Status 필터링할 상태
	 * @return 해당 상태의 친구 목록
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Friends")
	TArray<FHarmoniaFriendInfo> GetFriendsByStatus(EHarmoniaFriendStatus Status) const;

	//~=============================================================================
	// 초대 시스템
	//~=============================================================================

	/**
	 * 친구를 게임에 초대
	 * @param UserId 초대할 친구 ID
	 * @param SessionId 게임 세션 ID
	 * @param Message 초대 메시지
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Invite")
	void InviteToGame(const FString& UserId, const FString& SessionId, const FString& Message = TEXT(""));

	/**
	 * 친구를 파티에 초대
	 * @param UserId 초대할 친구 ID
	 * @param Message 초대 메시지
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Invite")
	void InviteToParty(const FString& UserId, const FString& Message = TEXT(""));

	/**
	 * 친구를 음성 채팅에 초대
	 * @param UserId 초대할 친구 ID
	 * @param ChannelId 음성 채널 ID
	 * @param Message 초대 메시지
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Invite")
	void InviteToVoiceChat(const FString& UserId, const FString& ChannelId, const FString& Message = TEXT(""));

	/**
	 * 초대 수락
	 * @param InviteId 초대 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Invite")
	void AcceptInvite(const FString& InviteId);

	/**
	 * 초대 거절
	 * @param InviteId 초대 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Invite")
	void RejectInvite(const FString& InviteId);

	/**
	 * 받은 초대 목록 가져오기
	 * @return 받은 초대 목록
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Invite")
	const TArray<FHarmoniaInviteInfo>& GetPendingInvites() const { return PendingInvites; }

	//~=============================================================================
	// 음성 대화 기능
	//~=============================================================================

	/**
	 * 음성 채널 생성
	 * @param ChannelName 채널 이름
	 * @param MaxParticipants 최대 참가자 수
	 * @param bIsPrivate 비공개 채널 여부
	 * @return 생성된 채널 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice")
	FString CreateVoiceChannel(const FString& ChannelName, int32 MaxParticipants = 16, bool bIsPrivate = false);

	/**
	 * 음성 채널 참여
	 * @param ChannelId 참여할 채널 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice")
	void JoinVoiceChannel(const FString& ChannelId);

	/**
	 * 음성 채널 퇴장
	 * @param ChannelId 퇴장할 채널 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice")
	void LeaveVoiceChannel(const FString& ChannelId);

	/**
	 * 마이크 음소거 설정
	 * @param bMuted 음소거 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice")
	void SetMicrophoneMuted(bool bMuted);

	/**
	 * 특정 사용자 음소거 설정
	 * @param UserId 음소거할 사용자 ID
	 * @param bMuted 음소거 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice")
	void SetUserMuted(const FString& UserId, bool bMuted);

	/**
	 * 현재 음성 채널 정보 가져오기
	 * @param ChannelId 채널 ID
	 * @param OutChannelInfo 채널 정보 출력
	 * @return 채널을 찾았는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice")
	bool GetVoiceChannelInfo(const FString& ChannelId, FHarmoniaVoiceChannelInfo& OutChannelInfo) const;

	/**
	 * 현재 참여 중인 음성 채널 ID 가져오기
	 * @return 현재 참여 중인 채널 ID (없으면 빈 문자열)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice")
	FString GetCurrentVoiceChannelId() const { return CurrentVoiceChannelId; }

	/**
	 * 마이크 음소거 상태 확인
	 * @return 마이크가 음소거되어 있는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice")
	bool IsMicrophoneMuted() const { return bIsMicrophoneMuted; }

	/**
	 * 음성 채팅 상태 가져오기
	 * @return 현재 음성 채팅 상태
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice")
	EHarmoniaVoiceChatStatus GetVoiceChatStatus() const { return VoiceChatStatus; }

	//~=============================================================================
	// 연결 및 상태 관리
	//~=============================================================================

	/**
	 * 온라인 서비스에 연결
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online")
	void Connect();

	/**
	 * 온라인 서비스에서 연결 해제
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online")
	void Disconnect();

	/**
	 * 연결 상태 확인
	 * @return 연결되어 있는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online")
	bool IsConnected() const { return bIsConnected; }

	/**
	 * 현재 사용자 ID 가져오기
	 * @return 현재 로그인한 사용자의 ID
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online")
	FString GetCurrentUserId() const { return CurrentUserId; }

	/**
	 * 현재 사용자 이름 가져오기
	 * @return 현재 로그인한 사용자의 표시 이름
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online")
	FString GetCurrentUserDisplayName() const { return CurrentUserDisplayName; }

	//~=============================================================================
	// 이벤트 델리게이트
	//~=============================================================================

	/** 친구 목록이 업데이트되었을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnFriendListUpdated OnFriendListUpdated;

	/** 친구 상태가 변경되었을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnFriendStatusChanged OnFriendStatusChanged;

	/** 친구 요청을 받았을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnFriendRequestReceived OnFriendRequestReceived;

	/** 친구 요청 결과 (성공/실패) */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnFriendRequestResult OnFriendRequestResult;

	/** 초대를 받았을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnInviteReceived OnInviteReceived;

	/** 음성 채팅 상태가 변경되었을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnVoiceChatStatusChanged OnVoiceChatStatusChanged;

	/** 사용자 검색 완료 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Online|Events")
	FOnUserSearchCompleted OnUserSearchCompleted;

private:
	//~=============================================================================
	// 내부 헬퍼 함수
	//~=============================================================================

	/** 친구 목록 캐시 업데이트 */
	void UpdateFriendListCache();

	/** 친구 요청 시뮬레이션 (서버 구현 전 테스트용) */
	void SimulateFriendRequestResponse(const FString& UserId, bool bSuccess);

	/** 초대 전송 내부 로직 */
	void SendInviteInternal(const FString& UserId, EHarmoniaInviteType InviteType, const FString& Message, const FString& AdditionalData);

	/** 음성 채널 상태 업데이트 */
	void UpdateVoiceChannelStatus(const FString& ChannelId, EHarmoniaVoiceChatStatus NewStatus);

	//~=============================================================================
	// 캐싱된 데이터
	//~=============================================================================

	/** 친구 목록 캐시 */
	UPROPERTY()
	TArray<FHarmoniaFriendInfo> CachedFriendList;

	/** 받은 친구 요청 목록 */
	UPROPERTY()
	TArray<FHarmoniaFriendRequest> PendingFriendRequests;

	/** 받은 초대 목록 */
	UPROPERTY()
	TArray<FHarmoniaInviteInfo> PendingInvites;

	/** 음성 채널 정보 맵 */
	UPROPERTY()
	TMap<FString, FHarmoniaVoiceChannelInfo> VoiceChannels;

	//~=============================================================================
	// 상태 변수
	//~=============================================================================

	/** 온라인 서비스 연결 상태 */
	UPROPERTY()
	bool bIsConnected;

	/** 현재 사용자 ID */
	UPROPERTY()
	FString CurrentUserId;

	/** 현재 사용자 표시 이름 */
	UPROPERTY()
	FString CurrentUserDisplayName;

	/** 현재 참여 중인 음성 채널 ID */
	UPROPERTY()
	FString CurrentVoiceChannelId;

	/** 마이크 음소거 상태 */
	UPROPERTY()
	bool bIsMicrophoneMuted;

	/** 음성 채팅 상태 */
	UPROPERTY()
	EHarmoniaVoiceChatStatus VoiceChatStatus;

	/** 음소거된 사용자 ID 목록 */
	UPROPERTY()
	TSet<FString> MutedUsers;

	/** 타이머 핸들 (주기적 업데이트용) */
	FTimerHandle FriendListUpdateTimerHandle;
};
