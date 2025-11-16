# Harmonia Online Subsystem

Harmonia Online Subsystem은 친구 관리, 초대, 음성 대화 등의 커뮤니티 기능을 제공하는 게임 인스턴스 서브시스템입니다.

## 개요

이 서브시스템은 UI를 직접 제공하지 않고, UI에 필요한 **데이터 캐싱**과 **기능 함수**를 제공합니다. 게임 UI는 이 서브시스템을 통해 온라인 기능에 접근할 수 있습니다.

## 주요 기능

### 1. 친구 관리
- 사용자 검색
- 친구 요청 보내기/받기
- 친구 추가/제거
- 친구 목록 캐싱 및 실시간 업데이트
- 친구 상태 추적 (온라인, 오프라인, 게임 중 등)

### 2. 초대 시스템
- 게임 초대
- 파티 초대
- 음성 채팅 초대
- 초대 수락/거절

### 3. 음성 대화
- 음성 채널 생성 및 관리
- 음성 채널 참여/퇴장
- 마이크 음소거
- 사용자별 음소거

### 4. 데이터 캐싱
- 친구 목록 캐시
- 친구 요청 목록 캐시
- 초대 목록 캐시
- 음성 채널 정보 캐시

### 5. 멀티스레드 및 비동기 처리
- **모든 네트워크 작업은 백그라운드 스레드에서 실행됩니다**
- UI 프리즈 없이 부드러운 게임 경험 제공
- 게임 스레드에서 안전한 델리게이트 브로드캐스트
- AsyncTask 기반의 효율적인 작업 관리

비동기로 처리되는 주요 함수:
- `Connect()` - 온라인 서비스 연결 (1.0초 시뮬레이션 지연)
- `SearchUsers()` - 사용자 검색 (0.3초 시뮬레이션 지연)
- `SendFriendRequest()` - 친구 요청 전송 (0.2초 시뮬레이션 지연)
- `RefreshFriendList()` - 친구 목록 새로고침 (0.4초 시뮬레이션 지연)
- `JoinVoiceChannel()` - 음성 채널 연결 (0.5초 시뮬레이션 지연)

**중요**: 위 함수들을 호출해도 즉시 반환되며, 결과는 델리게이트를 통해 전달됩니다. UI는 반드시 델리게이트를 바인딩하여 비동기 작업 완료를 처리해야 합니다.

## 사용 방법

### 서브시스템 가져오기

#### C++에서
```cpp
#include "HarmoniaOnlineSubsystem.h"

// GameInstance에서 서브시스템 가져오기
UGameInstance* GameInstance = GetGameInstance();
if (GameInstance)
{
    UHarmoniaOnlineSubsystem* OnlineSubsystem =
        GameInstance->GetSubsystem<UHarmoniaOnlineSubsystem>();

    if (OnlineSubsystem)
    {
        // 서브시스템 사용
    }
}
```

#### Blueprint에서
1. `Get Game Instance` 노드 사용
2. `Get Subsystem` 노드에서 `Harmonia Online Subsystem` 선택

### 연결 및 초기화

```cpp
// 온라인 서비스에 연결
OnlineSubsystem->Connect();

// 연결 상태 확인
bool bIsConnected = OnlineSubsystem->IsConnected();

// 현재 사용자 정보
FString UserId = OnlineSubsystem->GetCurrentUserId();
FString DisplayName = OnlineSubsystem->GetCurrentUserDisplayName();
```

### 친구 관리

#### 사용자 검색
```cpp
// 사용자 검색
OnlineSubsystem->SearchUsers(TEXT("PlayerName"), 20);

// 검색 결과 이벤트 바인딩
OnlineSubsystem->OnUserSearchCompleted.AddDynamic(
    this, &UMyClass::OnUserSearchCompleted);

void UMyClass::OnUserSearchCompleted(
    bool bSuccess,
    const TArray<FHarmoniaUserSearchResult>& Results)
{
    if (bSuccess)
    {
        for (const FHarmoniaUserSearchResult& Result : Results)
        {
            UE_LOG(LogTemp, Log, TEXT("Found user: %s"), *Result.DisplayName);
        }
    }
}
```

#### 친구 요청 보내기
```cpp
// 친구 요청 전송
OnlineSubsystem->SendFriendRequest(TEXT("UserId"), TEXT("안녕하세요!"));

// 친구 요청 결과 이벤트 바인딩
OnlineSubsystem->OnFriendRequestResult.AddDynamic(
    this, &UMyClass::OnFriendRequestResult);

void UMyClass::OnFriendRequestResult(bool bSuccess, const FString& Message)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Friend request sent: %s"), *Message);
    }
}
```

