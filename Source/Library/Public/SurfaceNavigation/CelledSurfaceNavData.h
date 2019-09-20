// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StaticIndexArray.h"







struct FCellCreationData
{
	TArray<FVector> CellVertices;

	TArray<int32> CellTriangles;

	TArray<int32> OuterVertices;

	FCellCreationData() {}

	FCellCreationData(const TArray<FVector>& CellVertices, const TArray<int32>& CellTriangles, const TArray<int32>& OuterVertices)
		: CellVertices(CellVertices)
		, CellTriangles(CellTriangles)
		, OuterVertices(OuterVertices)
	{}
};




/**
 * 
 */
class LIBRARY_API FCelledSurfaceNavData
{
	// Graph
	typedef int32 GraphNodeRef;
	typedef int32 VerticeRef;

	struct FGraphNode
	{
		VerticeRef Triangle[3] = { -1, -1, -1 };

		TArray<GraphNodeRef> Neighbours;

		FGraphNode() {}

		FGraphNode(VerticeRef V1, VerticeRef V2, VerticeRef V3)
		{
			Triangle[0] = V1;
			Triangle[1] = V2;
			Triangle[2] = V3;
		}

		void Invalidate()
		{
			Triangle[0] = -1;
			Triangle[1] = -1;
			Triangle[2] = -1;
		}

		bool IsValid() const
		{
			return Triangle[0] >= 0 && Triangle[1] >= 0 && Triangle[2] >= 0;
		}
		
		bool IsCollapsed() const
		{
			return Triangle[0] == Triangle[1] && Triangle[1] == Triangle[2];
		}

		bool HasVertice(VerticeRef OtherVertice) const
		{
			if (OtherVertice < 0) return false;
			return OtherVertice == Triangle[0] || OtherVertice == Triangle[1] || OtherVertice == Triangle[2];
		}

		bool IsConnectedTo(const FGraphNode& Other)
		{
			int SharedVerticesNum = 0;
			SharedVerticesNum += Other.HasVertice(Triangle[0]);
			SharedVerticesNum += Other.HasVertice(Triangle[1]);
			SharedVerticesNum += Other.HasVertice(Triangle[2]);
			return SharedVerticesNum >= 2;
		}
	};


	struct FGraphValidator
	{
		static bool IsValid(const FGraphNode& Element)
		{
			return Element.IsValid();
		}
		static void Invalidate(FGraphNode& Element)
		{
			Element = FGraphNode();
		}


		static bool IsValid(const FVector& Vector)
		{
			return FMath::Abs(Vector.X) < MAX_FLT && FMath::Abs(Vector.Y) < MAX_FLT && FMath::Abs(Vector.Z) < MAX_FLT;
		}
		static void Invalidate(FVector& Element)
		{
			Element = FVector(MAX_FLT);
		}
	};


	struct FCellData
	{
		TArray<VerticeRef> VerticesInside;

		TArray<GraphNodeRef> NodesInside;

		TArray<GraphNodeRef> BoundaryNodes;

		FCellData()
		{
		}

		bool IsEmpty() const { return VerticesInside.Num() <= 0; }
	};



public:
	FVector Center;

	float CellSize = 100;

public:
	FCelledSurfaceNavData()
		: Center(FVector(0))
	{}

	FCelledSurfaceNavData(FVector Center)
	: Center(Center)
	{}
	~FCelledSurfaceNavData(){}

	// Vertice data
private:
	TStaticIndexArray<FVector, FGraphValidator> Vertices;

	TStaticIndexArray<FGraphNode, FGraphValidator> Nodes;

protected:
	FVector GetNodeCenter(GraphNodeRef NodeRef) const;

	FVector GetNodeVertex(GraphNodeRef NodeRef, int8 VertexIndexFrom0to2) const;

	GraphNodeRef GetNodeCloseToLocation(const FVector& WorldLocation) const;

	// Cell data
private:
	TMap<FIntVector, FCellData> Cells;

protected:
	bool HasCellData(const FIntVector& CellCoordinate) const;
	bool IsCellEmpty(const FIntVector& CellCoordinate) const;

	/** Find or add cell and return reference */
	FCellData& GetOrAddCellData(const FIntVector& CellCoordinate);


	/** Find cell data without adding.
	 *  @return		true	if has cell.
	 */
	bool GetCellData(const FIntVector& CellCoordinate, FCellData& OutCellData) const;

	

public:

	void UpdateCell(const FIntVector& CellCoordinate, const FCellCreationData& Data);	

	void ClearCell(const FIntVector& CellCoordinate);
	void ClearAllCells();

	// Graph building/stitching
protected:
	static FIntVector CellNeighbourOffsets[6];
	
	void AttachToNeighbouringCells(const FIntVector& CellCoordinate);

	void DetachFromNeighbouringCells(const FIntVector& CellCoordinate);

	//#todo
	void StitchCells(FCellData& A, FCellData& B);
	void RipCells(FCellData& A, FCellData& B);
	void BuildGraphInCell(FCellData& Cell);

public:
	// Utility
	float GetCellSize() const;

	FVector GetCellExtent() const;
	FVector GetCellCenter(const FIntVector& CellCoordinate) const;
	FBox GetCellBox(const FIntVector& CellCoordinate) const;

	FIntVector GetCellCoordinate(const FVector& WorldCoordinate) const;

	TArray<FIntVector> GetCellsContainingBox(const FBox& WorldBox) const;


	bool ProjectPointToNavigation(const FVector& WorldLocation, FVector& OutLocation) const;


	//Debug functions
protected:
	const UWorld* World;
public:
	void SetWorld(const UWorld* World) { this->World = World; }
	void DrawCellBounds(const FIntVector& CellCoords, FColor CellColor, float Lifetime = 1, float Thickness = 0) const;
	void DrawCellGraph(const FIntVector& CellCoords, float Lifetime = 1) const;
	
	void DrawGraph(float Lifetime = 5) const;
};
