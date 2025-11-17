# HarmoniaKit Crafting System

## 개요 (Overview)

HarmoniaKit의 제작 시스템은 아이템 및 장비를 제작할 수 있는 완전한 기능을 제공합니다.

**주요 기능:**
- ✅ 레시피 기반 제작 시스템
- ✅ 캐스팅 타임과 애니메이션 지원
- ✅ 성공/실패/대성공 확률 시스템
- ✅ 아이템 등급 시스템 (Common ~ Mythic)
- ✅ 재료 소비 및 확인
- ✅ 레시피 학습 시스템
- ✅ 멀티플레이어 지원 (Replication)
- ✅ 저장/로드 지원
- ✅ 데이터 테이블 기반 설정

---

## 아이템 등급 시스템 (Item Grade System)

### EItemGrade Enum

아이템과 장비는 다음 등급으로 분류됩니다:

| 등급 | 영문 | 설명 | 기본 색상 |
|------|------|------|----------|
| Common | 일반 | 가장 기본적인 등급 | White |
| Uncommon | 고급 | 약간 향상된 등급 | Green |
| Rare | 희귀 | 드문 등급 | Blue |
| Epic | 영웅 | 매우 드문 등급 | Purple |
| Legendary | 전설 | 전설적인 등급 | Orange |
| Mythic | 신화 | 최고 등급 | Red |

### Grade Configuration DataTable

등급별 속성을 설정하려면 `FItemGradeConfig` 데이터 테이블을 생성하세요:

```cpp
USTRUCT(BlueprintType)
struct FItemGradeConfig : public FTableRowBase
{
    EItemGrade Grade;           // 등급
    FText DisplayName;          // 표시 이름
    FLinearColor GradeColor;    // UI 색상
    float StatMultiplier;       // 스탯 배율
    float DurabilityMultiplier; // 내구도 배율
    float PriceMultiplier;      // 가격 배율
};
```

**예시:**
- Common: StatMultiplier = 1.0
- Rare: StatMultiplier = 1.5
- Legendary: StatMultiplier = 3.0

---

## 제작 시스템 구성 요소

### 1. HarmoniaCraftingComponent

캐릭터 또는 플레이어에게 추가하는 컴포넌트입니다.

**주요 함수:**
```cpp
// 제작 시작
bool StartCrafting(FHarmoniaID RecipeId);

// 제작 취소
void CancelCrafting();

// 제작 가능 여부 확인
bool CanCraftRecipe(FHarmoniaID RecipeId, TArray<FCraftingMaterial>& OutMissingMaterials);

// 레시피 학습
bool LearnRecipe(FHarmoniaID RecipeId);

// 레시피 학습 여부 확인
bool HasLearnedRecipe(FHarmoniaID RecipeId);

// 제작 진행도 (0.0 ~ 1.0)
float GetCraftingProgress();

// 제작 중인지 확인
bool IsCrafting();
```

**주요 이벤트 (Delegates):**
```cpp
// 제작 시작 시
FOnCraftingStarted OnCraftingStarted;

// 제작 진행 중 (프로그레스 바 업데이트용)
FOnCraftingProgress OnCraftingProgress;

// 제작 완료 시
FOnCraftingCompleted OnCraftingCompleted;

// 제작 취소 시
FOnCraftingCancelled OnCraftingCancelled;

// 레시피 학습 시
FOnRecipeLearned OnRecipeLearned;
```

### 2. 데이터 테이블 구조

#### FCraftingRecipeData (레시피 정의)

제작 레시피를 정의하는 데이터 테이블 구조입니다.

