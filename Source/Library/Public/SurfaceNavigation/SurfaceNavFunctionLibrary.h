// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SurfaceNavigationSystem.h"
#include "SurfaceNavFunctionLibrary.generated.h"

struct FEdgeData;

/**
 * 
 */
UCLASS()
class LIBRARY_API USurfaceNavFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation", meta = (WorldContext = "WorldContextObject"))
	static USurfaceNavigationSystem* GetNavigationSystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation", meta = (WorldContext = "WorldContextObject"))
	static void FindPathSync(const UObject* WorldContextObject, FVector From, FVector To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters);


	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation", meta = (WorldContext = "WorldContextObject"))
	static bool ProjectOnNavigation(const UObject* WorldContextObject, FVector SourceLocation, FVector& OutLocation);

	static void DrawSurfaceGraph(const UObject* WorldContext, const FTransform& Transform, const FSurfaceNavLocalData& NavData, float Duration = 5, FColor LinkColor = FColor::Green, FColor NodeColor = FColor::Blue);
};
