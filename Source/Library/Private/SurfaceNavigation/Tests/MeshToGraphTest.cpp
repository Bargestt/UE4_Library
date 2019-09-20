// Fill out your copyright notice in the Description page of Project Settings.

#include "MeshToGraphTest.h"
#include "SurfaceSampler.h"
#include "Components/BoxComponent.h"
#include "SimpleMeshDrawingComponent.h"
#include "DrawDebugHelpers.h"

#include "MarchingCubesBuilder.h"



// Sets default values
AMeshToGraphTest::AMeshToGraphTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Sampler = CreateDefaultSubobject<USurfaceSamplerBase>(TEXT("Sampler"));
	Sampler->VoxelSize = 50;

	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(FVector(250));
	Box->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = Box;

	Mesh = CreateDefaultSubobject<USimpleMeshDrawingComponent>(TEXT("Display"));
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetRelativeLocation(FVector::ZeroVector);

	Material = UMaterial::GetDefaultMaterial(MD_Surface);
}



void AMeshToGraphTest::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Mesh->SetRelativeLocation(FVector::ZeroVector);

	SheduleSample();
}

void AMeshToGraphTest::SheduleSample()
{
	FVector Extent = GetActorTransform().TransformVector(RootComponent->Bounds.BoxExtent);
	Sampler->ScheduleSampleTask(FBox::BuildAABB(GetActorLocation(), Extent), this, &AMeshToGraphTest::SampleFinished);
}

void AMeshToGraphTest::SampleFinished(FSamplerResult Result)
{	
	Mesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Indices;


 	FMarchingCubesBuilder Builder(Result.Points, Result.Dimensions);
	Builder.FindBoundaryEdges = true;
 	Builder.Build();
 	Builder.GetData(Vertices, Indices);

	Mesh->CreateMeshSection(0, Vertices, Indices, {}, {}, {}, {}, {}, {}, {}, false);
	Mesh->SetMaterial(0, Material);

	TArray<int32> OuterIndices;
	Builder.GetOuterVertices(OuterIndices);
	for (int32 Index : OuterIndices)
	{
		if (Vertices.IsValidIndex(Index)) 
		{
			DrawDebugPoint(GetWorld(), GetActorTransform().TransformPosition(Vertices[Index]), 15, FColor::Green, false, 5);
		}
	}

	FGraph Graph;
	CreateGraph(Vertices, Indices, Graph);

	float Time = 5;
	for (int Index = 0; Index < Graph.Nodes.Num() ; Index++)
	{
		FVector Center = GetActorTransform().TransformPosition(Graph.GetCenter(Index, 5));
		DrawDebugPoint(GetWorld(), Center, 15, FColor::Blue, false, Time);

		for (int32 Adj : Graph.Nodes[Index].Neighbours)
		{
			if (Graph.Nodes.IsValidIndex(Adj))
			{
				DrawDebugLine(GetWorld(), Center, GetActorTransform().TransformPosition(Graph.GetCenter(Adj, 5)), FColor::Cyan, false, Time, 0, 2);
			}
		}
	}
	
}

bool AMeshToGraphTest::CreateGraph(const TArray<FVector>& Vertices, const TArray<int32>& Indices, FGraph& OutGraph)
{
	FGraph Graph;
	Graph.Vertices = Vertices;
	
	for (int NodeIndex = 0; NodeIndex < Indices.Num() ; NodeIndex+=3)
	{
		FGraphNode Node;

		Node.Vertices[0] = Indices[NodeIndex + 0];
		Node.Vertices[1] = Indices[NodeIndex + 1];
		Node.Vertices[2] = Indices[NodeIndex + 2];
		Graph.Nodes.Add(Node);
	}

	// Dumb method as proof of concept
	for (int NodeIndex = 0; NodeIndex < Graph.Nodes.Num() ; NodeIndex++)
	{
		FGraphNode& CurrentNode = Graph.Nodes[NodeIndex];

		for (int Index = 0; Index < Graph.Nodes.Num() ; Index++)
		{
			if(Index == NodeIndex) continue;

			FGraphNode& OtherNode = Graph.Nodes[Index];

			int Matches = 0;
			Matches += CurrentNode.HasVertice(OtherNode.Vertices[0]);
			Matches += CurrentNode.HasVertice(OtherNode.Vertices[1]);
			Matches += CurrentNode.HasVertice(OtherNode.Vertices[2]);

			if(Matches >= 2)
			{
				CurrentNode.Neighbours.AddUnique(Index);
			}	
		}
	}	

	OutGraph = Graph;
	return true;
}





int32 AMeshToGraphTest::RemoveDoubles(TArray<FVector>& Vertices, TArray<int32>& Indices)
{
	//return -1;

	TArray<FVector> UniqueVertices;
	TArray<int32> IndexRemap;

	UniqueVertices.Reserve(Vertices.Num());
	IndexRemap.Init(0, Vertices.Num());

	int32 DuplicateCount = 0;
	for (int Index = 0; Index < Vertices.Num() ; Index++)
	{
		int32 UniqIndex = UniqueVertices.Find(Vertices[Index]);
		if (UniqIndex < 0)
		{
			UniqIndex = UniqueVertices.Add(Vertices[Index]);
		}
		else
		{
			DuplicateCount++;
		}
		IndexRemap[Index] = UniqIndex;
	}

	for (int Index = 0; Index < Indices.Num() ; Index++)
	{
		Indices[Index] = IndexRemap[Indices[Index]];
	}
	Vertices = UniqueVertices;

	return DuplicateCount;
}

//////////////////////////////////////////////////////////////////////////


