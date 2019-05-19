// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GraphNode.generated.h"






/**
 * Just fiddling arount to know better A* Algorithm works
 * Won't use because much better implementations are exist
 * Ex. template version in engine
 */
class UStaticMeshComponent;

UCLASS()
class LIBRARY_API AGraphNode : public AActor
{
	GENERATED_BODY()

	UStaticMeshComponent* Visual;	

public:
	UPROPERTY(EditAnywhere)
	bool bAutoLink;

	UPROPERTY(EditAnywhere)
	float AutoLinkRadius;


	UPROPERTY(EditAnywhere)
	TArray<AGraphNode*> Neighbours;

	
public:	
	// Sets default values for this actor's properties
	AGraphNode();	   	  	

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(CallInEditor, Category = "Nodes")
	void AutoLink();

	UFUNCTION(CallInEditor, Category = "Nodes")
	void DrawAllLinks();

	UFUNCTION(CallInEditor, Category = "Nodes")
	void AutoLinkAll();

public:

	AGraphNode* Parent;
	float F;
	float G;
	float H = MAX_FLT;

	void ResetPathValues();
	void ResetAll();



	UPROPERTY(EditAnywhere, Category = "PathFind")
	AGraphNode* Target;

	UFUNCTION(CallInEditor, Category = "PathFind")
	void FindPathToTarget();


protected:

	static bool AStar(AGraphNode* From, AGraphNode* To, TArray<AGraphNode*>& OutPath);
};

