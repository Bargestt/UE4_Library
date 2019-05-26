// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurfaceNavLocalData.h"
#include "SurfaceNavigationVolume.generated.h"

class UBoxComponent;
class UProceduralMeshComponent;




/**
 * 
 */
UCLASS(hidecategories = (Object, Display, Rendering, Physics, Input, Blueprint), ConversionRoot, NotBlueprintable)
class LIBRARY_API ASurfaceNavigationVolume : public AActor
{
	GENERATED_BODY()

	FSurfaceNavLocalData NavData;

	UPROPERTY(VisibleAnywhere, Category=Collision, meta=(AllowPrivateAccess = "true"))
	UBoxComponent* Box;

	UPROPERTY()
	UProceduralMeshComponent* Mesh;

public:

	UPROPERTY(EditAnywhere, Category = "SurfaceNavigation")
	bool bShowMesh;

	UPROPERTY(EditAnywhere, Category = "SurfaceNavigation")
	bool bShowGraph;

	UPROPERTY(EditAnywhere, Category = "SurfaceNavigation")
	float StepSize = 50;

	UPROPERTY(EditAnywhere, Category = "SurfaceNavigation", meta=(ClampMin = 0.01, ClampMax = 0.99))
	float SurfaceValue;

	UPROPERTY(VisibleAnywhere, Category = "SurfaceNavigation")
	FIntVector Dimensions;


public:

	ASurfaceNavigationVolume();

	virtual void OnConstruction(const FTransform& Transform) override;

	UBoxComponent* GetBoxComponent() const { return Box; }


protected:
	void CreateNavData();

	void SetupMeshFromPoints(const TArray<FVector4>& Points, const FIntVector& Dimensions);

	void SamplePoints(const FIntVector& Dimensions, float CellSize, TArray<FVector4>& OutPoints) const;

	
public:

	// Interface functions


	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation")
	FIntVector ToCellCoordinates(const FVector& Location, bool WorldSpace = true) const;

	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation")
	FVector ToWorldCoordinates(const FIntVector& CellCoordinates) const;

	UFUNCTION(BlueprintCallable, Category = "SurfaceNavigation")
	bool IsCoordinatesValid(const FIntVector& CellCoordinates);

};
