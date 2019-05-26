// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavLocalData.h"
#include "GraphAStar.h"


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

int32 FSurfaceNavLocalData::FindClosestEdge(const FVector& WorldLocation)
{
// 	FVector LocalPosition;
// 	FIntVector Coordinates = ToCellCoordinates(LocalPosition, false);
// 
// 	if (!IsCoordinatesValid(Coordinates))
// 	{
// 		return -1;
// 	}
// 
// 	int StorageOffset = Dimensions.X*Dimensions.Y*Dimensions.Z;
// 
// 	TArray<int32> ClosestEdges;
// 	for (int Index = 0; Index < 12; Index++)
// 	{
// 		int32 EdgeIndex = GetEdgeIndexGlobal(Coordinates, Index, StorageOffset);
// 		const FEdgeData& Edge = NavData.Edges[EdgeIndex];
// 
// 		if (Edge.ConnectedEdges.Num() > 0)
// 		{
// 			ClosestEdges.Add(EdgeIndex);
// 		}
// 	}
// 
// 	if (ClosestEdges.Num() <= 0) return -1; // No edge found
// 
// 
// 
// 	int32 ClosestEdgeIndex = ClosestEdges[0];
// 	float ClosestDist = FLT_MAX;
// 	for (int32 EdgeIndex : ClosestEdges)
// 	{
// 		const FEdgeData& Edge = NavData.Edges[EdgeIndex];
// 
// 		float Dist = (Edge.EdgeVertex - LocalPosition).SizeSquared();
// 		if (ClosestDist > Dist)
// 		{
// 			ClosestDist = Dist;
// 			ClosestEdgeIndex = EdgeIndex;
// 		}
// 	}
// 
// 	//OutEdge = NavData.Edges[ClosestEdgeIndex];
// 	return ClosestEdgeIndex;

	return -1;
}
