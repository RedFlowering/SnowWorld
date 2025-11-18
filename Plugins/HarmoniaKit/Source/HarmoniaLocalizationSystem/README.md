# Harmonia Localization System

다국어 지원 시스템으로, 게임 중 언어 전환, 텍스트/오디오 현지화, 지역별 포맷팅 등을 제공합니다.

## 주요 기능

### 1. 동적 언어 전환
- 게임 재시작 없이 실시간 언어 변경
- 언어 변경 시 자동으로 UI 갱신
- 16개 언어 지원 (영어, 한국어, 일본어, 중국어 등)

### 2. 텍스트 테이블 관리
- CSV/JSON 기반 번역 데이터
- DataTable을 통한 관리
- 런타임 데이터 추가/제거 지원

### 3. 폰트 자동 교체
- 언어별 폰트 설정
- 한글, 일본어, 중국어 등 언어별 최적화된 폰트 사용

### 4. 오디오 현지화
- 언어별 음성/효과음 지원
- 자동 폴백 (현재 언어에 오디오가 없으면 영어로 폴백)

### 5. 날짜/시간/숫자 포맷팅
- 지역별 표기법 자동 적용
- 12/24시간제 지원
- 통화 기호 및 천 단위 구분 기호 지역화

### 6. RTL(Right-to-Left) 지원
- 아랍어 등 우측에서 좌측으로 쓰는 언어 지원
- 텍스트 방향 자동 감지

### 7. 번역 누락 감지
- 개발 중 번역되지 않은 텍스트 자동 표시
- 누락된 번역 목록 추출 및 CSV 내보내기

### 8. 문맥 기반 번역
- 같은 단어도 상황(UI, 대화, 아이템 등)에 따라 다르게 번역

## 사용 방법

### C++ 사용

```cpp
// Subsystem 가져오기
UHarmoniaLocalizationSubsystem* LocalizationSubsystem =
    GetGameInstance()->GetSubsystem<UHarmoniaLocalizationSubsystem>();

// 언어 변경
LocalizationSubsystem->SetCurrentLanguage(EHarmoniaLanguage::Korean);

// 텍스트 가져오기
FText WelcomeText = LocalizationSubsystem->GetLocalizedText(FName("UI_Welcome"));

// 포맷팅된 텍스트 가져오기
TMap<FString, FString> FormatArgs;
FormatArgs.Add(TEXT("ItemName"), TEXT("Legendary Sword"));
FText ItemReceivedText = LocalizationSubsystem->GetLocalizedTextFormatted(
    FName("Notification_ItemReceived"), FormatArgs);

// 오디오 가져오기
USoundBase* LocalizedSound = LocalizationSubsystem->GetLocalizedAudio(FName("Voice_Welcome"));

// 숫자 포맷팅
FString FormattedNumber = LocalizationSubsystem->FormatNumber(1234.56f, 2);
// 영어: "1,234.56"
// 독일어: "1.234,56"

// 날짜 포맷팅
FDateTime Now = FDateTime::Now();
FString FormattedDate = LocalizationSubsystem->FormatDate(Now);
// 영어: "11/18/2025"
// 한국어: "2025/11/18"

// 통화 포맷팅
FString FormattedCurrency = LocalizationSubsystem->FormatCurrency(1000.0f);
// 영어: "$1,000.00"
// 한국어: "₩1,000.00"
```

### Blueprint 사용

```
1. Get Localization Subsystem 노드 사용
2. Set Current Language 노드로 언어 변경
3. Get Localized Text 노드로 번역된 텍스트 가져오기
4. Format Number/Date/Time 노드로 지역화된 포맷팅
```

### 데이터 테이블 설정

1. Content Browser에서 우클릭 → Miscellaneous → Data Table 생성
2. Row Structure를 `HarmoniaLocalizationTextEntry`로 선택
3. CSV 파일 임포트 또는 직접 편집
4. 런타임에 DataTable 등록:

```cpp
LocalizationSubsystem->AddLocalizationDataTable(MyDataTable);
```

### CSV 파일 형식

```csv
Name,TextKey,English,Korean,Japanese,...
UI_Welcome,UI_Welcome,"Welcome!","환영합니다!","ようこそ！",...
```

예제 파일: `Content/Harmonia/Localization/ExampleLocalization.csv`

