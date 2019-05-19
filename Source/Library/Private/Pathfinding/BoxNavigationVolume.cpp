// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxNavigationVolume.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "GraphAStar.h"


const FIntVector FBoxNavData::Directions[] = {
		{1,0,1}, {1,1,1}, {1,-1,1},
		{-1,0,1},{-1,1,1},{-1,1,1},
		{-1,0,1},{1,0,1}, {0,0,1},

		{1,0,0}, {1,1,0}, {1,-1,0},
		{-1,0,0},{-1,1,0},{-1,1,0},
		{-1,0,0},{1,0,0},

		{1,0,-1}, {1,1,-1}, {1,-1,-1},
		{-1,0,-1},{-1,1,-1},{-1,1,-1},
		{-1,0,-1},{1,0,-1}, {0,0,-1},
};


// Sets default values
ABoxNavigationVolume::ABoxNavigationVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;

	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetCanEverAffectNavigation(false);
	Box->SetBoxExtent(FVector(250));

	CellSize = 50;

	FromCell = FIntVector(0, 2, 2);
	ToCell = FIntVector(9, 9, 2);

	bCellsCreated = false;
	bUpdateCells = false;
	bShowCells = false;
}

void ABoxNavigationVolume::CreateCells()
{
	UWorld* World = GetWorld();

	FVector HalfExtent = Box->GetScaledBoxExtent();
	FVector CellHalfExtent = FVector(CellSize / 2);

	FVector Offset = -HalfExtent + CellHalfExtent;
	FTransform Origin = GetActorTransform();

	FCollisionShape Shape = FCollisionShape::MakeBox(CellHalfExtent);
	

	Dimensions = FIntVector( HalfExtent / CellHalfExtent);
	NavData.Dimensions = Dimensions;

	if (bShowCells) 
	{
		FlushPersistentDebugLines(World);
	}

	NavData.NavCells.SetNum(Dimensions.X);
	for (int X = 0; X < Dimensions.X; X++)
	{
		NavData.NavCells[X].SetNum(Dimensions.Y);
		for (int Y = 0; Y < Dimensions.Y; Y++)
		{
			NavData.NavCells[X][Y].SetNum(Dimensions.Z);
			for (int Z = 0; Z < Dimensions.Z; Z++)
			{
				FVector PositionLocal = FVector(X, Y, Z) * CellSize + Offset;
				FVector PostionWorld = Origin.TransformPosition(PositionLocal);

				bool WasOverlap = World->OverlapAnyTestByChannel(PostionWorld, Origin.GetRotation(), ECC_Visibility, Shape);				
				NavData.NavCells[X][Y][Z] = { PositionLocal, !WasOverlap };

				if (bShowCells)
				{
					DrawDebugBox(World, PostionWorld, CellHalfExtent, Origin.GetRotation(), WasOverlap ? FColor::Red : FColor::Green, false, 5);
				}
			}
		}
	}

	bCellsCreated = true;
}

void ABoxNavigationVolume::ShowPath()
{
	if (bUpdateCells)
	{
		CreateCells();
	}

	UWorld* World = GetWorld();

	FVector HalfExtent = Box->GetScaledBoxExtent();
	FVector CellHalfExtent = FVector(CellSize / 2);

	FVector Offset = -HalfExtent + CellHalfExtent;
	FTransform Origin = GetActorTransform();


	DrawDebugBox(World, Origin.TransformPosition(FVector(FromCell) * CellSize + Offset), CellHalfExtent, Origin.GetRotation(), FColor::Blue, false, 5);
	DrawDebugBox(World, Origin.TransformPosition(FVector(ToCell) * CellSize + Offset), CellHalfExtent, Origin.GetRotation(), FColor::Blue, false, 5);

	TArray<FIntVector> Path;

	FindPath(FromCell, ToCell, Path);

	if (Path.Num() > 1)
	{
		for (int Index = 0; Index < Path.Num() - 1 ; Index++)
		{
			FVector From = Origin.TransformPosition(FVector(Path[Index]) * CellSize + Offset);
			FVector To = Origin.TransformPosition(FVector(Path[Index+1]) * CellSize + Offset);
			DrawDebugLine(World, From, To, FColor::Cyan, false, 5, 0, 3);
		}
	}

}



namespace FBoxNavHelpers
{
	struct FBoxNavFilter
	{
		FBoxNavFilter(const FBoxNavData& NavData) : NavDataRef(NavData) {}

		float GetHeuristicScale() const
		{
			return 1.0f;
		}

		float GetHeuristicCost(const FIntVector StartNodeRef, const FIntVector EndNodeRef) const
		{
			return GetTraversalCost(StartNodeRef, EndNodeRef);
		}

		float GetTraversalCost(const FIntVector StartNodeRef, const FIntVector EndNodeRef) const
		{
			return FMath::Abs(StartNodeRef.X - EndNodeRef.X) + FMath::Abs(StartNodeRef.Y - EndNodeRef.Y) + FMath::Abs(StartNodeRef.Z - EndNodeRef.Z);
		}

		bool IsTraversalAllowed(const FIntVector NodeA, const FIntVector NodeB) const
		{
			return NavDataRef.GetCell(NodeA).bTraversable && NavDataRef.GetCell(NodeB).bTraversable;
		}

		bool WantsPartialSolution() const
		{
			return true;
		}

	protected:
		const FBoxNavData& NavDataRef;

	};
}

bool ABoxNavigationVolume::FindPath(const FIntVector& From, const FIntVector& To, TArray<FIntVector>& OutPath)
{
	FGraphAStar<FBoxNavData> Pathfinder(NavData);
	FBoxNavHelpers::FBoxNavFilter Filter(NavData);

	EGraphAStarResult Result = Pathfinder.FindPath(From, To, Filter, OutPath);
	return Result == SearchSuccess;
}




