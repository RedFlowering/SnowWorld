# 하모니아 키트 - 생활 컨텐츠 시스템

## 개요
하모니아 키트에 5가지 주요 생활 컨텐츠 시스템이 추가되었습니다.

## 추가된 시스템

### 1. 낚시 시스템 (Fishing System)
**파일:**
- `HarmoniaFishingSystemDefinitions.h` - 데이터 정의
- `HarmoniaFishingComponent.h/cpp` - 컴포넌트

**주요 기능:**
- 🎣 낚시 미니게임 (타이밍, 바 균형, QTE, 릴 장력 관리)
- 🐟 다양한 희귀도의 물고기 (Common ~ Legendary)
- 🌊 낚시터 타입 (강, 호수, 바다, 연못, 특별)
- 📖 물고기 도감 시스템
- ⬆️ 낚시 레벨 및 경험치 시스템
- 🌤️ 시간/날씨/계절 조건
- 📊 물고기 크기, 무게, 품질 점수

**사용 예:**
```cpp
UHarmoniaFishingComponent* FishingComp = Character->FindComponentByClass<UHarmoniaFishingComponent>();
FishingComp->StartFishing(FishingSpotData);
```

---

### 2. 채집 시스템 (Gathering System)
**파일:**
- `HarmoniaGatheringSystemDefinitions.h` - 데이터 정의
- `HarmoniaGatheringComponent.h/cpp` - 컴포넌트

**주요 기능:**
- ⛏️ 광물 채굴
- 🌿 약초 채집
- 🪓 벌목(나무 채집)
- 🧶 섬유, 석재, 크리스탈, 꽃, 버섯 등 다양한 자원
- 🔧 도구 시스템 (곡괭이, 도끼, 낫, 망치)
- ⚡ 도구 내구도 및 등급 시스템
- 🎯 크리티컬 채집
- 💪 채집 특성 시스템 (패시브 스킬)
- 📈 자원 타입별 개별 레벨링

**사용 예:**
```cpp
UHarmoniaGatheringComponent* GatherComp = Character->FindComponentByClass<UHarmoniaGatheringComponent>();
GatherComp->EquipTool(PickaxeData);
GatherComp->StartGathering(MineralResourceID);
```

---

### 3. 요리 시스템 (Cooking System)
**파일:**
- `HarmoniaCookingSystemDefinitions.h` - 데이터 정의
- `HarmoniaCookingComponent.h/cpp` - 컴포넌트

**주요 기능:**
- 🍳 음식 제작 시스템
- 🎨 요리 품질 (실패작 ~ 걸작)
- 🔥 다양한 조리 방법 (끓이기, 굽기, 찌기, 베이킹 등)
- ⚡ 강력한 버프 효과 (체력/마나 회복, 공격력/방어력/속도 증가 등)
- 📚 레시피 시스템 (숨겨진 레시피 포함)
- 👨‍🍳 요리 특성 (속도, 품질, 재료 절약 등)
- 🍲 음식 타입 (식사, 수프, 디저트, 음료, 간식, 엘릭서, 샐러드)

**사용 예:**
```cpp
UHarmoniaCookingComponent* CookingComp = Character->FindComponentByClass<UHarmoniaCookingComponent>();
CookingComp->DiscoverRecipe(RecipeID);
CookingComp->StartCooking(RecipeID);
CookingComp->ConsumeFood(FoodID, Quality);
```

---

### 4. 농사 시스템 (Farming System)
**파일:**
- `HarmoniaFarmingSystemDefinitions.h` - 데이터 정의
- `HarmoniaFarmingComponent.h/cpp` - 컴포넌트

**주요 기능:**
- 🌱 작물 재배 및 성장 시스템
- 📊 성장 단계 (씨앗 → 싹 → 성장 → 성숙 → 수확 → 시듦)
- 🌾 다양한 작물 타입 (채소, 과일, 곡물, 약초, 꽃, 나무)
- 💧 물주기 및 수분 시스템
- 🧪 비료 시스템 (성장 속도, 수확량, 품질 향상)
- 🌍 토양 품질 시스템
- 🌸 계절 시스템
- 🔄 재수확 가능한 작물
- 🌾 씨앗 획득 시스템
- 🏆 작물 품질 점수

**사용 예:**
```cpp
UHarmoniaFarmingComponent* FarmingComp = Character->FindComponentByClass<UHarmoniaFarmingComponent>();
FarmingComp->AddFarmPlot(PlotID, SoilData);
FarmingComp->PlantCrop(CropID, PlotID);
FarmingComp->WaterCrop(PlotID);
FarmingComp->ApplyFertilizer(PlotID, FertilizerData);
FarmingComp->HarvestCrop(PlotID);
```

---

