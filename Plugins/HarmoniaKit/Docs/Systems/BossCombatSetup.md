# 보스 전투 셋업 가이드

이 가이드는 언리얼 엔진의 **HarmoniaKit** 플러그인을 사용하여 플레이어 대 보스 전투 시나리오를 구현하는 방법을 상세히 설명합니다.

## 필수 조건

**HarmoniaKit** 플러그인이 활성화되어 있어야 하며 다음 에셋들이 필요합니다:
- 플레이어용 스켈레탈 메쉬 (Skeletal Mesh)
- 보스용 스켈레탈 메쉬
- 애니메이션 에셋 (공격, 피격, 사망 등)

---

## 1. 플레이어 캐릭터 설정

플레이어 캐릭터는 근접 전투의 기초를 제공하는 `AHarmoniaMeleeCharacterExample`을 기반으로 합니다.

### 1.1. 캐릭터 블루프린트 생성
1.  콘텐츠 브라우저에서 우클릭 후 **Blueprint Class**를 선택합니다.
2.  부모 클래스로 `HarmoniaMeleeCharacterExample`을 검색하여 선택합니다.
3.  이름을 `BP_HeroCharacter`로 지정합니다.

### 1.2. 컴포넌트 구성
`BP_HeroCharacter`를 열고 다음 컴포넌트들을 설정합니다:

*   **Mesh**: 플레이어 스켈레탈 메쉬를 할당합니다.
*   **MeleeCombatComponent**:
    *   **Combo Data Table**: 공격 콤보를 정의한 데이터 테이블을 할당합니다.
    *   **Hit Detection**: 메쉬 소켓(예: `weapon_r_socket`)에 히트박스가 설정되어 있는지 확인합니다.
*   **LockOnComponent**:
    *   **Target Tag**: `Enemy` 또는 `Boss` 등 보스가 사용하는 태그로 설정합니다.
    *   **Max Distance**: 적절한 락온 거리(예: 1500)를 설정합니다.

### 1.3. 입력(Input) 설정
1.  **Input Mapping Context (IMC)**를 생성합니다 (예: `IMC_Combat`).
2.  다음 **Input Actions (IA)**을 생성합니다:
    *   `IA_LightAttack` (약공격)
    *   `IA_HeavyAttack` (강공격)
    *   `IA_Block` (방어)
    *   `IA_Dodge` (회피)
    *   `IA_LockOn` (락온)
3.  `BP_HeroCharacter` 디테일 패널의 **Input** 카테고리에서 해당 Input Action들을 프로퍼티에 할당합니다.
4.  `IMC_Combat`에서 키 매핑을 설정합니다.

---

## 2. 보스 몬스터 설정

보스는 페이즈, 체력바, 고급 AI를 지원하는 `AHarmoniaBossMonster`를 기반으로 합니다.

### 2.1. 보스 블루프린트 생성
1.  `HarmoniaBossMonster`를 상속받는 새 블루프린트 클래스를 생성합니다.
2.  이름을 `BP_Boss_Guardian`으로 지정합니다.

### 2.2. 몬스터 데이터 에셋 (Monster Data Asset)
1.  콘텐츠 브라우저 우클릭 -> **Miscellaneous** -> **Data Asset**.
2.  `HarmoniaMonsterData`를 선택합니다.
3.  이름을 `DA_Boss_Guardian`으로 지정합니다.
4.  스탯을 구성합니다:
    *   **Health**: 높은 값 (예: 5000).
    *   **Damage**: 공격 기본 데미지.
    *   **Abilities**: 공격용 게임플레이 어빌리티 추가 (예: `GA_Boss_Smash`, `GA_Boss_Roar`).

### 2.3. 보스 블루프린트 구성
`BP_Boss_Guardian`을 열고 설정합니다:
*   **Monster Data**: `DA_Boss_Guardian`을 할당합니다.
*   **Monster Level**: 원하는 레벨로 설정 (예: 10).
*   **Boss UI**:
    *   **Show Health Bar**: `True` (체크).
    *   **Boss Title**: "고대 수호자" (원하는 이름).
