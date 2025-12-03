// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#include "System/HarmoniaDungeonGenerator.h"
#include "Actors/HarmoniaDungeonInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

FDungeonLayout UHarmoniaDungeonGenerator::GenerateDungeonLayout(const FDungeonGenerationSettings& Settings, int32 Seed)
{
	FDungeonLayout Layout;
	Layout.Seed = Seed == 0 ? FMath::Rand() : Seed;
	Layout.GridSize = Settings.DungeonSize;
	Layout.TileSize = 2000.0f;

	FRandomStream RandomStream(Layout.Seed);

	// 1. �??�성
	Layout.Rooms = GenerateRooms(Settings, RandomStream);

	// 2. �??�???�당
	AssignRoomTypes(Layout.Rooms, Settings, RandomStream);

	// 3. 복도 ?�성
	Layout.Corridors = GenerateCorridors(Layout.Rooms, Settings, RandomStream);

	// 4. ?�이??계산
	CalculateRoomDifficulty(Layout.Rooms);

	// 5. 몬스????계산
	CalculateMonsterCounts(Layout.Rooms, 1);

	return Layout;
}

FDungeonLayout UHarmoniaDungeonGenerator::GenerateInfiniteDungeonFloor(int32 FloorNumber, const FInfiniteDungeonFloor& FloorInfo)
{
	FDungeonGenerationSettings Settings;
	Settings.DungeonSize = FIntPoint(20 + FloorNumber * 2, 20 + FloorNumber * 2);
	Settings.MinRoomCount = 5 + FloorNumber;
	Settings.MaxRoomCount = 15 + FloorNumber * 2;
	Settings.BossRoomChance = FloorInfo.bHasBoss ? 1.0f : 0.0f;

	FDungeonLayout Layout = GenerateDungeonLayout(Settings, FloorNumber);

	// �?번호???�른 ?�이??�?몬스????증�?
	CalculateMonsterCounts(Layout.Rooms, FloorNumber);

	return Layout;
}

