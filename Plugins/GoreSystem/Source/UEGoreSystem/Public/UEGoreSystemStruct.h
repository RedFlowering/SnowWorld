// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UEGoreSystemAnimInstance.h"
#include "UEGoreSystemStruct.generated.h"

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemDetachedReference
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TMap<FName, USkeletalMeshComponent*> References;

	FUEGoreSystemDetachedReference()
	{
		References = TMap<FName, USkeletalMeshComponent*>();
	}
};

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemDecoArray
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TArray<class UStreamableRenderAsset*> Decos;

	FUEGoreSystemDecoArray()
	{
		Decos = TArray<class UStreamableRenderAsset*>();
	}
};

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemHealthValues
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TArray<FName> BoneName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TArray<float> Health;

	FUEGoreSystemHealthValues()
	{
		BoneName = TArray<FName>();
		Health = TArray < float>();
	}

	FUEGoreSystemHealthValues(const TArray < FName> InName, const TArray<float> InHealth)
	{
		BoneName = InName;
		Health = InHealth;
	}

	bool Contains(const FName InName)
	{
		return BoneName.Contains(InName);
	}

	bool GetHealth(const FName InName, float& OutHealth)
	{
		if (!BoneName.Contains(InName))
			return false;

		OutHealth = Health[BoneName.IndexOfByKey(InName)];
		return true;
	}

	bool SetHealth(const FName InName, float InHealth)
	{
		if (!BoneName.Contains(InName))
			return false;

		Health[BoneName.IndexOfByKey(InName)] = InHealth;
		return true;
	}

	bool SetHealth(const TTuple<FName, float> InValue)
	{
		if (!BoneName.Contains(InValue.Key))
			return false;

		Health[BoneName.IndexOfByKey(InValue.Key)] = InValue.Value;
		return true;
	}

	bool Add(const FName InName, float InHealth)
	{
		if (BoneName.Contains(InName))
			return false;

		BoneName.Add(InName);
		Health.Add(InHealth);
		return true;
	}

	bool Add(const TTuple<FName, float> InValue)
	{
		if (BoneName.Contains(InValue.Key))
			return false;

		BoneName.Add(InValue.Key);
		Health.Add(InValue.Value);
		return true;
	}
};

FORCEINLINE bool operator==(const FUEGoreSystemHealthValues& Param1, const FUEGoreSystemHealthValues& Param2)
{
	return Param1.BoneName == Param2.BoneName;
}

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemHit
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FName BoneName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FVector Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FVector Normal;

	FUEGoreSystemHit()
	{
		BoneName = FName();
		Location = FVector(0.0f);
		Normal = FVector(0.0f);
	}
};

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemSounds
{
	GENERATED_USTRUCT_BODY()

	/** Sound on hit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class USoundCue* Hit;
	/** Sound on destroy*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class USoundCue* Break;

	FUEGoreSystemSounds()
	{
		Hit = nullptr;
		Break = nullptr;
	}
};

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemEffects
{
	GENERATED_USTRUCT_BODY()

	/** Niagara effect to spawn on Hit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class UNiagaraSystem* Hit;
	/** Legacy cascade effect to spawn on Hit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class UParticleSystem* HitLegacy;
	/** Niagara effect to spawn and attach on Destroy*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class UNiagaraSystem* Spill;
	/** Legacy cascade effect to spawn on Spill*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class UParticleSystem* SpillLegacy;
	/** Decal to spawn (both)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TSubclassOf<class ADecalActor> Decal;
	/** Time before destroying the decals*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		float DecalLifespan;

	FUEGoreSystemEffects()
	{
		Hit = nullptr;
		HitLegacy = nullptr;
		Spill = nullptr;
		SpillLegacy = nullptr;
		Decal = NULL;
		DecalLifespan = 25.0f;
	}
};

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemStruct
{
	GENERATED_USTRUCT_BODY()

	/** Mesh to use once the detached limb is spawned*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class USkeletalMesh* Mesh;
	/** Override materials*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TArray<UMaterialInterface*> MaterialsOverride;
	/** Deco mesh list to spawn when detach happen. You can spawn multiple decos for each entry. [USING CUSTOM SOCKETS INSTEAD OF THE SKELETAL'S BONES IS ADVICED]*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TMap<FName, FUEGoreSystemDecoArray> DecoMesh;
	/** Should override VFXs for this bone?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		uint8 OverrideEffects : 1;
	/** New VFXs to use*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem", meta = (EditCondition = "OverrideEffects"))
		FUEGoreSystemEffects Effects;
	/** Should override VFXs for this bone?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		uint8 OverrideSounds : 1;
	/** New SFXs to use*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem", meta = (EditCondition = "OverrideSounds"))
		FUEGoreSystemSounds Sounds;

	/** [READ ONLY] Detached mesh, if spawned*/
	UPROPERTY(BlueprintReadOnly, Category = "GoreSystem")
		class USkeletalMeshComponent* DetachedMesh;

	FUEGoreSystemStruct()
	{
		Mesh = nullptr;
		MaterialsOverride = TArray<UMaterialInterface*>();
		OverrideEffects = false;
		Effects = FUEGoreSystemEffects();
		OverrideSounds = false;
		Sounds = FUEGoreSystemSounds();

		DetachedMesh = nullptr;
	}
};