#### 친구 요청 받기
```cpp
// 친구 요청 이벤트 바인딩
OnlineSubsystem->OnFriendRequestReceived.AddDynamic(
    this, &UMyClass::OnFriendRequestReceived);

void UMyClass::OnFriendRequestReceived(const FHarmoniaFriendRequest& Request)
{
    UE_LOG(LogTemp, Log, TEXT("Friend request from: %s"), *Request.DisplayName);

    // 요청 수락
    OnlineSubsystem->AcceptFriendRequest(Request.UserId);

    // 또는 거절
    // OnlineSubsystem->RejectFriendRequest(Request.UserId);
}
```

#### 친구 목록 가져오기
```cpp
// 캐싱된 친구 목록 가져오기
const TArray<FHarmoniaFriendInfo>& FriendList =
    OnlineSubsystem->GetCachedFriendList();

// 상태별 필터링
TArray<FHarmoniaFriendInfo> OnlineFriends =
    OnlineSubsystem->GetFriendsByStatus(EHarmoniaFriendStatus::Online);

// 친구 목록 업데이트 이벤트 바인딩
OnlineSubsystem->OnFriendListUpdated.AddDynamic(
    this, &UMyClass::OnFriendListUpdated);

void UMyClass::OnFriendListUpdated(const TArray<FHarmoniaFriendInfo>& FriendList)
{
    // UI 업데이트
    RefreshFriendListUI();
}
```

#### 친구 상태 변경 감지
```cpp
// 친구 상태 변경 이벤트 바인딩
OnlineSubsystem->OnFriendStatusChanged.AddDynamic(
    this, &UMyClass::OnFriendStatusChanged);

void UMyClass::OnFriendStatusChanged(const FHarmoniaFriendInfo& FriendInfo)
{
    UE_LOG(LogTemp, Log, TEXT("%s is now %s"),
        *FriendInfo.DisplayName,
        *UEnum::GetValueAsString(FriendInfo.Status));

    // UI 업데이트
    UpdateFriendStatusInUI(FriendInfo);
}
```

### 초대 시스템

#### 친구 초대
```cpp
// 게임 초대
OnlineSubsystem->InviteToGame(TEXT("FriendUserId"), TEXT("SessionId123"), TEXT("함께 플레이하실래요?"));

// 파티 초대
OnlineSubsystem->InviteToParty(TEXT("FriendUserId"), TEXT("파티에 참가하세요!"));

// 음성 채팅 초대
OnlineSubsystem->InviteToVoiceChat(TEXT("FriendUserId"), TEXT("ChannelId"), TEXT("음성 채팅으로 대화해요!"));
```

#### 초대 받기
```cpp
// 초대 수신 이벤트 바인딩
OnlineSubsystem->OnInviteReceived.AddDynamic(
    this, &UMyClass::OnInviteReceived);

void UMyClass::OnInviteReceived(const FHarmoniaInviteInfo& InviteInfo)
{
    UE_LOG(LogTemp, Log, TEXT("Invite from: %s"), *InviteInfo.SenderName);

    // 초대 수락
    OnlineSubsystem->AcceptInvite(InviteInfo.InviteId);

    // 또는 거절
    // OnlineSubsystem->RejectInvite(InviteInfo.InviteId);
}

// 받은 초대 목록
const TArray<FHarmoniaInviteInfo>& PendingInvites =
    OnlineSubsystem->GetPendingInvites();
```

### 음성 대화

#### 음성 채널 생성 및 참여
```cpp
// 음성 채널 생성
FString ChannelId = OnlineSubsystem->CreateVoiceChannel(
    TEXT("우리 팀"), // 채널 이름
    16,              // 최대 참가자 수
    false            // 공개 채널
);

// 음성 채널 참여
OnlineSubsystem->JoinVoiceChannel(ChannelId);

// 음성 채팅 상태 변경 이벤트 바인딩
OnlineSubsystem->OnVoiceChatStatusChanged.AddDynamic(
    this, &UMyClass::OnVoiceChatStatusChanged);

void UMyClass::OnVoiceChatStatusChanged(
    const FString& ChannelId,
    EHarmoniaVoiceChatStatus Status)
{
    switch (Status)
    {
    case EHarmoniaVoiceChatStatus::Connected:
        UE_LOG(LogTemp, Log, TEXT("Voice chat connected"));
        break;
    case EHarmoniaVoiceChatStatus::Speaking:
        UE_LOG(LogTemp, Log, TEXT("Speaking..."));
        break;
    case EHarmoniaVoiceChatStatus::Muted:
        UE_LOG(LogTemp, Log, TEXT("Microphone muted"));
        break;
    }
}
```

