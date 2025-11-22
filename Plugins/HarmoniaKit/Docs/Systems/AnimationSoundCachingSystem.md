# Animation & Sound Caching System - Tag-Based Asset Management

태그 기반 애니메이션 및 사운드 관리 시스템입니다. 데이터 테이블을 사용하여 모든 애니메이션과 사운드를 중앙에서 관리하고, GameplayTag로 빠르게 조회하여 사용할 수 있습니다.

## 핵심 개념

### 장점
- ✅ **중앙 집중 관리**: 모든 애니메이션/사운드를 데이터 테이블에서 관리
- ✅ **태그 기반 조회**: GameplayTag로 빠른 검색 및 사용
- ✅ **일괄 수정 용이**: 테이블만 수정하면 모든 곳에 반영
- ✅ **메모리 효율적**: 게임 시작 시 한 번만 로드 후 캐싱
- ✅ **유연한 오버라이드**: 필요한 경우 특정 값만 런타임에서 변경 가능

## 1. 애니메이션 캐싱 시스템

### 데이터 구조

**FHarmoniaAnimationData** - 애니메이션 데이터
```cpp
- AnimationTag: 애니메이션 식별 태그 (예: "Anim.Player.Attack.Sword.Light")
- AnimMontage: 애니메이션 몽타주 (권장)
- AnimSequence: 애니메이션 시퀀스
- BlendSpace: 블렌드 스페이스
- PlayRate: 재생 속도 (기본: 1.0)
- BlendInTime: 블렌드 인 시간
- BlendOutTime: 블렌드 아웃 시간
- bEnableRootMotion: 루트 모션 활성화
- Priority: 우선순위 (인터럽트 관련)
- GameplayTags: 관련 게임플레이 태그
```

### 설정 방법

#### 1. DataTable 생성
```
콘텐츠 브라우저 → Data Table → FHarmoniaAnimationData
이름: DT_PlayerAnimations, DT_MonsterAnimations 등
```

#### 2. DataTable 예제

| Row Name | AnimationTag | AnimMontage | PlayRate | BlendInTime | Priority |
|----------|--------------|-------------|----------|-------------|----------|
| Anim.Player.Attack.Sword.Light | Anim.Player.Attack.Sword.Light | AM_Sword_Light | 1.0 | 0.2 | 5 |
| Anim.Player.Attack.Sword.Heavy | Anim.Player.Attack.Sword.Heavy | AM_Sword_Heavy | 0.8 | 0.3 | 7 |
| Anim.Player.Dodge.Forward | Anim.Player.Dodge.Forward | AM_Dodge_Forward | 1.2 | 0.1 | 8 |

#### 3. DefaultGame.ini 설정
```ini
[/Script/HarmoniaKit.HarmoniaAnimationCacheSubsystem]
+AnimationDataTablePaths=/Game/Data/DT_PlayerAnimations.DT_PlayerAnimations
+AnimationDataTablePaths=/Game/Data/DT_MonsterAnimations.DT_MonsterAnimations
bPreloadAllAnimations=true
bPreloadAssets=false
```

### 사용 방법

#### C++에서 사용
```cpp
// 서브시스템 가져오기
UHarmoniaAnimationCacheSubsystem* AnimCache =
    GetGameInstance()->GetSubsystem<UHarmoniaAnimationCacheSubsystem>();

// 애니메이션 재생
FGameplayTag AnimTag = FGameplayTag::RequestGameplayTag("Anim.Player.Attack.Sword.Light");
float Duration = AnimCache->PlayAnimationByTag(Character, AnimTag);

// 오버라이드 옵션 사용
FHarmoniaAnimationPlaybackContext Context;
Context.bOverridePlayRate = true;
Context.PlayRateOverride = 1.5f; // 1.5배속으로 재생
AnimCache->PlayAnimationByTag(Character, AnimTag, Context);

// 애니메이션 중지
AnimCache->StopAnimationByTag(Character, AnimTag);

// 카테고리별 모든 애니메이션 중지 (예: 모든 공격 애니메이션)
FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag("Anim.Player.Attack");
AnimCache->StopAnimationsByParentTag(Character, AttackTag, 0.25f);
```

#### Blueprint에서 사용
```
노드: Play Animation By Tag
입력:
- Character: 대상 캐릭터
- Animation Tag: 태그 (예: "Anim.Player.Attack.Sword.Light")
- Context: 재생 옵션 (선택)
출력:
- Duration: 애니메이션 길이
```

### Tag 구조