```cpp
USTRUCT(BlueprintType)
struct FCraftingRecipeData : public FTableRowBase
{
    // 기본 정보
    FHarmoniaID RecipeId;                          // 레시피 ID
    FText RecipeName;                              // 레시피 이름
    FText RecipeDescription;                       // 레시피 설명
    TSoftObjectPtr<UTexture2D> RecipeIcon;        // 레시피 아이콘
    FGameplayTagContainer CategoryTags;            // 카테고리 태그

    // 재료
    TArray<FCraftingMaterial> RequiredMaterials;   // 필요 재료

    // 제작 시간 및 애니메이션
    float CastingTime;                             // 제작 시간 (초)
    TSoftObjectPtr<UAnimMontage> CraftingMontage; // 제작 애니메이션

    // 확률 설정
    float BaseSuccessChance;                       // 기본 성공 확률 (0.0 ~ 1.0)
    float CriticalSuccessChance;                   // 대성공 확률 (0.0 ~ 1.0)

    // 결과물
    TArray<FCraftingResultItem> SuccessResults;         // 성공 시 결과
    TArray<FCraftingResultItem> FailureResults;         // 실패 시 결과
    TArray<FCraftingResultItem> CriticalSuccessResults; // 대성공 시 추가 보상

    // 요구 사항
    int32 RequiredLevel;                           // 필요 레벨
    int32 RequiredSkillLevel;                      // 필요 스킬 레벨
    bool bRequiresLearning;                        // 학습 필요 여부

    // 보상
    int32 ExperienceReward;                        // 경험치 보상
};
```

#### FCraftingMaterial (재료)

제작에 필요한 재료를 정의합니다.

```cpp
USTRUCT(BlueprintType)
struct FCraftingMaterial
{
    FHarmoniaID ItemId;     // 아이템 ID
    int32 Amount;           // 필요 개수
    bool bConsumeOnCraft;   // 제작 시 소비 여부
};
```

**예시:**
```
- Iron Ore x5 (소비)
- Hammer x1 (소비하지 않음 - 도구)
```

#### FCraftingResultItem (결과물)

제작 결과로 얻을 아이템을 정의합니다.

```cpp
USTRUCT(BlueprintType)
struct FCraftingResultItem
{
    FHarmoniaID ItemId;     // 아이템 ID
    int32 Amount;           // 개수
    EItemGrade Grade;       // 아이템 등급
    float Probability;      // 획득 확률 (0.0 ~ 1.0)
};
```

**예시:**
```
성공 시:
- Iron Sword x1 (Grade: Common, Probability: 1.0)

대성공 시 (추가):
- Quality Gem x1 (Grade: Rare, Probability: 0.3)  // 30% 확률
```

---

## 사용 예시 (Usage Examples)

### 1. 컴포넌트 설정

**블루프린트에서:**
1. 캐릭터 또는 플레이어 블루프린트 열기
2. `HarmoniaCraftingComponent` 추가
3. Details 패널에서 다음 설정:
   - `RecipeDataTable`: 레시피 데이터 테이블 선택
   - `GradeConfigDataTable`: 등급 설정 데이터 테이블 선택
   - `CategoryDataTable`: 카테고리 데이터 테이블 선택 (선택사항)

**C++에서:**
```cpp
// Character.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crafting")
UHarmoniaCraftingComponent* CraftingComponent;

// Character.cpp (Constructor)
CraftingComponent = CreateDefaultSubobject<UHarmoniaCraftingComponent>(TEXT("CraftingComponent"));
```

### 2. 레시피 데이터 테이블 생성

**에디터에서:**
1. Content Browser에서 우클릭
2. Miscellaneous > Data Table
3. Row Structure: `FCraftingRecipeData` 선택
4. 레시피 추가 및 설정

**예시 레시피 (Iron Sword):**
```
Row Name: Recipe_IronSword
RecipeId: Recipe_IronSword
RecipeName: "철 검"
RecipeDescription: "기본적인 철 검을 제작합니다."
CategoryTags: Crafting.Category.Weapon

RequiredMaterials:
  [0]:
    ItemId: Item_IronOre
    Amount: 5
    bConsumeOnCraft: true
  [1]:
    ItemId: Item_Wood
    Amount: 2
    bConsumeOnCraft: true

CastingTime: 3.0
CraftingMontage: AM_Crafting_Smithing

BaseSuccessChance: 0.9  // 90% 성공률
CriticalSuccessChance: 0.1  // 10% 대성공률

SuccessResults:
  [0]:
    ItemId: Item_IronSword
    Amount: 1
    Grade: Common
    Probability: 1.0

CriticalSuccessResults:
  [0]:
    ItemId: Item_QualityGem
    Amount: 1
    Grade: Rare
    Probability: 0.5  // 대성공 시 50% 확률로 추가 획득

FailureResults:
  [0]:
    ItemId: Item_IronOre
    Amount: 2  // 실패 시 재료 일부 반환
    Grade: Common
    Probability: 1.0

RequiredLevel: 5
RequiredSkillLevel: 0
bRequiresLearning: true
ExperienceReward: 100
```

