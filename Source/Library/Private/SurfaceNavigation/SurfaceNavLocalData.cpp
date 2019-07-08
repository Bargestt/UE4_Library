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
			return (NavDataRef.GetEdgeData(StartNodeRef).EdgeVertex - NavDataRef.GetEdgeData(EndNodeRef).EdgeVertex).Size();
		}

		bool IsTraversalAllowed(const int32 NodeA, const int32 NodeB) const
		{
			return NavDataRef.GetEdgeData(NodeA).IsInitialized() && NavDataRef.GetEdgeData(NodeB).IsInitialized();
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

	// Failsafe method, use edge finder for better performance	
	float CurDist, ClosestDist = FLT_MAX;
	for (int Index = 0; Index < Graph.Num(); Index++)
	{
		CurDist = (Graph[Index].EdgeVertex - Location).SizeSquared();
		if (CurDist < ClosestDist)
		{
			ClosestDist = CurDist;
			ClosestIndex = Index;
		}
	}
	return ClosestIndex;
}

FVector FSurfaceNavLocalData::ToLocation(int32 EdgeIndex) const
{
	return Graph.IsValidIndex(EdgeIndex) ? Graph[EdgeIndex].EdgeVertex : FSurfaceNavigation::InvalidLocation;
}


TArray<FVector> FSurfaceNavLocalData::ToLocations(const TArray<int32>& EdgeIndices, FVector Center /*= FVector::ZeroVector*/) const
{
	TArray<FVector> Points;
	Points.Reserve(EdgeIndices.Num());
	for (int32 Index : EdgeIndices)
	{
		FVector Position = GetEdgeData(Index).EdgeVertex + Center;
		Points.Add(Position);
	}
	return Points;
}



