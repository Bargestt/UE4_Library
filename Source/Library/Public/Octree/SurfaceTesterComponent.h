// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Octree/OctreeTesterComponent.h"
#include "SurfaceTesterComponent.generated.h"


struct FFace
{
	FVector Origin;
	FQuat Orient;
};

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class LIBRARY_API USurfaceTesterComponent : public UOctreeTesterComponent
{
	GENERATED_BODY()

	TArray<FFace> Faces;

public:

	UPROPERTY(EditAnywhere, Category = "SurfaceTester")
	bool ShowOctreeCells;
	
public:
	USurfaceTesterComponent();

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	UFUNCTION(BlueprintCallable, Category = "SurfaceTester")
	void GenerateFaces();

	TArray<FFace> GetFaces() const { return Faces; }
};
