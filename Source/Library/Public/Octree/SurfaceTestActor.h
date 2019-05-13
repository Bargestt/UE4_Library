// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurfaceTestActor.generated.h"

class USurfaceTesterComponent;

UCLASS()
class LIBRARY_API ASurfaceTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(Category = "Tester", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USurfaceTesterComponent* Tester;

	// Sets default values for this actor's properties
	ASurfaceTestActor();


	virtual void OnConstruction(const FTransform& Transform) override;
	
};
