// Fill out your copyright notice in the Description page of Project Settings.

#include "TestActor.h"
#include "Components/BillboardComponent.h"


// Sets default values
ATestActor::ATestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	BB = CreateDefaultSubobject<UBillboardComponent>(TEXT("Visual"));
	BB->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<UTexture2D> ObjFind(TEXT("Texture2D'/Engine/EditorResources/EmptyActor.EmptyActor'"));
	BB->SetSprite(ObjFind.Object);
	BB->bIsScreenSizeScaled = true;
	BB->ScreenSize = 0.001f;
}