### 5. 음악/연주 시스템 (Music/Performance System)
**파일:**
- `HarmoniaMusicSystemDefinitions.h` - 데이터 정의
- `HarmoniaMusicComponent.h/cpp` - 컴포넌트

**주요 기능:**
- 🎵 악기 연주 시스템 (류트, 플루트, 하프, 드럼, 호른, 바이올린, 피아노, 기타)
- 🎶 음악 장르 (클래식, 민속, 전투, 치유, 버프, 디버프)
- 🎯 리듬 미니게임 (타이밍 기반 QTE)
- ✨ 연주 품질 (Poor ~ Legendary)
- 💫 강력한 범위 버프 (아군 전체 지원)
- 📜 악보 습득 시스템
- 🎸 악기 등급 및 내구도
- 🎭 연주 특성 (품질, 버프 효과, 범위, 앙코르 확률 등)
- 👥 주변 플레이어/NPC에 버프 적용

**사용 예:**
```cpp
UHarmoniaMusicComponent* MusicComp = Character->FindComponentByClass<UHarmoniaMusicComponent>();
MusicComp->EquipInstrument(LuteData);
MusicComp->LearnMusicSheet(MusicID);
MusicComp->StartPerformance(MusicID);
// 리듬 게임 입력
MusicComp->HitRhythmNote(NoteIndex, InputTime, bPerfectHit);
```

---

## 공통 시스템

### 레벨링 시스템
모든 생활 컨텐츠는 개별 레벨 및 경험치 시스템을 가집니다:
- ⬆️ 레벨업 시 스킬 포인트 획득
- 📊 레벨에 따른 콘텐츠 언락
- 💪 경험치 배율 적용 가능

### 특성 시스템
각 시스템은 패시브 스킬인 특성 시스템을 지원:
- ⚡ 속도 향상
- 📈 품질 향상
- 💰 보상 증가
- 🎯 특수 효과 확률

### 품질 시스템
대부분의 활동은 품질 평가를 받습니다:
- 🥉 품질에 따른 보상 차등
- 🏆 최고 품질 달성 시 특별 보너스
- 📊 품질 점수 기록

---

## 설계 철학

### 1. 모듈화
- 각 시스템은 독립적인 컴포넌트로 구현
- 필요한 시스템만 선택적으로 사용 가능

### 2. 확장성
- 데이터 주도 설계 (Data-Driven Design)
- 블루프린트에서 쉽게 커스터마이징 가능
- 새로운 콘텐츠 추가 용이

### 3. 상호작용
- 요리 시스템 ← 낚시/채집/농사 (재료 제공)
- 음악 시스템 → 모든 시스템 (버프 제공)
- 크로스 시너지 효과

### 4. 깊이 있는 게임플레이
- 미니게임으로 스킬 기반 플레이
- 전략적 선택 (도구, 특성, 타이밍)
- 수집 및 성장 요소

---

## 다음 단계

### 권장 확장 사항:
1. **UI 시스템** - 각 생활 컨텐츠용 전용 UI
2. **도감 시스템** - 통합 컬렉션 시스템
3. **업적 시스템** - 생활 컨텐츠 관련 업적
4. **NPC 상인** - 생활 아이템 거래
5. **길드/협동 시스템** - 다인 협동 콘텐츠
6. **이벤트 시스템** - 계절 이벤트, 특별 자원 등
7. **마스터 시스템** - 각 분야의 전문가 육성

### 통합 시스템:
- 인벤토리 시스템과 연동
- 퀘스트 시스템 연동
- 진행도 저장/로드
- 온라인 랭킹 시스템

---

## 테스트 체크리스트

### 낚시
- [ ] 다양한 물고기 잡기
- [ ] 미니게임 완벽 성공
- [ ] 도감 등록 확인
- [ ] 레벨업 테스트

### 채집
- [ ] 모든 자원 타입 채집
- [ ] 도구 내구도 소모
- [ ] 크리티컬 발생
- [ ] 특성 효과 확인

### 요리
- [ ] 레시피로 음식 제작
- [ ] 다양한 품질 달성
- [ ] 버프 효과 확인
- [ ] 음식 섭취

### 농사
- [ ] 작물 심기 및 성장 확인
- [ ] 물주기 시스템
- [ ] 비료 효과
- [ ] 계절별 작물
- [ ] 수확 및 재수확

### 음악
- [ ] 악기 장착
- [ ] 악보 배우기
- [ ] 리듬 게임 플레이
- [ ] 범위 버프 적용
- [ ] 품질별 효과 차이

---

## 빌드 참고사항

모든 컴포넌트는 `HarmoniaKit` 모듈에 포함됩니다.
필요한 경우 `HarmoniaKit.Build.cs`에 추가 의존성을 설정하세요.

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    // 기타 필요한 모듈...
});
```

---

**작성일:** 2025-11-22
**버전:** 1.0
**작성자:** Harmonia Kit Development Team
