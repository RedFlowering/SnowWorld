# HarmoniaKit Crafting System

## 개요 (Overview)

HarmoniaKit의 제작 시스템은 아이템 및 장비를 제작할 수 있는 완전한 기능을 제공합니다.

**주요 기능:**
- ✅ 레시피 기반 제작 시스템
- ✅ **제작 스테이션/작업대 시스템** (대장간, 요리, 연금술 등)
- ✅ **부위별 장비 시스템** (머리, 옷, 장갑, 신발, 가방, 악세사리 등)
- ✅ 캐스팅 타임과 애니메이션 지원
- ✅ 성공/실패/대성공 확률 시스템
- ✅ 아이템 등급 시스템 (Common ~ Mythic)
- ✅ 재료 소비 및 확인
- ✅ 레시피 학습 시스템
- ✅ 멀티플레이어 지원 (Replication)
- ✅ 저장/로드 지원
- ✅ 데이터 테이블 기반 설정

---

## 장비 시스템 (Equipment System)

HarmoniaKit는 **완전한 부위별 장비 시스템**을 제공합니다!

### 지원되는 장비 슬롯

| 슬롯 | 용도 | 예시 |
|------|------|------|
| **Head** | 머리 | 투구, 모자, 헬멧 |
| **Chest** | 상의/갑옷 | 갑옷, 셔츠, 조끼 |
| **Legs** | 하의 | 바지, 각반, 레깅스 |
| **Feet** | 신발 | 부츠, 신발, 장화 |
| **Hands** | 장갑 | 장갑, 건틀릿 |
| **Back** | 가방/망토 | 백팩, 망토, 케이프 |
| **MainHand** | 주 무기 | 검, 도끼, 활 |
| **OffHand** | 보조 무기/방패 | 방패, 단검, 토치 |
| **Accessory1** | 악세사리 1 | 반지, 목걸이 |
| **Accessory2** | 악세사리 2 | 반지, 귀걸이 |

**사용법:**
```cpp
// 장비 장착 (블루프린트 또는 C++)
EquipmentComponent->EquipItem(ItemId, EEquipmentSlot::Head);
EquipmentComponent->EquipItem(ItemId, EEquipmentSlot::Chest);
EquipmentComponent->EquipItem(ItemId, EEquipmentSlot::Hands);
```

---

## 제작 스테이션 시스템 (Crafting Station System)

각 레시피를 **특정 제작 스테이션(작업대)**에서만 제작할 수 있도록 설정할 수 있습니다!

### 지원되는 제작 스테이션

| 스테이션 | 용도 | 예시 레시피 |
|----------|------|-------------|
| **None** | 어디서나 (손 제작) | 간단한 도구, 횃불 |
| **Anvil** | 대장간 모루 | 무기, 갑옷, 금속 도구 |
| **Forge** | 용광로 | 금속 제련, 주조 |
| **WorkBench** | 작업대 | 목공, 가구, 건축 |
| **CookingPot** | 요리 냄비 | 요리, 스튜 |
| **CampFire** | 캠프파이어 | 구운 고기, 간단한 요리 |
| **AlchemyTable** | 연금술 테이블 | 포션, 엘릭서 |
| **SewingTable** | 재봉틀 | 옷, 가방, 천 갑옷 |
| **TanningRack** | 무두질 선반 | 가죽, 가죽 갑옷 |
| **Loom** | 베틀 | 천, 옷감 |
| **GrindStone** | 숫돌 | 무기 강화, 날 세우기 |
| **Enchanting** | 마법 부여대 | 인챈트, 마법 부여 |
| **Custom** | 커스텀 (태그 사용) | 게임플레이 태그로 지정 |

**사용법:**
```cpp
// 작업대에 접근 시
CraftingComponent->SetCurrentStation(ECraftingStationType::Anvil);

// 작업대에서 벗어날 때
CraftingComponent->ClearCurrentStation();

// 현재 작업대에서 제작 가능한 레시피 조회
TArray<FCraftingRecipeData> Recipes = CraftingComponent->GetRecipesForCurrentStation();
```

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

