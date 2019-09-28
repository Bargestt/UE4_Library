// Fill out your copyright notice in the Description page of Project Settings.

#include "CelledSurfaceNavData.h"
#include "DrawDebugHelpers.h"



FVector FCelledSurfaceNavData::GetNodeCenter(GraphNodeRef NodeRef) const
{
	const FGraphNode& Node = Nodes[NodeRef];
	return (Vertices[Node.Triangle[0]] + Vertices[Node.Triangle[1]] + Vertices[Node.Triangle[2]]) / 3;
}

FVector FCelledSurfaceNavData::GetNodeVertex(GraphNodeRef NodeRef, int8 VertexIndexFrom0to2) const
{
	const FGraphNode& Node = Nodes[NodeRef];
	return Vertices[Node.Triangle[VertexIndexFrom0to2]];
}

FCelledSurfaceNavData::GraphNodeRef FCelledSurfaceNavData::GetNodeCloseToLocation(const FVector& WorldLocation) const
{
	FIntVector Coordinate = GetCellCoordinate(WorldLocation);	

	const FCellData& Cell = Cells.FindRef(Coordinate);
	if (Cell.IsEmpty())
	{
		return -1;
	}

	float MinDist = TNumericLimits<float>::Max();
	GraphNodeRef ClosestNode = -1;
	for (GraphNodeRef NodeRef : Cell.NodesInside)
	{
		const FGraphNode& Node = Nodes[NodeRef];
		FPlane Plane(Vertices[Node.Triangle[0]], Vertices[Node.Triangle[1]], Vertices[Node.Triangle[2]]);
		float CurDist = FMath::Abs(Plane.PlaneDot(WorldLocation));

		if (CurDist < MinDist)
		{
			MinDist = CurDist;
			ClosestNode = NodeRef;
		}
	}
	return ClosestNode;
}

bool FCelledSurfaceNavData::HasCellData(const FIntVector& CellCoordinate) const
{
	return Cells.Contains(CellCoordinate);
}

bool FCelledSurfaceNavData::IsCellEmpty(const FIntVector& CellCoordinate) const
{
	const FCellData* Cell = Cells.Find(CellCoordinate);
	return Cell ? Cell->IsEmpty() : false;
}


FCelledSurfaceNavData::FCellData& FCelledSurfaceNavData::GetOrAddCellData(const FIntVector& CellCoordinate)
{
	return Cells.FindOrAdd(CellCoordinate);
}



bool FCelledSurfaceNavData::GetCellData(const FIntVector& CellCoordinate, FCellData& OutCellData) const
{
	if (const FCellData* find = Cells.Find(CellCoordinate))
	{
		OutCellData = *find;
		return true;
	}
	OutCellData = FCellData();
	return false; 
}



void FCelledSurfaceNavData::UpdateCell(const FIntVector& CellCoordinate, const FCellCreationData& Data)
{
	ClearCell(CellCoordinate);

	UE_LOG(LogTemp, Warning, TEXT("Coord: %s, Data: Indices: %d, Vertices: %d"), *CellCoordinate.ToString(), Data.CellTriangles.Num(), Data.CellVertices.Num());
	
	FCellData& Cell = GetOrAddCellData(CellCoordinate);
	FBox CellBox = GetCellBox(CellCoordinate);

	int32 VerticesOutside = 0;

	Cell.VerticesInside.Reserve(Data.CellVertices.Num());
	TMap<int32, int32> Remap;
	for (int Index = 0; Index < Data.CellVertices.Num() ; Index++)
	{
		const FVector& Vertex = Data.CellVertices[Index];
		if (!CellBox.IsInsideOrOn(Vertex))
		{
			VerticesOutside++;
		}

		int32 AddedAt = Vertices.Add(Vertex);
		if (AddedAt >= 0)
		{
			Remap.Add(Index, AddedAt);			
		}
		Cell.VerticesInside.Add(AddedAt);
	}

	if (VerticesOutside > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found %d vertices outside cell bounds when updating cell %s"), VerticesOutside, *CellCoordinate.ToString());
	}

	// Apply remap to triangle indices
	TArray<int32> Triangles;
	Triangles.Reserve(Data.CellTriangles.Num());
	for (int Index = 0; Index < Data.CellTriangles.Num() ; Index++)
	{
		Triangles.Add(Remap[Data.CellTriangles[Index]]);
	}

	TSet<int32> OuterVertices;
	OuterVertices.Reserve(Data.OuterVertices.Num());
	for (int Index = 0; Index < Data.OuterVertices.Num(); Index++)
	{
		OuterVertices.Add(Remap[Data.OuterVertices[Index]]);
	}

	//Add graph nodes
	Cell.NodesInside.Reserve(Triangles.Num() / 3);
	for (int NodeIndex = 0; NodeIndex < Triangles.Num(); NodeIndex += 3)
	{
		FGraphNode Node(Triangles[NodeIndex + 0], Triangles[NodeIndex + 1], Triangles[NodeIndex + 2]);
		int32 AddedAt = Nodes.Add(Node);
		if (AddedAt >= 0)
		{
			Cell.NodesInside.Add(AddedAt);
			if (OuterVertices.Contains(Node.Triangle[0]) || OuterVertices.Contains(Node.Triangle[1]) || OuterVertices.Contains(Node.Triangle[2]))
			{
				Cell.BoundaryNodes.Add(AddedAt);
			}
		}
	}
	BuildGraphInCell(Cell);	

	AttachToNeighbouringCells(CellCoordinate);
	
	UE_LOG(LogTemp, Warning, TEXT("Add. Vertices data: Size: %d, Free: %d, Occupied: %d"), Vertices.NumTotal(), Vertices.NumHoles(), Vertices.NumOccupied());
}




