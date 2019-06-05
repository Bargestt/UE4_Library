// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavigationSystem.h"
#include "Kismet/GameplayStatics.h"

#include "SurfaceNavigationActor.h"
#include "Components/BillboardComponent.h"

DEFINE_LOG_CATEGORY(SurfaceNavigation);


//////////////////////////////////////////////////////////////////////////
//		ASurfaceNavigationActor
//////////////////////////////////////////////////////////////////////////

ASurfaceNavigationActor::ASurfaceNavigationActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
#if WITH_EDITOR
	BillboardVisual = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Visual"));
	BillboardVisual->SetupAttachment(RootComponent);
	BillboardVisual->RelativeScale3D = FVector(.5f);
	BillboardVisual->bIsScreenSizeScaled = true;
	ConstructorHelpers::FObjectFinder<UTexture2D> SpriteObj(TEXT("Texture2D'/Game/SurfaceNavigation/Icon.Icon'"));
	if (SpriteObj.Succeeded()) {
		BillboardVisual->SetSprite(SpriteObj.Object);
	}	
#endif //#if WITH_EDITOR

	NavigationSystem = CreateDefaultSubobject<USurfaceNavigationSystem>(TEXT("NavSystem"));
}
//////////////////////////////////////////////////////////////////////////





USurfaceNavigationSystem::USurfaceNavigationSystem()
{
	Info = TEXT("Info: No info");
}

void USurfaceNavigationSystem::FindPathSync(const FVector& From, const FVector& To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters) const
{

}



USurfaceNavigationSystem* USurfaceNavigationSystem::GetNavigationSystem(const UObject* WorldContextObject)
{
	TArray<AActor*> NavSystems;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ASurfaceNavigationActor::StaticClass(), NavSystems);	

	if (NavSystems.Num() > 1)
	{
		UE_LOG(SurfaceNavigation, Error, TEXT("Excess surface naviation systems detected! WTF???"));
		
		// TODO: Select navigator in persistent level
	}

	if (NavSystems.Num() > 0)
	{
		return Cast<USurfaceNavigationSystem>(NavSystems[0]);
	}	
	
	return nullptr;	
}

void USurfaceNavigationSystem::FindPathSync(const UObject* WorldContextObject, const FVector& From, const FVector& To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters)
{
	USurfaceNavigationSystem* NavSystem = GetNavigationSystem(WorldContextObject);

	if (NavSystem == nullptr)
	{
		UE_LOG(SurfaceNavigation, Error, TEXT("No navigation system for pathfind request"));
	}

	NavSystem->FindPathSync(From, To, OutResult, Parameters);
}
