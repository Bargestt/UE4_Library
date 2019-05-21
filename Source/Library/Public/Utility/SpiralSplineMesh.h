// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include <Components/SplineComponent.h>
#include "SpiralSplineMesh.generated.h"

DECLARE_STATS_GROUP(TEXT("SpiralSplineMesh"), STATGROUP_SpiralSplineMesh, STATCAT_Advanced);

class USplineComponent;
class USplineMeshComponent;
struct FSplinePoint;

/**
 * Generated Spiral wound around spline
 */
UCLASS( ClassGroup=(Utilty), meta=(BlueprintSpawnableComponent) )
class LIBRARY_API USpiralSplineMesh : public USplineComponent
{
	GENERATED_BODY()

	UPROPERTY()
	USplineComponent* Spiral;

	UPROPERTY()
	TArray<USplineMeshComponent*> Segments;

	UPROPERTY(VisibleAnywhere, Category = "Debug")
	int VisibleSegments;
	UPROPERTY(VisibleAnywhere, Category = "Debug")
	int HiddenSegments;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	UMaterialInterface* SegmentMaterial;

	/** Spiral revolutions in done */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral")
	float Revolutions;
	/** Start revolutions from this angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral")
	float StartAngle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral")
	float EndPointRadius;

	/** Controls transition sharpness from straight spline to spiral */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral")
	float TransitionExponent;

	/** How often spline points should be created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral")
	float BaseStep;
	/** Modify BaseStep when spline is close to spiral form */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiral", AdvancedDisplay)
	float SpiralBaseStepMod;

	/** Limit spiral size */
	UPROPERTY(EditAnywhere, Category = "Spiral", AdvancedDisplay)
	int MaxPointsNum;

public:
	// Sets default values for this component's properties
	USpiralSplineMesh();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	

	virtual void OnVisibilityChanged() override;

public:
	virtual void OnRegister() override;

	virtual void SetMaterial(int32 ElementIndex, class UMaterialInterface* Material) override;

protected:	

	void UpdateSplinePoints() const;

	void CreateSplineMeshes();

	void UpdateSplineMeshes(int StartFrom = 0);
	
public:		
	UFUNCTION(BlueprintCallable, Category = "Spline|Spiral")
	void CopyFromOther(const USpiralSplineMesh* Other);

	/** Call this function after changing parameters */
	UFUNCTION(BlueprintCallable, Category = "Spline|Spiral")
	void UpdateSpiral(bool UpdateMesh);

	
	UFUNCTION(BlueprintCallable, Category = "Spline|Spiral")
	USplineComponent* GetSpiral()const;

	UFUNCTION(BlueprintCallable, Category = "Spline|Spiral")
	TArray<USplineMeshComponent*> GetSegments() const;

	UFUNCTION(BlueprintCallable, Category = "Spline|Spiral")
	void SetMesh(UStaticMesh* NewMesh);

};