### 3. 제작 시작 (블루프린트)

```
Event Graph:
1. Get Crafting Component
2. Call "Can Craft Recipe" (RecipeId: Recipe_IronSword)
   - Branch: If True
     - Call "Start Crafting" (RecipeId: Recipe_IronSword)
     - Display UI: "Crafting started..."
   - Branch: If False
     - Display UI: "Cannot craft - missing materials or requirements"
```

### 4. 제작 진행도 표시 (블루프린트)

```
Widget Blueprint (Crafting UI):

Bind to "On Crafting Progress":
  - Update Progress Bar: Set Percent = Progress (0.0 ~ 1.0)
  - Update Text: "Crafting... {RemainingTime}s"

Bind to "On Crafting Completed":
  - Branch by Result:
    - Success: Display "Crafting succeeded!"
    - CriticalSuccess: Display "Critical success! Bonus items received!"
    - Failure: Display "Crafting failed..."
  - Hide Progress Bar
```

### 5. 레시피 학습 시스템

```cpp
// 레시피 학습 (예: 레시피 책 사용 시)
CraftingComponent->LearnRecipe(FHarmoniaID(TEXT("Recipe_IronSword")));

// 학습 여부 확인
if (CraftingComponent->HasLearnedRecipe(RecipeId))
{
    // 레시피를 알고 있음
}

// 학습한 모든 레시피 가져오기
TArray<FHarmoniaID> LearnedRecipes = CraftingComponent->GetLearnedRecipes();
```

### 6. C++ 사용 예시

```cpp
void AMyCharacter::TryCraftItem(FHarmoniaID RecipeId)
{
    if (!CraftingComponent)
        return;

    // 제작 가능 확인
    TArray<FCraftingMaterial> MissingMaterials;
    if (CraftingComponent->CanCraftRecipe(RecipeId, MissingMaterials))
    {
        // 제작 시작
        bool bSuccess = CraftingComponent->StartCrafting(RecipeId);
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Started crafting: %s"), *RecipeId.ToString());
        }
    }
    else
    {
        // 부족한 재료 표시
        for (const FCraftingMaterial& Material : MissingMaterials)
        {
            UE_LOG(LogTemp, Warning, TEXT("Missing: %s x%d"), *Material.ItemId.ToString(), Material.Amount);
        }
    }
}

void AMyCharacter::SetupCraftingEvents()
{
    if (!CraftingComponent)
        return;

    // 제작 완료 이벤트 바인딩
    CraftingComponent->OnCraftingCompleted.AddDynamic(this, &AMyCharacter::OnCraftingCompleted);
}

void AMyCharacter::OnCraftingCompleted(FHarmoniaID RecipeId, ECraftingResult Result, const TArray<FCraftingResultItem>& ResultItems)
{
    switch (Result)
    {
        case ECraftingResult::Success:
            UE_LOG(LogTemp, Log, TEXT("Crafting succeeded!"));
            break;

        case ECraftingResult::CriticalSuccess:
            UE_LOG(LogTemp, Log, TEXT("Critical success! Bonus items!"));
            break;

        case ECraftingResult::Failure:
            UE_LOG(LogTemp, Warning, TEXT("Crafting failed..."));
            break;
    }

    // 결과물 로깅
    for (const FCraftingResultItem& Item : ResultItems)
    {
        UE_LOG(LogTemp, Log, TEXT("Received: %s x%d (Grade: %d)"),
            *Item.ItemId.ToString(), Item.Amount, (int32)Item.Grade);
    }
}
```

---

## 확률 시스템 (Probability System)

