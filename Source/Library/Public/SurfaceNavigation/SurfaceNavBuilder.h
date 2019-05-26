// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FEdgeData;
class FSurfaceNavLocalData;

namespace FSurfaceNavigation
{
	static const FVector InvalidLocation = FVector(FLT_MAX);

	static bool IsValidLocation(const FVector& Location)
	{
		return  Location.X < InvalidLocation.X && Location.X > -InvalidLocation.X &&
			Location.Y < InvalidLocation.Y && Location.Y > -InvalidLocation.Y &&
			Location.Z < InvalidLocation.Z && Location.Z > -InvalidLocation.Z;
	}
}


class LIBRARY_API FSurfaceNavBuilder
{
	static const FIntVector EdgeToCellOffset[12];	

	TArray<FEdgeData> AllEdges;

	FIntVector Dimensions;
	
	// Offset for storing edges, 0 offset for X edges, 1 Offset for Y edges, 2 Offsets for Z edges
	int32 StorageOffset;

	FSurfaceNavLocalData& SaveTarget;

public:
	FSurfaceNavBuilder(FSurfaceNavLocalData& SaveTarget) : SaveTarget(SaveTarget){}


	bool BuildGraph(const TArray<FVector4>& Points, const FIntVector& Dimensions, float SurfaceValue);

	void Finish();

	//TODO: Cleanup graph, spatial partitioning, data extraction


protected:
	bool BuildGraph_Internal(const TArray<FVector4>& Points, const FIntVector& Dimensions, float SurfaceValue, TArray<FEdgeData>& OutEdges);


	struct FCell
	{
		const FVector4 Verts[8];
		const FIntVector Coordinates;
	};

	bool AddEdgeData(const FCell& Cell, float SurfaceLevel, const FIntVector& Cells, TArray<FEdgeData>& AllEdgesArray);


	FORCEINLINE int GetEdgeIndexGlobal(const FIntVector& CellCoordinate, int EdgeIndexLocal) const
	{
		FIntVector ActualCellCoord = CellCoordinate + EdgeToCellOffset[EdgeIndexLocal];
		int Index = ActualCellCoord.X + ActualCellCoord.Y*Dimensions.X + ActualCellCoord.Z*Dimensions.X*Dimensions.Y;

		if (EdgeIndexLocal == 3 || EdgeIndexLocal == 1 || EdgeIndexLocal == 5 || EdgeIndexLocal == 7) Index += StorageOffset;
		if (EdgeIndexLocal >= 8) Index += 2 * StorageOffset;
		return Index;
	}

	static FORCEINLINE FVector VertexLerp(float SurfaceLevel, FVector4 P1, FVector4 P2)
	{
		float t = (SurfaceLevel - P1.W) / (P2.W - P1.W);
		return FMath::Lerp(FVector(P1), FVector(P2), t);
	}

};