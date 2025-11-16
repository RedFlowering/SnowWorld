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
 * 음성 효과 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaVoiceEffectType : uint8
{
	None UMETA(DisplayName = "효과 없음"),
	Echo UMETA(DisplayName = "에코"),
	Reverb UMETA(DisplayName = "리버브"),
	Cave UMETA(DisplayName = "동굴"),
	Underwater UMETA(DisplayName = "수중"),
	Radio UMETA(DisplayName = "무전기"),
	Robot UMETA(DisplayName = "로봇"),
	LowPass UMETA(DisplayName = "저역 통과 필터"),
	HighPass UMETA(DisplayName = "고역 통과 필터"),
	Distortion UMETA(DisplayName = "왜곡"),
	Custom UMETA(DisplayName = "커스텀")
};

/**
 * 환경 프리셋 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaEnvironmentPreset : uint8
{
	Default UMETA(DisplayName = "기본"),
	SmallRoom UMETA(DisplayName = "작은 방"),
	LargeRoom UMETA(DisplayName = "큰 방"),
	Hall UMETA(DisplayName = "홀"),
	Cave UMETA(DisplayName = "동굴"),
	Underwater UMETA(DisplayName = "수중"),
	Outdoor UMETA(DisplayName = "실외"),
	Forest UMETA(DisplayName = "숲"),
	Mountain UMETA(DisplayName = "산"),
	Canyon UMETA(DisplayName = "협곡")
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

/**
 * 음성 효과 설정 구조체
 */
USTRUCT(BlueprintType)
struct HARMONIAONLINESUBSYSTEM_API FHarmoniaVoiceEffectSettings
{
	GENERATED_BODY()

	/** 효과 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice")
	EHarmoniaVoiceEffectType EffectType;

	/** 효과 강도 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Intensity;

	/** 에코/리버브 감쇠 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float DecayTime;

	/** 에코/리버브 지연 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float DelayTime;

	/** 습도/밀도 (리버브 특성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Density;

	/** 확산도 (리버브 특성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Diffusion;

	/** 저주파 컷오프 (Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "20.0", ClampMax = "20000.0"))
	float LowPassCutoff;

	/** 고주파 컷오프 (Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "20.0", ClampMax = "20000.0"))
	float HighPassCutoff;

	/** 왜곡 레벨 (0.0 ~ 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DistortionLevel;

	/** 드라이/웨트 믹스 (0.0 = 원본, 1.0 = 완전 효과) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DryWetMix;

	/** 효과 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|Voice")
	bool bEnabled;

	FHarmoniaVoiceEffectSettings()
		: EffectType(EHarmoniaVoiceEffectType::None)
		, Intensity(0.5f)
		, DecayTime(1.0f)
		, DelayTime(0.1f)
		, Density(0.5f)
		, Diffusion(0.5f)
		, LowPassCutoff(5000.0f)
		, HighPassCutoff(200.0f)
		, DistortionLevel(0.0f)
		, DryWetMix(0.5f)
		, bEnabled(true)
	{
	}

	/** 환경 프리셋으로 설정 생성 */
	static FHarmoniaVoiceEffectSettings FromPreset(EHarmoniaEnvironmentPreset Preset);
};
