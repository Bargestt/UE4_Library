// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MarchingCubesDisplay.generated.h"

class UBoxComponent;
class UProceduralMeshComponent;
class UMaterialInterface;

USTRUCT()
struct FSection
{
	GENERATED_BODY()
	TArray<FVector> Vertices;
	TArray<int32> Indices;
};


USTRUCT()
struct FDrawContext
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<UMaterialInterface*, FSection> Sections;

	FDrawContext(){	}

	void DrawMesh(const TArray<FVector>& Verts, const TArray<int32>& Indices, UMaterialInterface* Material);

};

/**
 * Marching cube combination demonstration
 */
UCLASS()
class LIBRARY_API AMarchingCubesDisplay : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	UProceduralMeshComponent* Mesh;

	UPROPERTY()
	UBoxComponent* BoxFrame;	


	UPROPERTY()
	UMaterialInterface* MeshMaterial;
	UPROPERTY()
	UMaterialInterface* RedMaterial;
	UPROPERTY()
	UMaterialInterface* GreenMaterial;
	UPROPERTY()
	UMaterialInterface* BlueMaterial;



	TArray<FVector, TFixedAllocator<8>> CubeVertices;

	float CubeSize;



	UPROPERTY()
	TArray<UMaterialInterface*> Materials;

	FDrawContext Context;

public:
	
	UPROPERTY(EditAnywhere, Category = "Configuration", EditFixedSize)
	TArray<bool> CubeVertexStatus;

	UPROPERTY(EditAnywhere, Category = "Configuration", meta=(ClampMin = 0, ClampMax = 255))
	int32 ConfigNumber;

	UPROPERTY(EditAnywhere, Category = "Configuration")
	bool bShowAll;

public:	
	// Sets default values for this actor's properties
	AMarchingCubesDisplay();
	
public:
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;


private:
	
	void DrawConfig(TArray<bool> Config, FVector Offset = FVector(0));

	void DrawAllCases();



	void DrawMesh(const TArray<FVector>& Verts, const TArray<int32>& Indices, UMaterialInterface* DrawMaterial = nullptr);

	void DrawLine(FVector From, FVector To, float LineSize = 5);

	void DrawBox(FVector Pos, float Size);

	
	
	static void GetLineMesh(FVector From, FVector To, float Size, TArray<FVector>& OutVerts, TArray<int32>& OutIndices);

	static void GetBoxMesh(FVector Pos, float Size, TArray<FVector>& OutVerts, TArray<int32>& OutIndices);

	void ApplyContext(FDrawContext& Context);


};
