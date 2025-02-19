// Copyright 2024 HGsofts, All Rights Reserved.

#include "CosmeticMeshStorage.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "CosmeticSystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CosmeticMeshStorage)

UCosmeticMeshStorage* UCosmeticMeshStorage::Get(const UWorld* InWorld)
{
	if (InWorld)
	{
		return InWorld->GetSubsystem<UCosmeticMeshStorage>();
	}

	return nullptr;
}

USkeletalMesh* UCosmeticMeshStorage::GetMergedMesh(const TSet<FCosmeticItemID>& MeshPartIDs)
{
	for (FMergedMeshHandle& Elem : MergedMeshes)
	{
		if (Equal(MeshPartIDs, Elem.MeshPartIDs))
		{
			return Elem.MergedMesh;
		}		
	}
	return nullptr;
}

bool UCosmeticMeshStorage::Equal(const TSet<FCosmeticItemID>& lhs, const TSet<FCosmeticItemID>& rhs)
{
	if (lhs.Num() == rhs.Num())
	{		
		for (auto It = lhs.CreateConstIterator(); It; ++It)
		{
			if (!rhs.Contains(*It))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void UCosmeticMeshStorage::RegisterMergedMesh(FMergedMeshHandle& Data)
{
	//skip already have value 
	MergedMeshes.Add(Data);
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("RegisterMergedMesh %d"), MergedMeshes.Num()), true, true, FLinearColor::Red, 1.0f);
}

void UCosmeticMeshStorage::Clear()
{	
	MergedMeshes.Empty(UCosmeticSystemSettings::Get()->CheckClearCount);

	/*
	TArray<UObject*> ReferredToObjects;
	for (int32 Index = 0; Index < MergedMeshes.Num(); Index++)
	{
		ReferredToObjects.Empty(ReferredToObjects.Num());
		FReferenceFinder ObjectReferenceCollector(ReferredToObjects, MergedMeshes[Index].MergedMesh, false, true, true, false);
		ObjectReferenceCollector.FindReferences(MergedMeshes[Index].MergedMesh);

		if (ReferredToObjects.Num() <= 1)
		{
			MergedMeshes.RemoveAtSwap(Index, 1, false);
			Index--;
		}
	}
	MergedMeshes.Shrink();	
	*/
}
