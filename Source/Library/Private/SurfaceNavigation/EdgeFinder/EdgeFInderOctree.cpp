// Fill out your copyright notice in the Description page of Project Settings.

#include "EdgeFInderOctree.h"
#include "DrawDebugHelpers.h"
#include "SurfaceNavLocalData.h"

int8 FPointsOctree::MaxElementsPerNode = 8;


int32 EdgeFInderOctree::FindEdgeIndex(const FVector& Location) const
{
	return -1;
}

bool EdgeFInderOctree::IsReady() const
{
	return false; 
}

bool EdgeFInderOctree::Rebuild()
{
	return false;
}


void EdgeFInderOctree::BuildOctree(float CellSize, float Size)
{
	Tree = FPointsOctree(FVector(0), Size);

	MapCellSize = CellSize;

	const TArray<FEdgeData>& Edges = NavData.GetGraph();

	for (int Index = 0; Index < Edges.Num(); Index++)
	{
		const FEdgeData& Edge = Edges[Index];

		Tree.AddElementToLocation(Edge.EdgeVertex, Index);
	}
}


bool EdgeFInderOctree::GetEdgesFromTree(const FVector& Location, TArray<int32>& OutEdges)
{
	OutEdges = Tree.GetElementsByLocation(Location);
	return Tree.IsInside(Location);
}

void FPointsOctree::FTreeNode::DrawDebug(const UWorld* World, const FTransform& Transform, bool IncludeChildren /*= false*/) const
{
	DrawDebugBox(World, Transform.TransformPosition(Center), FVector(Bounds), FQuat::Identity, FColor::Green, false, 15);

	if (IncludeChildren)
	{
		for (const FTreeNode& Child : Children)
		{
			Child.DrawDebug(World, Transform, IncludeChildren);
		}
	}
}