AHarmoniaDungeonInstance* UHarmoniaDungeonGenerator::SpawnDungeonFromLayout(UObject* WorldContextObject, const FDungeonLayout& Layout, const UDungeonDataAsset* DungeonData)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// ?�전 ?�스?�스 ?�터 ?�성
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHarmoniaDungeonInstance* DungeonInstance = World->SpawnActor<AHarmoniaDungeonInstance>(
		AHarmoniaDungeonInstance::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!DungeonInstance)
	{
		return nullptr;
	}

	// ?�전 ?�이???�정
	DungeonInstance->DungeonData = DungeonData;

	// ?�폰 ?�인???�성 (?�이?�웃 기반)
	for (const FDungeonRoom& Room : Layout.Rooms)
	{
		// �??�?�에 ?�라 몬스???�폰 ?�인??추�?
		if (Room.RoomType == EDungeonRoomType::Combat || Room.RoomType == EDungeonRoomType::Boss)
		{
			for (int32 i = 0; i < Room.MonsterCount; i++)
			{
				FDungeonSpawnPoint SpawnPoint;
				SpawnPoint.SpawnTransform = FTransform(Room.WorldPosition + FVector(FMath::FRandRange(-500.0f, 500.0f), FMath::FRandRange(-500.0f, 500.0f), 0.0f));
				SpawnPoint.WaveNumber = 1;
				SpawnPoint.bIsActive = true;

				DungeonInstance->SpawnPoints.Add(SpawnPoint);
			}
		}

		// ?�작 방�? ?�장 지??
		if (Room.RoomType == EDungeonRoomType::Start)
		{
			DungeonInstance->EntranceTransform = FTransform(Room.WorldPosition);
		}

		// 출구 방�? ?�장 지??
		if (Room.RoomType == EDungeonRoomType::Exit)
		{
			DungeonInstance->ExitTransform = FTransform(Room.WorldPosition);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Dungeon spawned from layout: %d rooms, %d corridors"), Layout.Rooms.Num(), Layout.Corridors.Num());

	return DungeonInstance;
}

bool UHarmoniaDungeonGenerator::ValidateDungeonLayout(const FDungeonLayout& Layout)
{
	// 1. 최소 �?개수 ?�인
	if (Layout.Rooms.Num() < 3)
	{
		return false;
	}

	// 2. ?�작 방과 출구 �??�인
	bool bHasStart = false;
	bool bHasExit = false;

	for (const FDungeonRoom& Room : Layout.Rooms)
	{
		if (Room.RoomType == EDungeonRoomType::Start)
		{
			bHasStart = true;
		}
		if (Room.RoomType == EDungeonRoomType::Exit)
		{
			bHasExit = true;
		}
	}

	if (!bHasStart || !bHasExit)
	{
		return false;
	}

	// 3. 모든 방이 ?�결?�어 ?�는지 ?�인
	if (!AreAllRoomsConnected(Layout.Rooms))
	{
		return false;
	}

	return true;
}

TArray<FDungeonRoom> UHarmoniaDungeonGenerator::GenerateRooms(const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream)
{
	TArray<FDungeonRoom> Rooms;

	int32 RoomCount = RandomStream.RandRange(Settings.MinRoomCount, Settings.MaxRoomCount);
	int32 Attempts = 0;
	const int32 MaxAttempts = 1000;

	while (Rooms.Num() < RoomCount && Attempts < MaxAttempts)
	{
		Attempts++;

		FDungeonRoom NewRoom;

		// ?�덤 �??�기
		int32 RoomWidth = RandomStream.RandRange(Settings.MinRoomSize.X, Settings.MaxRoomSize.X);
		int32 RoomHeight = RandomStream.RandRange(Settings.MinRoomSize.Y, Settings.MaxRoomSize.Y);

		// ?�덤 ?�치
		NewRoom.GridPosition = FIntPoint(
			RandomStream.RandRange(1, Settings.DungeonSize.X - RoomWidth - 1),
			RandomStream.RandRange(1, Settings.DungeonSize.Y - RoomHeight - 1)
		);

		NewRoom.RoomSize = FVector(RoomWidth * 2000.0f, RoomHeight * 2000.0f, 400.0f);
		NewRoom.WorldPosition = FVector(
			NewRoom.GridPosition.X * 2000.0f,
			NewRoom.GridPosition.Y * 2000.0f,
			0.0f
		);

		// 겹침 체크
		if (!IsRoomOverlapping(NewRoom, Rooms))
		{
			Rooms.Add(NewRoom);
		}
	}

	return Rooms;
}

TArray<FDungeonCorridor> UHarmoniaDungeonGenerator::GenerateCorridors(const TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream)
{
	TArray<FDungeonCorridor> Corridors;

	if (Rooms.Num() < 2)
	{
		return Corridors;
	}

	// 최소 ?�패???�리�?모든 �??�결
	TArray<bool> Connected;
	Connected.SetNumZeroed(Rooms.Num());
	Connected[0] = true;

	for (int32 i = 1; i < Rooms.Num(); i++)
	{
		// 가??가까운 ?�결??�?찾기
		float MinDistance = MAX_FLT;
		int32 ClosestConnected = -1;

		for (int32 j = 0; j < i; j++)
		{
			if (Connected[j])
			{
				float Distance = FVector::Dist(Rooms[i].WorldPosition, Rooms[j].WorldPosition);
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					ClosestConnected = j;
				}
			}
		}

		if (ClosestConnected != -1)
		{
			// 복도 ?�성
			FDungeonCorridor Corridor;
			Corridor.StartRoom = Rooms[ClosestConnected].GridPosition;
			Corridor.EndRoom = Rooms[i].GridPosition;

			// 간단??L??복도 (?�평 -> ?�직 ?�는 ?�직 -> ?�평)
			FVector Start = Rooms[ClosestConnected].WorldPosition;
			FVector End = Rooms[i].WorldPosition;

			if (RandomStream.FRand() > 0.5f)
			{
				// ?�평 먼�?
				Corridor.PathPoints.Add(Start);
				Corridor.PathPoints.Add(FVector(End.X, Start.Y, Start.Z));
				Corridor.PathPoints.Add(End);
			}
			else
			{
				// ?�직 먼�?
				Corridor.PathPoints.Add(Start);
				Corridor.PathPoints.Add(FVector(Start.X, End.Y, Start.Z));
				Corridor.PathPoints.Add(End);
			}

			Corridors.Add(Corridor);
			Connected[i] = true;
		}
	}

	// 추�? ?�결 (루프 ?�성)
	if (Settings.bAllowLoops)
	{
		int32 ExtraConnections = Rooms.Num() / 5;
		for (int32 i = 0; i < ExtraConnections; i++)
		{
			int32 Room1 = RandomStream.RandRange(0, Rooms.Num() - 1);
			int32 Room2 = RandomStream.RandRange(0, Rooms.Num() - 1);

			if (Room1 != Room2)
			{
				FDungeonCorridor Corridor;
				Corridor.StartRoom = Rooms[Room1].GridPosition;
				Corridor.EndRoom = Rooms[Room2].GridPosition;

				FVector Start = Rooms[Room1].WorldPosition;
				FVector End = Rooms[Room2].WorldPosition;

				Corridor.PathPoints.Add(Start);
				Corridor.PathPoints.Add(FVector(End.X, Start.Y, Start.Z));
				Corridor.PathPoints.Add(End);

				Corridors.Add(Corridor);
			}
		}
	}

	return Corridors;
}