```
Anim
├─ Anim.Player
│  ├─ Anim.Player.Attack
│  │  ├─ Anim.Player.Attack.Sword
│  │  │  ├─ Anim.Player.Attack.Sword.Light
│  │  │  ├─ Anim.Player.Attack.Sword.Heavy
│  │  │  ├─ Anim.Player.Attack.Sword.Combo1
│  │  │  ├─ Anim.Player.Attack.Sword.Combo2
│  │  │  └─ Anim.Player.Attack.Sword.Combo3
│  ├─ Anim.Player.Dodge
│  │  ├─ Anim.Player.Dodge.Forward
│  │  ├─ Anim.Player.Dodge.Backward
│  │  ├─ Anim.Player.Dodge.Left
│  │  └─ Anim.Player.Dodge.Right
│  └─ Anim.Player.HitReaction
│     ├─ Anim.Player.HitReaction.Light
│     └─ Anim.Player.HitReaction.Heavy
└─ Anim.Monster
   └─ Anim.Monster.Goblin
      ├─ Anim.Monster.Goblin.Attack
      └─ Anim.Monster.Goblin.Death
```

---

## 2. 사운드 캐싱 시스템

### 데이터 구조

**FHarmoniaSoundData** - 사운드 데이터
```cpp
- SoundTag: 사운드 식별 태그 (예: "Sound.SFX.Hit.Metal")
- SoundCue: 사운드 큐 (권장, 랜덤화 가능)
- SoundWave: 사운드 웨이브
- SoundBase: 사운드 베이스 (일반)
- VolumeMultiplier: 볼륨 배율 (기본: 1.0)
- PitchMultiplier: 피치 배율 (기본: 1.0)
- VolumeRange: 볼륨 랜덤 범위 (예: 0.9~1.1)
- PitchRange: 피치 랜덤 범위 (예: 0.95~1.05)
- bIs2D: 2D 사운드 여부 (UI, 음악 등)
- AttenuationSettings: 감쇠 설정 (3D 사운드용)
- MaxConcurrentInstances: 최대 동시 재생 수
- MinTimeBetweenPlays: 최소 재생 간격 (초)
```

### 설정 방법

#### 1. DataTable 생성
```
콘텐츠 브라우저 → Data Table → FHarmoniaSoundData
이름: DT_SFX, DT_Music, DT_Voice 등
```

#### 2. DataTable 예제

| Row Name | SoundTag | SoundCue | VolumeMultiplier | VolumeRange | MaxConcurrentInstances | bIs2D |
|----------|----------|----------|------------------|-------------|------------------------|-------|
| Sound.SFX.Hit.Metal | Sound.SFX.Hit.Metal | SC_Hit_Metal | 0.8 | (0.9, 1.1) | 5 | false |
| Sound.SFX.Weapon.Sword.Swing | Sound.SFX.Weapon.Sword.Swing | SC_Sword_Swing | 0.6 | (0.95, 1.05) | 3 | false |
| Sound.Music.Combat.Intense | Sound.Music.Combat.Intense | MUS_Combat_Intense | 0.7 | (1.0, 1.0) | 1 | true |

#### 3. DefaultGame.ini 설정
```ini
[/Script/HarmoniaKit.HarmoniaSoundCacheSubsystem]
+SoundDataTablePaths=/Game/Audio/DT_SFX.DT_SFX
+SoundDataTablePaths=/Game/Audio/DT_Music.DT_Music
+SoundDataTablePaths=/Game/Audio/DT_Voice.DT_Voice
bPreloadAllSounds=true
bPreloadAssets=false
```

### 사용 방법

#### C++에서 사용 (2D 사운드)
```cpp
// 서브시스템 가져오기
UHarmoniaSoundCacheSubsystem* SoundCache =
    GetGameInstance()->GetSubsystem<UHarmoniaSoundCacheSubsystem>();

// 2D 사운드 재생 (UI, 음악)
FGameplayTag SoundTag = FGameplayTag::RequestGameplayTag("Sound.Music.Combat.Intense");
UAudioComponent* AudioComp = SoundCache->PlaySound2DByTag(this, SoundTag);

// 오버라이드 옵션 사용
FHarmoniaSoundPlaybackContext Context;
Context.bOverrideVolume = true;
Context.VolumeOverride = 0.5f;
Context.FadeInDuration = 2.0f; // 2초 페이드 인
SoundCache->PlaySound2DByTag(this, SoundTag, Context);
```

