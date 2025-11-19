// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaOnlineTypes.h"
#include "HarmoniaOnlineAsyncTasks.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "VoiceChat.h"
#include "HarmoniaOnlineSubsystem.generated.h"

class IVoiceChat;

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendListUpdated, const TArray<FHarmoniaFriendInfo>&, FriendList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendStatusChanged, const FHarmoniaFriendInfo&, FriendInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendRequestReceived, const FHarmoniaFriendRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFriendRequestResult, bool, bSuccess, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHarmoniaInviteReceived, const FHarmoniaInviteInfo&, InviteInfo);
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
	// 음성 효과 (환경 음향 효과)
	//~=============================================================================

	/**
	 * 음성 효과 적용
	 * @param Settings 적용할 효과 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Effects")
	void ApplyVoiceEffect(const FHarmoniaVoiceEffectSettings& Settings);

	/**
	 * 환경 프리셋으로 음성 효과 적용
	 * @param Preset 환경 프리셋
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Effects")
	void ApplyEnvironmentPreset(EHarmoniaEnvironmentPreset Preset);

	/**
	 * 특정 사용자에게만 음성 효과 적용
	 * @param UserId 사용자 ID
	 * @param Settings 적용할 효과 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Effects")
	void ApplyVoiceEffectToUser(const FString& UserId, const FHarmoniaVoiceEffectSettings& Settings);

	/**
	 * 음성 효과 제거
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Effects")
	void ClearVoiceEffect();

	/**
	 * 특정 사용자의 음성 효과 제거
	 * @param UserId 사용자 ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Effects")
	void ClearVoiceEffectFromUser(const FString& UserId);

	/**
	 * 현재 적용된 음성 효과 설정 가져오기
	 * @return 현재 효과 설정
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Effects")
	FHarmoniaVoiceEffectSettings GetCurrentVoiceEffectSettings() const { return CurrentVoiceEffectSettings; }

	/**
	 * 음성 효과가 활성화되어 있는지 확인
	 * @return 효과 활성화 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Effects")
	bool IsVoiceEffectEnabled() const { return CurrentVoiceEffectSettings.bEnabled; }

	/**
	 * 음성 효과 강도 조절
	 * @param Intensity 효과 강도 (0.0 ~ 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Effects")
	void SetVoiceEffectIntensity(float Intensity);

	//~=============================================================================
	// 공간 음성 채팅 (Spatial Voice Chat)
	//~=============================================================================

	/**
	 * 공간 음성 채팅 설정
	 * @param Settings 공간 음성 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Spatial")
	void SetSpatialVoiceSettings(const FHarmoniaSpatialVoiceSettings& Settings);

	/**
	 * 현재 공간 음성 설정 가져오기
	 * @return 현재 공간 음성 설정
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Spatial")
	FHarmoniaSpatialVoiceSettings GetSpatialVoiceSettings() const { return SpatialVoiceSettings; }

	/**
	 * 플레이어의 위치 및 환경 업데이트 (프레임마다 호출)
	 * @param PlayerId 플레이어 ID
	 * @param Location 플레이어 위치
	 * @param Rotation 플레이어 회전
	 * @param Environment 현재 환경 프리셋
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Spatial")
	void UpdatePlayerVoiceTransform(const FString& PlayerId, FVector Location, FRotator Rotation, EHarmoniaEnvironmentPreset Environment);

	/**
	 * 청자(나)의 현재 환경 설정
	 * @param Environment 내가 있는 환경 프리셋
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Spatial")
	void SetListenerEnvironment(EHarmoniaEnvironmentPreset Environment);

	/**
	 * 청자(나)의 현재 환경 가져오기
	 * @return 현재 환경 프리셋
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Spatial")
	EHarmoniaEnvironmentPreset GetListenerEnvironment() const { return ListenerEnvironment; }

	/**
	 * 특정 플레이어의 음성 상태 가져오기
	 * @param PlayerId 플레이어 ID
	 * @param OutState 출력: 플레이어 음성 상태
	 * @return 찾았는지 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Spatial")
	bool GetPlayerVoiceState(const FString& PlayerId, FHarmoniaPlayerVoiceState& OutState) const;

	/**
	 * 모든 플레이어의 음성 상태 가져오기
	 * @return 모든 플레이어의 음성 상태 배열
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Spatial")
	TArray<FHarmoniaPlayerVoiceState> GetAllPlayerVoiceStates() const;

	/**
	 * 공간 음성 시스템 활성화/비활성화
	 * @param bEnabled 활성화 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Online|Voice|Spatial")
	void SetSpatialVoiceEnabled(bool bEnabled);

	/**
	 * 공간 음성 시스템이 활성화되어 있는지 확인
	 * @return 활성화 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Online|Voice|Spatial")
	bool IsSpatialVoiceEnabled() const { return bSpatialVoiceEnabled; }

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
	FOnHarmoniaInviteReceived OnInviteReceived;

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

	/** 공간 음성: 모든 플레이어 음성 상태 업데이트 (타이머) */
	void UpdateAllPlayerVoiceStates();

	/** 공간 음성: 특정 플레이어 음성 효과 계산 및 적용 */
	void UpdatePlayerVoiceEffect(const FString& PlayerId);

	/** 공간 음성: 청자 기준으로 환경 효과 블렌딩 */
	FHarmoniaVoiceEffectSettings CalculateListenerBasedEffect(
		EHarmoniaEnvironmentPreset SpeakerEnvironment,
		EHarmoniaEnvironmentPreset InListenerEnvironment,
		float Distance) const;

	/** 공간 음성: 거리에 따른 감쇠 계산 */
	float CalculateDistanceAttenuation(float Distance) const;

	/** 공간 음성: 장애물 감쇠 체크 (레이캐스트) */
	float CalculateOcclusionAttenuation(const FVector& From, const FVector& To) const;

	//~=============================================================================
	// OnlineSubsystem 델리게이트 핸들러
	//~=============================================================================

	/** 친구 목록 읽기 완료 */
	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	/** 친구 초대 완료 */
	void OnSendInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

	/** 친구 요청 수락 완료 */
	void OnAcceptInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

	/** 친구 삭제 완료 */
	void OnDeleteFriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

	/** 세션 생성 완료 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/** 세션 참가 완료 */
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/** 로그인 완료 */
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	/** 음성 채팅 연결 상태 변경 */
	void OnVoiceChatConnectComplete(const FVoiceChatResult& Result);

	/** 음성 채널 참가 완료 */
	void OnVoiceChatChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result);

	/** 음성 채널 퇴장 완료 */
	void OnVoiceChatChannelLeaveComplete(const FString& ChannelName, const FVoiceChatResult& Result);

	//~=============================================================================
	// OnlineSubsystem 인터페이스
	//~=============================================================================

	/** Primary Online Subsystem (EOS or Steam) */
	IOnlineSubsystem* OnlineSubsystem;

	/** Voice Chat 인터페이스 (EOS Voice) */
	TSharedPtr<IVoiceChat> VoiceChat;

	/** Friends 인터페이스 */
	IOnlineFriendsPtr FriendsInterface;

	/** Session 인터페이스 */
	IOnlineSessionPtr SessionInterface;

	/** User 인터페이스 */
	IOnlineUserPtr UserInterface;

	/** Presence 인터페이스 */
	IOnlinePresencePtr PresenceInterface;

	/** 로그인 델리게이트 핸들 (IOnlineIdentity는 여전히 핸들 패턴 사용) */
	FDelegateHandle OnLoginCompleteHandle;

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

	/** 현재 적용된 음성 효과 설정 */
	UPROPERTY()
	FHarmoniaVoiceEffectSettings CurrentVoiceEffectSettings;

	/** 사용자별 음성 효과 설정 맵 */
	UPROPERTY()
	TMap<FString, FHarmoniaVoiceEffectSettings> UserVoiceEffects;

	//~=============================================================================
	// 공간 음성 채팅 변수
	//~=============================================================================

	/** 공간 음성 설정 */
	UPROPERTY()
	FHarmoniaSpatialVoiceSettings SpatialVoiceSettings;

	/** 플레이어별 음성 상태 맵 (PlayerId -> State) */
	UPROPERTY()
	TMap<FString, FHarmoniaPlayerVoiceState> PlayerVoiceStates;

	/** 청자(나)의 현재 환경 */
	UPROPERTY()
	EHarmoniaEnvironmentPreset ListenerEnvironment;

	/** 청자(나)의 현재 위치 */
	UPROPERTY()
	FVector ListenerLocation;

	/** 청자(나)의 현재 회전 */
	UPROPERTY()
	FRotator ListenerRotation;

	/** 공간 음성 시스템 활성화 여부 */
	UPROPERTY()
	bool bSpatialVoiceEnabled;

	/** 타이머 핸들 (주기적 업데이트용) */
	FTimerHandle FriendListUpdateTimerHandle;

	/** 공간 음성 업데이트 타이머 핸들 */
	FTimerHandle SpatialVoiceUpdateTimerHandle;
};
