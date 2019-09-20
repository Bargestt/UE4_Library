// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurfaceSampler.h"
#include "MeshToGraphTest.generated.h"

class USurfaceSamplerBase;
class USimpleMeshDrawingComponent;

UCLASS()
class LIBRARY_API AMeshToGraphTest : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Instanced, meta=(AllowPrivateAccess))
	USurfaceSamplerBase* Sampler;

	UPROPERTY()
	USimpleMeshDrawingComponent* Mesh;
	
public:	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;


	// Sets default values for this actor's properties
	AMeshToGraphTest();
	

	virtual void OnConstruction(const FTransform& Transform) override;


	UFUNCTION(CallInEditor)
	void SheduleSample();
protected:
	
	void SampleFinished(FSamplerResult Result);



	struct FGraphNode
	{
		int32 Vertices[3] = { -1, -1, -1 };

		TArray<int32> Neighbours;

		FGraphNode() {}		

		bool HasVertice(int32 OtherVertice) const
		{			
			if (OtherVertice < 0) return false;
			return OtherVertice == Vertices[0] || OtherVertice == Vertices[1] || OtherVertice == Vertices[2];
		}
	};

	struct FGraph
	{
		TArray<FGraphNode> Nodes;

		TArray<FVector> Vertices;

		FVector GetVertex(int32 NodeRef, int8 Vertex) const { return Vertices[Nodes[NodeRef].Vertices[Vertex]];	}

		FVector GetCenter(int32 NodeRef) const { return (GetVertex(NodeRef, 0) + GetVertex(NodeRef, 1) + GetVertex(NodeRef, 2)) / 3; }

		FVector GetCenter(int32 NodeRef, float NormalOffset) const { return GetCenter(NodeRef) + GetNormal(NodeRef)*NormalOffset; }

		FVector GetNormal(int32 NodeRef) const { return FVector::CrossProduct(GetVertex(NodeRef, 2) - GetVertex(NodeRef, 0), GetVertex(NodeRef, 1) - GetVertex(NodeRef, 0)).GetSafeNormal(); }
	
	};

	bool CreateGraph(const TArray<FVector>& Vertices, const TArray<int32>& Indices, FGraph& OutGraph);


	int32 RemoveDoubles(TArray<FVector>& Vertices, TArray<int32>& Indices);


	// Mesh simpolify

};