#### C++에서 사용 (3D 사운드)
```cpp
// 위치에서 사운드 재생
FGameplayTag SoundTag = FGameplayTag::RequestGameplayTag("Sound.SFX.Hit.Metal");
FVector HitLocation = GetActorLocation();
UAudioComponent* AudioComp = SoundCache->PlaySoundAtLocationByTag(
    this,
    SoundTag,
    HitLocation
);

// 컴포넌트에 부착하여 재생
USceneComponent* WeaponMesh = GetMesh();
SoundCache->PlaySoundAttachedByTag(
    SoundTag,
    WeaponMesh,
    FName("weapon_tip")
);
```

#### Blueprint에서 사용
```
노드: Play Sound 2D By Tag
입력:
- Sound Tag: 태그 (예: "Sound.Music.Combat.Intense")
- Context: 재생 옵션 (선택)
출력:
- Audio Component: 재생 중인 오디오 컴포넌트

노드: Play Sound At Location By Tag
입력:
- Sound Tag: 태그
- Location: 재생 위치
- Rotation: 회전 (선택)
- Context: 재생 옵션 (선택)
출력:
- Audio Component: 재생 중인 오디오 컴포넌트
```

#### 사운드 중지
```cpp
// 특정 태그의 모든 사운드 중지
SoundCache->StopSoundsByTag(SoundTag, 0.5f); // 0.5초 페이드 아웃

// 카테고리별 모든 사운드 중지
FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag("Sound.SFX.Weapon");
SoundCache->StopSoundsByParentTag(WeaponTag, 0.5f);
```

### Tag 구조

```
Sound
├─ Sound.SFX
│  ├─ Sound.SFX.Hit
│  │  ├─ Sound.SFX.Hit.Metal
│  │  ├─ Sound.SFX.Hit.Wood
│  │  ├─ Sound.SFX.Hit.Stone
│  │  └─ Sound.SFX.Hit.Flesh
│  ├─ Sound.SFX.Weapon
│  │  └─ Sound.SFX.Weapon.Sword
│  │     ├─ Sound.SFX.Weapon.Sword.Swing
│  │     └─ Sound.SFX.Weapon.Sword.Hit
│  └─ Sound.SFX.Footstep
│     ├─ Sound.SFX.Footstep.Stone
│     ├─ Sound.SFX.Footstep.Wood
│     └─ Sound.SFX.Footstep.Grass
├─ Sound.Music
│  ├─ Sound.Music.Combat
│  │  └─ Sound.Music.Combat.Intense
│  └─ Sound.Music.Ambient
│     └─ Sound.Music.Ambient.Peaceful
└─ Sound.Voice
   └─ Sound.Voice.Player
      ├─ Sound.Voice.Player.Grunt
      └─ Sound.Voice.Player.Death
```

---

## 고급 기능

### 1. 동시성 관리 (Concurrency)

사운드 데이터 테이블에서 설정:
```
MaxConcurrentInstances: 5  # 최대 5개까지 동시 재생
bStopOldestInstance: true  # 최대치 초과 시 가장 오래된 것 중지
MinTimeBetweenPlays: 0.1   # 0.1초마다 한 번만 재생 가능
```

### 2. 랜덤화

```
VolumeRange: (0.9, 1.1)    # 볼륨 90%~110% 랜덤
PitchRange: (0.95, 1.05)   # 피치 95%~105% 랜덤
```

매번 재생할 때마다 약간씩 다른 소리가 나서 자연스러움.

### 3. 공간 음향 (Spatial Audio)

```
bIs2D: false  # 3D 사운드
AttenuationSettings: /Game/Audio/ATT_Default  # 감쇠 설정
bOverrideAttenuation: true
AttenuationDistance: 2000.0  # 2000 유닛까지 들림
```

### 4. GameplayTag 쿼리

```cpp
// 모든 "Hit" 사운드 가져오기
FGameplayTag HitTag = FGameplayTag::RequestGameplayTag("Sound.SFX.Hit");
TArray<FHarmoniaSoundData> HitSounds = SoundCache->GetSoundsByTag(HitTag, false);

// 특정 GameplayTag 조합으로 검색
FGameplayTagContainer SearchTags;
SearchTags.AddTag(FGameplayTag::RequestGameplayTag("Sound.Category.Combat"));
SearchTags.AddTag(FGameplayTag::RequestGameplayTag("Sound.Type.Music"));
TArray<FHarmoniaSoundData> CombatMusic = SoundCache->GetSoundsByGameplayTags(SearchTags, true);
```

---

## 실전 예제

### 예제 1: 검 공격 애니메이션 + 사운드

