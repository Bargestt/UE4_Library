// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceTestActor.h"
#include "SurfaceTesterComponent.h"


// Sets default values
ASurfaceTestActor::ASurfaceTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Tester = CreateDefaultSubobject<USurfaceTesterComponent>(TEXT("Tester"));
}



void ASurfaceTestActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Tester->UpdateCells();
	Tester->GenerateFaces();
}

