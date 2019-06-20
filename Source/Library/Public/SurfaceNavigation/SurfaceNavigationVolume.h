// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SurfaceNavigationVolume.generated.h"

class UBoxComponent;

/**
 * Basic Surface Navigation volume
 * Requires NavigationSystem actor in world to work
 */
UCLASS(hidecategories = (Object, Display, Rendering, Physics, Input, Blueprint), ConversionRoot, NotBlueprintable)
class LIBRARY_API ASurfaceNavigationVolume : public AActor
{
	GENERATED_BODY()
	
#if WITH_EDITORONLY_DATA	
	UPROPERTY(VisibleAnywhere, Category = "SurfaceNavigation")
	FString Status;
#endif //  WITH_EDITORONLY_DATA

	UPROPERTY(VisibleAnywhere, Category="SurfaceNavigation", meta=(AllowPrivateAccess = "true"))
	UBoxComponent* Box;

public:

	ASurfaceNavigationVolume();

	virtual void PostRegisterAllComponents() override;

	virtual void PostUnregisterAllComponents() override;

#if WITH_EDITOR
	virtual void PostEditUndo() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	virtual void OnConstruction(const FTransform& Transform) override;


	UBoxComponent* GetBoxComponent() const { return Box; }
};