void FCelledSurfaceNavData::ClearCell(const FIntVector& CellCoordinate)
{
	if (IsCellEmpty(CellCoordinate)) return;

	DetachFromNeighbouringCells(CellCoordinate);


	FCellData& Cell = GetOrAddCellData(CellCoordinate);
	for (int32 Index : Cell.VerticesInside)
	{
		Vertices.RemoveAt(Index);
	}
	for (int32 Index : Cell.NodesInside)
	{
		Nodes.RemoveAt(Index);
	}	

	Cell.VerticesInside.Empty();
	Cell.NodesInside.Empty();
	Cell.BoundaryNodes.Empty();

	UE_LOG(LogTemp, Warning, TEXT("Clear. Vertices data: Size: %d, Free: %d, Occupied: %d"), Vertices.NumTotal(), Vertices.NumHoles(), Vertices.NumOccupied());
}

void FCelledSurfaceNavData::ClearAllCells()
{
	Cells.Empty();
	Vertices.Empty();

	UE_LOG(LogTemp, Warning, TEXT("Force clear"));
}



FIntVector FCelledSurfaceNavData::CellNeighbourOffsets[6] =
{
	FIntVector(1, 0, 0),
	FIntVector(-1, 0, 0),

	FIntVector(0, 1, 0),
	FIntVector(0, -1, 0),

	FIntVector(0, 0, 1),
	FIntVector(0, 0, -1)
};




void FCelledSurfaceNavData::AttachToNeighbouringCells(const FIntVector& CellCoordinate)
{
	if (IsCellEmpty(CellCoordinate)) return;
	
	
	for (int Index = 0; Index < 6 ; Index++)
	{
		FIntVector NeighbourCell = CellCoordinate + CellNeighbourOffsets[Index];
		if(IsCellEmpty(NeighbourCell)) continue;
		StitchCells(GetOrAddCellData(CellCoordinate), GetOrAddCellData(NeighbourCell));
	}
}

void FCelledSurfaceNavData::DetachFromNeighbouringCells(const FIntVector& CellCoordinate)
{
	if (IsCellEmpty(CellCoordinate)) return;	

	for (int Index = 0; Index < 6; Index++)
	{
		FIntVector NeighbourCell = CellCoordinate + CellNeighbourOffsets[Index];
		if (IsCellEmpty(NeighbourCell)) continue;		
		RipCells(GetOrAddCellData(CellCoordinate), GetOrAddCellData(NeighbourCell));
	}
}

void FCelledSurfaceNavData::StitchCells(FCellData& A, FCellData& B)
{
	//#todo Stitching algorithm
}

void FCelledSurfaceNavData::RipCells(FCellData& A, FCellData& B)
{
	//#todo ripping algorithm
}

void FCelledSurfaceNavData::BuildGraphInCell(FCellData& Cell)
{
	//#todo fast graph building algorithm

	// Dumb method as proof of concept
	for (int NodeIndex = 0; NodeIndex < Cell.NodesInside.Num(); NodeIndex++)
	{
		GraphNodeRef CurrentNodeRef = Cell.NodesInside[NodeIndex];
		FGraphNode& CurrentNode = Nodes.GetRef(CurrentNodeRef);

		for (int Index = 0; Index < Cell.NodesInside.Num(); Index++)
		{
			GraphNodeRef OtherNodeRef = Cell.NodesInside[Index];
			if (OtherNodeRef == CurrentNodeRef) continue;

			const FGraphNode& OtherNode = Nodes[OtherNodeRef];

			if (CurrentNode.IsConnectedTo(OtherNode))
			{
				CurrentNode.Neighbours.AddUnique(OtherNodeRef);
			}
		}

		Nodes.ValidateAt(CurrentNodeRef);
	}
}