```cpp
// 애니메이션 재생
FGameplayTag AttackAnimTag = FGameplayTag::RequestGameplayTag("Anim.Player.Attack.Sword.Light");
AnimCache->PlayAnimationByTag(Character, AttackAnimTag);

// 검 휘두르는 소리
FGameplayTag SwingSound = FGameplayTag::RequestGameplayTag("Sound.SFX.Weapon.Sword.Swing");
SoundCache->PlaySoundAttachedByTag(SwingSound, Character->GetMesh(), FName("weapon_tip"));
```

### 예제 2: 피격 시 애니메이션 + 사운드

```cpp
// 피격 애니메이션
FGameplayTag HitAnimTag = FGameplayTag::RequestGameplayTag("Anim.Player.HitReaction.Light");
AnimCache->PlayAnimationByTag(Character, HitAnimTag);

// 피격 사운드 (타격 위치에서)
FGameplayTag HitSound = FGameplayTag::RequestGameplayTag("Sound.SFX.Hit.Flesh");
SoundCache->PlaySoundAtLocationByTag(this, HitSound, HitLocation);
```

### 예제 3: 전투 음악 페이드 인/아웃

```cpp
// 전투 시작 - 음악 페이드 인
FHarmoniaSoundPlaybackContext Context;
Context.FadeInDuration = 2.0f;
FGameplayTag CombatMusic = FGameplayTag::RequestGameplayTag("Sound.Music.Combat.Intense");
UAudioComponent* MusicComp = SoundCache->PlaySound2DByTag(this, CombatMusic, Context);

// 전투 종료 - 음악 페이드 아웃
SoundCache->StopSoundsByTag(CombatMusic, 2.0f); // 2초 페이드 아웃
```

### 예제 4: 발소리 (표면 타입별)

```cpp
// 지면 타입에 따른 발소리
EPhysicalSurface SurfaceType = GetGroundSurfaceType();

FGameplayTag FootstepTag;
switch (SurfaceType)
{
    case SurfaceType_Stone:
        FootstepTag = FGameplayTag::RequestGameplayTag("Sound.SFX.Footstep.Stone");
        break;
    case SurfaceType_Wood:
        FootstepTag = FGameplayTag::RequestGameplayTag("Sound.SFX.Footstep.Wood");
        break;
    case SurfaceType_Grass:
        FootstepTag = FGameplayTag::RequestGameplayTag("Sound.SFX.Footstep.Grass");
        break;
}

SoundCache->PlaySoundAtLocationByTag(this, FootstepTag, GetActorLocation());
```

---

## 성능 고려사항

### 메모리
- `bPreloadAllAnimations/Sounds=true`: 게임 시작 시 모든 메타데이터 로드 (권장)
- `bPreloadAssets=false`: 에셋은 필요할 때 로드 (메모리 절약, 권장)
- `bPreloadAssets=true`: 에셋까지 미리 로드 (로딩 빠름, 메모리 많이 사용)

### 캐싱
- 첫 조회: LoadSynchronous() 호출 (약간 느림)
- 이후 조회: 캐시에서 즉시 반환 (매우 빠름)
- Tag 조회: O(log n) - TMap 사용

### 동시성
- 사운드 동시 재생 수 제한으로 CPU/메모리 절약
- 자동 정리 시스템으로 재생 완료된 오디오 컴포넌트 제거

---

## 문제 해결

### "Animation data not found" 경고
1. DataTable에 해당 태그의 Row가 있는지 확인
2. `AnimationDataTablePaths` 설정이 올바른지 확인
3. Row Name과 AnimationTag 필드가 일치하는지 확인

### 애니메이션이 재생되지 않음
1. AnimMontage가 올바르게 설정되어 있는지 확인
2. 캐릭터에 AnimInstance가 있는지 확인
3. AnimBP에 Slot 노드가 있는지 확인

### "Sound data not found" 경고
1. DataTable에 해당 태그의 Row가 있는지 확인
2. `SoundDataTablePaths` 설정이 올바른지 확인

### 사운드가 재생되지 않음
1. SoundCue 또는 SoundWave가 올바르게 설정되어 있는지 확인
2. MaxConcurrentInstances 제한에 걸렸는지 확인
3. MinTimeBetweenPlays 제한에 걸렸는지 확인

### 3D 사운드가 들리지 않음
1. bIs2D가 false로 설정되어 있는지 확인
2. AttenuationSettings가 설정되어 있는지 확인
3. 재생 위치가 리스너 범위 내에 있는지 확인
