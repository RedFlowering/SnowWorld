// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "HarmoniaOnlineTypes.generated.h"

/**
 * 친구 상태 열거형
 */
UENUM(BlueprintType)
enum class EHarmoniaFriendStatus : uint8
{
	Offline UMETA(DisplayName = "오프라인"),
	Online UMETA(DisplayName = "온라인"),
	InGame UMETA(DisplayName = "게임 중"),
	Away UMETA(DisplayName = "자리 비움"),
	Busy UMETA(DisplayName = "다른 용무 중")
};

/**
 * 친구 요청 상태
 */
UENUM(BlueprintType)
enum class EHarmoniaFriendRequestStatus : uint8
{
	None UMETA(DisplayName = "없음"),
	Pending UMETA(DisplayName = "대기 중"),
	Accepted UMETA(DisplayName = "수락됨"),
	Rejected UMETA(DisplayName = "거절됨")
};

/**
 * 음성 대화 상태
 */
UENUM(BlueprintType)
enum class EHarmoniaVoiceChatStatus : uint8
{
	Disconnected UMETA(DisplayName = "연결 안 됨"),
	Connecting UMETA(DisplayName = "연결 중"),
	Connected UMETA(DisplayName = "연결됨"),
	Speaking UMETA(DisplayName = "말하는 중"),
	Muted UMETA(DisplayName = "음소거됨")
};

/**
 * 초대 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaInviteType : uint8
{
	Game UMETA(DisplayName = "게임 초대"),
	Party UMETA(DisplayName = "파티 초대"),
	VoiceChat UMETA(DisplayName = "음성 채팅 초대")
};

/**
 * 친구 정보 구조체
 */
USTRUCT(BlueprintType)
struct HARMONIAONLINESUBSYSTEM_API FHarmoniaFriendInfo
{
	GENERATED_BODY()

	/** 친구의 고유 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString UserId;

	/** 친구의 표시 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString DisplayName;

	/** 현재 친구 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	EHarmoniaFriendStatus Status;

	/** 친구가 현재 플레이 중인 게임 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString CurrentGameName;

	/** 마지막 접속 시간 (UTC Timestamp) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FDateTime LastOnlineTime;

	/** 프로필 이미지 URL */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString AvatarUrl;

	/** 친구 추가 날짜 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FDateTime FriendSince;

	FHarmoniaFriendInfo()
		: Status(EHarmoniaFriendStatus::Offline)
		, LastOnlineTime(FDateTime::MinValue())
		, FriendSince(FDateTime::MinValue())
	{
	}
};

/**
 * 친구 요청 정보 구조체
 */
USTRUCT(BlueprintType)
struct HARMONIAONLINESUBSYSTEM_API FHarmoniaFriendRequest
{
	GENERATED_BODY()

	/** 요청한 사용자 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString UserId;

	/** 사용자 표시 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString DisplayName;

	/** 요청 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	EHarmoniaFriendRequestStatus Status;

	/** 요청 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FDateTime RequestTime;

	/** 요청 메시지 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString Message;

	FHarmoniaFriendRequest()
		: Status(EHarmoniaFriendRequestStatus::None)
		, RequestTime(FDateTime::MinValue())
	{
	}
};

/**
 * 음성 채널 정보 구조체
 */
USTRUCT(BlueprintType)
struct HARMONIAONLINESUBSYSTEM_API FHarmoniaVoiceChannelInfo
{
	GENERATED_BODY()

	/** 채널 고유 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString ChannelId;

	/** 채널 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString ChannelName;

	/** 채널에 참여 중인 사용자 ID 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	TArray<FString> ParticipantIds;

	/** 최대 참가자 수 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	int32 MaxParticipants;

	/** 채널이 비공개인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	bool bIsPrivate;

	FHarmoniaVoiceChannelInfo()
		: MaxParticipants(16)
		, bIsPrivate(false)
	{
	}
};

/**
 * 초대 정보 구조체
 */
USTRUCT(BlueprintType)
struct HARMONIAONLINESUBSYSTEM_API FHarmoniaInviteInfo
{
	GENERATED_BODY()

	/** 초대 고유 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString InviteId;

	/** 초대를 보낸 사용자 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString SenderId;

	/** 초대를 보낸 사용자 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString SenderName;

	/** 초대 타입 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	EHarmoniaInviteType InviteType;

	/** 초대 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FDateTime InviteTime;

	/** 초대 메시지 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString Message;

	/** 세션 ID (게임/파티 초대인 경우) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString SessionId;

	/** 채널 ID (음성 채팅 초대인 경우) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString ChannelId;

	FHarmoniaInviteInfo()
		: InviteType(EHarmoniaInviteType::Game)
		, InviteTime(FDateTime::MinValue())
	{
	}
};

/**
 * 사용자 검색 결과 구조체
 */
USTRUCT(BlueprintType)
struct HARMONIAONLINESUBSYSTEM_API FHarmoniaUserSearchResult
{
	GENERATED_BODY()

	/** 사용자 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString UserId;

	/** 사용자 표시 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	FString DisplayName;

	/** 사용자 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	EHarmoniaFriendStatus Status;

	/** 이미 친구인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	bool bIsFriend;

	/** 친구 요청을 보냈는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Online")
	bool bHasPendingRequest;

	FHarmoniaUserSearchResult()
		: Status(EHarmoniaFriendStatus::Offline)
		, bIsFriend(false)
		, bHasPendingRequest(false)
	{
	}
};
