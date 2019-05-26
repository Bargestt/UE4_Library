// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavigationVolume.h"
#include "Engine/CollisionProfile.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"

#include "SurfaceNavigation/SurfaceNavFunctionLibrary.h"
#include "SurfaceNavigation/SurfaceNavLocalData.h"
#include "MarchingCubesFunctionLibrary.h"
#include "SurfaceNavigation/SurfaceNavBuilder.h"






ASurfaceNavigationVolume::ASurfaceNavigationVolume()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->Mobility = EComponentMobility::Static;
	Box->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Box->SetCanEverAffectNavigation(false);
	Box->SetBoxExtent(FVector(100));

	RootComponent = Box;


	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("Material'/Game/Base/Materials/MTwoSided.MTwoSided'"));
	if (Mat.Succeeded()) 
	{
		Mesh->SetMaterial(0, Mat.Object);
	}

	bShowMesh = true;
	bShowGraph = true;
	SurfaceValue = .5f;
}



void ASurfaceNavigationVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CreateNavData();
}


void ASurfaceNavigationVolume::CreateNavData()
{
	Dimensions = FIntVector(GetBoxComponent()->GetScaledBoxExtent() * 2 / StepSize);
	SurfaceValue = FMath::Clamp(SurfaceValue, 0.01f, 0.99f);

	TArray<FVector4> Points;
	SamplePoints(Dimensions, StepSize, Points);

	FSurfaceNavBuilder Builder(NavData);
	Builder.BuildGraph(Points, Dimensions, SurfaceValue);

	
	SetupMeshFromPoints(Points, Dimensions);
	if (bShowGraph) 
	{

		FlushPersistentDebugLines(GetWorld());
		USurfaceNavFunctionLibrary::DrawSurfaceGraph(this, GetActorTransform(), NavData);
	}
}




void ASurfaceNavigationVolume::SetupMeshFromPoints(const TArray<FVector4>& Points, const FIntVector& Dimensions)
{
	if (bShowMesh) 
	{
		const UWorld* World = GetWorld();
		const FTransform ToWorld = GetActorTransform();

		TArray<FVector> Vertices;
		TArray<int32> Indices;

		UMarchingCubesFunctionLibrary::GenerateMesh(Points, Dimensions, SurfaceValue, Vertices, Indices);
		Mesh->CreateMeshSection(0, Vertices, Indices, {}, {}, {}, {}, {}, {}, {}, false);
		Mesh->SetWorldScale3D(FVector(1));
	}

	Mesh->SetVisibility(bShowMesh);
}

void ASurfaceNavigationVolume::SamplePoints(const FIntVector& Dimensions, float CellSize, TArray<FVector4>& OutPoints) const
{
	const UWorld* World = GetWorld();

	const FVector CellOffset = -GetBoxComponent()->GetScaledBoxExtent() + CellSize / 2;

	const FTransform ToWorld = GetActorTransform();

	const FCollisionShape TestBox = FCollisionShape::MakeBox(FVector(CellSize / 2));

	FCollisionObjectQueryParams QueryParameters;
	QueryParameters.AddObjectTypesToQuery(ECC_WorldStatic);
	QueryParameters.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams Params;
	Params.bFindInitialOverlaps = true;


	OutPoints.Reset(Dimensions.X*Dimensions.Y*Dimensions.Z);
	for (int Z = 0; Z < Dimensions.Z; Z++)
	{
		for (int Y = 0; Y < Dimensions.Y; Y++)
		{
			for (int X = 0; X < Dimensions.X; X++)
			{
				FVector TestLocation = FVector(X, Y, Z) * CellSize + CellOffset;
				bool WasOverlap = World->OverlapAnyTestByObjectType(ToWorld.TransformPositionNoScale(TestLocation), ToWorld.GetRotation(), QueryParameters, TestBox, Params);

				OutPoints.Add(FVector4(TestLocation, WasOverlap ? 1 : 0));
			}
		}
	}
}





FIntVector ASurfaceNavigationVolume::ToCellCoordinates(const FVector& Location, bool WorldSpace /*= true*/) const
{
	FVector LocalPosition = WorldSpace ? GetActorTransform().InverseTransformPositionNoScale(Location) : Location;

	return FIntVector((LocalPosition + GetBoxComponent()->GetUnscaledBoxExtent() - FVector(StepSize / 2)) / StepSize);
}

FVector ASurfaceNavigationVolume::ToWorldCoordinates(const FIntVector& CellCoordinates) const
{
	const FVector CellOffset = -GetBoxComponent()->GetScaledBoxExtent() + StepSize / 2;

	FVector LocalLocation = FVector(CellCoordinates) * StepSize + CellOffset;

	return GetActorTransform().TransformPositionNoScale(LocalLocation);
}




bool ASurfaceNavigationVolume::IsCoordinatesValid(const FIntVector& CellCoordinates)
{
	return
		CellCoordinates.X >= 0 && CellCoordinates.X < Dimensions.X &&
		CellCoordinates.Y >= 0 && CellCoordinates.Y < Dimensions.Y &&
		CellCoordinates.Z >= 0 && CellCoordinates.Z < Dimensions.Z;
}