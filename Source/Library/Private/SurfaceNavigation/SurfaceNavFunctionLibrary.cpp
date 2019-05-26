// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "SurfaceNavigation/SurfaceNavLocalData.h"













void USurfaceNavFunctionLibrary::DrawSurfaceGraph(const UObject* WorldContext, const FTransform& Transform,const FSurfaceNavLocalData& NavData, float Duration /*= 5*/, FColor LinkColor /*= FColor::Green*/, FColor NodeColor /*= FColor::Blue*/)
{
	const UWorld* World = WorldContext->GetWorld();

	const TArray<FEdgeData>& Edges = NavData.GetGraph();

	if (World == nullptr) return;

	const int PointSize = 7;
	const int LinkSize = 0;

	for (int Index = 0; Index < Edges.Num(); Index++)
	{
		const FEdgeData& Edge = Edges[Index];

		FVector PointPosition = Transform.TransformPositionNoScale(Edge.EdgeVertex);

		DrawDebugPoint(World, PointPosition, PointSize, NodeColor, false, Duration);
		for (int link : Edge.ConnectedEdges)
		{
			DrawDebugLine(World, PointPosition, Transform.TransformPosition(Edges[link].EdgeVertex), LinkColor, false, Duration, 0, LinkSize);
		}
	}
}
