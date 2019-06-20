// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavigationVolume.h"
#include "Engine/CollisionProfile.h"
#include "Components/BoxComponent.h"


#include "SurfaceNavFunctionLibrary.h"
#include "SurfaceNavigationSystem.h"



ASurfaceNavigationVolume::ASurfaceNavigationVolume()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->Mobility = EComponentMobility::Static;
	Box->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Box->SetCanEverAffectNavigation(false);
	Box->SetBoxExtent(FVector(100));	

	RootComponent = Box;

#if WITH_EDITORONLY_DATA
	Status = TEXT("Add SurfaceNavigationSystem to the level");
#endif
}

void ASurfaceNavigationVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();	

	USurfaceNavigationSystem* NavSys = USurfaceNavFunctionLibrary::GetNavigationSystem(this);
	if (GIsEditor && NavSys)
	{
		NavSys->VolumeAdded(this);
	}

#if WITH_EDITORONLY_DATA
	Status = NavSys ? TEXT("Valid") : TEXT("SurfaceNavigationSystem is missing");
#endif
}

void ASurfaceNavigationVolume::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	USurfaceNavigationSystem* NavSys = USurfaceNavFunctionLibrary::GetNavigationSystem(this);
	if (GIsEditor && NavSys)
	{
		NavSys->VolumeRemoved(this);
	}
#if WITH_EDITORONLY_DATA
	Status = NavSys ? TEXT("Valid") : TEXT("SurfaceNavigationSystem is missing");
#endif
}

#if WITH_EDITOR
void ASurfaceNavigationVolume::PostEditUndo()
{
	Super::PostEditUndo();
	USurfaceNavigationSystem* NavSys = USurfaceNavFunctionLibrary::GetNavigationSystem(this);
	if (GIsEditor && NavSys)
	{
		NavSys->VolumeUpdated(this);
	}
}

void ASurfaceNavigationVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	USurfaceNavigationSystem* NavSys = USurfaceNavFunctionLibrary::GetNavigationSystem(this);
	if (GIsEditor && NavSys)
	{
		NavSys->VolumeUpdated(this);
	}
}
#endif // WITH_EDITOR

void ASurfaceNavigationVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	USurfaceNavigationSystem* NavSys = USurfaceNavFunctionLibrary::GetNavigationSystem(this);
	if (GIsEditor && NavSys)
	{
		NavSys->VolumeUpdated(this);
	}	
}