// ===== 제작 스테이션 관련 =====

// 현재 제작 스테이션 설정
void SetCurrentStation(ECraftingStationType StationType, FGameplayTagContainer StationTags = FGameplayTagContainer());

// 제작 스테이션 해제 (손 제작으로 전환)
void ClearCurrentStation();

// 현재 스테이션 조회
ECraftingStationType GetCurrentStation();

// 현재 스테이션에서 제작 가능한 레시피 조회
TArray<FCraftingRecipeData> GetRecipesForCurrentStation(FGameplayTag CategoryTag = FGameplayTag());
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
    ECraftingStationType RequiredStation;          // 필요 제작 스테이션 (None = 어디서나 제작 가능)
    FGameplayTagContainer RequiredStationTags;     // 필요 스테이션 태그 (Custom 타입용)
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
   - `StationDataTable`: 제작 스테이션 데이터 테이블 선택 (선택사항)

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
RequiredStation: Anvil  // 대장간 모루에서만 제작 가능
RequiredStationTags: (Empty)
bRequiresLearning: true
ExperienceReward: 100
```

**예시 레시피 (Bread - 손 제작):**
```
Row Name: Recipe_Bread
RecipeId: Recipe_Bread
RecipeName: "빵"
RecipeDescription: "간단한 빵을 만듭니다."
CategoryTags: Crafting.Category.Food

RequiredMaterials:
  [0]:
    ItemId: Item_Wheat
    Amount: 3
    bConsumeOnCraft: true

CastingTime: 2.0
CraftingMontage: AM_Crafting_Cooking

BaseSuccessChance: 1.0  // 100% 성공
CriticalSuccessChance: 0.0

SuccessResults:
  [0]:
    ItemId: Item_Bread
    Amount: 1
    Grade: Common
    Probability: 1.0

RequiredLevel: 1
RequiredStation: None  // 어디서나 제작 가능 (손 제작)
bRequiresLearning: false
```

**예시 레시피 (Health Potion - 연금술):**
```
Row Name: Recipe_HealthPotion
RecipeId: Recipe_HealthPotion
RecipeName: "체력 포션"
RecipeDescription: "체력을 회복하는 포션을 만듭니다."
CategoryTags: Crafting.Category.Alchemy

RequiredMaterials:
  [0]:
    ItemId: Item_Herb
    Amount: 2
    bConsumeOnCraft: true
  [1]:
    ItemId: Item_WaterBottle
    Amount: 1
    bConsumeOnCraft: true

CastingTime: 5.0
CraftingMontage: AM_Crafting_Alchemy

BaseSuccessChance: 0.85  // 85% 성공률
CriticalSuccessChance: 0.15  // 15% 대성공률

SuccessResults:
  [0]:
    ItemId: Item_HealthPotion
    Amount: 1
    Grade: Common
    Probability: 1.0

CriticalSuccessResults:
  [0]:
    ItemId: Item_HealthPotion
    Amount: 2  // 대성공 시 2개 제작
    Grade: Uncommon
    Probability: 1.0

RequiredLevel: 3
RequiredStation: AlchemyTable  // 연금술 테이블에서만 제작 가능
bRequiresLearning: true
ExperienceReward: 50
```

### 3. 제작 스테이션 사용 (블루프린트)

**작업대 접근 시:**
```
Event: OnInteract (대장간 모루)
1. Get Crafting Component
2. Call "Set Current Station" (StationType: Anvil)
3. Open Crafting UI
4. Call "Get Recipes For Current Station" → Update UI with recipes
```

**작업대에서 벗어날 때:**
```
Event: OnEndOverlap (대장간 모루)
1. Get Crafting Component
2. Call "Clear Current Station"
3. Close Crafting UI
```

**예시 (C++):**
```cpp
// 작업대 액터 (예: 대장간 모루)
void AAnvilStation::OnInteract(AActor* InteractingActor)
{
    ACharacter* Character = Cast<ACharacter>(InteractingActor);
    if (!Character) return;

    UHarmoniaCraftingComponent* CraftingComp = Character->FindComponentByClass<UHarmoniaCraftingComponent>();
    if (CraftingComp)
    {
        CraftingComp->SetCurrentStation(ECraftingStationType::Anvil);
        UE_LOG(LogTemp, Log, TEXT("Player is now using the Anvil"));
    }
}

