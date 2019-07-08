// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceSampler.h"
#include "DrawDebugHelpers.h"






void FSamplingTask::SampleData(FSamplerResult& OutResult)
{
	float VoxelSize = FMath::Max(FMath::Abs(Parameters.VoxelSize), 10.0f);

	const FVector BoxCenter = SampleBox.GetCenter();
	const FVector BoxExtent = SampleBox.GetExtent();


	const FVector CellSize = FVector(VoxelSize);
	const FVector CellOffset = -BoxExtent + CellSize / 2;

	FIntVector Dimensions = FIntVector(BoxExtent * 2 / CellSize);


	const FCollisionShape TestBox = FCollisionShape::MakeBox(FVector(CellSize / 2));

	FCollisionObjectQueryParams QueryParameters;
	QueryParameters.AddObjectTypesToQuery(ECC_WorldStatic);
	QueryParameters.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams Params;
	Params.bFindInitialOverlaps = true;

	OutResult.Box = SampleBox;
	OutResult.Dimensions = Dimensions;
	OutResult.Points.Reset(Dimensions.X*Dimensions.Y*Dimensions.Z);
	for (int Z = 0; Z < Dimensions.Z; Z++)
	{
		for (int Y = 0; Y < Dimensions.Y; Y++)
		{
			for (int X = 0; X < Dimensions.X; X++)
			{
				FVector TestLocation = FVector(X, Y, Z) * CellSize + CellOffset;
				FVector WorldLocation = BoxCenter + TestLocation;
				bool WasOverlap = World->OverlapAnyTestByObjectType(WorldLocation, FQuat::Identity, QueryParameters, TestBox, Params);

				if (Parameters.bDrawDebug)
				{
					DrawDebugPoint(World, WorldLocation, 5, FColor::White, false, 10);
				}

				OutResult.Points.Add(FVector4(TestLocation, WasOverlap ? 1 : 0));
			}
		}
	}
	if (Parameters.bDrawDebug)
	{
		DrawDebugBox(World, BoxCenter, BoxExtent, FQuat::Identity, FColor::White, false, 10, 0, 1);
	}

	UE_LOG(SurfaceNavigation, Log, TEXT("Sampled box:%s, voxel:%.5g, dimensions:%s, pointsNum:%d"), *OutResult.Box.GetExtent().ToString(), VoxelSize, *OutResult.Dimensions.ToString(), OutResult.Points.Num());
		
}

