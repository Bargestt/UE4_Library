// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurfaceNavBuilder.h"





struct FEdgeData
{
	TArray<int> ConnectedEdges;

	FVector EdgeVertex;

	FEdgeData()
	{
		ConnectedEdges = TArray<int>();
		EdgeVertex = FSurfaceNavigation::InvalidLocation;
	}

	bool IsInitialized() const
	{
		return FSurfaceNavigation::IsValidLocation(EdgeVertex);
	}
};


struct FSurfacePathfindResult
{
	TArray<int32> Path;

	int32 From;
	int32 To;	

	bool IsPartial() const { return Path.Num() > 0 && !IsSuccess(); }
	bool IsSuccess() const { return Path.Last() == To; }
};

//
//TODO: make spatial partitioning and rid of unused edges
//

/**
 * 
 */
class LIBRARY_API FSurfaceNavLocalData
{
	friend class FSurfaceNavBuilder;

	TArray<FEdgeData> Graph;

	FIntVector GridSize;
	
public:
	FSurfaceNavLocalData() {}
	~FSurfaceNavLocalData() {}

	const TArray<FEdgeData>& GetGraph()const { return Graph; }



	const FEdgeData& GetEdge(int32 Index) const { return Graph[Index]; }	




	FSurfacePathfindResult FindPath(int32 FromNode, int32 ToNode) const;
	



	//////////////////////////////////////////////////////////////////////////
	// FGraphAStar: TGraph
	typedef int32 FNodeRef;

	int32 GetNeighbourCount(FNodeRef NodeRef) const { return GetEdge(NodeRef).ConnectedEdges.Num(); }
	bool IsValidRef(FNodeRef NodeRef) const { return Graph.IsValidIndex(NodeRef); }
	FNodeRef GetNeighbour(const FNodeRef NodeRef, const int32 NeiIndex) const { return GetEdge(NodeRef).ConnectedEdges[NeiIndex]; }
	//////////////////////////////////////////////////////////////////////////

	bool IsNodeTraversable(int32 Index) const { return Graph.IsValidIndex(Index) && GetEdge(Index).IsInitialized(); }


	
	int32 FindClosestEdge(const FVector& WorldLocation);

};