제작 시스템은 다음 순서로 확률을 판정합니다:

1. **대성공 판정**: `CriticalSuccessChance` 확률로 판정
   - 성공 시: `SuccessResults` + `CriticalSuccessResults` 지급

2. **성공/실패 판정**: `BaseSuccessChance` 확률로 판정
   - 성공 시: `SuccessResults` 지급
   - 실패 시: `FailureResults` 지급

3. **아이템별 확률**: 각 `FCraftingResultItem`의 `Probability` 적용
   - 1.0 = 100% 획득
   - 0.5 = 50% 확률로 획득
   - 0.1 = 10% 확률로 획득

**예시:**
```
BaseSuccessChance: 0.8     // 80% 성공률
CriticalSuccessChance: 0.2  // 20% 대성공률

시나리오 1: 대성공 (20% 확률)
  - Iron Sword x1 (100%)
  - Bonus Gem x1 (50% 확률) <- 아이템별 확률 적용

시나리오 2: 성공 (60% 확률 = 80% - 20%)
  - Iron Sword x1 (100%)

시나리오 3: 실패 (20% 확률)
  - Iron Ore x2 (100%) <- 재료 일부 반환
```

---

## 등급별 스탯 적용 (Grade-based Stats)

장비 시스템과 통합하여 등급에 따라 스탯을 조정할 수 있습니다:

```cpp
// 등급 설정 가져오기
FItemGradeConfig GradeConfig;
if (CraftingComponent->GetGradeConfig(EItemGrade::Rare, GradeConfig))
{
    // 스탯에 배율 적용
    float FinalAttack = BaseAttack * GradeConfig.StatMultiplier;
    float FinalDurability = BaseDurability * GradeConfig.DurabilityMultiplier;
    float FinalPrice = BasePrice * GradeConfig.PriceMultiplier;
}

// UI에서 등급 색상 표시
FLinearColor GradeColor = CraftingComponent->GetGradeColor(EItemGrade::Legendary);
FText GradeName = CraftingComponent->GetGradeDisplayName(EItemGrade::Legendary);
```

---

## 저장/로드 (Save/Load)

제작 시스템은 학습한 레시피를 저장/로드할 수 있습니다.

```cpp
// 저장
FLearnedRecipeSaveData SaveData = CraftingComponent->GetLearnedRecipesSaveData();
// SaveData를 세이브 게임에 저장

// 로드
// 세이브 게임에서 SaveData 로드
CraftingComponent->LoadLearnedRecipesFromSaveData(SaveData);
```

**HarmoniaSaveGame 통합:**
```cpp
// HarmoniaSaveGame.h에 추가
UPROPERTY()
FLearnedRecipeSaveData CraftingData;

// 저장 시
SaveGame->CraftingData = CraftingComponent->GetLearnedRecipesSaveData();

// 로드 시
CraftingComponent->LoadLearnedRecipesFromSaveData(SaveGame->CraftingData);
```

---

## 디버그 기능 (Debug Features)

개발 중 테스트를 위한 디버그 함수들:

```cpp
#if WITH_EDITOR

// 모든 레시피 학습
CraftingComponent->Debug_LearnAllRecipes();

// 현재 제작 즉시 완료
CraftingComponent->Debug_InstantCraft();

#endif
```

**블루프린트에서 사용:**
1. Development Only 노드로 래핑
2. 키 입력에 바인딩하여 테스트

---

## 멀티플레이어 지원 (Multiplayer)

제작 시스템은 완전히 리플리케이션을 지원합니다:

- `ActiveSession`: 서버에서 클라이언트로 복제됨
- `LearnedRecipes`: 서버에서 클라이언트로 복제됨
- 모든 제작 요청은 서버에서 검증됨
- 클라이언트는 UI 업데이트만 담당

**네트워크 플로우:**
1. 클라이언트: `StartCrafting()` 호출
2. 클라이언트 → 서버: `ServerStartCrafting` RPC
3. 서버: 재료 확인 및 소비
4. 서버: `ActiveSession` 업데이트 (자동 복제)
5. 클라이언트: `OnRep_ActiveSession()` 호출
6. 클라이언트: 애니메이션 재생 및 UI 업데이트
7. 서버: 제작 완료 후 결과 분배
8. 서버 → 클라이언트: `ClientCraftingCompleted` RPC
9. 클라이언트: 완료 이벤트 처리

