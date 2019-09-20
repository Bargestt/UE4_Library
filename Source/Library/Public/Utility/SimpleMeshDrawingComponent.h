// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "SimpleMeshDrawingComponent.generated.h"

USTRUCT()
struct FSection
{
	GENERATED_BODY()

	TArray<FVector> Vertices;
	TArray<int32> Indices;
};

/** Draw meshes here before applying it to USimpleMeshDrawingComponent */
USTRUCT(BlueprintType)
struct FDrawContext
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<UMaterialInterface*, FSection> Sections;

	FDrawContext() {}

	void DrawMesh(const TArray<FVector>& Verts, const TArray<int32>& Indices, UMaterialInterface* Material);

};

/**
 * Use procedural mesh component to draw primitives
 * When creating mesh proxy is too long and complicated * 
 */
UCLASS(classGroup="Utility", Blueprintable, Blueprintable, meta=(BlueprintSpawnableComponent))
class LIBRARY_API USimpleMeshDrawingComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<UMaterialInterface*> Materials;
	

public:

	/** Finalize drawing operations in context */
	UFUNCTION(BlueprintCallable, Category="Components|SimpleMeshDrawing")
	void ApplyContext(FDrawContext Context);


	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	void DrawImmediately(UMaterialInterface* DrawMaterial, const TArray<FVector>& MeshVertices, const TArray<int32>& MeshIndices, bool WorldSpace = false);


	/** Draw mesh to context */
	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void DrawToContext(UPARAM(ref) FDrawContext& Context, UMaterialInterface* DrawMaterial, const TArray<FVector>& MeshVertices, const TArray<int32>& MeshIndices);

	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void GetContextSectionSizes_Index(const FDrawContext& Context, int32 SectionIndex, int32& VerticesNum, int32& IndicesNum);
	
	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void GetContextSectionSizes(const FDrawContext& Context, UMaterialInterface* DrawMaterial, int32& VerticesNum, int32& IndicesNum);


	//////////////////////////////////////////////////////////////////////////
	////		 Drawing functions
	//////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void DrawLine(UPARAM(ref) FDrawContext& Context, UMaterialInterface* DrawMaterial, FVector From, FVector To, float Size);

	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void DrawBox(UPARAM(ref) FDrawContext& Context, UMaterialInterface* DrawMaterial, FVector BoxPosition, FVector BoxExtent);


	//////////////////////////////////////////////////////////////////////////
	////		Primitive generation
	//////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void GetLineMesh(FVector From, FVector To, float Size, TArray<FVector>& OutVerts, TArray<int32>& OutIndices);

	UFUNCTION(BlueprintCallable, Category = "Components|SimpleMeshDrawing")
	static void GetBoxMesh(FVector Pos, FVector Extent, TArray<FVector>& OutVerts, TArray<int32>& OutIndices);
};
