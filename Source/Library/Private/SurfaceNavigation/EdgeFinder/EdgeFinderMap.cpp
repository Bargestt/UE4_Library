// Fill out your copyright notice in the Description page of Project Settings.

#include "EdgeFinderMap.h"
#include "SurfaceNavLocalData.h"


const float FEdgeFinderMap::DefaultCellSize = 100;

const FIntVector FEdgeFinderMap::ExpansionDirection[] =
{
	FIntVector(1, 0, 0),
	FIntVector(-1, 0, 0),
	FIntVector(0, 1, 0),
	FIntVector(0,-1, 0),
	FIntVector(0, 0, 1),
	FIntVector(0, 0,-1)
};

FEdgeFinderMap::FEdgeFinderMap(const FSurfaceNavLocalData& NavData) : FEdgeFinder(NavData)
{
	bIsBuilt = false;
	MapCellSize = DefaultCellSize;
}

int32 FEdgeFinderMap::FindEdgeIndex(const FVector& Location) const
{
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, FString::FromInt(EdgeMap.Num()));
	return FindEdgeInMap(Location, false);
}

bool FEdgeFinderMap::IsReady() const
{
	return bIsBuilt;
}

bool FEdgeFinderMap::Rebuild()
{
	MapCellSize = 50; // NavData.GetBuildGridCellSize();

	if (MapCellSize <= 0)
	{
		MapCellSize = DefaultCellSize;
	}
	
	const TArray<FEdgeData>& Edges = NavData.GetGraph();

	for (int Index = 0; Index < Edges.Num(); Index++)
	{
		const FEdgeData& Edge = Edges[Index];

		int32 GridIndex = ToIndex(Edge.EdgeVertex);

		if (EdgeRefs* Find = EdgeMap.Find(GridIndex))
		{
			Find->Add(Index);
		}
		else
		{
			EdgeMap.Add(GridIndex, { Index });
		}
	}

	bIsBuilt = true;
	return true;
}


int32 FEdgeFinderMap::ToIndex(const FVector& Location) const
{
	FVector Loc = Location;
	FIntVector GridLoc = FIntVector(Loc.GridSnap(MapCellSize));
	return GridLoc.X + GridLoc.Y*Dimensions.X + GridLoc.Z*Dimensions.Y*Dimensions.Z;
}

int32 FEdgeFinderMap::FindEdgeInMap(const FVector& Location, bool SearchNeighbours /*= false*/) const
{
	int32 Index = ToIndex(Location);

	const EdgeRefs* refs = EdgeMap.Find(Index);
	if (refs != nullptr && refs->Num() > 0)
	{
		return FindClosest(Location, *refs);
	}
	else if(SearchNeighbours)
	{
		//Select nearest neighbors till found edge 
		

	}
	
	return -1;
}

int32 FEdgeFinderMap::FindClosest(const FVector& Location, const TArray<int32>& EdgeIndices) const
{
	int32 ClosestIndex = -1;
	const TArray<FEdgeData>& Edges = NavData.GetGraph();

	float CurDist, ClosestDist = FLT_MAX;
	for (int Index = 0; Index < Edges.Num(); Index++)
	{
		CurDist = (Edges[Index].EdgeVertex - Location).SizeSquared();
		if (CurDist < ClosestDist)
		{
			ClosestDist = CurDist;
			ClosestIndex = Index;
		}
	}
	return ClosestIndex;
}

TArray<int32> FEdgeFinderMap::GetNeighboursCoords(const FIntVector& CenterCoords, const FIntVector& Direction, int32 Distance) const
{
	if (Distance <= 1) return { };

	TArray<int32> FoundNeighbours;

	for (int Offset = 1; Offset < Distance ; Offset++)
	{
		
	}

	return FoundNeighbours;
}