## 지원 언어

- English (영어)
- Korean (한국어)
- Japanese (日本語)
- Chinese Simplified (简体中文)
- Chinese Traditional (繁體中文)
- Spanish (Español)
- French (Français)
- German (Deutsch)
- Russian (Русский)
- Portuguese (Português)
- Italian (Italiano)
- Arabic (العربية)
- Turkish (Türkçe)
- Polish (Polski)
- Thai (ไทย)
- Vietnamese (Tiếng Việt)

## 번역 컨텍스트

번역 컨텍스트를 사용하면 같은 단어도 상황에 따라 다르게 번역할 수 있습니다:

- None: 컨텍스트 없음
- UI: 사용자 인터페이스
- Dialogue: 대화
- Item: 아이템
- Quest: 퀘스트
- Skill: 스킬
- Character: 캐릭터
- Tutorial: 튜토리얼
- Error: 에러 메시지
- Notification: 알림
- Custom: 커스텀

## 이벤트 및 델리게이트

### OnLanguageChanged
언어가 변경될 때 호출됩니다.

```cpp
LocalizationSubsystem->OnLanguageChanged.AddDynamic(this, &AMyActor::OnLanguageChanged);

void AMyActor::OnLanguageChanged(EHarmoniaLanguage OldLanguage, EHarmoniaLanguage NewLanguage)
{
    // UI 갱신 등
}
```

### OnMissingTranslation
번역이 누락된 경우 호출됩니다 (개발 모드에서 유용).

```cpp
LocalizationSubsystem->OnMissingTranslation.AddDynamic(this, &AMyActor::OnMissingTranslation);

void AMyActor::OnMissingTranslation(FName TextKey, EHarmoniaLanguage Language, ETranslationContext Context)
{
    // 로그 또는 디버그 표시
}
```

## 번역 누락 관리

### 누락된 번역 추적

```cpp
// 추적 활성화/비활성화
LocalizationSubsystem->SetMissingTranslationTracking(true);

// 누락된 번역 목록 가져오기
TArray<FName> MissingTranslations = LocalizationSubsystem->GetMissingTranslations();

// CSV로 내보내기
LocalizationSubsystem->ExportMissingTranslationsToCSV(TEXT("MissingTranslations.csv"));

// 목록 초기화
LocalizationSubsystem->ClearMissingTranslations();
```

## 언어별 설정 커스터마이징

```cpp
FHarmoniaLanguageSettings KoreanSettings;
KoreanSettings.Language = EHarmoniaLanguage::Korean;
KoreanSettings.TextDirection = ETextDirection::LeftToRight;
KoreanSettings.NumberFormat = ENumberFormat::Default;
KoreanSettings.DateFormat = EDateFormat::YYYYMMDD;
KoreanSettings.TimeFormat = ETimeFormat::Hour24;
KoreanSettings.DecimalSeparator = TEXT(".");
KoreanSettings.ThousandsSeparator = TEXT(",");
KoreanSettings.CurrencySymbol = TEXT("₩");
KoreanSettings.bCurrencySymbolBefore = true;

LocalizationSubsystem->SetLanguageSettings(EHarmoniaLanguage::Korean, KoreanSettings);
```

## 모범 사례

1. **TextKey 네이밍 컨벤션**: `Category_Name` 형식 사용 (예: `UI_Settings`, `Item_Sword`)
2. **포맷 인자**: 중괄호 사용 (예: `{PlayerName}`, `{ItemName}`)
3. **컨텍스트 활용**: 동일한 단어도 컨텍스트를 다르게 설정
4. **폴백 언어**: 항상 영어 번역은 제공 (폴백으로 사용됨)
5. **번역가 노트**: TranslatorNotes 필드로 번역 가이드 제공

## 성능 고려사항

- 첫 번째 조회 후 자동으로 캐시됨
- DataTable 추가/제거 시 캐시가 초기화됨
- 오디오는 필요할 때 동기 로딩 (비동기 로딩은 별도 구현 필요)

## 향후 개선 사항

- [ ] JSON 설정 파일 로드/저장 구현
- [ ] 비동기 오디오 로딩
- [ ] 플러럴라이제이션(복수형) 지원
- [ ] 젠더 기반 번역
- [ ] 런타임 번역 핫리로드
