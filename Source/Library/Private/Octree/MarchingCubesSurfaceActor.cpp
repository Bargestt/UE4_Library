// Fill out your copyright notice in the Description page of Project Settings.

#include "MarchingCubesSurfaceActor.h"
#include "OctreeTesterComponent.h"
#include "MarchingCubesFunctionLibrary.h"
#include "ProceduralMeshComponent.h"


// Sets default values
AMarchingCubesSurfaceActor::AMarchingCubesSurfaceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Tester = CreateDefaultSubobject<UOctreeTesterComponent>(TEXT("Tester"));
	RootComponent = Tester;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

}

void AMarchingCubesSurfaceActor::GenerateMesh()
{
	TArray<TArray<TArray<FVector4>>> Points;

	const FIntVector& Size = Tester->Result.Dimensions;

	Points.SetNum(Size.X);
	for (int X = 0; X < Size.X; X++)
	{
		Points[X].SetNum(Size.Y);
		for (int Y = 0; Y < Size.Y; Y++)
		{
			Points[X][Y].SetNum(Size.Z);
			for (int Z = 0; Z < Size.Z; Z++)
			{
				const FTesterCell& cell = Tester->Result.GetCell(X, Y, Z);
				Points[X][Y][Z] = FVector4(cell.RelativeLocation, cell.IsSafe ? 2 : 0);
			}
		}
	}


	TArray<FVector> Vertices;
	TArray<int32> Indices;
	UMarchingCubesFunctionLibrary::GenerateMesh(Points, 1, Vertices, Indices);

	Mesh->CreateMeshSection(0, Vertices, Indices, {}, {}, {}, {}, {}, {}, {}, false);
}

void AMarchingCubesSurfaceActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Tester->UpdateCells();

	GenerateMesh();
}


