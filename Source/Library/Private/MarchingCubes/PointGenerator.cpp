// Fill out your copyright notice in the Description page of Project Settings.

#include "PointGenerator.h"
#include "MarchingCubesFunctionLibrary.h"



FPointsArray3D UPointGenerator::GeneratePoints(FIntVector Size, float CellSize)
{
	FPointsArray3D Points;

	Points.Points.SetNum(Size.X);
	for (int X = 0; X < Size.X; X++)
	{
		Points.Points[X].SetNum(Size.Y);
		for (int Y = 0; Y < Size.Y; Y++)
		{
			Points.Points[X][Y].Init(FVector4(0), Size.Z);
			for (int Z = 0; Z < Size.Z; Z++)
			{
				Points.Points[X][Y][Z] = FVector4(FVector(X, Y, Z)*CellSize, GeneratePoint(FIntVector(X, Y, Z), Size));
			}
		}
	}

	return Points;
}





float UPointGenerator::GeneratePoint_Implementation(FIntVector Coordinates, FIntVector Size) const
{
	return FMath::RandRange(0, 16);
}






