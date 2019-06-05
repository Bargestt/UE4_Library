// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SurfaceNavBuilder.h"
#include "EdgeFinder.h"



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
	//GENERATED_BODY()

	TArray<int32> Path;

	int32 From;
	int32 To;	

	bool IsPartial() const { return Path.Num() > 0 && !IsSuccess(); }
	bool IsSuccess() const { return Path.Last() == To; }
};





/**
 * Graph with connected edges
 * By default edge search is very slow, build EdgeFinder to speed it up
 */
class LIBRARY_API FSurfaceNavLocalData
{
	TArray<FEdgeData> Graph;

	FBox GraphBox;

	TUniquePtr<FEdgeFinder> EdgeFinder;

	   
	FTransform Origin;

	FIntVector BuildGridDimensions;

	float BuildGridCellSize;
	
public:
	FSurfaceNavLocalData() {
		InitFinder();
	};
	~FSurfaceNavLocalData() {};

	const TArray<FEdgeData>& GetGraph() const { return Graph; }

	const FEdgeData& GetEdge(int32 Index) const { return Graph[Index]; }	

	FSurfacePathfindResult FindPath(int32 FromNode, int32 ToNode) const;
	
	TArray<FVector> FindPath(const FVector& FromLocation,const FVector& ToLocation) const;


	//////////////////////////////////////////////////////////////////////////
	// FGraphAStar: TGraph
	typedef int32 FNodeRef;

	int32 GetNeighbourCount(FNodeRef NodeRef) const { return GetEdge(NodeRef).ConnectedEdges.Num(); }
	bool IsValidRef(FNodeRef NodeRef) const { return Graph.IsValidIndex(NodeRef); }
	FNodeRef GetNeighbour(const FNodeRef NodeRef, const int32 NeiIndex) const { return GetEdge(NodeRef).ConnectedEdges[NeiIndex]; }	
	//////////////////////////////////////////////////////////////////////////


	bool IsNodeTraversable(int32 Index) const { return Graph.IsValidIndex(Index) && GetEdge(Index).IsInitialized(); }
	
	int32 FindClosestEdgeIndex(const FVector& Location) const;

	FVector ToLocation(int32 EdgeIndex) const;

	bool IsInside(const FVector& Location) const;
	

	FIntVector GetBuildGridDimensions() const { return BuildGridDimensions; }

	float GetBuildGridCellSize() const { return BuildGridCellSize; }

protected:
	void InitFinder();

	void GraphChanged();

private:

	friend class FSurfaceNavBuilder;
	void SetGraph(TArray<FEdgeData> NewGraph, FIntVector NewDimensions)
	{
		Graph = MoveTemp(NewGraph);
		BuildGridDimensions = NewDimensions;
		GraphChanged();
	}

};
