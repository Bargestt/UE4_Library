// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


DECLARE_LOG_CATEGORY_EXTERN(MarchingCubesBuilder, Log, All);


class FMarchingCubesBuilder
{
public:
	//Bitmask of edges intersected by isosurface
	static const uint16 EdgeTable[256];

	//Triangles for configuration
	static const int8 TriTable[256][16];

	// index of both vertices of edge in cell
	static const int8 Edges[12][2];

	// Each cube 'owns' only edges connected to first vertex. Other edges owned by neighboring cubes
	// This table converts local edge index to cube coordinate offset pointing to owner of edge
	static const FIntVector EdgeToCubeOffset[12];

private:

	// Offset for storing edges, 0*StorageOffset offset for X edges, 1*StorageOffset Offset for Y edges, 2*StorageOffset Offsets for Z edges
	int32 StorageOffset;


	const TArray<FVector4>& Points;

	const FIntVector Dimensions;


	TMap<int32, int32> GlobalIndexToVertice;

	TArray<FVector> Vertices;

	TArray<int32> Indices;


	bool InputIsValid = true;

public:

	float SurfaceLevel = 0.5f;

	// Disabling this option will result in duplicate vertices on triangle connections
	bool RemoveDuplicateVertices = true;


public:
	FMarchingCubesBuilder(const TArray<FVector4>& Points, const FIntVector Dimensions)
		: Points(Points)
		, Dimensions(Dimensions)
		, StorageOffset(Dimensions.X*Dimensions.Y*Dimensions.Z)
	{
		ValidateInput();
	}

	bool IsValid() const { return InputIsValid; }

	void GetData(TArray<FVector>& OutVertices, TArray<int32>& OutIndices) const
	{
		OutIndices = Indices;
		OutVertices = Vertices;
	}

	void Build()
	{
		if (!IsValid()) return;

		GlobalIndexToVertice.Empty();
		Vertices.Empty();
		Indices.Empty();

		for (int X = 0; X < Dimensions.X - 1; X++)
		{
			for (int Y = 0; Y < Dimensions.Y - 1; Y++)
			{
				for (int Z = 0; Z < Dimensions.Z - 1; Z++)
				{
					PoligonizeCube(FIntVector(X, Y, Z));
				}
			}
		}

		if (RemoveDuplicateVertices == false)
		{
			Indices.Init(0, Vertices.Num());
			for (int Index = 0; Index < Indices.Num(); Index++)
			{
				Indices[Index] = Index;
			}
		}

		UE_LOG(MarchingCubesBuilder, Log, TEXT("Built mesh. Vertices: %d, Triangles: %d"), Vertices.Num(), Indices.Num() / 3);
	}

protected:
	void PoligonizeCube(FIntVector CubeCoords)
	{
		const FVector4 Verts[8] =
		{
			GetPoint(CubeCoords + FIntVector(0, 0, 0)),
			GetPoint(CubeCoords + FIntVector(1, 0, 0)),
			GetPoint(CubeCoords + FIntVector(1, 1, 0)),
			GetPoint(CubeCoords + FIntVector(0, 1, 0)),

			GetPoint(CubeCoords + FIntVector(0, 0, 1)),
			GetPoint(CubeCoords + FIntVector(1, 0, 1)),
			GetPoint(CubeCoords + FIntVector(1, 1, 1)),
			GetPoint(CubeCoords + FIntVector(0, 1, 1))
		};

		/*
		  Determine the index into the edge table which
		  tells us which vertices are inside of the surface
		*/
		uint8 cubeindex = 0;
		for (int Index = 0; Index < 8; Index++)
		{
			if (Verts[Index].W > SurfaceLevel)
			{
				cubeindex |= 1 << Index;
			}
		}
		/* Cube is entirely in/out of the surface */
		if (EdgeTable[cubeindex] == 0) return;

		/* Create the triangle */
		for (int i = 0; TriTable[cubeindex][i] != -1; i++)
		{
			int LocalEdgeIndex = TriTable[cubeindex][i];
			int a = Edges[LocalEdgeIndex][0];
			int b = Edges[LocalEdgeIndex][1];

			if (RemoveDuplicateVertices)
			{
				uint32 GlobalEdgeIndex = GetEdgeIndexGlobal(CubeCoords, LocalEdgeIndex);
				int32 UniqueVerticeIndex;
				if (const int32* find = GlobalIndexToVertice.Find(GlobalEdgeIndex))
				{
					UniqueVerticeIndex = *find;
				}
				else
				{
					UniqueVerticeIndex = GlobalIndexToVertice.Add(GlobalEdgeIndex, Vertices.Add(VertexLerp(SurfaceLevel, Verts[a], Verts[b])));
				}
				Indices.Add(UniqueVerticeIndex);
			}
			else
			{
				Vertices.Add(VertexLerp(SurfaceLevel, Verts[a], Verts[b]));
			}
		}
	}	

