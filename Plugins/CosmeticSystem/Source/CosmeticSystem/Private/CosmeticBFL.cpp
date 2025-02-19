// Copyright 2024 HGsofts, All Rights Reserved.


#include "CosmeticBFL.h"
#include "GameFramework/Character.h"
#include "CosmeticComponent.h"

ACosmeticActor* UCosmeticBFL::GetCosmeticActor(const ACharacter* Target)
{
	if (Target)
	{
		if (UCosmeticComponent* CosmeticComp = Target->FindComponentByClass<UCosmeticComponent>())
		{
			return CosmeticComp->GetCosmeticActorInstance();
		}
	}

	return nullptr;
}

USkeletalMeshComponent* UCosmeticBFL::GetCosmeticMegedSkeletalMeshComponent(const ACharacter* Target, FName ComponentTag)
{
	if (Target)
	{
		if (UCosmeticComponent* CosmeticComp = Target->FindComponentByClass<UCosmeticComponent>())
		{
			if (CosmeticComp->GetCosmeticActorInstance())
			{
				return CosmeticComp->GetCosmeticActorInstance()->GetMergeTargetSkeletalMesh(ComponentTag);			
			}
		}
	}

	return nullptr;
}

USkeletalMesh* UCosmeticBFL::GetCosmeticMegedSkeletalMesh(const ACharacter* Target, FName ComponentTag)
{	
	USkeletalMeshComponent* MeshComp = GetCosmeticMegedSkeletalMeshComponent(Target, ComponentTag);
	if (MeshComp)
	{
		return MeshComp->GetSkeletalMeshAsset();
	}

	return nullptr;
}

UCosmeticComponent* UCosmeticBFL::GetCosmeticComponent(const ACharacter* Target)
{
	if (Target)
	{
		return Target->FindComponentByClass<UCosmeticComponent>();		
	}

	return nullptr;
}

int32 UCosmeticBFL::Conv_CosmeticItemIDToInt(FCosmeticItemID CosmeticItemID)
{
	return CosmeticItemID.Identifier;
}

FString UCosmeticBFL::Conv_CosmeticItemIDToString(FCosmeticItemID CosmeticItemID)
{
	return FString::Printf(TEXT("%02d %02d %02d %02d"), CosmeticItemID.GetPartA() / 1000000, 
		CosmeticItemID.GetPartB() / 10000, CosmeticItemID.GetPartC() / 100, CosmeticItemID.GetPartD());
}

bool UCosmeticBFL::Equal_CosmeticItemID(const FCosmeticItemID& A, const FCosmeticItemID& B)
{
	return A == B;
}

bool UCosmeticBFL::NotEqual_CosmeticItemID(const FCosmeticItemID& A, const FCosmeticItemID& B)
{
	return A != B;
}