#### 마이크 컨트롤
```cpp
// 마이크 음소거
OnlineSubsystem->SetMicrophoneMuted(true);

// 마이크 음소거 해제
OnlineSubsystem->SetMicrophoneMuted(false);

// 음소거 상태 확인
bool bIsMuted = OnlineSubsystem->IsMicrophoneMuted();

// 특정 사용자 음소거
OnlineSubsystem->SetUserMuted(TEXT("AnnoyingUserId"), true);
```

#### 음성 채널 퇴장
```cpp
// 현재 채널 퇴장
FString CurrentChannelId = OnlineSubsystem->GetCurrentVoiceChannelId();
OnlineSubsystem->LeaveVoiceChannel(CurrentChannelId);
```

## 데이터 구조

### FHarmoniaFriendInfo
친구 정보를 담는 구조체
```cpp
struct FHarmoniaFriendInfo
{
    FString UserId;              // 사용자 ID
    FString DisplayName;         // 표시 이름
    EHarmoniaFriendStatus Status; // 상태
    FString CurrentGameName;     // 현재 게임
    FDateTime LastOnlineTime;    // 마지막 접속 시간
    FString AvatarUrl;           // 프로필 이미지 URL
    FDateTime FriendSince;       // 친구 추가 날짜
};
```

### FHarmoniaFriendRequest
친구 요청 정보
```cpp
struct FHarmoniaFriendRequest
{
    FString UserId;                        // 사용자 ID
    FString DisplayName;                   // 표시 이름
    EHarmoniaFriendRequestStatus Status;   // 요청 상태
    FDateTime RequestTime;                 // 요청 시간
    FString Message;                       // 요청 메시지
};
```

### FHarmoniaInviteInfo
초대 정보
```cpp
struct FHarmoniaInviteInfo
{
    FString InviteId;            // 초대 ID
    FString SenderId;            // 보낸 사람 ID
    FString SenderName;          // 보낸 사람 이름
    EHarmoniaInviteType InviteType; // 초대 타입
    FDateTime InviteTime;        // 초대 시간
    FString Message;             // 초대 메시지
    FString SessionId;           // 세션 ID (게임/파티)
    FString ChannelId;           // 채널 ID (음성)
};
```

### FHarmoniaVoiceChannelInfo
음성 채널 정보
```cpp
struct FHarmoniaVoiceChannelInfo
{
    FString ChannelId;              // 채널 ID
    FString ChannelName;            // 채널 이름
    TArray<FString> ParticipantIds; // 참가자 목록
    int32 MaxParticipants;          // 최대 참가자 수
    bool bIsPrivate;                // 비공개 여부
};
```

## 이벤트 델리게이트

| 델리게이트 | 설명 |
|-----------|------|
| `OnFriendListUpdated` | 친구 목록이 업데이트되었을 때 |
| `OnFriendStatusChanged` | 친구 상태가 변경되었을 때 |
| `OnFriendRequestReceived` | 친구 요청을 받았을 때 |
| `OnFriendRequestResult` | 친구 요청 결과 (성공/실패) |
| `OnInviteReceived` | 초대를 받았을 때 |
| `OnVoiceChatStatusChanged` | 음성 채팅 상태가 변경되었을 때 |
| `OnUserSearchCompleted` | 사용자 검색이 완료되었을 때 |

## UI 통합 예제

### Widget Blueprint에서 사용

1. **초기화**
```
Event Construct
  └─> Get Game Instance
       └─> Get Subsystem (Harmonia Online Subsystem)
            ├─> Set as variable "OnlineSubsystem"
            └─> Connect
                 └─> Bind Event to OnFriendListUpdated
```

2. **친구 목록 표시**
```
OnFriendListUpdated Event
  └─> Get Cached Friend List
       └─> For Each Friend
            └─> Create Friend List Item Widget
                 └─> Add to Scroll Box
```

3. **검색 기능**
```
Search Button Clicked
  └─> Get Search Text
       └─> OnlineSubsystem->SearchUsers
            └─> OnUserSearchCompleted Event
                 └─> Display Results
```

## 비동기 처리 아키텍처

### 작동 방식

모든 네트워크 작업은 `HarmoniaAsyncHelpers` 유틸리티를 통해 비동기로 처리됩니다:

1. **백그라운드 스레드**: 네트워크 요청 및 데이터 처리
2. **게임 스레드**: 결과 수신 및 델리게이트 브로드캐스트

```cpp
// 내부적으로 이렇게 작동합니다:
HarmoniaAsyncHelpers::SimulateNetworkOperation<ResultType>(
    []() -> ResultType {
        // 백그라운드 스레드에서 실행
        // HTTP 요청, 데이터 파싱 등
        return Result;
    },
    [this](const ResultType& Result) {
        // 게임 스레드에서 실행
        // 캐시 업데이트, 델리게이트 브로드캐스트
        OnCompleted.Broadcast(Result);
    },
    0.5f // 시뮬레이션 지연 (실제 구현 시 제거)
);
```

