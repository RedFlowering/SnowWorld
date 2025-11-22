# HarmoniaKit 치트 명령어 빠른 참조

## 자주 사용하는 명령어

```
HarmoniaHelp                      - 모든 명령어 목록 표시
HarmoniaResetCheats              - 모든 치트 리셋
```

## 체력/자원

```
HarmoniaHealFull                 - 체력 완전 회복
HarmoniaSetHealth 1000           - 체력 설정
HarmoniaSetMaxHealth 2000        - 최대 체력 설정
HarmoniaSetMana 500              - 마나 설정
HarmoniaSetStamina 300           - 스태미나 설정
```

## 재화

```
HarmoniaGiveGold 10000           - 골드 지급
HarmoniaSetGold 999999           - 골드 설정
```

## 전투

```
HarmoniaToggleInvincible         - 무적 토글
HarmoniaToggleGodMode            - 신 모드 (무적 + 무한자원)
HarmoniaToggleOneHitKill         - 원킬 모드
HarmoniaSetDamageMultiplier 5.0  - 데미지 5배
```

## 이동

```
HarmoniaSetSpeed 2.0             - 속도 2배
HarmoniaSetSpeed 0.5             - 속도 반으로
HarmoniaToggleFly                - 비행 모드
HarmoniaToggleNoClip             - 벽 통과 모드
HarmoniaTeleport 0 0 1000        - 좌표 이동
```

## 시간/날씨

```
HarmoniaSetTime 12 0             - 정오로 설정
HarmoniaSetTime 0 0              - 자정으로 설정
HarmoniaSetTimeScale 10.0        - 시간 10배속
HarmoniaSetTimeScale 0           - 시간 정지
HarmoniaSetWeather Clear         - 맑음
HarmoniaSetWeather Rain          - 비
HarmoniaSetWeather Snow          - 눈
HarmoniaSetWeather Storm         - 폭풍
```

## 디버그

```
HarmoniaToggleDebugInfo          - 디버그 정보 표시
HarmoniaKillAllEnemies           - 모든 적 제거
HarmoniaSpawnEnemy MonsterName 5 - 적 5마리 스폰
```

## 블루프린트 빠른 함수

```cpp
Quick Heal                       - 즉시 회복
Quick Toggle Invincible          - 무적 토글
Quick Give Gold                  - 골드 10000
Quick Toggle Fly                 - 비행 토글
Quick Double Speed               - 속도 2배
Quick Reset Cheats               - 치트 리셋
```

## 개발 세팅 조합 예시

### 빠른 테스트 준비
```
HarmoniaSetMaxHealth 10000
HarmoniaHealFull
HarmoniaGiveGold 999999
HarmoniaSetSpeed 2.0
```

### 전투 테스트
```
HarmoniaToggleInvincible
HarmoniaSetDamageMultiplier 10.0
```

### 이동 테스트
```
HarmoniaToggleFly
HarmoniaSetSpeed 5.0
```

### 시간/날씨 테스트
```
HarmoniaSetTimeScale 100.0
HarmoniaSetWeather Storm
```

## 단축키 추천

권장하는 키 바인딩:

- **F1** - Quick Heal
- **F2** - Quick Toggle Fly
- **F3** - Quick Give Gold
- **F4** - Quick Toggle Invincible
- **F5** - Quick Reset Cheats
- **F12** - 개발자 메뉴 열기

## 팁

1. **자동완성 사용**: 콘솔에서 `Harmonia`를 입력하고 Tab 키를 눌러 명령어 목록 확인
2. **이전 명령어**: 콘솔에서 위/아래 화살표로 이전 명령어 재사용
3. **여러 명령어**: 세미콜론(;)으로 명령어 연결 가능
4. **매크로 만들기**: 자주 쓰는 조합은 블루프린트 함수로 만들기

---

자세한 내용은 `CheatSystem_Guide.md`를 참조하세요.
