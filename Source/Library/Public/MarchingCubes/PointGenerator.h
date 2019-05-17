// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MarchingCubesFunctionLibrary.h"
#include "PointGenerator.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LIBRARY_API UPointGenerator : public UObject
{
	GENERATED_BODY()	

public:

	UFUNCTION(BlueprintCallable, Category = "PointGenerator")
	FPointsArray3D GeneratePoints(FIntVector Size, float CellSize = 50);


protected:	
	
	UFUNCTION(BlueprintNativeEvent)
	float GeneratePoint(FIntVector Coordinates, FIntVector Size) const;



};
