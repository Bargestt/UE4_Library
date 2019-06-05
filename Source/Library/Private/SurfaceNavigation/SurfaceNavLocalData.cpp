// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavLocalData.h"
#include "GraphAStar.h"

#include "EdgeFinder.h"
#include "EdgeFinderMap.h"

#include "DrawDebugHelpers.h"








namespace FSurfaceNavigation
{
	struct FSurfaceNavFilter
	{
		FSurfaceNavFilter(const FSurfaceNavLocalData& NavData) : NavDataRef(NavData) {}

		float GetHeuristicScale() const
		{
			return 1.0f;
		}

		float GetHeuristicCost(const int32 StartNodeRef, const int32 EndNodeRef) const
		{
			return GetTraversalCost(StartNodeRef, EndNodeRef);
		}

		float GetTraversalCost(const int32 StartNodeRef, const int32 EndNodeRef) const
		{
			return (NavDataRef.GetEdge(StartNodeRef).EdgeVertex - NavDataRef.GetEdge(EndNodeRef).EdgeVertex).Size();
		}

		bool IsTraversalAllowed(const int32 NodeA, const int32 NodeB) const
		{
			return NavDataRef.GetEdge(NodeA).IsInitialized() && NavDataRef.GetEdge(NodeB).IsInitialized();
		}

		bool WantsPartialSolution() const
		{
			return true;
		}

	protected:
		const FSurfaceNavLocalData& NavDataRef;

	};
}

FSurfacePathfindResult FSurfaceNavLocalData::FindPath(int32 FromNode, int32 ToNode) const
{
	FSurfacePathfindResult Result;

	if (IsNodeTraversable(FromNode) && IsValidRef(ToNode))
	{
		FGraphAStar<FSurfaceNavLocalData> Pathfinder(*this);

		TArray<int32> Path;
		EGraphAStarResult AStartResult = Pathfinder.FindPath(FromNode, ToNode, FSurfaceNavigation::FSurfaceNavFilter(*this), Path);


		Result.Path = Path;
		Result.From = FromNode;
		Result.To = ToNode;
	}

	return Result;
}

TArray<FVector> FSurfaceNavLocalData::FindPath(const FVector& FromLocation, const FVector& ToLocation) const
{
	int32 StartNode = FindClosestEdgeIndex(FromLocation);
	int32 EndNode = FindClosestEdgeIndex(ToLocation);

	FSurfacePathfindResult Result = FindPath(StartNode, EndNode);
	TArray<FVector> Path = {FromLocation};
	for (int32 index : Result.Path)
	{
		Path.Add(Graph[index].EdgeVertex);
	}
	
	return Path;
}

DECLARE_CYCLE_STAT(TEXT("SurfaceNavigation ~ GetCellIndex"), STAT_GetCellIndex, STATGROUP_SurfaceNavigation);

int32 FSurfaceNavLocalData::FindClosestEdgeIndex(const FVector& Location) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetCellIndex);

	int32 ClosestIndex = -1;	

	if (EdgeFinder.IsValid() && EdgeFinder->IsReady())
	{
		ClosestIndex = EdgeFinder->FindEdgeIndex(Location);
	}
	else
	{	// Failsafe method, mostly for comparing performance		
		float CurDist, ClosestDist = FLT_MAX;
		for (int Index = 0; Index < Graph.Num() ; Index++)
		{
			CurDist = (Graph[Index].EdgeVertex - Location).SizeSquared();
			if (CurDist < ClosestDist)
			{
				ClosestDist = CurDist;
				ClosestIndex = Index;
			}			
		}	
	}
	GEngine->AddOnScreenDebugMessage(1, 2, FColor::Cyan, (EdgeFinder.IsValid() && EdgeFinder->IsReady())? TEXT("Finder") : TEXT("Dumb"));	
	return ClosestIndex;
}


FVector FSurfaceNavLocalData::ToLocation(int32 EdgeIndex) const
{
	return Graph.IsValidIndex(EdgeIndex) ? Graph[EdgeIndex].EdgeVertex : FSurfaceNavigation::InvalidLocation;
}

bool FSurfaceNavLocalData::IsInside(const FVector& Location) const
{
	FVector Extent = FVector(BuildGridDimensions) * BuildGridCellSize;
	return Extent.X > Location.X && Extent.Y > Location.Y && Extent.Z > Location.Z;
}

void FSurfaceNavLocalData::InitFinder()
{
	EdgeFinder = TUniquePtr<FEdgeFinderMap>(new FEdgeFinderMap(*this));
}

void FSurfaceNavLocalData::GraphChanged()
{
	InitFinder();
	if (EdgeFinder != nullptr)
	{
		EdgeFinder->Rebuild();
	}
}

