# Animation Effect System - Tag-Based Effect Management

태그 기반 애니메이션 이펙트 관리 시스템입니다. 데이터 테이블을 사용하여 VFX/SFX를 중앙 집중식으로 관리하고, 애니메이션 노티파이에서는 태그만 지정하면 자동으로 로드됩니다.

## 핵심 개념

### 장점
- ✅ **중앙 집중 관리**: 모든 이펙트를 데이터 테이블에서 관리
- ✅ **일괄 수정 용이**: 데이터 테이블만 수정하면 모든 애니메이션에 반영
- ✅ **메모리 효율적**: 게임 시작 시 한 번만 로드 후 캐싱
- ✅ **태그 기반 검색**: GameplayTag로 빠른 조회 (O(log n))
- ✅ **일관성 보장**: 같은 타입의 이펙트는 항상 같은 설정 사용

### 구조
```
┌─────────────────────────────────────────────┐
│  DataTable (FHarmoniaAnimationEffectData)   │
│  - Effect.Hit.Slash.Light                   │
│  - Effect.Hit.Slash.Heavy                   │
│  - Effect.Trail.Sword.Normal                │
│  └─────────────────────────────────────────┘
                    ▼
         ┌──────────────────────────┐
         │ HarmoniaEffectCacheSubsystem │
         │  (게임 시작 시 로드 및 캐싱)    │
         └──────────────────────────┘
                    ▼
      ┌─────────────────────────────┐
      │ AnimNotify_PlayTaggedEffect  │
      │ - EffectTag 설정만으로 사용   │
      └─────────────────────────────┘
```

## 설정 방법

### 1. 데이터 테이블 생성

1. 콘텐츠 브라우저에서 우클릭 → **Miscellaneous → Data Table**
2. Row Structure로 **FHarmoniaAnimationEffectData** 선택
3. 테이블 이름: `DT_AnimationEffects` (또는 원하는 이름)

### 2. 데이터 테이블 설정

각 Row에 다음 정보 입력:

```
Row Name: Effect.Hit.Slash.Light
├─ EffectTag: Effect.Hit.Slash.Light
├─ DisplayName: "Light Slash Hit"
├─ NiagaraSystem: NS_SlashHit_Light
├─ Sound: SFX_SlashHit_Light
├─ EffectScale: (1.0, 1.0, 1.0)
├─ VolumeMultiplier: 0.8
├─ AttachSocketName: weapon_tip
└─ bAttachToSocket: true
```

### 3. 서브시스템 설정

`DefaultGame.ini` 또는 프로젝트 세팅에 추가:

```ini
[/Script/HarmoniaKit.HarmoniaEffectCacheSubsystem]
; 데이터 테이블 경로 설정
+EffectDataTablePaths=/Game/Data/DT_AnimationEffects.DT_AnimationEffects
+EffectDataTablePaths=/Game/Data/DT_WeaponTrailEffects.DT_WeaponTrailEffects

; 게임 시작 시 모든 이펙트 미리 로드
bPreloadAllEffects=true

; 참조된 에셋도 미리 로드 (메모리 사용 증가, 런타임 성능 향상)
bPreloadAssets=false
```

또는 Blueprint에서:

```cpp
// GameInstance Blueprint의 Init에서
HarmoniaEffectCacheSubsystem->EffectDataTablePaths.Add("/Game/Data/DT_AnimationEffects");
HarmoniaEffectCacheSubsystem->ReloadEffectCache();
```

## 사용 방법

### AnimNotify에서 사용

#### 1. PlayTaggedEffect (Hit Effects용)

1. 애니메이션 에디터에서 노티파이 추가
2. **Play Tagged Effect** 선택
3. `EffectTag`에 원하는 태그 입력 (예: `Effect.Hit.Slash.Light`)
4. 필요시 Override 옵션으로 특정 값만 변경

