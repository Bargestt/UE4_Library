// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SurfaceNavigation.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SurfaceNavLocalData.h"
#include "SurfaceNavigationSystem.generated.h"




USTRUCT(BlueprintType)
struct LIBRARY_API FSurfacePathfindingParams
{
	GENERATED_BODY()
};


USTRUCT(BlueprintType)
struct LIBRARY_API FSurfacePathfindingResult
{
	GENERATED_BODY()
};



/**
 * 
 */
UCLASS(collapseCategories)
class LIBRARY_API USurfaceNavigationSystem : public UObject
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "NavigationSystem")
	FString Info;




	FSurfaceNavLocalData NavigationData;

public:
	USurfaceNavigationSystem();

	void FindPathSync(const FVector& From, const FVector& To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters) const;



public:

	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation", meta = (WorldContext = "WorldContextObject"))
	static USurfaceNavigationSystem* GetNavigationSystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation", meta = (WorldContext = "WorldContextObject"))
	static void FindPathSync(const UObject* WorldContextObject, const FVector& From, const FVector& To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters);
};