*   **Phases (페이즈)**:
    *   `CheckPhaseTransition`을 오버라이드하거나, 기본 체력 기반 임계값을 사용합니다. `OnHealthChanged` 로직을 통해 `ChangeToPhase`를 트리거할 수도 있습니다.
    *   *참고*: 각 페이즈에서 발생하는 일(예: 새 어빌리티 활성화)을 정의하려면 C++나 블루프린트에서 추가 로직 구현이 필요할 수 있습니다.

### 2.4. AI 컨트롤러 및 비헤이비어 트리 (Behavior Tree)
1.  **Blackboard** (`BB_Boss`)와 **Behavior Tree** (`BT_Boss`)를 생성합니다.
2.  **Blackboard Keys**:
    *   `TargetActor` (Object)
    *   `DistanceToTarget` (Float)
    *   `State` (Enum/Int)
    *   `CanAttack` (Bool)
3.  **Behavior Tree 로직**:
    *   **Selector**:
        *   **Sequence (Combat)**:
            *   Decorator: `TargetActor`가 유효한가?
            *   Service: `BTService_UpdateMonsterState`.
            *   Task: `BTTask_MonsterFindTarget` (타겟이 없을 경우).
            *   Task: `BTTask_PursueTarget` (사거리 내로 이동).
            *   Task: `BTTask_MonsterAttack` (공격 어빌리티 실행).
4.  보스가 사용하는 `HarmoniaMonsterAIController`(또는 커스텀 컨트롤러)에 이 비헤이비어 트리를 할당합니다.

---

## 3. 레벨 및 게임 모드 설정

### 3.1. 게임 모드 (Game Mode)
1.  **GameMode**를 생성합니다 (예: `GM_BossFight`).
2.  **Default Pawn Class**를 `BP_HeroCharacter`로 설정합니다.
3.  **Player Controller Class**를 사용 중인 컨트롤러(또는 기본값)로 설정합니다.

### 3.2. 레벨 설정
1.  새 레벨을 생성합니다.
2.  **World Settings**: GameMode Override를 `GM_BossFight`로 설정합니다.
3.  **NavMesh**: 전투 구역을 덮도록 `NavMeshBoundsVolume`을 배치합니다. `P` 키를 눌러 생성을 확인합니다.
4.  **Player Start**: `PlayerStart` 액터를 배치합니다.
5.  **Spawn Boss**: `BP_Boss_Guardian`을 레벨에 배치합니다.

---

## 4. 전투 테스트

1.  모든 블루프린트를 **Compile & Save** 합니다.
2.  **Play (PIE)**를 누릅니다.
3.  **확인 사항**:
    *   플레이어가 스폰되고 이동/공격이 가능한가.
    *   보스가 플레이어를 감지하는가 (AI Controller의 Perception 설정 확인).
    *   보스가 플레이어를 향해 이동하는가.
    *   사거리 내에서 보스가 공격하는가.
    *   보스 체력바(UI)가 표시되는가.
    *   체력이 감소함에 따라 페이즈 전환이 일어나는가 (설정된 경우).

## 5. 문제 해결 (Troubleshooting)

*   **보스가 움직이지 않나요?** NavMesh 생성이 제대로 되었는지, AI Controller가 빙의(Possess)되었는지 확인하세요.
*   **데미지가 안 들어가나요?** 무기와 메쉬의 Collision Preset을 확인하세요. 데미지 처리를 위한 Gameplay Tag가 등록되어 있는지 확인하세요.
*   **어빌리티가 발동하지 않나요?** `GrantAbilities`가 호출되었는지, 보스의 마나/스태미나가 충분한지 확인하세요.

---

## 6. 시각적 피드백 (Number Renderer & Combo)

전투의 타격감을 높이기 위해 데미지 숫자와 콤보 카운터를 표시하는 **Number Renderer** 시스템을 구현합니다.