**Override 옵션:**
- `OverrideSocketName`: 소켓 이름 변경
- `bOverrideScale`: 스케일 변경
- `ScaleMultiplier`: 스케일 곱하기 (데이터 테이블 값에 추가 적용)
- `VolumeMultiplier`: 볼륨 곱하기

#### 2. PlayTaggedWeaponTrail (Weapon Trail용)

1. 애니메이션 에디터에서 노티파이 스테이트 추가
2. **Play Tagged Weapon Trail** 선택
3. `EffectTag`에 원하는 태그 입력 (예: `Effect.Trail.Sword.Normal`)
4. 필요시 Trail 관련 Override 옵션 설정

**Override 옵션:**
- `bOverrideTrailSockets`: 트레일 시작/끝 소켓 변경
- `bOverrideTrailWidth`: 트레일 너비 변경
- `WidthMultiplier`: 너비 곱하기
- `bOverrideColor`: 트레일 색상 변경

### C++에서 사용

```cpp
// 서브시스템 가져오기
UHarmoniaEffectCacheSubsystem* EffectCache =
    GetGameInstance()->GetSubsystem<UHarmoniaEffectCacheSubsystem>();

// 이펙트 데이터 조회
FGameplayTag EffectTag = FGameplayTag::RequestGameplayTag("Effect.Hit.Slash.Light");
if (const FHarmoniaAnimationEffectData* EffectData = EffectCache->GetEffectData(EffectTag))
{
    // EffectData 사용
    UNiagaraSystem* VFX = EffectData->NiagaraSystem.LoadSynchronous();
    USoundBase* SFX = EffectData->Sound.LoadSynchronous();
}

// 모든 "Hit" 이펙트 가져오기
TArray<FHarmoniaAnimationEffectData> HitEffects =
    EffectCache->GetEffectsByTag(FGameplayTag::RequestGameplayTag("Effect.Hit"), false);
```

## GameplayTag 구조

### 기본 계층 구조

```
Effect
├─ Effect.Hit
│  ├─ Effect.Hit.Slash
│  │  ├─ Effect.Hit.Slash.Light
│  │  ├─ Effect.Hit.Slash.Medium
│  │  └─ Effect.Hit.Slash.Heavy
│  ├─ Effect.Hit.Blunt
│  │  ├─ Effect.Hit.Blunt.Light
│  │  └─ Effect.Hit.Blunt.Heavy
│  └─ Effect.Hit.Pierce
│     ├─ Effect.Hit.Pierce.Light
│     └─ Effect.Hit.Pierce.Heavy
├─ Effect.Trail
│  ├─ Effect.Trail.Sword
│  │  ├─ Effect.Trail.Sword.Normal
│  │  ├─ Effect.Trail.Sword.Fire
│  │  └─ Effect.Trail.Sword.Ice
│  ├─ Effect.Trail.Axe
│  │  └─ Effect.Trail.Axe.Heavy
│  └─ Effect.Trail.Dagger
│     └─ Effect.Trail.Dagger.Fast
└─ Effect.Impact
   ├─ Effect.Impact.Metal
   ├─ Effect.Impact.Wood
   ├─ Effect.Impact.Stone
   └─ Effect.Impact.Flesh
```

### 확장 가능

프로젝트에 맞게 자유롭게 태그 추가:

```
Effect.Hit.Magic.Fire
Effect.Hit.Magic.Ice
Effect.Trail.Bow.Arrow
Effect.Impact.Water.Splash
```

## 예제

### 데이터 테이블 예제

| Row Name | EffectTag | NiagaraSystem | Sound | EffectScale | VolumeMultiplier | AttachSocketName |
|----------|-----------|---------------|-------|-------------|------------------|------------------|
| Effect.Hit.Slash.Light | Effect.Hit.Slash.Light | NS_SlashHit_Light | SFX_Slash_Light | (1,1,1) | 0.8 | weapon_tip |
| Effect.Hit.Slash.Heavy | Effect.Hit.Slash.Heavy | NS_SlashHit_Heavy | SFX_Slash_Heavy | (1.5,1.5,1.5) | 1.0 | weapon_tip |
| Effect.Trail.Sword.Normal | Effect.Trail.Sword.Normal | NS_SwordTrail | SFX_Swoosh | (1,1,1) | 0.5 | weapon_trail_start |

