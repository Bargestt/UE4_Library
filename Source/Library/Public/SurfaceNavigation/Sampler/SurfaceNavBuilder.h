// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SurfaceNavigation.h"
#include "MarchingCubesBuilder.h"

DECLARE_STATS_GROUP(TEXT("SurfaceNavigation"), STATGROUP_SurfaceNavigation, STATCAT_Advanced);

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
	TArray<FEdgeData> AllEdges;

	FIntVector Dimensions;

	// Offset for storing edges, 0*StorageOffset offset for X edges, 1*StorageOffset Offset for Y edges, 2*StorageOffset Offsets for Z edges
	int32 StorageOffset;

public:
	float SurfaceValue;

public:
	FSurfaceNavBuilder() : SurfaceValue(.5f) {}

	bool BuildGraph(const TArray<FVector4>& Points, const FIntVector& Dimensions, FSurfaceNavLocalData& SaveTarget);

	bool BuildGraph(const TArray<FVector4>& Points, const FIntVector& Dimensions, TArray<FEdgeData>& OutGraph);


protected:

	void CleanUp();

	bool BuildGraph_Internal(const TArray<FVector4>& Points, const FIntVector& Dimensions, float SurfaceValue, TArray<FEdgeData>& OutEdges);


	struct FCell
	{
		const FVector4 Verts[8];
		const FIntVector Coordinates;
	};

	bool AddEdgeData(const FCell& Cell, float SurfaceLevel, const FIntVector& Cells, TArray<FEdgeData>& AllEdgesArray);


	FORCEINLINE int GetEdgeIndexGlobal(const FIntVector& CellCoordinate, int EdgeIndexLocal) const
	{
		FIntVector ActualCellCoord = CellCoordinate + FMarchingCubesBuilder::EdgeToCubeOffset[EdgeIndexLocal];
		int Index = ActualCellCoord.X + ActualCellCoord.Y*Dimensions.X + ActualCellCoord.Z*Dimensions.X*Dimensions.Y;

		if (EdgeIndexLocal == 3 || EdgeIndexLocal == 1 || EdgeIndexLocal == 5 || EdgeIndexLocal == 7) Index += StorageOffset;
		if (EdgeIndexLocal >= 8) Index += 2 * StorageOffset;
		return Index;

		return 0;
	}

	static FORCEINLINE FVector VertexLerp(float SurfaceLevel, FVector4 P1, FVector4 P2)
	{
		float t = (SurfaceLevel - P1.W) / (P2.W - P1.W);
		return FMath::Lerp(FVector(P1), FVector(P2), t);
	}

};