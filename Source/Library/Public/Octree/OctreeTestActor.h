// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OctreeTestActor.generated.h"

class UOctreeTesterComponent;

UCLASS()
class LIBRARY_API AOctreeTestActor : public AActor
{
	GENERATED_BODY()
	

	UPROPERTY(Category = "Tester", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UOctreeTesterComponent* Tester;

	// Sets default values for this actor's properties
	AOctreeTestActor();

	

public:
	virtual void OnConstruction(const FTransform& Transform) override;

};