### 애니메이션 설정 예제

**Light Attack Animation:**
1. 프레임 10: PlayTaggedEffect (`Effect.Hit.Slash.Light`)
2. 프레임 5-15: PlayTaggedWeaponTrail (`Effect.Trail.Sword.Normal`)

**Heavy Attack Animation:**
1. 프레임 20: PlayTaggedEffect (`Effect.Hit.Slash.Heavy`)
   - ScaleMultiplier: 1.5 (더 크게)
   - VolumeMultiplier: 1.2 (더 크게)
2. 프레임 10-25: PlayTaggedWeaponTrail (`Effect.Trail.Sword.Normal`)
   - WidthMultiplier: 1.5 (더 두껍게)

## 디버깅

### 로그 확인

```cpp
// LogHarmoniaEffectCache 카테고리 확인
LogHarmoniaEffectCache: Initializing Harmonia Effect Cache Subsystem
LogHarmoniaEffectCache: Loaded 15 effects from DataTable: DT_AnimationEffects
LogHarmoniaEffectCache: Total effects loaded: 15
```

### 에디터 명령어

```cpp
// Blueprint에서 호출 가능
HarmoniaEffectCacheSubsystem->ReloadEffectCache()  // 캐시 다시 로드
HarmoniaEffectCacheSubsystem->GetCachedEffectCount()  // 캐시된 이펙트 수
HarmoniaEffectCacheSubsystem->GetAllEffectTags()  // 모든 태그 목록
```

## 마이그레이션 가이드

### 기존 AnimNotify에서 전환

**Before (PlayHitEffect):**
```cpp
// 각 노티파이마다 개별 설정
HitParticleSystem: NS_SlashHit
HitSound: SFX_Slash
VolumeMultiplier: 0.8
AttachSocketName: weapon_tip
```

**After (PlayTaggedEffect):**
```cpp
// 태그만 설정
EffectTag: Effect.Hit.Slash.Light
// 나머지는 데이터 테이블에서 자동 로드
```

### 장점
1. 같은 이펙트를 100개 애니메이션에서 사용해도 데이터 테이블 1줄만 수정
2. 메모리 효율적 (에셋 중복 참조 없음)
3. 일관성 보장 (실수로 다른 설정 사용 방지)

## 성능 고려사항

### 메모리
- `bPreloadAllEffects=true`: 게임 시작 시 모든 메타데이터 로드 (권장)
- `bPreloadAssets=false`: 에셋은 필요할 때 로드 (메모리 절약)
- `bPreloadAssets=true`: 에셋까지 미리 로드 (로딩 빠름, 메모리 많이 사용)

### 조회 성능
- Tag 조회: O(log n) - TMap 사용
- 첫 조회 시 에셋 로드: LoadSynchronous() 호출
- 이후 조회: 캐시에서 즉시 반환

### Lazy Loading
```cpp
// 캐시에 없으면 자동 로드
bUseLazyLoading=true  // 기본값, 권장
```

## 문제 해결

### "Effect data not found" 경고
1. 데이터 테이블에 해당 태그의 Row가 있는지 확인
2. `EffectDataTablePaths` 설정이 올바른지 확인
3. Row Name과 EffectTag 필드가 일치하는지 확인

### 이펙트가 재생되지 않음
1. NiagaraSystem 또는 ParticleSystem이 설정되어 있는지 확인
2. 에셋 경로가 유효한지 확인 (레퍼런스 뷰어로 확인)
3. 소켓 이름이 메시에 존재하는지 확인

### 게임 시작 시 로딩 느림
1. `bPreloadAssets=false`로 변경 (필요할 때만 로드)
2. 데이터 테이블을 여러 개로 분할 (무기별, 타입별)
3. Async 로드 사용: `PreloadEffectAssets(true)`
