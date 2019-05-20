// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MarchingCubesSurfaceActor.generated.h"

class UOctreeTesterComponent;
class UProceduralMeshComponent;

/** Generates surface using Marching cubes algorithm and data from OctreeTester */
UCLASS()
class LIBRARY_API AMarchingCubesSurfaceActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(Category = "Tester", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UOctreeTesterComponent* Tester;

	UPROPERTY()
	UProceduralMeshComponent* Mesh;
	
public:	
	// Sets default values for this actor's properties
	AMarchingCubesSurfaceActor();

	UFUNCTION(BlueprintCallable, Category = MarchingCubesSurfaceActor)
	void GenerateMesh();
	
	
public:
	virtual void OnConstruction(const FTransform& Transform) override;

};