float FCelledSurfaceNavData::GetCellSize() const
{
	return CellSize;
}

FVector FCelledSurfaceNavData::GetCellExtent() const
{
	return FVector(CellSize / 2);
}

FVector FCelledSurfaceNavData::GetCellCenter(const FIntVector& CellCoordinate) const
{
	return Center + FVector(GetCellSize()) * FVector(CellCoordinate);
}

FBox FCelledSurfaceNavData::GetCellBox(const FIntVector& CellCoordinate) const
{
	return FBox::BuildAABB(GetCellCenter(CellCoordinate), GetCellExtent());
}

FIntVector FCelledSurfaceNavData::GetCellCoordinate(const FVector& WorldCoordinate) const
{
	return FIntVector((WorldCoordinate - Center).GridSnap(CellSize) / CellSize);
}

TArray<FIntVector> FCelledSurfaceNavData::GetCellsContainingBox(const FBox& WorldBox) const
{
	FIntVector CellMin = GetCellCoordinate(WorldBox.Min);
	FIntVector CellMax = GetCellCoordinate(WorldBox.Max);
	FIntVector Size = CellMax - CellMin;

	TArray<FIntVector> Boxes;
	Boxes.Reserve(Size.X * Size.Y * Size.Z);
	for (int X = CellMin.X; X <= CellMax.X; X++)
	{
		for (int Y = CellMin.Y; Y <= CellMax.Y; Y++)
		{
			for (int Z = CellMin.Z; Z <= CellMax.Z; Z++)
			{
				Boxes.Add(FIntVector(X, Y, Z));
			}
		}
	}

	return Boxes;
}



bool FCelledSurfaceNavData::ProjectPointToNavigation(const FVector& WorldLocation, FVector& OutLocation) const
{
	GraphNodeRef NodeRef = GetNodeCloseToLocation(WorldLocation);
	if (NodeRef < 0) return false;
	
	OutLocation = GetNodeCenter(NodeRef);
	return true;
}

void FCelledSurfaceNavData::DrawCellBounds(const FIntVector& CellCoords, FColor CellColor, float Lifetime /*= 1*/, float Thickness /*= 0*/) const
{
	if (!World) return;
	DrawDebugBox(World, GetCellCenter(CellCoords), GetCellExtent(), FQuat::Identity, CellColor, false, Lifetime, 0, Thickness);
}


void FCelledSurfaceNavData::DrawCellGraph(const FIntVector& CellCoords, float Lifetime /*= 1*/) const
{
	if (!World) return;

	const FColor VertexColor = FColor::Cyan;
	const FColor EdgeColor = FColor::Green;
	const FColor LinkColor = FColor::Blue;

	const float VertexSize = 3;
	const float EdgeSize = 1;
	const float LinkSize = 2;

	const FCellData& CellData = Cells.FindRef(CellCoords);
	if (CellData.IsEmpty()) return;
	
	for (int32 Vertex : CellData.VerticesInside)
	{
		check(Vertices.IsElementAt(Vertex));
		DrawDebugPoint(World, Vertices[Vertex], VertexSize, VertexColor, false, Lifetime);
	}

	for (GraphNodeRef NodeRef : CellData.NodesInside)
	{
		const FGraphNode& Node = Nodes[NodeRef];

		FVector NodeCenter = GetNodeCenter(NodeRef);

		DrawDebugPoint(World, NodeCenter, LinkSize, LinkColor, false, Lifetime);
		DrawDebugLine(World, Vertices[Node.Triangle[1]], Vertices[Node.Triangle[0]], EdgeColor, false, Lifetime, 0, EdgeSize);
		DrawDebugLine(World, Vertices[Node.Triangle[2]], Vertices[Node.Triangle[1]], EdgeColor, false, Lifetime, 0, EdgeSize);
		DrawDebugLine(World, Vertices[Node.Triangle[0]], Vertices[Node.Triangle[2]], EdgeColor, false, Lifetime, 0, EdgeSize);

		for (GraphNodeRef ConnectedNodeRef : Node.Neighbours)
		{			
			FVector NodeCenter2 = GetNodeCenter(ConnectedNodeRef);
			DrawDebugLine(World, NodeCenter, NodeCenter2, LinkColor, false, Lifetime, 0, LinkSize);
		}
	}
}

void FCelledSurfaceNavData::DrawGraph(float Lifetime /*= 5*/) const
{
	TArray<FIntVector> CellCoords;
	Cells.GenerateKeyArray(CellCoords);
	for (FIntVector Coord : CellCoords)
	{
		DrawCellGraph(Coord, Lifetime);
	}
}