void UHarmoniaDungeonGenerator::AssignRoomTypes(TArray<FDungeonRoom>& Rooms, const FDungeonGenerationSettings& Settings, FRandomStream& RandomStream)
{
	if (Rooms.Num() == 0)
	{
		return;
	}

	// �?번째 방�? ?�작 �?
	Rooms[0].RoomType = EDungeonRoomType::Start;

	// 마�?�?방�? 출구??보스 �?
	if (RandomStream.FRand() < Settings.BossRoomChance)
	{
		Rooms[Rooms.Num() - 1].RoomType = EDungeonRoomType::Boss;
	}
	else
	{
		Rooms[Rooms.Num() - 1].RoomType = EDungeonRoomType::Exit;
	}

	// ?�머지 방들
	for (int32 i = 1; i < Rooms.Num() - 1; i++)
	{
		float Rand = RandomStream.FRand();

		if (Rand < Settings.SecretRoomChance)
		{
			Rooms[i].RoomType = EDungeonRoomType::Secret;
		}
		else if (Rand < Settings.SecretRoomChance + Settings.TreasureRoomChance)
		{
			Rooms[i].RoomType = EDungeonRoomType::Treasure;
		}
		else if (Rand < 0.7f)
		{
			Rooms[i].RoomType = EDungeonRoomType::Combat;
		}
		else
		{
			Rooms[i].RoomType = EDungeonRoomType::Normal;
		}
	}
}

bool UHarmoniaDungeonGenerator::IsRoomOverlapping(const FDungeonRoom& Room, const TArray<FDungeonRoom>& ExistingRooms) const
{
	for (const FDungeonRoom& ExistingRoom : ExistingRooms)
	{
		// AABB 충돌 검??
		FVector2D Room1Min(Room.GridPosition.X, Room.GridPosition.Y);
		FVector2D Room1Max = Room1Min + FVector2D(Room.RoomSize.X / 2000.0f, Room.RoomSize.Y / 2000.0f);

		FVector2D Room2Min(ExistingRoom.GridPosition.X, ExistingRoom.GridPosition.Y);
		FVector2D Room2Max = Room2Min + FVector2D(ExistingRoom.RoomSize.X / 2000.0f, ExistingRoom.RoomSize.Y / 2000.0f);

		if (Room1Min.X < Room2Max.X && Room1Max.X > Room2Min.X &&
			Room1Min.Y < Room2Max.Y && Room1Max.Y > Room2Min.Y)
		{
			return true;
		}
	}

	return false;
}

TArray<FIntPoint> UHarmoniaDungeonGenerator::FindPath(FIntPoint Start, FIntPoint End, const FDungeonGenerationSettings& Settings) const
{
	// 간단??A* 구현 (?�후 개선 가??
	TArray<FIntPoint> Path;
	
	FIntPoint Current = Start;
	Path.Add(Current);

	// 맨하??거리�?간단??구현
	while (Current != End)
	{
		if (Current.X < End.X)
		{
			Current.X++;
		}
		else if (Current.X > End.X)
		{
			Current.X--;
		}
		else if (Current.Y < End.Y)
		{
			Current.Y++;
		}
		else if (Current.Y > End.Y)
		{
			Current.Y--;
		}

		Path.Add(Current);
	}

	return Path;
}

bool UHarmoniaDungeonGenerator::AreAllRoomsConnected(const TArray<FDungeonRoom>& Rooms) const
{
	if (Rooms.Num() == 0)
	{
		return false;
	}

	// BFS�??�결???�인
	TSet<FIntPoint> Visited;
	TArray<FIntPoint> Queue;

	Queue.Add(Rooms[0].GridPosition);
	Visited.Add(Rooms[0].GridPosition);

	while (Queue.Num() > 0)
	{
		FIntPoint Current = Queue[0];
		Queue.RemoveAt(0);

		// ?�재 방에 ?�결??모든 �??�색
		for (const FDungeonRoom& Room : Rooms)
		{
			if (Room.ConnectedRooms.Contains(Current) && !Visited.Contains(Room.GridPosition))
			{
				Visited.Add(Room.GridPosition);
				Queue.Add(Room.GridPosition);
			}
		}
	}

	return Visited.Num() == Rooms.Num();
}

void UHarmoniaDungeonGenerator::CalculateRoomDifficulty(TArray<FDungeonRoom>& Rooms)
{
	if (Rooms.Num() == 0)
	{
		return;
	}

	// ?�작 방으로�??�의 거리 기반 ?�이??
	FVector StartPos = Rooms[0].WorldPosition;

	for (FDungeonRoom& Room : Rooms)
	{
		float Distance = FVector::Dist(Room.WorldPosition, StartPos);
		Room.Difficulty = FMath::Clamp(FMath::RoundToInt(Distance / 5000.0f), 1, 10);
	}
}

void UHarmoniaDungeonGenerator::CalculateMonsterCounts(TArray<FDungeonRoom>& Rooms, int32 FloorNumber)
{
	for (FDungeonRoom& Room : Rooms)
	{
		switch (Room.RoomType)
		{
		case EDungeonRoomType::Combat:
			Room.MonsterCount = FMath::RandRange(3, 8) * FloorNumber;
			break;

		case EDungeonRoomType::Boss:
			Room.MonsterCount = 1; // 보스 1마리
			break;

		case EDungeonRoomType::Treasure:
			Room.MonsterCount = FMath::RandRange(1, 3) * FloorNumber;
			break;

		case EDungeonRoomType::Secret:
			Room.MonsterCount = FMath::RandRange(5, 10) * FloorNumber;
			break;

		default:
			Room.MonsterCount = 0;
			break;
		}
	}
}