void AAnvilStation::OnEndInteract(AActor* InteractingActor)
{
    ACharacter* Character = Cast<ACharacter>(InteractingActor);
    if (!Character) return;

    UHarmoniaCraftingComponent* CraftingComp = Character->FindComponentByClass<UHarmoniaCraftingComponent>();
    if (CraftingComp)
    {
        CraftingComp->ClearCurrentStation();
        UE_LOG(LogTemp, Log, TEXT("Player left the Anvil"));
    }
}
```

### 4. 제작 시작 (블루프린트)

```
Event Graph:
1. Get Crafting Component
2. Check Current Station (if recipe requires specific station)
3. Call "Can Craft Recipe" (RecipeId: Recipe_IronSword)
   - Branch: If True
     - Call "Start Crafting" (RecipeId: Recipe_IronSword)
     - Display UI: "Crafting started..."
   - Branch: If False
     - Display UI: "Cannot craft - missing materials/wrong station/requirements not met"
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

제작 시스템은 **완전히 리플리케이션을 지원**합니다!

### 동기화되는 항목

| 항목 | 동기화 방식 | 설명 |
|------|------------|------|
| **ActiveSession** | Replicated (OnRep) | 제작 진행 상태 |
| **LearnedRecipes** | Replicated | 학습한 레시피 목록 |
| **CurrentStation** | Replicated (OnRep) | 현재 작업대 |
| **CurrentStationTags** | Replicated | 작업대 태그 |
| **제작 애니메이션** | Multicast RPC | 모든 플레이어가 볼 수 있음! |
| **제작 결과물** | 인벤토리 리플리케이션 | 자동 동기화 |

### 네트워크 플로우

**제작 시작:**
1. 클라이언트: `StartCrafting()` 호출
2. 클라이언트 → 서버: `ServerStartCrafting` RPC
3. 서버: 재료 확인, 스테이션 검증, 재료 소비
4. 서버: `ActiveSession` 업데이트 (자동 복제)
5. **서버 → 모든 클라이언트: `MulticastPlayCraftingAnimation` RPC** ✨
6. 모든 클라이언트: 애니메이션 재생 (다른 플레이어도 볼 수 있음!)
7. 클라이언트: `OnRep_ActiveSession()` 호출 → UI 업데이트

**제작 완료:**
8. 서버: 제작 완료 후 확률 판정 및 결과 분배
9. **서버 → 모든 클라이언트: `MulticastStopCraftingAnimation` RPC** ✨
10. 모든 클라이언트: 애니메이션 정지
11. 서버: 인벤토리에 아이템 추가 (자동 복제)
12. 서버 → 클라이언트: `ClientCraftingCompleted` RPC
13. 클라이언트: 완료 이벤트 처리 (UI 알림 등)

### 주요 특징

✅ **다른 플레이어가 제작하는 모습을 볼 수 있습니다!**
- Multicast RPC를 통해 애니메이션이 모든 클라이언트에 동기화됩니다.
- 대장간에서 다른 플레이어가 무기를 제작하는 모습을 실시간으로 볼 수 있습니다.

✅ **모든 검증은 서버에서 수행**
- 재료 소비, 스테이션 검증, 확률 판정은 모두 서버에서만 실행
- 클라이언트는 작업을 요청만 할 수 있음 (치팅 방지)

✅ **결과물 자동 동기화**
- 인벤토리 컴포넌트의 리플리케이션을 통해 자동으로 아이템 동기화
- 추가 작업 필요 없음

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
