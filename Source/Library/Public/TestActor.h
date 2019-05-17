// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestActor.generated.h"


UCLASS(Blueprintable)
class LIBRARY_API ATestActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	UBillboardComponent* BB;
	
public:	



	// Sets default values for this actor's properties
	ATestActor();
	
	

};
