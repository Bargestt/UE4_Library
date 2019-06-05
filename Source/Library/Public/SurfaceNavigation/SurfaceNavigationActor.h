// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SurfaceNavigation.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurfaceNavigationActor.generated.h"

class USurfaceNavigationSystem;





/**
 * 
 */
UCLASS(NotBlueprintable, autoExpandCategories=("SurfaceNavigation", "NavigationSystem"), hideCategories=("Rendering", "LOD", "Cooking", "Input"))
class LIBRARY_API ASurfaceNavigationActor : public AActor
{
	GENERATED_BODY()


#if WITH_EDITOR
	UPROPERTY()
	UBillboardComponent* BillboardVisual;
#endif // 


	UPROPERTY(EditInstanceOnly, Instanced, Category = "SurfaceNavigation", meta = (AllowPrivateAccess))
	USurfaceNavigationSystem* NavigationSystem;

public:
	ASurfaceNavigationActor();


	FORCEINLINE USurfaceNavigationSystem* GetNavigationSystem() const { return NavigationSystem; }
};
