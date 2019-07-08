// Fill out your copyright notice in the Description page of Project Settings.

#include "SurfaceNavFunctionLibrary.h"

#include "SurfaceNavigation/SurfaceNavLocalData.h"
#include "SurfaceNavigationActor.h"
#include "SurfaceNavigationSystem.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"




USurfaceNavigationSystem* USurfaceNavFunctionLibrary::GetNavigationSystem(const UObject* WorldContextObject)
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
		return Cast<ASurfaceNavigationActor>(NavSystems[0])->GetNavigationSystem();
	}

	return nullptr;
}

void USurfaceNavFunctionLibrary::FindPathSync(const UObject* WorldContextObject, FVector From, FVector To, FSurfacePathfindingResult& OutResult, FSurfacePathfindingParams Parameters)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);


	if (World && (World->IsGameWorld() || World->WorldType == EWorldType::Editor))
	{
		USurfaceNavigationSystem* NavSystem = GetNavigationSystem(WorldContextObject);
		
		if (NavSystem == nullptr)
		{
			UE_LOG(SurfaceNavigation, Error, TEXT("No navigation system for pathfind request"));
		}
		
		NavSystem->FindPathSync(From, To, OutResult, Parameters);
	}
	
}







bool USurfaceNavFunctionLibrary::ProjectOnNavigation(const UObject* WorldContextObject, FVector SourceLocation, FVector& OutLocation)
{	
	if (USurfaceNavigationSystem* NavigationSystem = GetNavigationSystem(WorldContextObject))
	{
		return NavigationSystem->GetClosestNodeLocation(SourceLocation, OutLocation);
	}
	return false;
}

void USurfaceNavFunctionLibrary::DrawSurfaceGraph(const UObject* WorldContext, const FTransform& Transform,const FSurfaceNavLocalData& NavData, float Duration /*= 5*/, FColor LinkColor /*= FColor::Green*/, FColor NodeColor /*= FColor::Blue*/)
{
	const UWorld* World = WorldContext->GetWorld();

	const TArray<FEdgeData>& Edges = NavData.GetGraph();

	if (World == nullptr) return;

	const int PointSize = 7;
	const int LinkSize = 0;

	for (int Index = 0; Index < Edges.Num(); Index++)
	{
		const FEdgeData& Edge = Edges[Index];

		FVector PointPosition = Transform.TransformPositionNoScale(Edge.EdgeVertex);

		DrawDebugPoint(World, PointPosition, PointSize, NodeColor, false, Duration);
		for (int link : Edge.ConnectedEdges)
		{
			DrawDebugLine(World, PointPosition, Transform.TransformPosition(Edges[link].EdgeVertex), LinkColor, false, Duration, 0, LinkSize);
		}
	}
}
