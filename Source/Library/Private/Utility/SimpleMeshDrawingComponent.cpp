// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleMeshDrawingComponent.h"




void FDrawContext::DrawMesh(const TArray<FVector>& Verts, const TArray<int32>& Indices, UMaterialInterface* Material)
{
	FSection* pSection = Sections.Find(Material);
	FSection& Section = pSection ? *pSection : Sections.Add(Material);

	int32 VerticesBeginIndex = Section.Vertices.Num();
	int32 IndicesBeginIndex = Section.Indices.Num();

	Section.Vertices.Append(Verts);
	Section.Indices.Append(Indices);

	if (VerticesBeginIndex > 0)
	{
		for (int Index = IndicesBeginIndex; Index < Section.Indices.Num(); Index++)
		{
			Section.Indices[Index] += VerticesBeginIndex;
		}
	}
}

void USimpleMeshDrawingComponent::ApplyContext(FDrawContext Context)
{	
	for (auto& SectionPair : Context.Sections)
	{
		UMaterialInterface* Material = SectionPair.Key;
		FSection& Section = SectionPair.Value;

		int32 Index = Materials.Find(Material);
		if (Index < 0)
		{
			Index = Materials.Add(Material);
		}

		CreateMeshSection(Index, Section.Vertices, Section.Indices, {}, {}, {}, {}, {}, {}, {}, false);
		if (Material != nullptr)
		{
			SetMaterial(Index, Material);
		}
	}
}







void USimpleMeshDrawingComponent::DrawImmediately(UMaterialInterface* DrawMaterial, const TArray<FVector>& MeshVertices, const TArray<int32>& MeshIndices, bool WorldSpace/* = false*/)
{
	FDrawContext Context;
	if (WorldSpace)
	{
		TArray<FVector> LocalSpaceVertices(MeshIndices);

		const FTransform& Tr = GetComponentTransform();		
		for (int Index = 0; Index < LocalSpaceVertices.Num(); Index++)
		{
			LocalSpaceVertices[Index] = Tr.InverseTransformPosition(MeshVertices[Index]);
		}		
		Context.DrawMesh(LocalSpaceVertices, MeshIndices, DrawMaterial);
	}
	else
	{
		Context.DrawMesh(MeshVertices, MeshIndices, DrawMaterial);
	}
	
	ApplyContext(Context);
}

void USimpleMeshDrawingComponent::DrawToContext(FDrawContext& Context, UMaterialInterface* DrawMaterial, const TArray<FVector>& MeshVertices, const TArray<int32>& MeshIndices)
{
	Context.DrawMesh(MeshVertices, MeshIndices, DrawMaterial);
}

void USimpleMeshDrawingComponent::GetContextSectionSizes_Index(const FDrawContext& Context, int32 SectionIndex, int32& VerticesNum, int32& IndicesNum)
{
	if (SectionIndex >= Context.Sections.Num())
	{
		VerticesNum = INDEX_NONE;
		IndicesNum = INDEX_NONE;
		return;
	}
	int32 Index = 0;
	for (auto& Pair : Context.Sections)
	{
		if (Index == SectionIndex)
		{
			const FSection& MeshSection = Pair.Value;
			VerticesNum = MeshSection.Vertices.Num();
			IndicesNum = MeshSection.Indices.Num();
			return;
		}
		Index++;
	}
}

void USimpleMeshDrawingComponent::GetContextSectionSizes(const FDrawContext& Context, UMaterialInterface* DrawMaterial, int32& VerticesNum, int32& IndicesNum)
{
	if (const FSection* find = Context.Sections.Find(DrawMaterial))
	{
		VerticesNum = find->Vertices.Num();
		IndicesNum = find->Indices.Num();
		return;
	}

	VerticesNum = INDEX_NONE;
	IndicesNum = INDEX_NONE;
}




void USimpleMeshDrawingComponent::DrawLine(FDrawContext& Context, UMaterialInterface* DrawMaterial, FVector From, FVector To, float Size)
{
	TArray<FVector> Vertices;
	TArray<int32> Indices;

	GetLineMesh(From, To, Size, Vertices, Indices);

	Context.DrawMesh(Vertices, Indices, DrawMaterial);
}

void USimpleMeshDrawingComponent::DrawBox(FDrawContext& Context, UMaterialInterface* DrawMaterial, FVector BoxPosition, FVector BoxExtent)
{
	TArray<FVector> Vertices;
	TArray<int32> Indices;

	GetBoxMesh(BoxPosition, BoxExtent, Vertices, Indices);

	Context.DrawMesh(Vertices, Indices, DrawMaterial);
}



void USimpleMeshDrawingComponent::GetLineMesh(FVector From, FVector To, float Size, TArray<FVector>& OutVerts, TArray<int32>& OutIndices)
{
	FVector Dir = (To - From).GetSafeNormal(Size);
	if (Dir.IsNearlyZero())
	{
		Dir = FVector::ForwardVector;
		To = From + Dir * Size * 2;
	}

	FQuat Rotation = Dir.ToOrientationQuat();

	FVector Up = Rotation.GetUpVector();
	FVector Right = Rotation.GetRightVector();

	OutVerts =
	{
		From + Up * Size,
		From - Up * Size,
		To + Up * Size,
		To - Up * Size,

		From + Right * Size,
		From - Right * Size,
		To + Right * Size,
		To - Right * Size
	};

	OutIndices =
	{
		0, 1, 3,
		0, 3, 2,
		4, 5, 7,
		4, 7, 6
	};
}


void USimpleMeshDrawingComponent::GetBoxMesh(FVector Pos, FVector Extent, TArray<FVector>& OutVerts, TArray<int32>& OutIndices)
{
	FBox Box = FBox::BuildAABB(Pos, Extent);
	OutVerts =
	{
		FVector(Box.Min.X, Box.Min.Y, Box.Max.Z),
		FVector(Box.Max.X, Box.Min.Y, Box.Max.Z),
		FVector(Box.Min.X, Box.Min.Y, Box.Min.Z),
		FVector(Box.Max.X, Box.Min.Y, Box.Min.Z),
		FVector(Box.Min.X, Box.Max.Y, Box.Max.Z),
		FVector(Box.Max.X, Box.Max.Y, Box.Max.Z),
		FVector(Box.Min.X, Box.Max.Y, Box.Min.Z),
		FVector(Box.Max.X, Box.Max.Y, Box.Min.Z)
	};
	OutIndices =
	{
		3,2,0,
		3,0,1,
		7,3,1,
		7,1,5,
		6,7,5,
		6,5,4,
		2,6,4,
		2,4,0,
		1,0,4,
		1,4,5,
		7,6,2,
		7,2,3
	};
}