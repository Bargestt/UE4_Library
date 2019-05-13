// Fill out your copyright notice in the Description page of Project Settings.

#include "OctreeTestActor.h"
#include "OctreeTesterComponent.h"

// Sets default values
AOctreeTestActor::AOctreeTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Tester = CreateDefaultSubobject<UOctreeTesterComponent>(TEXT("Tester"));
}

void AOctreeTestActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Tester->UpdateCells();
}


