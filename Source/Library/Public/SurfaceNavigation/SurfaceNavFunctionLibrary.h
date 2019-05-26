// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SurfaceNavFunctionLibrary.generated.h"

struct FEdgeData;
class FSurfaceNavLocalData;

/**
 * 
 */
UCLASS()
class LIBRARY_API USurfaceNavFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:

	static void DrawSurfaceGraph(const UObject* WorldContext, const FTransform& Transform, const FSurfaceNavLocalData& NavData, float Duration = 5, FColor LinkColor = FColor::Green, FColor NodeColor = FColor::Blue);
};