	void ValidateInput()
	{
		InputIsValid = true;
		if (Dimensions.X * Dimensions.Y * Dimensions.Z > Points.Num())
		{
			UE_LOG(MarchingCubesBuilder, Error, TEXT("Dimension mismatch. Not enough Points to match Dimetsions"));
			InputIsValid = false;
		}
		if (Dimensions.GetMin() <= 0)
		{
			UE_LOG(MarchingCubesBuilder, Warning, TEXT("Not enogh points to form a cube."));
			InputIsValid = false;
		}
	}

	const FVector4& GetPoint(const FIntVector& Coords) const
	{
		return GetPoint(Coords.X, Coords.Y, Coords.Z);
	}
	const FVector4& GetPoint(int32 X, int32 Y, int32 Z) const
	{
		return Points[X + Dimensions.X * Y + Dimensions.X*Dimensions.Y * Z];
	}

	FORCEINLINE int32 GetEdgeIndexGlobal(const FIntVector& CubeCoordinate, int EdgeIndexLocal) const
	{
		FIntVector OwningCubeCoord = CubeCoordinate + EdgeToCubeOffset[EdgeIndexLocal];
		int Index = OwningCubeCoord.X + OwningCubeCoord.Y*(Dimensions.X) + OwningCubeCoord.Z*(Dimensions.X*Dimensions.Y);

		if (EdgeIndexLocal == 3 || EdgeIndexLocal == 1 || EdgeIndexLocal == 5 || EdgeIndexLocal == 7) Index += StorageOffset;
		if (EdgeIndexLocal >= 8) Index += 2 * StorageOffset;
		return Index;
	}

public:

	static FORCEINLINE FVector VertexLerp(float SurfaceLevel, FVector4 P1, FVector4 P2)
	{
		float t = (SurfaceLevel - P1.W) / (P2.W - P1.W);
		return FMath::Lerp(FVector(P1), FVector(P2), t);
	}
	/** Demonstration function. Does no reset output arrays. Does not check for duplicate vertices 
	 * @param	CubeVertices	8 vertices of cube. XYZ coordinates, W - surface value
	 */
	static void PoligonizeSingle(const FVector4 CubeVertices[8], float SurfaceLevel, TArray<FVector>& OutVertices, TArray<int32> OutIndices)
	{
		uint8 cubeindex = 0;
		for (int Index = 0; Index < 8; Index++)
		{
			if (CubeVertices[Index].W > SurfaceLevel)
			{
				cubeindex |= 1 << Index;
			}
		}
		/* Cube is entirely in/out of the surface */
		if (EdgeTable[cubeindex] == 0) return;

		/* Create the triangle */
		for (int i = 0; TriTable[cubeindex][i] != -1; i++)
		{
			int LocalEdgeIndex = TriTable[cubeindex][i];
			int a = Edges[LocalEdgeIndex][0];
			int b = Edges[LocalEdgeIndex][1];		
			int32 AddedAtIndex = OutVertices.Add(VertexLerp(SurfaceLevel, CubeVertices[a], CubeVertices[b]));
			OutIndices.Add(AddedAtIndex);
		}
	}
};