USTRUCT(BlueprintType)
struct UEGORESYSTEM_API FUEGoreSystemSettings
{
	GENERATED_USTRUCT_BODY()

	/** VFXs to spawn for all the bones (can be overridden for each "BodyMap" entry)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FUEGoreSystemEffects GlobalEffects;
	/** SFXs to spawn for all the bones (can be overridden for each "BodyMap" entry)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FUEGoreSystemSounds GlobalSounds;
	
	/** Animation instance you want to use for detached limbs (must derive from UUEGoreSystemAnimInstance and make sure to set it up accordingly to the example and documentation)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		class TSubclassOf<UUEGoreSystemAnimInstance> GlobalAnimInstance;

	/** Initial health to use for all the bones (can be overridden using "HealthMap")*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		float GlobalLimbsHealth;

	/** Settings for each bone you want to detach with this system*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TMap<FName, FUEGoreSystemStruct> BodyMap;
	/** List of bones that can be detached even if the character is still alive*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TArray<FName> IgnoreIsDead;
	/** Override "GlobalLimbsHealth" for each bone*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TMap<FName, float> HealthMap;
	/** Apply a custom damage multiplier for each entry on this list*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TMap<FName, float> DamageMultiplierMap;	
	/** Collision profile name to use for detached limbs once physics is enabled ("Ragdoll" is default)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FName LimbsCollisionProfile;
	/** Collision profile name to use for decos spawned after detachment ("NoCollision" is default)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		FName DecoCollisionProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GoreSystem")
		TArray<TEnumAsByte<ECollisionChannel>> DecalCollisionChannels;

	FUEGoreSystemSettings()
	{		
		DamageMultiplierMap = TMap<FName, float>();
		GlobalEffects = FUEGoreSystemEffects();
		GlobalSounds = FUEGoreSystemSounds();
		GlobalAnimInstance = UUEGoreSystemAnimInstance::StaticClass();
		GlobalLimbsHealth = 1.0f;
		BodyMap = TMap<FName, FUEGoreSystemStruct>();
		IgnoreIsDead = TArray<FName>();
		DecoCollisionProfile = FName(TEXT("NoCollision"));
		LimbsCollisionProfile = FName(TEXT("Ragdoll"));
		DecalCollisionChannels.Add(ECollisionChannel::ECC_WorldStatic);
		DecalCollisionChannels.Add(ECollisionChannel::ECC_WorldDynamic);
	}
};

/**
 * 
 */
class UEGORESYSTEM_API UEGoreSystemStruct
{
public:
	UEGoreSystemStruct();
	~UEGoreSystemStruct();
};