### 6.1. 데미지 숫자 위젯 (WBP_DamageNumber)
1.  **Widget Blueprint** 생성 (`WBP_DamageNumber`).
2.  **Canvas Panel** 아래에 **Text Block** 추가.
    *   변수화 (`Is Variable` 체크) -> 이름: `DamageText`.
    *   폰트 크기, 색상(흰색/노란색), 외곽선 설정.
3.  **애니메이션** 생성 (`FloatUpAnim`):
    *   0.0초 ~ 1.0초: **Translation Y**를 0에서 -100으로 이동 (위로 떠오름).
    *   0.5초 ~ 1.0초: **Opacity**를 1에서 0으로 변경 (서서히 사라짐).
4.  **이벤트 그래프**:
    *   `Construct` 이벤트에서 `Play Animation` (FloatUpAnim) 호출.
    *   애니메이션 종료 시 `Remove from Parent` 호출.
    *   커스텀 이벤트 `SetDamage(Amount, Type)` 생성 -> 텍스트 설정 및 색상 변경 로직 추가 (예: 크리티컬은 빨간색).

### 6.2. 콤보 카운터 위젯 (WBP_ComboCounter)
1.  **Widget Blueprint** 생성 (`WBP_ComboCounter`).
2.  **Text Block** 추가: "Combo: 0" 형식.
3.  **애니메이션** 생성 (`BumpAnim`):
    *   콤보 갱신 시 텍스트 크기가 살짝 커졌다 작아지는 스케일 애니메이션.
4.  **이벤트 그래프**:
    *   커스텀 이벤트 `UpdateCombo(Count)` 생성 -> 텍스트 갱신 및 `BumpAnim` 재생.
    *   일정 시간 동안 콤보가 없으면 숨기는 로직 추가 (Timer 활용).

### 6.3. Number Renderer 컴포넌트 구현
데미지 숫자를 스폰하는 관리자 역할을 하는 컴포넌트입니다.

1.  **Actor Component** 블루프린트 생성 (`BPC_NumberRenderer`).
2.  함수 `SpawnDamageNumber(Location, Amount)` 생성:
    *   `Create Widget` (Class: `WBP_DamageNumber`).
    *   `Add to Viewport` (또는 `WidgetComponent`를 월드에 스폰).
    *   `Set Position in Viewport`를 사용하여 월드 좌표(`Location`)를 화면 좌표로 변환하여 배치.
    *   위젯의 `SetDamage` 호출.

### 6.4. 시스템 통합

#### 보스에게 데미지 숫자 표시
1.  `BP_Boss_Guardian` (또는 `HarmoniaMonsterBase` 자식) 블루프린트를 엽니다.
2.  `OnMonsterDamaged` 델리게이트(이벤트)를 할당합니다.
3.  이벤트 발생 시:
    *   플레이어 컨트롤러나 HUD에서 `BPC_NumberRenderer`에 접근 (또는 전역 접근 가능한 방식 사용).
    *   `SpawnDamageNumber` 호출.
    *   위치: 보스의 `GetActorLocation` + 랜덤 오프셋.
    *   데미지: 이벤트에서 넘어온 `DamageAmount`.

#### 플레이어 콤보 표시
1.  `BP_HeroCharacter` 블루프린트를 엽니다.
2.  `MeleeCombatComponent`의 콤보 상태를 확인합니다.
3.  `Tick` 이벤트 또는 공격 실행 시:
    *   `MeleeCombatComponent` -> `GetCurrentComboIndex` 호출.
    *   이전 프레임의 콤보 수와 비교하여 변경되었으면 `WBP_ComboCounter` -> `UpdateCombo` 호출.
    *   콤보가 0이면 위젯 숨김.

이렇게 하면 플레이어가 공격할 때마다 화려한 데미지 숫자와 콤보 카운트가 표시되어 전투의 만족감을 크게 높일 수 있습니다.