### 스레드 안전성

- **캐싱된 데이터 읽기**: 게임 스레드에서만 접근하므로 안전
- **델리게이트 브로드캐스트**: 항상 게임 스레드에서 실행됨
- **비동기 작업**: AsyncTask가 스레드 관리를 자동으로 처리

## 서버 연동

현재 구현은 **시뮬레이션 모드**로 작동합니다. 실제 온라인 서비스와 연동하려면 각 비동기 작업의 백그라운드 스레드 부분에 서버 API 호출을 추가해야 합니다.

### 구현 예시: HTTP 요청으로 변경

```cpp
// HarmoniaOnlineSubsystem.cpp의 SearchUsers 함수 내부
HarmoniaAsyncHelpers::ExecuteAsync<FHarmoniaAsyncTaskResult<TArray<FHarmoniaUserSearchResult>>>(
    [SearchQuery, MaxResults]() -> FHarmoniaAsyncTaskResult<TArray<FHarmoniaUserSearchResult>>
    {
        // TODO 주석 부분을 실제 HTTP 요청으로 대체:

        // 1. HTTP 모듈 사용 (동기 방식)
        TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL(FString::Printf(TEXT("https://api.example.com/users/search?q=%s"), *SearchQuery));
        Request->SetVerb(TEXT("GET"));
        Request->ProcessRequest(); // 백그라운드 스레드에서 블로킹 호출

        // 2. 응답 파싱
        FString ResponseStr = Request->GetResponse()->GetContentAsString();
        TArray<FHarmoniaUserSearchResult> Results;
        // JSON 파싱 로직...

        return FHarmoniaAsyncTaskResult<TArray<FHarmoniaUserSearchResult>>(true, Results);
    },
    [this](const FHarmoniaAsyncTaskResult<TArray<FHarmoniaUserSearchResult>>& Result)
    {
        // 게임 스레드에서 실행 - 수정 불필요
        OnUserSearchCompleted.Broadcast(Result.bSuccess, Result.Result);
    }
);
```

### 수정이 필요한 함수 및 위치

| 함수 | 파일 위치 | TODO 주석 위치 | 구현 내용 |
|------|----------|--------------|----------|
| `Connect()` | HarmoniaOnlineSubsystem.cpp:548 | 인증 및 연결 | 로그인 API, 토큰 획득 |
| `SearchUsers()` | HarmoniaOnlineSubsystem.cpp:76 | 사용자 검색 | 검색 API, JSON 파싱 |
| `SendFriendRequest()` | HarmoniaOnlineSubsystem.cpp:129 | 친구 요청 전송 | POST 요청, 응답 확인 |
| `RefreshFriendList()` | HarmoniaOnlineSubsystem.cpp:233 | 친구 목록 가져오기 | GET 요청, 목록 파싱 |
| `JoinVoiceChannel()` | HarmoniaOnlineSubsystem.cpp:421 | 음성 채널 연결 | 음성 SDK 연동 |

각 함수의 `// TODO: 실제 서버 API 호출 구현` 주석 부분에 실제 구현을 추가하면 됩니다. **비동기 구조는 그대로 유지**하고, 백그라운드 스레드에서 실행되는 람다 함수 내부만 수정하면 됩니다.

## 자동 업데이트

친구 목록은 **30초마다 자동으로 업데이트**됩니다. 이는 `Connect()` 시 타이머가 시작되며, `Disconnect()` 시 중지됩니다.

수동으로 업데이트하려면:
```cpp
OnlineSubsystem->RefreshFriendList();
```

## 주의사항

1. **서브시스템은 GameInstance 수명과 동일**합니다. 레벨 전환 시에도 유지됩니다.
2. **이벤트 바인딩 해제**를 잊지 마세요 (위젯 파괴 시).
3. **음성 채팅**은 한 번에 하나의 채널만 참여 가능합니다.
4. **캐싱된 데이터**를 사용하면 네트워크 호출을 줄일 수 있습니다.

## 향후 계획

- [ ] 실제 서버 API 연동
- [ ] WebSocket을 통한 실시간 알림
- [ ] 파티 시스템 구현
- [ ] 게임 세션 관리
- [ ] 음성 채팅 실제 구현 (EOS Voice, Vivox 등)
- [ ] 블록/신고 기능
- [ ] 친구 그룹 관리
- [ ] 온라인 상태 메시지

## 라이선스

Copyright 2025 Snow Game Studio.
