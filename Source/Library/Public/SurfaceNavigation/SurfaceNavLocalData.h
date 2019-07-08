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

	bool IsPartial() const { return Path.Num() > 0 && Path.Last() != To; }
	bool IsSuccess() const { return Path.Num() > 0 && Path.Last() == To; }
};





/**
 * Graph with connected edges
 * By default edge search is very slow, build EdgeFinder to speed it up
 */
class LIBRARY_API FSurfaceNavLocalData
{
	TArray<FEdgeData> Graph;

public:
	FSurfaceNavLocalData() {};
	~FSurfaceNavLocalData() {};	
	

	FSurfacePathfindResult FindPath(int32 FromNode, int32 ToNode) const;
	
	TArray<FVector> FindPath(const FVector& FromLocation,const FVector& ToLocation) const;


	//////////////////////////////////////////////////////////////////////////
	// FGraphAStar: TGraph
	typedef int32 FNodeRef;

	int32 GetNeighbourCount(FNodeRef NodeRef) const { return GetEdgeData(NodeRef).ConnectedEdges.Num(); }
	bool IsValidRef(FNodeRef NodeRef) const { return Graph.IsValidIndex(NodeRef); }
	FNodeRef GetNeighbour(const FNodeRef NodeRef, const int32 NeiIndex) const { return GetEdgeData(NodeRef).ConnectedEdges[NeiIndex]; }	
	//////////////////////////////////////////////////////////////////////////

	bool IsNodeTraversable(int32 Index) const { return Graph.IsValidIndex(Index) && GetEdgeData(Index).IsInitialized(); }	


private:

	friend class FSurfaceNavBuilder;
	void SetGraph(TArray<FEdgeData> NewGraph, FIntVector NewDimensions)
	{
		Graph = MoveTemp(NewGraph);
	}


public:
	//////////////////////////////////////////////////////////////////////////
	// Getters
	// 

	const TArray<FEdgeData>& GetGraph() const { return Graph; }
	
	/** Unsafe access */
	const FEdgeData& GetEdgeData(int32 Index) const { return Graph[Index]; }


	int32 FindClosestEdgeIndex(const FVector& Location) const;

	FVector ToLocation(int32 EdgeIndex) const;

	TArray<FVector> ToLocations(const TArray<int32>& EdgeIndices, FVector Center = FVector::ZeroVector) const;
	
};
