// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MarchingCubesFunctionLibrary.generated.h"


/** Helper structure to hold 3 dimensional array 
 *  XYZ vertex position + W vertex value
 * */
USTRUCT(BlueprintType)
struct FPointsArray3D
{
	GENERATED_BODY()

	TArray<TArray<TArray<FVector4>>> Points;

	FORCEINLINE FVector GetLocation(int X, int Y, int Z) const
	{
		return FVector(Points[X][Y][Z]);
	}

	FORCEINLINE float GetValue(int X, int Y, int Z) const
	{
		return Points[X][Y][Z].W;
	}

	/** Has at least 1 point */
	FORCEINLINE bool IsValid() const
	{
		return Points.Num() > 0 && Points[0].Num() > 0 && Points[0][0].Num() > 0;
	}
};


USTRUCT(BlueprintType)
struct FMeshGenerationParams
{
	GENERATED_BODY()

	/** Use to lessen allocations number. Very minor performance improvement */
	UPROPERTY(BlueprintReadWrite)
	int EstimatedTriangleNum;

	static const FMeshGenerationParams DefaultMeshGenerationParams;

	FMeshGenerationParams()
	{
		EstimatedTriangleNum = 1000;
	}


};




/**
 * 
 */
UCLASS()
class LIBRARY_API UMarchingCubesFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
	static const int EdgeTable[256];
	static const int TriTable[256][16];

	// index of both vertices of edge in cell
	static const int Edges[12][2];

public:


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility|MarchingCubes")
	static FIntVector GetDimensions(const FPointsArray3D& Points);


	UFUNCTION(BlueprintCallable, Category = "Utility|MarchingCubes", meta=(WorldContext="WorldContext"))
	static void DrawPoints(const UObject* WorldContext, const FTransform& OriginTransform, const FPointsArray3D& PointsData, float MaxValue = 1);





	/**
	 * @param	Points	3D array, XYZ vertex position + W vertex value
	 * @param	SurfaceLevel	Form surface along this value
	 */
	UFUNCTION(BlueprintCallable, Category = "Utility|MarchingCubes", meta = (AdvancedDisplay = "Params"))
	static void GenerateMesh(const FPointsArray3D& Points, float SurfaceLevel, const FMeshGenerationParams Params, TArray<FVector>& OutVertices, TArray<int32>& OutIndices)
	{
		GenerateMesh(Points.Points, SurfaceLevel, OutVertices, OutIndices, Params);
	}

	/**
	 * @param	Points	3D array, XYZ vertex position + W vertex value
	 * @param	SurfaceLevel	Form surface along this value
	 */
	static void GenerateMesh(const TArray<TArray<TArray<FVector4>>>& Points, float SurfaceLevel, TArray<FVector>& OutVertices, TArray<int32>& OutIndices, const FMeshGenerationParams& Params = FMeshGenerationParams::DefaultMeshGenerationParams);



protected:


	static FORCEINLINE FVector VertexLerp(float SurfaceLevel, FVector4 P1, FVector4 P2)
	{
		float t = (SurfaceLevel - P1.W) / (P2.W - P1.W);
		return FMath::Lerp(FVector(P1), FVector(P2), t);
	}

	struct FCell
	{
		const FVector4 Verts[8];
	};

	static FORCEINLINE void Poligonise(const FCell& Cell, float SurfaceLevel, TArray<FVector>& VertexArray, TArray<int32>& IndexArray)
	{
		/*
		  Determine the index into the edge table which
		  tells us which vertices are inside of the surface
		*/
		int cubeindex = 0;
		for (int Index = 0; Index < 8; Index++)
		{
			if (Cell.Verts[Index].W < SurfaceLevel)
			{
				cubeindex |= 1 << Index;
			}
		}

		/* Cube is entirely in/out of the surface */
		if (EdgeTable[cubeindex] == 0) return;

		/* Create the triangle */
		for (int i = 0; TriTable[cubeindex][i] != -1; i++)
		{
			int a = Edges[TriTable[cubeindex][i]][0];
			int b = Edges[TriTable[cubeindex][i]][1];
			VertexArray.Add(VertexLerp(SurfaceLevel, Cell.Verts[a], Cell.Verts[b]));

			IndexArray.Add(IndexArray.Num());
		}
	}

};