---

## 커스터마이징 (Customization)

### 레벨 시스템 통합

기본적으로 레벨 확인은 항상 성공합니다. 레벨 시스템이 있다면:

```cpp
// CraftingComponent를 상속받아 오버라이드
class UMyCraftingComponent : public UHarmoniaCraftingComponent
{
protected:
    virtual int32 GetCharacterLevel() const override
    {
        // 실제 레벨 반환
        AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner());
        return MyChar ? MyChar->GetLevel() : 1;
    }

    virtual int32 GetCraftingSkillLevel() const override
    {
        // 제작 스킬 레벨 반환
        AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner());
        return MyChar ? MyChar->GetCraftingSkill() : 1;
    }
};
```

### 카테고리 시스템

레시피를 카테고리별로 분류하려면 `FCraftingCategoryData` 데이터 테이블을 생성하세요:

```cpp
// 카테고리 태그 정의 (프로젝트 설정 > Gameplay Tags)
Crafting.Category.Weapon
Crafting.Category.Armor
Crafting.Category.Potion
Crafting.Category.Food

// 레시피에 카테고리 태그 추가
RecipeData.CategoryTags.AddTag(FGameplayTag::RequestGameplayTag("Crafting.Category.Weapon"));

// 카테고리별 레시피 필터링
TArray<FCraftingRecipeData> WeaponRecipes =
    CraftingComponent->GetAvailableRecipes(
        FGameplayTag::RequestGameplayTag("Crafting.Category.Weapon")
    );
```

---

## 모범 사례 (Best Practices)

1. **재료 검증**: 항상 `CanCraftRecipe()`로 재료를 먼저 확인하세요.

2. **애니메이션 길이**: `CastingTime`과 `CraftingMontage` 길이를 일치시키세요.

3. **확률 밸런싱**:
   - `BaseSuccessChance`: 0.7 ~ 0.95 권장
   - `CriticalSuccessChance`: 0.05 ~ 0.2 권장

4. **레시피 학습**:
   - 기본 레시피: `bRequiresLearning = false`
   - 고급 레시피: `bRequiresLearning = true`

5. **재료 반환**:
   - 실패 시 일부 재료 반환 (`FailureResults`) 권장
   - 플레이어 경험 향상

6. **UI 피드백**:
   - 진행도 표시 (`OnCraftingProgress`)
   - 결과 알림 (`OnCraftingCompleted`)
   - 부족한 재료 표시

---

## 문제 해결 (Troubleshooting)

### 제작이 시작되지 않음
- `RecipeDataTable`이 설정되었는지 확인
- 레시피 ID가 올바른지 확인
- `CanCraftRecipe()` 반환값 확인
- 인벤토리 컴포넌트가 있는지 확인

### 애니메이션이 재생되지 않음
- `CraftingMontage`가 올바르게 설정되었는지 확인
- 캐릭터에 `UAnimInstance`가 있는지 확인
- 몽타주가 캐릭터의 스켈레톤과 호환되는지 확인

### 재료가 소비되지 않음
- 인벤토리 컴포넌트가 올바르게 설정되었는지 확인
- `FCraftingMaterial.bConsumeOnCraft`가 true인지 확인
- 아이템 ID가 인벤토리의 아이템과 일치하는지 확인

### 멀티플레이어에서 동작하지 않음
- 컴포넌트의 `SetIsReplicatedByDefault(true)` 확인
- 서버에서 제작이 시작되는지 확인
- 네트워크 권한 확인

---

## API 레퍼런스

전체 API 문서는 헤더 파일을 참조하세요:
- `HarmoniaCraftingSystemDefinitions.h` - 데이터 구조
- `HarmoniaCraftingComponent.h` - 컴포넌트 API

---

## 라이선스

Copyright 2025 Snow Game Studio. All Rights Reserved.
