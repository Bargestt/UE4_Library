// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavBuilder.h"
#include "SurfaceNavLocalData.h"




DECLARE_CYCLE_STAT(TEXT("SurfaceNavigation ~ Build"), STAT_Build, STATGROUP_SurfaceNavigation);
DECLARE_CYCLE_STAT(TEXT("SurfaceNavigation ~ CleanUp"), STAT_CleanUp, STATGROUP_SurfaceNavigation);

DECLARE_DWORD_COUNTER_STAT(TEXT("SurfaceNavigation ~ CellsNumber"), STAT_CellsNumber, STATGROUP_SurfaceNavigation);



bool FSurfaceNavBuilder::BuildGraph(const TArray<FVector4>& Points, const FIntVector& Dimensions, TArray<FEdgeData>& OutGraph)
{
	if (BuildGraph_Internal(Points, Dimensions, SurfaceValue, AllEdges))
	{
		CleanUp();
		OutGraph = MoveTemp(AllEdges);
		return true;
	}

	return false;
}

bool FSurfaceNavBuilder::BuildGraph(const TArray<FVector4>& Points, const FIntVector& Dimensions, FSurfaceNavLocalData& SaveTarget)
{
	if (BuildGraph_Internal(Points, Dimensions, SurfaceValue, AllEdges))
	{
		CleanUp();
		SaveTarget.SetGraph(AllEdges, Dimensions);
		return true;
	}

	return false;
}

void FSurfaceNavBuilder::CleanUp()
{
	SCOPE_CYCLE_COUNTER(STAT_CleanUp);

	TMap<int, int> EdgeIndexChange;

	TArray<FEdgeData> NewEdges;
	for (int Index = 0; Index < AllEdges.Num() ; Index++)
	{
		FEdgeData& Edge = AllEdges[Index];

		if (Edge.IsInitialized())
		{
			int NewIndex = NewEdges.Add(Edge);
			EdgeIndexChange.Add(Index, NewIndex);
		}
	}

	for (int Index = 0; Index < NewEdges.Num() ; Index++)
	{
		FEdgeData& Edge = NewEdges[Index];
		for (int j = 0; j < Edge.ConnectedEdges.Num() ; j++)
		{
			Edge.ConnectedEdges[j] = EdgeIndexChange[Edge.ConnectedEdges[j]];
		}
	}

	AllEdges = MoveTemp(NewEdges);
}



bool FSurfaceNavBuilder::BuildGraph_Internal(const TArray<FVector4>& Points, const FIntVector& PointsDimensions, float SurfaceValue, TArray<FEdgeData>& OutEdges)
{	
	if (Points.Num() < PointsDimensions.X*PointsDimensions.Y*PointsDimensions.Z) return false;

	SCOPE_CYCLE_COUNTER(STAT_Build);
	SET_DWORD_STAT(STAT_CellsNumber, PointsDimensions.X*PointsDimensions.Y*PointsDimensions.Z);

	Dimensions = PointsDimensions;


	int SizeX = Dimensions.X;
	int	SizeXY = Dimensions.X * Dimensions.Y;
	auto GetPointIndex = [SizeX, SizeXY](int x, int y, int z) {	return x + y * SizeX + z * SizeXY; };

	StorageOffset = Dimensions.X*Dimensions.Y*Dimensions.Z;
	
	FIntVector CellsNum = Dimensions - FIntVector(1);
	int LastCellIndex = GetEdgeIndexGlobal(CellsNum - FIntVector(1), 10) + 1;
	OutEdges.Init(FEdgeData(), LastCellIndex);  // will have some free space though

	for (int Z = 0; Z < CellsNum.Z; Z++)
	{
		for (int Y = 0; Y < CellsNum.Y; Y++)
		{
			for (int X = 0; X < CellsNum.X; X++)
			{
				FCell Cell = {
					{
						Points[GetPointIndex(X,		Y,		Z)],
						Points[GetPointIndex(X + 1, Y,		Z)],
						Points[GetPointIndex(X + 1, Y + 1,	Z)],
						Points[GetPointIndex(X,		Y + 1,	Z)],

						Points[GetPointIndex(X,		Y,		Z + 1)],
						Points[GetPointIndex(X + 1, Y,		Z + 1)],
						Points[GetPointIndex(X + 1, Y + 1,	Z + 1)],
						Points[GetPointIndex(X,		Y + 1,	Z + 1)]
					},
					FIntVector(X, Y, Z)
				};
				AddEdgeData(Cell, SurfaceValue, CellsNum, OutEdges);
			}
		}
	}

	return true;
}


bool FSurfaceNavBuilder::AddEdgeData(const FCell& Cell, float SurfaceLevel, const FIntVector& Cells, TArray<FEdgeData>& AllEdgesArray)
{
	typedef FMarchingCubesBuilder Lib;

	int cubeindex = 0;
	for (int Index = 0; Index < 8; Index++)
	{
		if (Cell.Verts[Index].W < SurfaceLevel)
		{
			cubeindex |= 1 << Index;
		}
	}
	/* Cube is entirely in/out of the surface */
	if (Lib::EdgeTable[cubeindex] == 0) return false;

	int Offset = Dimensions.X*Dimensions.Y*Dimensions.Z;

	for (int i = 0; Lib::TriTable[cubeindex][i] != -1; i += 3)
	{
		// Get 3 vertices of triangle
		int Edge1_IndexLocal = Lib::TriTable[cubeindex][i];
		int a1 = Lib::Edges[Edge1_IndexLocal][0];
		int b1 = Lib::Edges[Edge1_IndexLocal][1];
		FVector Vertex1 = VertexLerp(SurfaceLevel, Cell.Verts[a1], Cell.Verts[b1]);

		int Edge2_IndexLocal = Lib::TriTable[cubeindex][i + 1];
		int a2 = Lib::Edges[Edge2_IndexLocal][0];
		int b2 = Lib::Edges[Edge2_IndexLocal][1];
		FVector Vertex2 = VertexLerp(SurfaceLevel, Cell.Verts[a2], Cell.Verts[b2]);

		int Edge3_IndexLocal = Lib::TriTable[cubeindex][i + 2];
		int a3 = Lib::Edges[Edge3_IndexLocal][0];
		int b3 = Lib::Edges[Edge3_IndexLocal][1];
		FVector Vertex3 = VertexLerp(SurfaceLevel, Cell.Verts[a3], Cell.Verts[b3]);


		// Setup triangle vertices and links
		int Edge1_Index = GetEdgeIndexGlobal(Cell.Coordinates, Edge1_IndexLocal);
		int Edge2_Index = GetEdgeIndexGlobal(Cell.Coordinates, Edge2_IndexLocal);
		int Edge3_Index = GetEdgeIndexGlobal(Cell.Coordinates, Edge3_IndexLocal);
		FEdgeData& Edge1 = AllEdgesArray[Edge1_Index];
		FEdgeData& Edge2 = AllEdgesArray[Edge2_Index];
		FEdgeData& Edge3 = AllEdgesArray[Edge3_Index];

		Edge1.EdgeVertex = Vertex1;
		Edge1.ConnectedEdges.AddUnique(Edge2_Index);
		Edge1.ConnectedEdges.AddUnique(Edge3_Index);

		Edge2.EdgeVertex = Vertex2;
		Edge2.ConnectedEdges.AddUnique(Edge1_Index);
		Edge2.ConnectedEdges.AddUnique(Edge3_Index);

		Edge3.EdgeVertex = Vertex3;
		Edge3.ConnectedEdges.AddUnique(Edge2_Index);
		Edge3.ConnectedEdges.AddUnique(Edge1_Index);
	}

	return true;
}