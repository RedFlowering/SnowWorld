# HarmoniaKit Plugin Documentation

**Version:** 1.0  
**Last Updated:** 2025-11-23  
**Unreal Engine:** 5.3+

## 📖 목차

- [소개](#소개)
- [시작하기](#시작하기)
- [핵심 시스템](#핵심-시스템)
- [게임플레이 시스템](#게임플레이-시스템)
- [온라인 시스템](#온라인-시스템)
- [개발 도구](#개발-도구)
- [리팩토링 히스토리](#리팩토링-히스토리)

---

## 소개

HarmoniaKit은 Unreal Engine 5용 고성능 멀티플레이어 RPG 플러그인입니다.

### 주요 특징

✨ **고성능 전투 시스템** - Sense System 기반 멀티스레드 히트 감지  
⚔️ **다양한 전투 메커니즘** - 근접/원거리, 콤보, 방어, 회피  
🎮 **Gameplay Ability System** - 강력한 어빌리티 프레임워크  
🌍 **절차적 월드 생성** - 무한한 탐험 가능한 세계  
🔧 **모드 시스템** - 사용자 정의 콘텐츠 지원  
🌐 **온라인 서브시스템** - Steam, EOS 통합  
🛠️ **치트 시스템** - 개발 및 디버깅 도구 (Editor/Development 빌드 전용)

---

## 시작하기

### 📚 빠른 시작 가이드

초보자를 위한 단계별 가이드:

- **[근접 전투 빠른 시작](./GettingStarted/QuickStart_MeleeCombat.md)** - 근접 공격 구현
- **[회복 아이템 빠른 시작](./GettingStarted/RecoveryItemSystem_QuickStart.md)** - 포션 시스템
- **[던전 레이드 빠른 시작](./GettingStarted/DungeonRaidSystem_QuickStart.md)** - 던전 생성
- **[사망 패널티 빠른 시작](./GettingStarted/DeathPenaltySystem_QuickStart.md)** - 죽음 처리

---

## 핵심 시스템

### ⚔️ 전투 시스템

**[Combat System](./Systems/CombatSystem.md)**
- Gameplay Ability System 기반
- Sense System 멀티스레드 히트 감지
- 다양한 데미지 타입 (Instant, DoT, Explosion, Percentage)
- 히트 리액션 및 크리티컬 시스템

**[근접 전투](./Systems/MeleeCombatSystem.md)**
- 근접 무기 시스템
- 블록, 패리, 회피

**[콤보 공격](./Systems/ComboAttackSystem.md)**
- 체인 공격 시스템
- 콤보 윈도우 관리

### 🎭 상호작용 시스템

**[Sense Interaction System](./Systems/SenseInteractionSystem.md)**
- 멀티스레드 감지 시스템
- 동적 상호작용 타겟팅

### 🎬 애니메이션 시스템

**[Animation Effect System](./Systems/AnimationEffectSystem.md)**
- 애니메이션 동기화 이펙트

**[Animation Sound Caching](./Systems/AnimationSoundCachingSystem.md)**
- 사운드 최적화

### 💪 캐릭터 시스템

**[Stamina System](./Systems/StaminaSystem.md)**
- 스태미나 관리
- 행동별 소모량 설정

---

## 게임플레이 시스템

### 📜 퀘스트 시스템

**[Quest System](./Systems/QuestSystem.md)**
- 기본 및 고급 퀘스트 기능 통합
- 단계별(Phase) 퀘스트
- 동적 목표 및 보상 시스템

### 🛡️ 생존 시스템

**[Recovery Item System](./Systems/RecoveryItemSystem.md)**
- 포션, 음식, 붕대 등
- 즉시/지속 회복

**[Death Penalty System](./Systems/DeathPenaltySystem.md)**
- 사망 시 패널티
- 경험치/아이템 드롭

### 🏰 던전 시스템

**[Dungeon Raid System](./Systems/DungeonRaidSystem.md)**
- 절차적 던전 생성
- 난이도 시스템
- 보상 시스템

### 🔨 제작 시스템

**[Crafting System](./Systems/CraftingSystem.md)**
- 아이템 제작
- 레시피 시스템
- 제작 스킬

---

## 온라인 시스템

### 🌐 온라인 서브시스템

**[Harmonia Online Subsystem](./Systems/HarmoniaOnlineSubsystem.md)**
- Steam 통합
- Epic Online Services (EOS)
- 세션 관리
- 매치메이킹

---

## 개발 도구

### 🔧 모드 시스템

**[Mod System](./Systems/ModSystem.md)**
- 사용자 정의 콘텐츠 지원
- 모드 로딩 시스템

### 🌍 월드 생성

**[World Generator](./Systems/WorldGenerator.md)**
- 절차적 지형 생성
- 바이옴 시스템

### 💾 저장 시스템

**[Save System](./Systems/SaveSystem.md)**
- 자동/수동 저장
- 클라우드 저장

### 🛠️ 치트 시스템

**[Cheat System](./Systems/CheatSystem.md)**
- 개발 및 디버깅 도구
- 빌드 제한 설정 (Shipping 빌드 자동 비활성화)
- 콘솔 명령어 가이드

---

## 리팩토링 히스토리

프로젝트 개선 과정 기록:

- **[2025-11-22 리팩토링 보고서](./Refactoring/Refactoring_Report_2025-11-22.md)**
  - 전투 시스템 분리
  - Base Combat Component 생성
  - Combat Library 유틸리티
  
- **[치트 시스템 빌드 제한](./Refactoring/Refactoring_Report_2025-11-22_CheatSystem_BuildRestriction.md)**
  - Shipping 빌드에서 치트 비활성화
  - 조건부 컴파일 적용

---

## 문서 구조

```
Docs/
├── README.md (이 파일)
│
├── GettingStarted/          # 빠른 시작 가이드
│   ├── QuickStart_MeleeCombat.md
│   ├── RecoveryItemSystem_QuickStart.md
│   ├── DungeonRaidSystem_QuickStart.md
│   └── DeathPenaltySystem_QuickStart.md
│
├── Systems/                 # 시스템 상세 문서
│   ├── CombatSystem.md
│   ├── MeleeCombatSystem.md
│   ├── ComboAttackSystem.md
│   ├── SenseInteractionSystem.md
│   ├── AnimationEffectSystem.md
│   ├── AnimationSoundCachingSystem.md
│   ├── StaminaSystem.md
│   ├── QuestSystem.md
│   ├── RecoveryItemSystem.md
│   ├── DeathPenaltySystem.md
│   ├── DungeonRaidSystem.md
│   ├── CraftingSystem.md
│   ├── HarmoniaOnlineSubsystem.md
│   ├── ModSystem.md
│   ├── WorldGenerator.md
│   ├── SaveSystem.md
│   └── CheatSystem.md
│
└── Refactoring/            # 리팩토링 기록
    ├── Refactoring_Report_2025-11-22.md
    └── Refactoring_Report_2025-11-22_CheatSystem_BuildRestriction.md
```

---

## 기여 가이드

### 문서 작성 규칙

1. **명확한 예제**: 모든 기능에는 동작하는 예제 코드 포함
2. **시각적 구조**: 다이어그램과 구조도 사용
3. **단계별 가이드**: 초보자도 따라할 수 있도록 작성
4. **최신 상태 유지**: 코드 변경 시 문서도 업데이트

---

## 라이선스

Copyright © 2025 Snow Game Studio.  
All rights reserved.

---

**제작:** Antigravity AI & Snow Game Studio  
**날짜:** 2025-11-23
